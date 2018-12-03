/********************************************************************
created:	2018/11/14
filename: 	IPlugin.cc
author:		Crazii
purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "platform/PlatformManager.h"

namespace Blade 
{

#if BLADE_STATIC_PLUGIN

struct PluginFinder
{
	const TString& name;
	PluginFinder(const TString& n) :name(n) {}
	inline bool operator()(const IPlugin* p) const
	{
		return p->getName() == name;
	}
};

static bool Blade_AddPluginDatabase(IPlugin* plugin/*not fully-initialized yet*/, const TString& name)
{
	bool result = false;
	//StaticPluginDB::iterator i = std::find_if(PLUGIN_DATABASE.begin(), PLUGIN_DATABASE.end(), PluginFinder(name));
	//if (i == PLUGIN_DATABASE.end())
	{
		PLUGIN_DATABASE.push_back(plugin);
		result = true;
	}
	return result;
}
#endif

//////////////////////////////////////////////////////////////////////////
IPlugin::IPlugin(const TString& name)
{
#if BLADE_STATIC_PLUGIN
	//note: base constructor: object no fully initialized at this point
	Blade_AddPluginDatabase(this, name);
#else
	BLADE_UNREFERENCED(name);
#endif
}



}//namespace Blade 