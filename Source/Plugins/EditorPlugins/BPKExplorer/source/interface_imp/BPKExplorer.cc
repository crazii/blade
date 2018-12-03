/********************************************************************
	created:	2015/08/17
	filename: 	BPKExplorer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BPKExplorer.h"
#include "BPKEditorFile.h"
#include <ConfigTypes.h>
#include <databinding/ConfigDataHandler.h>
#include <interface/IEditorFramework.h>
#include <interface/EditorFileInfo.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IIconManager.h>
#include "BPKEditorCommand.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	BPKExplorer::BPKExplorer()
	{
		mContextMenu = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	BPKExplorer::~BPKExplorer()
	{

	}

	/************************************************************************/
	/* IEditor interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			BPKExplorer::initEditor()
	{
		TString iconPath = BTString("package.png");
		IconIndex index = IIconManager::getSingleton().loadSingleIconImage( iconPath );

		SEditorFileInfoEx info;
		info.mFileClassName = BPKEditorFile::BPK_FILE;
		info.mWindowType = ITableWindow::TABLE_WINDOW_TYPE;
		info.mFileDescription = BTLang("Blade package file");
		info.mFileExtension = BTString("bpk");
		info.mDefaultPath = BTString("../");
		info.mIconID = index;
		info.mMaxOpenLimit = 10;
		info.mNewFileConfig = NULL;
		info.mOperations = EFO_OPEN|EFO_SAVE;
		info.mNewFolder = false;
		IEditorFramework::getSingleton().registerFileType(info);

		mContextMenu = IMenuManager::getSingleton().createExtraMenu(BTString("BPK Explorer Menu"));

		IconIndex icon = IIconManager::getSingleton().loadSingleIconImage( BTString("back.png") );
		mContextMenu->addItem(BXLang("Back"), BPKNaviBackCmd::getSingletonPtr(), icon);

		icon = IIconManager::getSingleton().loadSingleIconImage( BTString("go.png") );
		mContextMenu->addItem(BXLang("Forward"), BPKNaviForwardCmd::getSingletonPtr(), icon);

		icon = IIconManager::getSingleton().loadSingleIconImage( BTString("folder_up.png") );
		mContextMenu->addItem(BXLang("Up"), BPKNaviUpperFolderCmd::getSingletonPtr(), icon);

		mContextMenu->addSeparator();

		icon = IIconManager::getSingleton().loadSingleIconImage( BTString("export.png") );
		mContextMenu->addItem(BXLang("Export"), BPKExportCmd::getSingletonPtr(), icon);


		mNavigationConfig.bind( BLADE_NEW ConfigAtom(BXLang("Navigation bar"), TString::EMPTY, CAF_READWRITE) );
		HCDHANDLER handler(BLADE_NEW ConfigDataHandler(&BPKNaviInputCmd::getCurrentPath, BPKNaviInputCmd::getSingletonPtr(), &BPKNaviInputCmd::setCurrentPath) );
		mNavigationConfig->setDataHandler( handler  );
	}

	//////////////////////////////////////////////////////////////////////////
	void			BPKExplorer::shutdownEditor()
	{
		IEditorFramework::getSingleton().unregisterFileType(BPKEditorFile::BPK_FILE);
	}

}//namespace Blade