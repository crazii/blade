/********************************************************************
	created:	2017/01/14
	filename: 	PluginBase.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <PluginBase.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString PluginBase::ADDON_PLUGIN_OPT = BTString("Plugin_Addon_Opt");

	//////////////////////////////////////////////////////////////////////////
	PluginBase::PluginBase(const TString& name)
		:IPlugin(name)
		,mName(name)
		,mEnabled(true)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IPlugin::AddOnObject			PluginBase::getAddOnObject(const TString& name) const
	{
		if (name == ADDON_PLUGIN_OPT)
			return &mConfig;
		else
			return NULL;
	}
	
}//namespace Blade