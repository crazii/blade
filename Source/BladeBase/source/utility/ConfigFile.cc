/********************************************************************
	created:	2009/04/06
	created:	6:4:2009   6:40
	filename: 	ConfigFile.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "InternalString.h"

namespace Blade
{
	/************************************************************************/
	/* ConfigSection                                                                     */
	/************************************************************************/
	enum EConfigDataMemoryType
	{
		CDMT_DEFAULT,
		CDMT_TEMPORARY
	};

	struct SConfigEntry : public IConfigEntry, public Allocatable
	{
		TString Name;
		mutable TString Value;
		/** @brief  */
		virtual const TString& getName() const {return Name;}
		/** @brief  */
		virtual const TString& getValue() const {return Value;}
		/** @brief  */
		virtual void setValue(const TString& value) const {Value = value;}
	};
	struct STempConfigEntry : public IConfigEntry, public TempAllocatable
	{
		TmpTString Name;
		mutable TmpTString Value;
		/** @brief  */
		virtual const TString& getName() const {return reinterpret_cast<const TString&>(Name);}
		/** @brief  */
		virtual const TString& getValue() const {return reinterpret_cast<const TString&>(Value);}
		/** @brief  */
		virtual void setValue(const TString& value) const {Value = value;}
	};

	template<typename T, int TYPE>
	struct ConfigSectionMemoryTraits : public Allocatable
	{
		typedef Allocatable	Base;
		typedef SConfigEntry EntryType;
		typedef TString StringType;

		typedef Map<StringType, index_t> EntryMap;
		typedef Vector<EntryType*> EntryList;
	};

	template<typename T>
	struct ConfigSectionMemoryTraits<T, CDMT_TEMPORARY> : public TempAllocatable
	{
		typedef TempAllocatable	Base;
		typedef STempConfigEntry EntryType;
		typedef TmpTString StringType;

		typedef TempMap<StringType, index_t> EntryMap;
		typedef TempVector<EntryType*> EntryList;
	};

	template<int MEMORY>
	class ConfigSection : public IConfigSection, public ConfigSectionMemoryTraits< ConfigSection<MEMORY> , MEMORY>
	{
	public:
		typedef ConfigSectionMemoryTraits< ConfigSection<MEMORY> , MEMORY> base_type;
		typedef typename base_type::StringType StringType;
		typedef typename base_type::EntryMap EntryMap;
		typedef typename base_type::EntryList EntryList;
		typedef typename base_type::EntryType EntryType;

		static const size_t DEFAULT_COUNT = 32;
		ConfigSection(const TString& name) :mName(name)	{ mConfigList.reserve(DEFAULT_COUNT);}
		inline ~ConfigSection()
		{
			for(size_t i = 0; i < mConfigList.size(); ++i)
				BLADE_DELETE mConfigList[i];
		}

		/** @brief get section name */
		virtual const TString&		getName() const
		{
			return reinterpret_cast<const TString&>(mName);
		}

		/** @brief read only query */
		virtual const TString&		getValue(const TString& key) const
		{
			assert( mConfigList.size() == mConfigMap.size() );
			typename EntryMap::const_iterator i = mConfigMap.find( reinterpret_cast<const StringType&>(key) );
			if( i == mConfigMap.end() )
				return TString::EMPTY;
			else
			{
				index_t index = i->second;
				if( index >= mConfigList.size() )
				{
					assert(false);
					return TString::EMPTY;
				}
				return mConfigList[index]->getValue();
			}
		}

		/** @brief [optional] [recommended] prepare memory for key-value count, optimizing memory*/
		virtual void			prepare(size_t count)
		{
			mConfigList.reserve(count);
		}

		/** @brief  add new or set to existing */
		virtual void			addOrSetValue(const TString& key, const TString& val)
		{
			assert( mConfigList.size() == mConfigMap.size() );
			std::pair<typename EntryMap::iterator,bool> ret = mConfigMap.insert( std::make_pair( reinterpret_cast<const StringType&>(key), INVALID_INDEX) );
			if( ret.second )
			{
				ret.first->second = mConfigList.size();
				EntryType entry;
				entry.Name = key;
				entry.Value = val;
				mConfigList.push_back( BLADE_NEW EntryType(entry) );
			}
			else
			{
				index_t index = ret.first->second;
				assert( index < mConfigList.size() );
				if( index >= mConfigList.size() )
					BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("unexpected error."));
				mConfigList[index]->setValue(val);
			}
		}

		/** @brief add new or get existing */
		const TString&		addOrGetValue(const TString& key, const TString& val)
		{
			assert( mConfigList.size() == mConfigMap.size() );
			std::pair<typename EntryMap::iterator,bool> ret = mConfigMap.insert( std::make_pair( reinterpret_cast<const StringType&>(key), INVALID_INDEX) );
			if( ret.second )
			{
				ret.first->second = mConfigList.size();
				EntryType entry;
				entry.Name = key;
				entry.Value = val;
				mConfigList.push_back( BLADE_NEW EntryType(entry) );
				return mConfigList.back()->getValue();
			}
			else
			{
				index_t index = ret.first->second;
				assert( index < mConfigList.size() );
				if( index >= mConfigList.size() )
					BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("unexpected error."));
				return mConfigList[index]->getValue();
			}
		}

		/** @brief set */
		virtual bool				setValue(const TString& key, const TString& val)
		{
			assert( mConfigList.size() == mConfigMap.size() );
			typename EntryMap::iterator i = mConfigMap.find( reinterpret_cast<const StringType&>(key) );
			if( i == mConfigMap.end() )
				return false;
			else
			{
				index_t index = i->second;
				assert( index < mConfigList.size() );
				if( index >= mConfigList.size() )
					BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("unexpected error."));
				mConfigList[index]->setValue(val);
				return true;
			}
		}

		/** @brief add new */
		virtual bool				addValue(const TString& key, const TString& val)
		{
			assert( mConfigList.size() == mConfigMap.size() );
			std::pair<typename EntryMap::iterator,bool> ret = mConfigMap.insert( std::make_pair( reinterpret_cast<const StringType&>(key), INVALID_INDEX) );
			if( ret.second )
			{
				ret.first->second = mConfigList.size();
				EntryType entry;
				entry.Name = key;
				entry.Value = val;
				mConfigList.push_back(BLADE_NEW EntryType(entry));
			}
			return ret.second;
		}

		/** @brief query for modification */
		virtual const TString*		findValue(const TString& key) const
		{
			assert( mConfigList.size() == mConfigMap.size() );
			typename EntryMap::const_iterator i = mConfigMap.find( reinterpret_cast<const StringType&>(key) );
			if( i == mConfigMap.end() )
				return NULL;
			else
			{
				index_t index = i->second;
				if( index >= mConfigList.size() )
				{
					assert(false);
					return NULL;
				}
				return &mConfigList[index]->getValue();
			}

		}

		/** @brief  */
		virtual size_t				getEntryCount() const
		{
			assert( mConfigList.size() == mConfigMap.size() );
			return mConfigMap.size();
		}

		/** @brief  */
		virtual const CFGFILE_ENTRY&getEntry(index_t index) const
		{
			assert( mConfigList.size() == mConfigMap.size() );
			if( index < mConfigList.size() )
				return *mConfigList[index];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );			
		}

	private:
		StringType	mName;
		EntryMap	mConfigMap;
		EntryList	mConfigList;
		friend class ConfigFile;
	};//class ConfigSection

	/************************************************************************/
	/* ConfigSetting                                                                     */
	/************************************************************************/
	template<typename T, int TYPE>
	struct ConfigSettingMemoryTraits : public Allocatable
	{
		typedef Allocatable	Base;
		typedef Map<TString, index_t> SectionMap;

		typedef ConfigSection<TYPE> SectionType;
		typedef Vector< SectionType* > SectionList;
	};

	template<typename T>
	struct ConfigSettingMemoryTraits<T, CDMT_TEMPORARY> : public TempAllocatable
	{
		typedef TempAllocatable	Base;
		typedef TempMap<TString, index_t> SectionMap;

		typedef ConfigSection<CDMT_TEMPORARY> SectionType;
		typedef TempVector< SectionType* > SectionList;
	};

	class ConfigSettingBase : public IConfigSetting
	{
	public:
		virtual ~ConfigSettingBase()	{}
		virtual IConfigSetting* clone() const = 0;
	};
	template<int MEMORY>
	class ConfigSetting : public ConfigSettingBase, public ConfigSettingMemoryTraits< ConfigSetting<MEMORY>, MEMORY>
	{
	public:
		typedef ConfigSettingMemoryTraits< ConfigSetting<MEMORY>, MEMORY> base_type;
		typedef typename base_type::SectionList SectionList;
		typedef typename base_type::SectionMap SectionMap;
		typedef typename base_type::SectionType SectionType;

		static const size_t DEFAULT_COUNT = 32;
		inline ConfigSetting()	{mSectionList.reserve(DEFAULT_COUNT);}
		inline ~ConfigSetting()
		{
			this->clear();
		}

		/** @brief read only query */
		virtual const CFGFILE_SECTION*	getSection(const TString& SectionName) const
		{
			assert( mSections.size() == mSectionList.size() );

			typename SectionMap::const_iterator i = mSections.find( SectionName );
			if( i == mSections.end() )
				return NULL;
			else
			{
				if( i->second >= mSectionList.size() )
					return NULL;
				return mSectionList[i->second];
			}
		}

		/** @brief query for modification */
		virtual CFGFILE_SECTION*		getSection(const TString& SectionName)
		{
			assert( mSections.size() == mSectionList.size() );

			typename SectionMap::const_iterator i = mSections.find( SectionName );
			if( i == mSections.end() )
				return NULL;
			else
			{
				if( i->second >= mSectionList.size() )
					return NULL;
				return mSectionList[i->second];
			}
		}

		/** @brief  */
		virtual size_t					getSectionCount() const
		{
			assert( mSections.size() == mSectionList.size() );
			return mSections.size();
		}

		/** @brief  */
		virtual const CFGFILE_SECTION&	getSection(index_t index) const
		{
			assert( mSections.size() == mSectionList.size() );
			if( index < mSectionList.size() )
				return *mSectionList[index];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );

		}

		/** @brief  */
		virtual CFGFILE_SECTION&		getSection(index_t index)
		{
			assert( mSections.size() == mSectionList.size() );
			if( index < mSectionList.size() )
				return *mSectionList[index];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );			
		}

		/** @brief insertion */
		virtual CFGFILE_SECTION&		operator[](const TString& SectionName)
		{
			assert( mSections.size() == mSectionList.size() );

			std::pair<typename SectionMap::iterator, bool> ret = mSections.insert( std::make_pair(SectionName,INVALID_INDEX) );
			if( ret.second )
			{
				ret.first->second = mSectionList.size();
				mSectionList.push_back( BLADE_NEW SectionType(SectionName) );
				return *mSectionList.back();
			}
			index_t index = ret.first->second;
			if( index < mSectionList.size() )
				return *mSectionList[index];
			else
				BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("unexpected error.") );
		}

		/** @brief  */
		virtual void					clear()
		{
			for(size_t i = 0; i < mSectionList.size(); ++i)
				BLADE_DELETE mSectionList[i];
			mSections.clear();
			mSectionList.clear();
		}

		/** @brief  */
		virtual IConfigSetting*	clone() const
		{
			typedef ConfigSetting<CDMT_DEFAULT> CloneType;
			CloneType* setting = BLADE_NEW CloneType();
			setting->prepare( mSections.size() );

			for(size_t i = 0; i < mSectionList.size(); ++i)
			{
				const SectionType& section = *mSectionList[i];

				IConfigSection& destSection = (*setting)[ section.getName() ];
				destSection.prepare( section.getEntryCount() );

				for(size_t j = 0; j < section.getEntryCount(); ++j)
				{
					const CFGFILE_ENTRY& entry = section.getEntry(j);
					destSection.addValue( entry.getName(), entry.getValue() );
				}
			}
			return setting;
		}

		/** @brief  */
		void	prepare(size_t count)	{mSectionList.reserve(count);}

	private:
		SectionMap	mSections;
		SectionList	mSectionList;

		ConfigSetting(const ConfigSetting&);
		ConfigSetting& operator=(const ConfigSetting&);
		friend class ConfigFile;
	};//class ConfigSetting

	/************************************************************************/
	/* ConfigFile                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	ConfigFile::ConfigFile()
	{
		typedef ConfigSetting<CDMT_TEMPORARY> DefaultConfigSetting;
		mConfigSettings = BLADE_NEW DefaultConfigSetting();
	}

	//////////////////////////////////////////////////////////////////////////
	ConfigFile::~ConfigFile()
	{
		this->close();
		BLADE_DELETE mConfigSettings;
	}

	//////////////////////////////////////////////////////////////////////////
	bool ConfigFile::open(const TString& pathname,IStream::EAccessMode mode,const TString& seperator)
	{
		if( !TextFile::open(pathname, mode)  )
			return false;

		return this->parseConfig(seperator);
	}

	//////////////////////////////////////////////////////////////////////////
	bool ConfigFile::open(const HSTREAM& stream,const TString& seperator)
	{
		if( !TextFile::open(stream) )
			return false;

		return this->parseConfig(seperator);
	}

	//////////////////////////////////////////////////////////////////////////
	void	ConfigFile::close()
	{
		TextFile::close();
		mConfigSettings->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	const CFGFILE_SETTING*	ConfigFile::getAllConfigs() const
	{
		return mConfigSettings;
	}

	//////////////////////////////////////////////////////////////////////////
	HCONFIGSETTING			ConfigFile::cloneConfigs() const
	{
		IConfigSetting* setting =  static_cast<ConfigSettingBase*>(mConfigSettings)->clone();
		return HCONFIGSETTING(setting);
	}

	//////////////////////////////////////////////////////////////////////////
	const CFGFILE_SECTION*	ConfigFile::getSection(const TString& sectionName) const
	{
		return mConfigSettings->getSection(sectionName);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&			ConfigFile::getEntry(const TString& key,const TString& section/* = TString::EMPTY*/) const
	{
		const CFGFILE_SECTION* psection = this->getSection(section);

		if( psection == NULL )
			return TString::EMPTY;

		return psection->getValue(key);
	}


	//////////////////////////////////////////////////////////////////////////
	bool	ConfigFile::saveConfig(const IConfigSetting& content)
	{
		size_t count = content.getSectionCount();
		for(index_t i = 0; i < count; ++i)
		{
			const CFGFILE_SECTION& section = content.getSection(i);

			//skip empty sections?
			//that sections may have a name but no entry indeed.
			size_t entryCount = section.getEntryCount();
			if( entryCount == 0)
				continue;

			this->write(BTString("["));
			this->write( section.getName() );
			this->write(BTString("]\n"));

			
			for(index_t j = 0; j < entryCount; ++j)
			{
				const CFGFILE_ENTRY& entry = section.getEntry(j);
				this->write( entry.getName() );
				this->write(BTString("="));
				this->write(entry.getValue() );
				this->write(BTString("\n"));
			}
		}
		return true;
	}

#define THROW_EXCEPTION() BLADE_EXCEPT(EXC_FILE_READ,BTString("invalid config file: ")+mFileStream->getName())

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigFile::parseString(const TString& content,const TString& seperator/* = BTString("=") */)
	{
		size_t	SectionCount = 0;
		index_t	SectionTokenBegin = 0;
		index_t	SectionTokenEnd = 0;
		TString	SectionName;
		SectionTokenBegin = content.find_first_of( tchar('['),0);
		if( SectionTokenBegin != INVALID_INDEX && SectionTokenBegin != 0)
		{
			//note: '[' must appear at the beginning of a line
			index_t lineStart = content.find_last_not_of(TEXT("\t "), SectionTokenBegin-1);
			if( lineStart != INVALID_INDEX && (content[lineStart] != TEXT('\n') && content[lineStart] != TEXT('\r')) )
				SectionTokenBegin = INVALID_INDEX;
		}

		do 
		{
			++SectionCount;
			if(SectionTokenBegin != INVALID_INDEX)
			{
				SectionTokenEnd = content.find_first_of( tchar(']'),SectionTokenBegin);
				if( SectionTokenEnd == INVALID_INDEX )
				{
					THROW_EXCEPTION();
					//return false;		//[] mismatch
				}

				SectionName = TStringHelper::trim( content.substr_nocopy( SectionTokenBegin+1,SectionTokenEnd - (SectionTokenBegin+1) ) );

				++SectionTokenEnd;
			}

			CFGFILE_SECTION& currentSection = (*mConfigSettings)[SectionName];
			//to next section start
			SectionTokenBegin = content.find_first_of( tchar('['),SectionTokenEnd);
			while( SectionTokenBegin != INVALID_INDEX )
			{
				//note: '[' must appear at the beginning of a line
				index_t lineStart = content.find_last_not_of(TEXT("\t "), SectionTokenBegin-1);
				if( lineStart != INVALID_INDEX && (content[lineStart] != TEXT('\n') && content[lineStart] != TEXT('\r')) )
					SectionTokenBegin = content.find_first_of( tchar('['), SectionTokenBegin+1);
				else
					break;
			}
			size_t entryStart = SectionTokenEnd;
			size_t entryEnd = SectionTokenBegin != INVALID_INDEX ? SectionTokenBegin : content.size();

			const TString entryText = content.substr_nocopy(entryStart, entryEnd - entryStart);

			TStringTokenizer entryList;
			entryList.tokenize( entryText ,TEXT("\r\n\t") );

			for( size_t i = 0; i < entryList.size(); ++i )
			{
				const TString& entry = entryList[i];
				if( entry.empty() )
					continue;
				if( entry[0] == TEXT('/') && entry[1] == TEXT('/') ) //comment
					continue;
				assert( TStringHelper::trim(entry) != TString::EMPTY );

				TStringTokenizer entryPair;
				entryPair.tokenize(entry, seperator.c_str() );
				if( entryPair.size() > 2 )
				{
					THROW_EXCEPTION();
					//return false;
				}
				else if( entryPair.size() == 1 && entryPair[0].size() != 0 )
				{
					entryPair.push_back( TString::EMPTY );
				}

				entryPair.set(0, TStringHelper::trim(entryPair[0]) );
				entryPair.set(1, TStringHelper::trim(entryPair[1]) );
				
				//insert entry
				const TString& key = entryPair[0];
				const TString& val = entryPair[1];
				if( key.size() != 0 /*&& EntryPair[1].size() != 0*/ )
					currentSection[ key ] = val;
			}
		} while ( SectionTokenBegin != INVALID_INDEX );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ConfigFile::parseConfig(const TString& seperator)
	{
		const TString content = TextFile::getText();
		return this->parseString(content,seperator);
	}
	
}//namespace Blade