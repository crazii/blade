/********************************************************************
	created:	2010/06/12
	filename: 	WorldEditorPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeWorldEditor.h>
#include "WorldEditorPlugin.h"
#include <interface/IPlatformManager.h>

#include <interface/IEditorFramework.h>

#include <WorldEditor_blang.h>
#include "interface_imp/WorldFile.h"
#include "interface_imp/WorldEditableManager.h"

namespace Blade
{
	template class Factory<IWorldEditableManager>;

	static const TString PLUGIN_NAME = BTString("Editor_WorldEditorPlugin");
	BLADE_DEF_PLUGIN(WorldEditorPlugin);

	//////////////////////////////////////////////////////////////////////////
	WorldEditorPlugin::WorldEditorPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WorldEditorPlugin::~WorldEditorPlugin()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldEditorPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("WorldEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldEditorPlugin::getDependency(TStringParam& dependencyList) const
	{
		//dependencyList.push_back(BTString("GameSystem"));	//editor framework moved to top level,so don't depend on game now
		dependencyList.push_back(BTString("UI"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldEditorPlugin::install()
	{
		NameRegisterFactory(WorldFile, IEditorFile, WORLD_FILE_TYPE);
		RegisterSingleton(WorldEditableManager,IWorldEditableManager);
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldEditorPlugin::uninstall()
	{
		//note: world file config comes from scenes (subsystem plugins), but after subsystem plugin unloaded, file config will hold invalid pointers
		//so we need clear it before all plugins being unloaded
		//or just make a clone?
		WorldFile::clearFileConfig();
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldEditorPlugin::initialize()
	{
		IEditorFramework::getSingleton().addEditor( WorldEditableManager::getSingletonPtr() );
	}

	//////////////////////////////////////////////////////////////////////////
	void				WorldEditorPlugin::shutdown()
	{
		IEditorFramework::getSingleton().removeEditor( WorldEditableManager::getSingletonPtr() );
	}

}//namespace Blade