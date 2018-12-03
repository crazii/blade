/********************************************************************
	created:	2010/04/02
	filename: 	UISystemPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "UISystemPlugin.h"
#include <interface/IPlatformManager.h>
#include "interface_imp/UISystem.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladeUI");
	BLADE_DEF_PLUGIN(UISystemPlugin);

	//////////////////////////////////////////////////////////////////////////
	UISystemPlugin::UISystemPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	UISystemPlugin::~UISystemPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystemPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("UIService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystemPlugin::getDependency(TStringParam& dependencyList) const
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS || BLADE_PLATFORM == BLADE_PLATFORM_LINUX
		dependencyList.push_back(BTString("KeyboardDevice"));
		dependencyList.push_back(BTString("MouseDevice"));
#endif
		dependencyList.push_back(BTString("WindowService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystemPlugin::install()
	{
		//NameRegisterFactory(UISystem,ISubsystem,UISystem::UI_SYSTEM_NAME);
		NameRegisterSingleton(UISystem,ISubsystem,UISystem::UI_SYSTEM_NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystemPlugin::uninstall()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystemPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystemPlugin::shutdown()
	{

	}
	
}//namespace Blade