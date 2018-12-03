/********************************************************************
	created:	2011/04/13
	filename: 	WorldEditableManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/logic/ILogicWorld.h>
//editor framework headers
#include <interface/IEditorFramework.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <interface/public/graphics/IGraphicsService.h>

#include <BladeFramework_blang.h>
#include <BladeEditor_blang.h>
#include <WorldEditor_blang.h>
#include "WorldEditableManager.h"
#include "WorldFile.h"


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	WorldEditableManager::WorldEditableManager()
		:mEditorFile(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WorldEditableManager::~WorldEditableManager()
	{

	}

	/************************************************************************/
	/* IEditor interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	WorldEditableManager::initEditor()
	{
		TString iconPath = BTString("globe.png");
		IconIndex index = IIconManager::getSingleton().loadSingleIconImage( iconPath );

		SEditorFileInfoEx info;
		info.mSupportedScenes.reserve(3);
		info.mSupportedScenes.push_back( BTString("GraphicsScene") );
		info.mSupportedScenes.push_back( BTString("GeometryScene") );
		info.mSupportedScenes.push_back( BTString("GameScene") );
		info.mFileExtension = WORLD_FILE_EXT;
		info.mFileDescription = BTLang(BLANG_EDITABLE_WORLD);
		info.mDefaultPath = BTString("../Data/worlds");
		info.mNewFileConfig = WorldFile::createFileConfig(info.mSupportedScenes);
		info.mMaxOpenLimit = 1;
		info.mIconID = index;
		info.mNewFolder = true;
		info.mOperations = EFO_NEW | EFO_SAVE | EFO_OPEN;

		info.mFileClassName = WORLD_FILE_TYPE;
		info.mWindowType = IViewportWindow::VIEWPORT_WINDOW_TYPE;

		IEditorFramework::getSingleton().registerFileType(info);

		//setup menu
		IMenu* menu = IMenuManager::getSingleton().addRootMenu(BXLang(BLANG_WORLD), TEXT('W') );

		IMenu* menu_create = menu->addSubMenu(BXLang(BLANG_CREATE_OBJECT), INVALID_ICON_INDEX, TEXT('C'));
		IMenu* menu_option = menu->addSubMenu(BXLang(BLANG_OPTIONS), INVALID_ICON_INDEX, TEXT('O'));

		for( WorldEdInfo::const_iterator i = mEdInfo.begin(); i != mEdInfo.end(); ++i )
		{
			const ED_INFO& edinfo = (*i)->mInfo;
			if(!edinfo.mNeedMenuCreation)
				continue;

			if(edinfo.mConfigOnCreation != NULL )
				menu_create->addItem(edinfo.mEditableName, &mCreateCmd, INVALID_ICON_INDEX, true, edinfo.mClassName[0]);
			else
				menu_create->addItem(edinfo.mEditableName, &mCreateCmd, INVALID_ICON_INDEX, false, edinfo.mClassName[0]);

			if(edinfo.mGlobalConfig != NULL )
				menu_option->addItem(edinfo.mEditableName, &mConfigCmd, INVALID_ICON_INDEX, true, edinfo.mClassName[0]);
		}

		//menu enable/disable notification
		IEditorFramework::getSingleton().getFrameworkNotification()->addFileTypeMenu( WORLD_FILE_TYPE, menu);
	}

	//////////////////////////////////////////////////////////////////////////
	void	WorldEditableManager::shutdownEditor()
	{
		for(WorldEdInfo::iterator i = mEdInfo.begin(); i != mEdInfo.end(); ++i)
		{
			BLADE_DELETE *i;
		}
		mEdInfo.clear();

		IEditorFramework::getSingleton().unregisterFileType(WORLD_FILE_TYPE);
	}

	/************************************************************************/
	/* IEditableManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	WorldEditableManager::registerEditableInfo(const ED_INFO& info)
	{
		WED_INFO* winfo = BLADE_NEW WED_INFO;
		winfo->mInfo = info;
		winfo->mInstanceCount = 0;

		bool result =  mEdInfo.insert(winfo).second;

		if( !result )
			BLADE_DELETE winfo;

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	WorldEditableManager::unregisterEditableInfo(const TString& editableName)
	{
		WED_INFO findee;
		findee.mInfo.mEditableName = editableName;
		WorldEdInfo::const_iterator i = mEdInfo.find(&findee);
		if( i == mEdInfo.end() )
			return false;

		BLADE_DELETE *i;

		mEdInfo.erase(i);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const ED_INFO*	WorldEditableManager::getEditableInfo(const TString& editableName) const
	{
		WED_INFO findee;
		findee.mInfo.mEditableName = editableName;
		WorldEdInfo::const_iterator i = mEdInfo.find(&findee);
		if( i == mEdInfo.end() )
			return NULL;
		else
			return &( (*i)->mInfo );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WorldEditableManager::addEditable(const HEDITABLE& editable)
	{
		if(  editable == NULL )
			return false;

		if( !mEditableSet.insert(editable).second )
			return false;

		WED_INFO* pInfo = this->getWorldEditableInfo(editable->getName());

		//none-registered editables
		if( pInfo == NULL )
			return true;
		
		if( ++pInfo->mInstanceCount == pInfo->mInfo.mInstanceLimit )
		{
			IMenu* menu = IMenuManager::getSingleton().findMenu( BXLang(BLANG_CREATE_OBJECT) )->getSubMenu( editable->getName() );
			if(menu != NULL)
				menu->setEnabled(false);

			if( pInfo->mInfo.mConfigOnCreation != NULL && pInfo->mInfo.mConfigOnCreation != pInfo->mInfo.mGlobalConfig )
				pInfo->mInfo.mConfigOnCreation->setReadOnly(true);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WorldEditableManager::renameEditable(IEditable* editable, const TString& newName)
	{
		HEDITABLE hEditable;
		for (WorldEditableSet::iterator i = mEditableSet.begin(); i != mEditableSet.end(); ++i)
		{
			if (*i == editable)
			{
				hEditable = *i;
				mEditableSet.erase(i);
				break;
			}
		}
		if (hEditable != NULL)
		{
			this->getEditorFile()->getStage()->renameEntity(editable->getTargetEntity(), newName);
			this->addEditable(hEditable);
		}
		return hEditable != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IEditable*		WorldEditableManager::getEditable(const TString& instanceName) const
	{
		EditableSetHelper FindHelper(instanceName);
		WorldEditableSet::const_iterator i = mEditableSet.find(FindHelper);
		if( i == mEditableSet.end() )
			return NULL;
		else
			return *i;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WorldEditableManager::removeEditable(const TString& instanceName)
	{
		EditableSetHelper EraseHelper(instanceName);
		return mEditableSet.erase(EraseHelper) == 1;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WorldEditableManager::isEditablesModified() const
	{
		for(WorldEditableSet::const_iterator i = mEditableSet.begin(); i != mEditableSet.end(); ++i)
		{
			const HEDITABLE& hEditable = *i;
			if( hEditable->isModified() )
				return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WorldEditableManager::saveAllEditable() const
	{
		for(WorldEditableSet::const_iterator i = mEditableSet.begin(); i != mEditableSet.end(); ++i)
		{
			const HEDITABLE& hEditable = *i;
			if( hEditable->isModified() )
				hEditable->save();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IEditorFile*	WorldEditableManager::getEditorFile() const
	{
		return mEditorFile;
	}

	/************************************************************************/
	/* IWorldEditableManager                                                                    */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	ILogicWorld*	WorldEditableManager::getWorld() const
	{
		if( mEditorFile == NULL )
			return NULL;
		else
			return static_cast<WorldFile*>(mEditorFile)->getWorld();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	WED_INFO*		WorldEditableManager::getWorldEditableInfo(const TString& editableName)
	{
		WED_INFO findee;
		findee.mInfo.mEditableName = editableName;
		WorldEdInfo::iterator i = mEdInfo.find(&findee);
		if( i == mEdInfo.end() )
			return NULL;
		else
			return (*i);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WorldEditableManager::setEditorFile(IEditorFile* file)
	{
		mEditorFile = file;

		//"create object" menu command : enable/disable
		this->clearAll();

		if( mEditorFile != NULL )
		{
			//WorldFile* wfile = static_cast<WorldFile*>(file);
			for( WorldEdInfo::iterator i = mEdInfo.begin(); i != mEdInfo.end(); ++i )
			{
				WED_INFO& info = *(*i);
				if (info.mInfo.mNeedAppFlag && info.mInfo.mType != NULL)
					info.mInfo.mAppFlag = IGraphicsService::getSingleton().generateGraphicsTypeAppFlag(*info.mInfo.mType);
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WorldEditableManager::clearAll()
	{
		mEditableSet.clear();

		for( WorldEdInfo::iterator i = mEdInfo.begin(); i != mEdInfo.end(); ++i)
		{
			WED_INFO& info = *(*i);
			info.mInstanceCount = 0;
			IMenu* menuCreate = IMenuManager::getSingleton().getRootMenu( BXLang(BLANG_WORLD) )->getSubMenu( BXLang(BLANG_CREATE_OBJECT) );
			IMenu* subMenu = menuCreate->getSubMenu(info.mInfo.mEditableName);
			if(subMenu != NULL)
				subMenu->setEnabled(true);
			if( info.mInfo.mConfigOnCreation != NULL )
				info.mInfo.mConfigOnCreation->setReadOnly(false);
		}
	}


}//namespace Blade
