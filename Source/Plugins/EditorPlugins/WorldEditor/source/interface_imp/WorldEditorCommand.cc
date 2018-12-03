/********************************************************************
	created:	2011/05/05
	filename: 	WorldEditorCommand.cc
	author:		Crazii
	purpose:	merge two class : WorldEditor & WorldEditableManager
				and move the command def from WorldEditor.cc to here
*********************************************************************/
#include <BladePCH.h>
#include <interface/IEditorFramework.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/logic/ILogicWorld.h>

#include <BladeFramework_blang.h>
#include <BladeEditor_blang.h>
#include <WorldEditor_blang.h>
#include "WorldEditorCommand.h"
#include "WorldEditableManager.h"
#include "WorldFile.h"


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	void WorldObjectCreateCommand::execute(ICommandUI* cui)
	{
		const TString& name = cui->getName();

		WED_INFO* pInfo = WorldEditableManager::getSingleton().getWorldEditableInfo(cui->getName());
		if( pInfo == NULL)
			return;

		ILogicWorld* world = WorldEditableManager::getSingleton().getWorld();
		if( world == NULL )
			return;

		if( pInfo->mInfo.mConfigOnCreation != NULL )
		{
			bool succeed = IEditorFramework::getSingleton().getEditorUI()->showSingleGroupConfig(BTLang(BLANG_CREATE) + BTString(" ") + name, pInfo->mInfo.mConfigOnCreation );
			if( succeed )
			{
				HEDITABLE hEditable(BLADE_FACTORY_CREATE(IEditable, pInfo->mInfo.mClassName));
				hEditable->initialize( pInfo->mInfo.mConfigOnCreation );
				WorldEditableManager::getSingleton().addEditable(hEditable);

				IEditorFile* file = WorldEditableManager::getSingleton().getEditorFile();
				if( file != NULL )
					file->setModified();
			}
		}

	}

	//////////////////////////////////////////////////////////////////////////
	static void worldDelegateCommand(const Delegate& realDelegate)
	{
		IEditorFile* file = WorldEditableManager::getSingleton().getEditorFile();
		if( file != NULL )
			file->setModified();
		realDelegate.call();
	}

	void WorldObjectGlobalConfigCommand::execute(ICommandUI* cui)
	{
		const TString& name = cui->getName();
		const WED_INFO* pWInfo = WorldEditableManager::getSingleton().getWorldEditableInfo(name);
		if( pWInfo == NULL)
			return;

		const ED_INFO* pInfo = &(pWInfo->mInfo);
	
		if( pInfo->mGlobalConfig != NULL )
		{
			IConfigDialog::INFO dlgInfo(pInfo->mModalConfig,  false, Delegate(worldDelegateCommand), &(pInfo->mGlobalConfigNotify));
			IEditorFramework::getSingleton().getEditorUI()->showSingleGroupConfig(name + BTString(" ") + BTLang(BLANG_OPTIONS), pInfo->mGlobalConfig, dlgInfo);
		}
	}

}//namespace Blade
