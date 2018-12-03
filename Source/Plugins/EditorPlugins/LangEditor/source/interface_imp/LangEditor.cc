/********************************************************************
	created:	2015/08/11
	filename: 	LangEditor.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <databinding/ConfigDataHandler.h>
#include <interface/IEditorFramework.h>
#include <interface/EditorFileInfo.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IIconManager.h>
#include "LangEditor.h"
#include "LangEditorFile.h"
#include "LangEditorCommand.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	LangEditor::LangEditor()
	{
		mMenu = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	LangEditor::~LangEditor()
	{

	}

	/************************************************************************/
	/* IEditor interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			LangEditor::initEditor()
	{
		TString iconPath = BTString("language.png");
		IconIndex index = IIconManager::getSingleton().loadSingleIconImage( iconPath );

		SEditorFileInfoEx info;
		info.mFileClassName = LangEditorFile::LANG_EDITOR_FILE;
		info.mWindowType = ITableWindow::TABLE_WINDOW_TYPE;
		info.mFileDescription = BTLang("Multi-Language definition");
		info.mFileExtension = BTString("blang");
		info.mDefaultPath = BTString("../Data");
		info.mIconID = index;
		info.mMaxOpenLimit = 10;
		info.mNewFileConfig = NULL;
		info.mOperations = EFO_NEW|EFO_OPEN|EFO_SAVE;
		info.mNewFolder = false;

		IEditorFramework::getSingleton().registerFileType(info);

		//note: toolbar need bind to a menu, we don't want to show the menu on IDE, maybe only popups
		//so create an extra menu
		mMenu = IMenuManager::getSingleton().createExtraMenu(BTString("LangEditMenu"));

		IconIndex icon = IIconManager::getSingleton().loadSingleIconImage(BTString("import.png") );
		mMenu->addItem(BXLang("Import backup"), ImportLangTextCommand::getSingletonPtr(), icon);
		icon = IIconManager::getSingleton().loadSingleIconImage(BTString("lang_add.png") );
		mMenu->addItem(BXLang("Add new language"), AddLangCommand::getSingletonPtr(), icon);
		icon = IIconManager::getSingleton().loadSingleIconImage(BTString("visibility.png") );
		mMenu->addItem(BXLang("Column visibility"), LangVisibilityCommand::getSingletonPtr(), icon);
		icon = IIconManager::getSingleton().loadSingleIconImage(BTString("go.png") );
		mMenu->addItem(BXLang("Goto next untransltaed string"), FindUnTranslatedtringCommand::getSingletonPtr(), icon);
		icon = IIconManager::getSingleton().loadSingleIconImage(BTString("go_red.png") );
		mMenu->addItem(BXLang("Goto next empty string"), FindEmptyStringCommand::getSingletonPtr(), icon);

		mFindConfig.bind( BLADE_NEW ConfigAtom(BXLang("Find String. Format [COLUMN NAME:]STRING."), TString::EMPTY, CAF_READWRITE) );
		HCDHANDLER handler(BLADE_NEW ConfigDataHandler(&FindStringCommand::getString, FindStringCommand::getSingletonPtr(), &FindStringCommand::execute) );
		mFindConfig->setDataHandler( handler  );
	}

	//////////////////////////////////////////////////////////////////////////
	void			LangEditor::shutdownEditor()
	{
		IEditorFramework::getSingleton().unregisterFileType(LangEditorFile::LANG_EDITOR_FILE);
	}
	
}//namespace Blade
