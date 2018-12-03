/********************************************************************
	created:	2013/04/09
	filename: 	ModelViewer.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ModelViewer.h"
#include "ModelViewerCommand.h"

#include <BladeModelViewer.h>
#include <interface/IModelResource.h>

#include <interface/IEditorFramework.h>
#include <interface/IEditorToolManager.h>
#include <interface/EditorFileInfo.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <BladeModelViewer_blang.h>
#include "ModelViewTool.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	ModelViewer::ModelViewer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelViewer::~ModelViewer()
	{

	}

	/************************************************************************/
	/* IEditor interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			ModelViewer::initEditor()
	{
		TString iconPath = BTString("model.png");
		IconIndex icon = IIconManager::getSingleton().loadSingleIconImage( iconPath );

		SEditorFileInfoEx info;
		info.mFileClassName = MODEL_FILE_TYPE;
		info.mFileDescription = BTLang(BLANG_MODELFILE);
		info.mFileExtension = ModelConsts::MODEL_FILE_EXT;
		info.mDefaultPath = BTString("../Data/model");
		info.mIconID = icon;
		info.mMaxOpenLimit = 10;
		info.mNewFileConfig = NULL;
		info.mWindowType = IViewportWindow::VIEWPORT_WINDOW_TYPE;
		info.mSupportedScenes.reserve(3);
		info.mSupportedScenes.push_back( BTString("GraphicsScene") );
		info.mSupportedScenes.push_back( BTString("GeometryScene") );
		info.mSupportedScenes.push_back( BTString("GameScene") );
		info.mOperations = EFO_OPEN;
		info.mNewFolder = true;

		IEditorFramework::getSingleton().registerFileType(info);

		//setup menu
		IMenu* menu = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_MODEL), TEXT('M') );

		menu->addCheckItem(BTLang("Show Submesh Bounding"), MVShowBoundingCmd::getSingletonPtr(), true);
		menu->addCheckItem(BTLang("Show Skeleton"), MVShowSkeletonCmd::getSingletonPtr(), true);

		//menu enable/disable notification
		IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeMenu( MODEL_FILE_TYPE, menu);

		//create tools
		icon = IIconManager::getSingleton().loadSingleIconImage( BTString("IK_test.png") );
		IEditorTool* IKTest = BLADE_NEW ModelIKTestTool(icon);
		mTools.push_back(IKTest);

		for(ToolList::iterator i = mTools.begin(); i != mTools.end(); ++i)
		{
			IEditorToolManager::getSingleton().addTool(*i);
			IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeTool( MODEL_FILE_TYPE, *i );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ModelViewer::shutdownEditor()
	{
		IEditorFramework::getSingleton().unregisterFileType(BTString(BLANG_MODELFILE));

		for(size_t i = 0; i < mTools.size(); ++i )
			BLADE_DELETE mTools[i];
		mTools.clear();
	}
	
}//namespace Blade