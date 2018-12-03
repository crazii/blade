/********************************************************************
	created:	2010/04/02
	filename: 	WindowSystemPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IPlatformManager.h>
#include <BladeWindow.h>
#include "WindowSystem.h"
#include "WindowSystemPlugin.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladeWindow");
	BLADE_DEF_PLUGIN(WindowSystemPlugin);

	static const TString WINDOW_SYSTEM_NAME = BTString("WindowSystem");

	//////////////////////////////////////////////////////////////////////////
	WindowSystemPlugin::WindowSystemPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WindowSystemPlugin::~WindowSystemPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				WindowSystemPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("WindowService"));
	}


	//////////////////////////////////////////////////////////////////////////
	void				WindowSystemPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("WindowDevice"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				WindowSystemPlugin::install()
	{
		//NameRegisterFactory(WindowSystem,ISubsystem,,WindowSystem::WINDOW_SYSTEM_NAME);
		NameRegisterSingleton(WindowSystem,ISubsystem,WindowSystem::WINDOW_SYSTEM_NAME);

		//IFramework::getSingleton().addSubSystem(WINDOW_SYSTEM_NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	void				WindowSystemPlugin::uninstall()
	{
		//Framework::getSingleton().remove
	}

	//////////////////////////////////////////////////////////////////////////
	void				WindowSystemPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				WindowSystemPlugin::shutdown()
	{

	}
	
}//namespace Blade