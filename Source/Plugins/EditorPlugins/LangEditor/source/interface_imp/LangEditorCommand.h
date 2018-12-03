/********************************************************************
	created:	2015/08/12
	filename: 	LangEditorCommand.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_LangEditorCommand_h__
#define __Blade_LangEditorCommand_h__
#include <utility/StringHelper.h>
#include <utility/BladeContainer.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/public/ui/IUICommand.h>
#include <interface/IEditorFramework.h>
#include <ConfigTypes.h>
#include "LangEditorFile.h"
#include "BladeBase_blang.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class LangEditCommand : public IUICommand
	{
	public:
		LangEditCommand()
		{
			mPopConfig.bind( BLADE_NEW ConfigGroup() );
		}

		/** @brief  */
		virtual void execute(ICommandUI* cui)
		{
			IEditorFile* f = IEditorFramework::getSingleton().getCurrentFile();
			if( f == NULL || f->getType() != LangEditorFile::LANG_EDITOR_FILE )
			{
				assert(false);
				return;
			}

			LangEditorFile* file = static_cast<LangEditorFile*>(f);
			if( !this->prepare(file) )
				return;

			IConfigDialog::INFO info(true, true);
			if( mPopConfig->getSubConfigCount() > 0 )
			{
				bool result = false;
				do
				{
					if (IEditorFramework::getSingleton().getEditorUI()->showSingleGroupConfig(cui->getName(), mPopConfig, info))
						result = this->executeImpl(file);
					else
						break;
				} while (!result);
			}
			else
				this->executeImpl(file);
		}

		/** @brief  */
		virtual bool prepare(LangEditorFile* /*file*/)	{return true;}

		/** @brief  */
		virtual bool executeImpl(LangEditorFile* file) = 0;

	protected:
		HCONFIG	mPopConfig;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ImportLangTextCommand : public LangEditCommand, public Singleton<ImportLangTextCommand>
	{
	public:
		ImportLangTextCommand()
		{
			ConfigAtom* config = BLADE_NEW ConfigAtom(BXLang("Select File"));
			config->initStringValue(BTString("../Intermediate_Data/lang"), CUIH_FILE, CAF_READWRITE);
			mPopConfig->addSubConfigPtr(config);

			config = BLADE_NEW ConfigAtom(BXLang("Erase Existing Data"), Variant(false), CAF_READWRITE);
			mPopConfig->addSubConfigPtr(config);

			config = BLADE_NEW ConfigAtom(BXLang("Strip Existing Data"), Variant(true), CAF_READWRITE);
			mPopConfig->addSubConfigPtr(config);
		}

		/** @brief  */
		virtual bool executeImpl(LangEditorFile* file)
		{
			IConfig* config = mPopConfig->getSubConfigByName( BTString("Select File") );
			if( config != NULL )
			{
				const TString& path = config->getValue();
				IConfig* cfgErase = mPopConfig->getSubConfigByName(BTString("Erase Existing Data"));
				IConfig* cfgStrip = mPopConfig->getSubConfigByName(BTString("Strip Existing Data"));
				assert(cfgErase != NULL && cfgStrip != NULL);
				bool erase = cfgErase->getBinaryValue();
				bool strip = cfgStrip->getBinaryValue();

				HSTREAM stream = IResourceManager::getSingleton().loadStream(path);
				if (stream == NULL || !stream->isValid())
				{
					IPlatformManager::getSingleton().showMessageBox(
						BTLang("Invalid file specified.").c_str(),
						BTLang(BLANG_ERROR).c_str(), IPlatformManager::MBT_OK);
					return false;
				}
			
				if (!file->importText(path, erase, strip))
				{
					IPlatformManager::getSingleton().showMessageBox(
						BTLang("Import backup text file failed.").c_str(),
						BTLang(BLANG_ERROR).c_str(), IPlatformManager::MBT_OK);
					return false;
				}
				return true;
			}
			else
				assert(false);
			return false;
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//TODO: do we support delete of languages?
	class AddLangCommand : public LangEditCommand, public Singleton<AddLangCommand>
	{
	public:
		AddLangCommand()
		{
			mLangSelect = BXLang("Select a New Language");
			mCopyFrom = BXLang("Copy From Existing Language");

			TStringParam list;
			ILangTableManager::getSingleton().getSupportedLanuages(list);
			ConfigAtom* config = BLADE_NEW ConfigAtom(mLangSelect, list, CAF_READWRITE);
			mPopConfig->addSubConfigPtr(config);

			TStringParam copyList;
			copyList.push_back(ILangTableManager::DEFAULT_LANG);
			config = BLADE_NEW ConfigAtom(mCopyFrom, copyList, CAF_READWRITE);
			mPopConfig->addSubConfigPtr(config);
		}

		/** @brief  */
		virtual bool prepare(LangEditorFile* file)
		{
			TStringParam list;
			ILangTableManager::getSingleton().getSupportedLanuages(list);

			TStringParam existing;
			file->getLanguages(existing);
			for(size_t i = 0; i < existing.size(); ++i)
			{
				for(size_t j = 0; j < list.size(); ++j)
				{
					if( list[j] == existing[i] )
					{
						list.erase(j);
						break;
					}
				}
			}

			if( list.size() == 0 )
			{
				IPlatformManager::getSingleton().showMessageBox(
					BTLang("All supported languages are added, no more to add.").c_str(),
					BTLang(BLANG_INFO).c_str(), IPlatformManager::MBT_OK);
				return false;
			}

			ConfigAtom* config = static_cast<ConfigAtom*>( mPopConfig->getSubConfigByName(mLangSelect) );
			config->setOptions(list);

			config = static_cast<ConfigAtom*>( mPopConfig->getSubConfigByName(mCopyFrom) );
			config->setOptions(existing);
			return true;
		}

		/** @brief  */
		virtual bool executeImpl(LangEditorFile* file)
		{
			IConfig* config = mPopConfig->getSubConfigByName(mLangSelect);
			const TString& newLang = config->getValue();

			config = mPopConfig->getSubConfigByName(mCopyFrom);
			const TString& copyLang = config->getValue(); //copy from this language
			file->addLanguage(newLang, copyLang);
			return true;
		}

	protected:
		TString	mLangSelect;
		TString	mCopyFrom;
	};

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	 class LangVisibilityCommand : public LangEditCommand, public Singleton<LangVisibilityCommand>
	 {
		 typedef TempVector<IConfig*> VisibilityList;
		 VisibilityList mList;
	 public:
		 LangVisibilityCommand()
		 {

		 }

		 /** @brief  */
		 virtual bool prepare(LangEditorFile* file)
		 {
			 ITableWindow* window = static_cast<ITableWindow*>( file->getEditorWindow() );
			 if( window == NULL )
			 {
				 assert(false);
				 return false;
			 }

			 //dynamically create config objects because the data may keep changing (by adding new languages)
			 TStringParam list;
			 file->getLanguages(list);
			 mList.clear();
			 mList.resize(list.size());
			 for(size_t i = 0; i < list.size(); ++i)
				 mList[i] = BLADE_NEW ConfigAtom(list[i], Variant(false), (list[i] == ILangTableManager::DEFAULT_LANG ? CAF_READ : CAF_READWRITE));	//invisible

			 for(size_t i = 0; i < window->getColumnCount(); ++i)
			 {
				 const HCONFIG& cfg = window->getColumnDesc(i);
				 for(size_t j = 0; j < list.size(); ++j)
				 {
					 if( list[j] == cfg->getName() )
					 {
						 mList[j]->setBinaryValue(true);
						 break;
					 }
				 }
			 }

			 for(size_t i = 0; i < list.size(); ++i)
				 mPopConfig->addSubConfigPtr( mList[i] );

			 return true;
		 }

		 /** @brief  */
		 virtual bool executeImpl(LangEditorFile* file)
		 {
			 for(size_t i = 0; i < mList.size(); ++i)
			 {
				 bool visible = mList[i]->getBinaryValue();
				 file->setLanguageVisibility(mList[i]->getName(), visible);
			 }
			 mList.clear();
			 //delete all config objects
			 mPopConfig->clearSubConfigs();
			 return true;
		 }
	 };

	 /************************************************************************/
	 /*                                                                      */
	 /************************************************************************/
	 //TODO: add options to find in one column only
	 class FindStringCommand : public Bindable, public Singleton<FindStringCommand>
	 {
	 public:
		 static bool findString(const Variant& v, const ITableWindow::FindContext&)
		 {
			 return v.getType() == VI_TSTRING && v.toTString().find(FindStringCommand::getSingleton().getString(0)) != TString::npos;
		 }

		 /** @brief  */
		 //virtual void execute(const TString& name, IMenu* menu)
		 const TString&	getString(index_t) const
		 {
			 IEditorFile* f = IEditorFramework::getSingleton().getCurrentFile();
			 if( f == NULL || f->getType() != LangEditorFile::LANG_EDITOR_FILE )
				 return TString::EMPTY;
			 LangEditorFile* file = static_cast<LangEditorFile*>(f);
			 //per-file storage
			 return file->getStringToFind();
		 }

		 /** @brief  */
		 bool execute(index_t, const TString& string2find)
		 {
			 IEditorFile* f = IEditorFramework::getSingleton().getCurrentFile();
			 if( f == NULL || f->getType() != LangEditorFile::LANG_EDITOR_FILE )
			 {
				 assert(false);
				 return true;
			 }
			 LangEditorFile* file = static_cast<LangEditorFile*>(f);
			 file->setStringToFind(string2find);

			 ITableWindow* window = static_cast<ITableWindow*>( file->getEditorWindow() );
			 ITableWindow::CELLPOS itempos;
			 
			 index_t pos = string2find.find(TEXT(':'));
			 if( pos != INVALID_INDEX )
			 {
				 TString prefix = TStringHelper::trim( string2find.substr_nocopy(0, pos) );
				 TString content = TStringHelper::trim( string2find.substr_nocopy(pos+1) );
				 if( !prefix.empty() && !content.empty() )
				 {
					 for(size_t i = 0; i < window->getColumnCount(); ++i)
					 {
						 if( window->getColumnDesc(i)->getName() == prefix )
						 {
							 file->setStringToFind(content);
							 
							 if( window->findCell(&FindStringCommand::findString, itempos, i) )
								 window->navigateTo(itempos, false, true);
							 return true;
						 }
					 }
				 }
			 }

			 if( window->findCell(&FindStringCommand::findString, itempos) )
				window->navigateTo(itempos, false, true);
			 file->setStringToFind(string2find);
			 return true;
		 }
	 };

	 /************************************************************************/
	 /*                                                                      */
	 /************************************************************************/
	 class FindEmptyStringCommand : public LangEditCommand, public Singleton<FindEmptyStringCommand>
	 {
	 public:
		 static bool isEmptyString(const Variant& v, const ITableWindow::FindContext&)
		 {
			 return v.getType() == VI_TSTRING && v == TString::EMPTY;
		 }

		 /** @brief  */
		 virtual bool executeImpl(LangEditorFile* file)
		 {
			 ITableWindow* window = static_cast<ITableWindow*>( file->getEditorWindow() );
			 ITableWindow::CELLPOS itempos;
			 if( window->findCell(&FindEmptyStringCommand::isEmptyString, itempos) )
				 window->navigateTo(itempos, false, true);
			 return true;
		 }
	 };

	 /************************************************************************/
	 /*                                                                      */
	 /************************************************************************/
	 class FindUnTranslatedtringCommand : public LangEditCommand, public Singleton<FindUnTranslatedtringCommand>
	 {
	 public:
		 static index_t getDefaultColumn(const ITableWindow* window = NULL)
		 {
			 static index_t val = 0;
			 if(window == NULL)
				 return val;
			 val = window->findColumn(ILangTableManager::DEFAULT_LANG);
			 return val;
		 }

		 static bool isUntranslatedString(const Variant& v, const ITableWindow::FindContext& context)
		 {
			index_t defCol = FindUnTranslatedtringCommand::getDefaultColumn();
			 if( v.getType() != VI_TSTRING || context.col == defCol )
				 return false;
			 return v == TString::EMPTY || v == context.window->getCell(context.row, defCol);
		 }

		 /** @brief  */
		 virtual bool executeImpl(LangEditorFile* file)
		 {
			 ITableWindow* window = static_cast<ITableWindow*>( file->getEditorWindow() );
			 FindUnTranslatedtringCommand::getDefaultColumn(window);
			 ITableWindow::CELLPOS itempos;
			 if( window->findCell(&FindUnTranslatedtringCommand::isUntranslatedString, itempos) )
				window->navigateTo(itempos, false, true);
			 return true;
		 }
	 };
	
}//namespace Blade


#endif // __Blade_LangEditorCommand_h__
