/********************************************************************
	created:	2013/10/28
	filename: 	LangTable.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LangTable_h__
#define __Blade_LangTable_h__
#include <interface/ILangTable.h>
#include <utility/StringList.h>
#include <utility/BladeContainer.h>
#include "LangTableFile.h"

namespace Blade
{
	//read only data
	typedef LangTableFile::LangStringTable LangStringTable;
	typedef LangTableFile::MultiLangTable MultiLangTable;
	//writable data
	typedef LangTableFile::WritableLangStringTable WritableLangStringTable;
	typedef LangTableFile::WritableMultiLangTable WritableMultiLangTable;

	//////////////////////////////////////////////////////////////////////////
	class LangTable : public ILangTable, public Allocatable
	{
	public:
		LangTable()		{mDataChanged = false;}
		~LangTable()	{ this->close(); }
		
		/*
		@describe get supported language list
		@param 
		@return 
		*/
		virtual size_t	getLangs(TStringParam* outLangs/* = NULL*/) const
		{
			if( outLangs != NULL )
			{
				outLangs->clear();
				outLangs->reserve( mWritableData.size() );
				for(WritableMultiLangTable::const_iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
					outLangs->push_back( i->first );
			}
			return mWritableData.size();
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	addLanguage(const TString& newLang, const TString& copyFromLang)
		{
			WritableMultiLangTable::iterator i = mWritableData.find(copyFromLang);
			if( i == mWritableData.end() )
			{
				assert(false && "copy source doesn't exist" );
				return false;
			}

			std::pair<WritableMultiLangTable::iterator, bool> ret = mWritableData.insert( std::make_pair(newLang, WritableLangStringTable() ) );
			if( !ret.second ) //already exist
			{
				assert(false);
				return false;
			}
			WritableLangStringTable& table = ret.first->second;
			table = i->second;
			return true;
		}

		/*
		@describe add language strings
		@param langString: original string in default language.
		@param translations: language type, and translated string for original string. i.e. [zh_CN,""],[fr,""].
		@return 
		@note if langString already exist and translation is not EMTPY, the existing entry will be updated using translations
		*/
		virtual index_t	addLangString(const TString& langString, const TStringParam& id, const TStringParam& translations)
		{
			assert(id.size() == translations.size());
			if( langString.empty() || translations.size() > mWritableData.size() || id.size() != translations.size())
				return INVALID_INDEX;

			WritableLangStringTable* table = &(mWritableData[ILangTableManager::DEFAULT_LANG]);

#if BLADE_DEBUG
			//all tables must have the same size, each string match by index
			for(WritableMultiLangTable::const_iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
				assert( i->second.size() == table->size() );
#endif

			WritableLangStringTable::iterator iter = std::find(table->begin(), table->end(), langString);
			index_t index = table->size();
			if( iter != table->end() )
				index = (index_t)(iter - table->begin());
			else
			{
				mDataChanged = true;
				table->push_back(langString);
			}

			//update 
			for(size_t i = 0; i < translations.size(); ++i)
			{
				const TString& tr = translations.at(i);
				const TString& lan = id.at(i);

				WritableLangStringTable* otherTable = &(mWritableData[lan]);
				if( otherTable == table )
					continue;
				mDataChanged = true;
				if( otherTable->size() == table->size() )
					(*otherTable)[index] = tr;
				else
					otherTable->push_back(tr);
			}

			//fill other languages with default language string
			for(WritableMultiLangTable::iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
			{
				if( i->second.size() != table->size() )
				{
					assert( i->second.size() == table->size() - 1);
					i->second.push_back(langString);
				}
			}

			//sort
			if(mDataChanged)
			{
				typedef TempVector<TString> TableCache;
				TableCache oldTable;
				TempVector<index_t> indexMap;
				oldTable.reserve(table->size());
				indexMap.resize(table->size(), INVALID_INDEX);

				oldTable.insert(oldTable.end(), table->begin(), table->end());
				std::sort(table->begin(), table->end(), FnTStringNCLess());

				//find the original index 
				for(size_t i = 0; i < indexMap.size(); ++i)
				{
					TableCache::iterator iter2 = std::find(oldTable.begin(), oldTable.end(), (*table)[i]);
					assert(iter2 != oldTable.end());
					indexMap[i] = index_t(iter2 - oldTable.begin());
				}

				//this maybe too slow, but:
				//1.most pre-loaded strings are const string without real memory allocations.
				//2.this function is used by tools only, it is acceptable.
				//TODO: better ways?
				for(WritableMultiLangTable::iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
				{
					WritableLangStringTable& t = i->second;
					if( &t == table )
						continue;
					WritableLangStringTable ot = t;
					t.clear();
					t.resize(ot.size());
					for(size_t j = 0; j < indexMap.size(); ++j)
						t[ j ] = ot[ indexMap[j] ];
				}
			}
			return index;
		}

		/*
		@describe remove language string by specifying the original string
		@param 
		@return 
		*/
		virtual bool	removeLangString(const TString& langString)
		{
			WritableLangStringTable* table = &(mWritableData[ILangTableManager::DEFAULT_LANG]);

#if BLADE_DEBUG
			//all tables must have the same size, each string match by index
			for(WritableMultiLangTable::const_iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
				assert( i->second.size() == table->size() );
#endif

			WritableLangStringTable::iterator iter = std::find(table->begin(), table->end(), langString);
			if( iter != table->end() )
			{
				mDataChanged = true;
				index_t index = (index_t)(iter - table->begin());
				for(WritableMultiLangTable::iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
					i->second.erase(i->second.begin() + (indexdiff_t)index);
				return true;
			}
			else
				return false;
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	removeAll()
		{
			mWritableData.clear();
			mDataChanged = true;
			return true;
		}

		/*
		@describe get string count for each lang in the table, all of them are the same as this  count.
		@param
		@return
		*/
		virtual size_t	getSize() const
		{
#if BLADE_DEBUG
			const WritableLangStringTable* table = &(mWritableData.begin()->second);
			//all tables must have the same size, each string match by index
			for(WritableMultiLangTable::const_iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
				assert( i->second.size() == table->size() );
#endif
			assert(mWritableData.size() >= 1);
			return mWritableData.begin()->second.size();
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	getTranslation(index_t index, ParamList& strings) const
		{
			const WritableLangStringTable* table = &(mWritableData.begin()->second);
			//all tables must have the same size, each string match by index
			for(WritableMultiLangTable::const_iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
			{
				if( i->second.size() != table->size() )
				{
					assert(false);
					return false;
				}
			}

			if( index >= table->size() )
			{
				assert(false);
				return false;
			}

			strings.clear();
			for(WritableMultiLangTable::const_iterator i = mWritableData.begin(); i != mWritableData.end(); ++i)
				strings[i->first] = i->second[index];
			return true;
		}

		/*
		@describe get all translation list for a language, the array of string in size of getSize()
		@param
		@return
		*/
		virtual const TString*	getStringList(const TString& lang) const
		{
			WritableMultiLangTable::const_iterator i = mWritableData.find(lang);
			if( i == mWritableData.end() || i->second.size() == 0 )
			{
				assert(false);
				return NULL;
			}
			return &(i->second[0]);
		}

		/** @brief  */
		bool		initialize(const HSTREAM& stream, bool writable)
		{
			if( mFile.isOpen() )
				return false;
			bool ret = mFile.open(stream, mLangTable);
			if( writable )
			{
				if( ret )
				{
					for(MultiLangTable::const_iterator i = mLangTable.begin(); i != mLangTable.end(); ++i)
					{
						WritableLangStringTable& table = mWritableData[i->first];
						table.insert(table.end(), i->second.begin(), i->second.end());
					}
				}
				else
					mWritableData[ILangTableManager::DEFAULT_LANG].clear();	//force add
			}
			return true;
		}

		/** @brief  */
		bool		save(const HSTREAM& stream)
		{
			if( stream == NULL || !stream->isValid() || !mDataChanged )
				return false;
			mDataChanged = !LangTableFile::save(stream, mWritableData);
			return !mDataChanged;
		}

		/** @brief  */
		void			close()							{ mLangTable.clear(); mWritableData.clear(); mFile.close(); mDataChanged = false;}

		/** @brief  */
		size_t			getLanguageCount() const		{return mLangTable.size();}

		/** @brief  */
		const TString&	getString(index_t index, const TString& lang = ILangTableManager::DEFAULT_LANG) const
		{
			MultiLangTable::const_iterator i = mLangTable.find(lang);
			if( i == mLangTable.end() )
			{
				assert(false);
				return TString::EMPTY;
			}
			const LangStringTable& table = i->second;
			return index < table.size() ? table[index] : TString::EMPTY;
		}

		/** @brief  */
		bool			isOpen() const					{return mFile.isOpen();}

		/** @brief  */
		const LangStringTable*		getTable(const TString& lang) const
		{
			if( lang.empty() )
				return NULL;

			MultiLangTable::const_iterator i = mLangTable.find(lang);
			if( i != mLangTable.end() )
				return &(i->second);
			else
				return NULL;
		}
	protected:
		//read only data
		MultiLangTable	mLangTable;
		//file holding persistent string buffer
		LangTableFile	mFile;
		//writable config: optional - available for tools
		WritableMultiLangTable	mWritableData;
		bool			mDataChanged;
	};


	class LangTableManager : public ILangTableManager, public Singleton<LangTableManager>
	{
	public:
		using Singleton<LangTableManager>::getSingleton;
		using Singleton<LangTableManager>::getSingletonPtr;
	public:
		LangTableManager();
		~LangTableManager();

		/*
		@describe 
		@param [in] langTableFile: the language file
		@param [in] lang current app's language setting
		@return 
		@note current language cannot be changed after initialization
		*/
		virtual bool	initialize(const HSTREAM& langTableFile, const TString& lang = DEFAULT_LANG);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	shutdown();

		/*
		@describe get translated string of current language.
		@param the orignal string in default language.
		@return 
		*/
		virtual const TString&	getLangString(const TString& orignal);

		/*
		@describe get language string by ID
		@param [in] ID: the ID of target string
		@return 
		*/
		virtual const TString&	getLangString(index_t ID) const;

		/*
		@describe get the language settings of the system
		@param
		@return
		*/
		virtual const TString&	getSystemLang() const;

		/*
		@describe get current language setting
		@param 
		@return 
		*/
		virtual const TString&	getCurrentLang() const;

		/*
		@describe load additional lang table for specific language
		@param 
		@return 
		*/
		virtual ILangTable*		loadLangTable(const HSTREAM& stream, bool creatOnFailure = true) const;

		/*
		@describe save lang table - used by tools
		@param 
		@return 
		*/
		virtual bool			saveLangTable(ILangTable* table, const HSTREAM& stream) const;

		/*
		@describe get language list needed to be support - used by tools
		@param
		@return
		*/
		virtual const TString*	getSupportedLanuages(size_t& count) const;

		/** @brief  */
		virtual size_t	getSupportedLanuages(TStringParam& list) const
		{
			size_t count = 0;
			const TString* langs = this->getSupportedLanuages(count);
			list.clear();
			list.reserve(count);
			for(size_t i = 0; i < count; ++i)
				list.push_back( langs[i] );
			return list.size();
		}


	protected:
		typedef StaticTStringMap<TString> LocalLangMap;

		TString			mCurrentLang;	//current setting
		LangTable		mLangTable;
		LocalLangMap	mLangMap;		//local string map
	};

	
}//namespace Blade

#endif //  __Blade_LangTable_h__