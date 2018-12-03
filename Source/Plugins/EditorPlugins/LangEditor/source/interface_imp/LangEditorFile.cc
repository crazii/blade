/********************************************************************
	created:	2015/08/11
	filename: 	LangEditorFile.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "LangEditorFile.h"
#include "LangEditor.h"
#include <ConfigTypes.h>
#include <interface/IResourceManager.h>
#include "LangEditorController.h"

namespace Blade
{

	const TString LangEditorFile::LANG_EDITOR_FILE = BTString("Language File");

	//////////////////////////////////////////////////////////////////////////
	LangEditorFile::LangEditorFile()
		:EditorFileBase(LANG_EDITOR_FILE)
	{
		mTable = NULL;
		this->setUpdater( LangEditorController::getSingletonPtr() );
	}

	//////////////////////////////////////////////////////////////////////////
	LangEditorFile::~LangEditorFile()
	{

	}

	/************************************************************************/
	/* IEditorFile interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				LangEditorFile::openFile(const TString& dir,const TString& name)
	{
		if( this->isFileOpened() || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}

		TString file = dir + BTString("/") + name;

		HSTREAM stream = IResourceManager::getSingleton().loadStream(file);
		mTable = ILangTableManager::getSingleton().loadLangTable(stream, false);

		if( mTable != NULL && mTable->getLangs() > 0 )
		{
			mFile = file;
			this->initialize();
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorFile::saveFile()
	{
		if( this->isOpened() && mTable != NULL && this->isModified() )
		{
			//get data from table editor window
			ITableWindow* window = static_cast<ITableWindow*>(this->getEditorWindow());
			size_t rowCount = window->getRowCount();
			size_t colCount = window->getColumnCount();	//window may have hidden column, but they are up to date before hidden,
														//we only need to update visible columns now

			for(size_t row = 0; row < rowCount; ++row)
			{
				TStringParam translations;
				TStringParam ids;
				TString langString;

				translations.reserve(colCount);
				ids.reserve(colCount);
				for(size_t col = 0; col < colCount; ++col)
				{
					const TString& lang = window->getColumnDesc(col)->getName();
					if( lang == ILangTableManager::DEFAULT_LANG )
						langString = window->getCell(row, col);
					else
					{
						ids.push_back(lang);
						translations.push_back((const TString&)window->getCell(row, col));
					}
				}
				assert(!langString.empty());
				mTable->addLangString(langString, ids, translations);
			}
			//save
			HSTREAM stream = IResourceManager::getSingleton().openStream(mFile, true);
			ILangTableManager::getSingleton().saveLangTable(mTable, stream);

			//always backup text
			TextFile text;
			TString dir,file, base, ext;
			TStringHelper::getFilePathInfo(mFile, dir, file, base, ext);
			//TODO: config back up folder via UI
			TString backup = BTString("../Intermediate_Data/lang/") + base;
			stream = IResourceManager::getSingleton().openStream(backup, false);
			if( text.open(stream) )
			{
				text.setDestEncoding(TE_UTF8);

				TStringParam langs;
				mTable->getLangs(&langs);
				size_t stringCount = mTable->getSize();
				for(size_t i = 0; i < langs.size(); ++i)
				{
					text.write( BTString("[") + langs[i] + BTString("]\n"));
					const TString* stringList = mTable->getStringList(langs[i]);
					for(size_t j = 0; j < stringCount; ++j)
						text.write(stringList[j] + BTString("\n"));
				}
			}
			else
				BLADE_LOG(Error, BTString("saving back up file failed:") + backup);
		}

		this->setUnModified();
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorFile::newFile(const TString& dir, const TString& name)
	{
		if( this->isFileOpened() || this->getEditorWindow() == NULL )
		{
			assert(false);
			return;
		}

		mTable = ILangTableManager::getSingleton().loadLangTable(HSTREAM::EMPTY);

		if( mTable != NULL )
		{
			mFile = dir + BTString("/") + name;
			this->initialize();
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void				LangEditorFile::closeFile()
	{
		if( this->isFileOpened() )
		{
			BLADE_DELETE mTable;
			mTable = NULL;
			mFile = TString::EMPTY;
			mColumnDesc.clear();
		}
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		LangEditorFile::initialize()
	{
		if( mTable == NULL )
		{
			assert(false);
			return false;
		}
		//setup columns
		TStringParam langs;
		size_t colCount = mTable->getLangs(&langs);

		//assert( colCount > 0 );
		mColumnDesc.resize( std::max<size_t>(1, colCount) );

		//default lang: read only
		mColumnDesc[0].bind( BLADE_NEW ConfigAtom(ILangTableManager::DEFAULT_LANG, TString::EMPTY, CAF_READ) );

		index_t index = 1;
		for(size_t i = 0; i < colCount; ++i)
		{
			const TString& lang = langs[i];

			if( lang == ILangTableManager::DEFAULT_LANG )
				continue;

			mColumnDesc[index++].bind( BLADE_NEW ConfigAtom(lang, TString::EMPTY, CAF_READWRITE) );
		}
		this->setOpened();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LangEditorFile::importText(const TString& path, bool eraseOriginal/* = false*/, bool stripOriginal/* = true*/)
	{
		ConfigFile file;
		HSTREAM stream  = IResourceManager::getSingleton().loadStream(path);
		if( !file.open(stream) )
			return false;

		const CFGFILE_SETTING* settings = file.getAllConfigs();
		if( settings == NULL || settings->getSectionCount() == 0 )
			return false;
		const CFGFILE_SECTION* defSection = settings->getSection(ILangTableManager::DEFAULT_LANG);
		if( defSection == NULL )
			return false;

		//all sections must have the same count of strings
		size_t count = defSection->getEntryCount();
		for(size_t i = 1; i < settings->getSectionCount(); ++i)
		{
			if( count != settings->getSection(i).getEntryCount() )
			{
				assert(false);
				return false;
			}
		}

		this->setModified();

		typedef TempTStringSet UnusedLangString;	//keep a unused original string list for stripping
		UnusedLangString UnusedStrings;

		if (stripOriginal)
		{
			size_t stringCount = mTable->getSize();
			const TString* list = mTable->getStringList(ILangTableManager::DEFAULT_LANG);
			UnusedStrings.insert(list, list + stringCount);
		}

		if( eraseOriginal )
			mTable->removeAll();

		//add columns
		if( mColumnDesc.size() < settings->getSectionCount() )
			mColumnDesc.reserve(settings->getSectionCount());
		mColumnDesc.reserve(settings->getSectionCount());
		for(size_t i = 0; i < settings->getSectionCount(); ++i)
		{
			const CFGFILE_SECTION& section = settings->getSection(i);
			if( &section == defSection )
				continue;

			LangColmnDesc::iterator iter = std::find_if(mColumnDesc.begin(), mColumnDesc.end(), FnHConfigFindByName(section.getName()));
			if( iter == mColumnDesc.end() ) 
			{
				mColumnDesc.push_back( HCONFIG::EMPTY );
				mColumnDesc.back().bind( BLADE_NEW ConfigAtom(section.getName(), TString::EMPTY, CAF_READWRITE) );
			}
		}

		for(size_t i = 0; i < count; ++i)
		{
			//get original/untranslated lang string
			const TString& originalString = defSection->getEntry(i).getName();
			TStringParam translations;
			TStringParam ids;

			translations.reserve(settings->getSectionCount());
			ids.reserve(settings->getSectionCount());
			for(size_t j = 0; j < settings->getSectionCount(); ++j)
			{
				const CFGFILE_SECTION& section = settings->getSection(j);
				if( &section == defSection )
					continue;

				ids.push_back(section.getName());
				translations.push_back(section.getEntry(i).getName());
			}

			UnusedStrings.erase(originalString);

			mTable->addLangString(originalString, ids, translations);
		}

		if (stripOriginal)
		{
			for (UnusedLangString::const_iterator iter = UnusedStrings.begin(); iter != UnusedStrings.end(); ++iter)
				mTable->removeLangString(*iter);
		}

		//update table view
		LangEditorController::getSingleton().resetView(this, static_cast<ITableWindow*>(this->getEditorWindow()));

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LangEditorFile::addLanguage(const TString& lang, const TString& copyFromLang)
	{
		FnHConfigFindByName finder(lang);
		FnHConfigFindByName finder2(copyFromLang);
		if( lang.empty() || std::find_if(mColumnDesc.begin(), mColumnDesc.end(), finder) != mColumnDesc.end()
			|| std::find_if(mColumnDesc.begin(), mColumnDesc.end(), finder2) == mColumnDesc.end() )
		{
			assert(false);
			return false;
		}
		this->setModified();

		if( !mTable->addLanguage(lang, copyFromLang) )
			return false;

		//add column
		mColumnDesc.push_back( HCONFIG::EMPTY );
		mColumnDesc.back().bind( BLADE_NEW ConfigAtom(lang, TString::EMPTY, CAF_READWRITE) );

		ITableWindow* window = static_cast<ITableWindow*>(this->getEditorWindow());
		window->appendColumn( mColumnDesc.back() );
		size_t col = window->getColumnCount()-1;
		//add data to table
		size_t rowCount = mTable->getSize();
		for(size_t i = 0; i < rowCount; ++i)
		{
			ParamList translations;
			bool ret = mTable->getTranslation(i, translations);
			if( ret )
			{
				const TString& val = translations[lang];
				window->getCell(i, col) = val;
			}
			else
				assert(false);
		}
		window->refreshColumn(col);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		LangEditorFile::getLanguages(TStringParam& langs)
	{
		langs.clear();
		for(size_t i = 0; i < mColumnDesc.size(); ++i)
			langs.push_back( mColumnDesc[i]->getName() );
		return langs.size();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LangEditorFile::setLanguageVisibility(const TString& lang,  bool visible)
	{
		LangColmnDesc::iterator iter = std::find_if(mColumnDesc.begin(), mColumnDesc.end(), FnHConfigFindByName(lang));
		if( lang.empty() || iter == mColumnDesc.end() )
		{
			assert(false);
			return false;
		}
		ITableWindow* window = static_cast<ITableWindow*>(this->getEditorWindow());
		size_t colCount = window->getColumnCount();
		size_t found = INVALID_INDEX;
		for(size_t i = 0; i < colCount; ++i)
		{
			const HCONFIG& desc = window->getColumnDesc(i);
			if(desc == *iter)
			{
				found = i;
				break;
			}
		}
		assert( mTable->getSize() == window->getRowCount() );
		size_t rowCount = mTable->getSize();

		if( found != INVALID_INDEX && !visible )
		{
			//note: store window column data before delete it
			const TString& targetLang = window->getColumnDesc(found)->getName();
			assert(targetLang != ILangTableManager::DEFAULT_LANG );
			index_t defCol = window->findColumn(ILangTableManager::DEFAULT_LANG);

			for(index_t i = 0; i < rowCount; ++i)
			{
				const TString langString = (const TString)window->getCell(i, defCol);
				const TString& val = window->getCell(i, found);
				TStringParam tran;
				TStringParam id;
				id.push_back(targetLang);
				tran.push_back(val);
				mTable->addLangString(langString, id, tran);
			}

			//delete column
			window->deleteColumn(found);
		}
		else if( found == INVALID_INDEX && visible )
		{
			window->appendColumn( *iter );
			size_t col = window->getColumnCount()-1;

			
			for(size_t i = 0; i < rowCount; ++i)
			{
				ParamList translations;
				mTable->getTranslation(i, translations);
				const TString& translation = translations[lang];
				window->getCell(i, col) = translation;
			}
			window->refreshColumn(col);
		}
		return true;
	}

}//namespace Blade