/********************************************************************
created:	2011/09/03
filename: 	BladeAtmospherePlugin.cc
author:		Crazii
purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeAtmospherePlugin.h"
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/IStageConfig.h>
#include <BladeAtmosphere_blang.h>
#include "interface_imp/Atmosphere.h"

namespace Blade
{
	template class Factory<IAtmosphere>;

	static const TString PLUGIN_NAME = BTString("BladeAtmosphere");
	BLADE_DEF_PLUGIN(AtmospherePlugin);

	//////////////////////////////////////////////////////////////////////////
	AtmospherePlugin::AtmospherePlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	AtmospherePlugin::~AtmospherePlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmospherePlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GraphicsAtmosphere"));
	}


	//////////////////////////////////////////////////////////////////////////
	void				AtmospherePlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmospherePlugin::install()
	{
		RegisterFactory(AtmosphereElement,GraphicsElement);
		RegisterSingleton(Atmosphere,IAtmosphere);
		IResourceManager::getSingleton().addSearchPath( TextureResource::TEXTURE_RESOURCE_TYPE, BTString("image:/sky_textures/") );

		Atmosphere::getSingleton().registerRenderType();
		IStageConfig::getSingleton().getSerializableMap().addSerializable(AtmosphereConsts::ATMOSPHERE_STAGE_DATA_TYPE, Atmosphere::getSingletonPtr() );
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmospherePlugin::uninstall()
	{
		Atmosphere::getSingleton().unregisterRenderType();
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmospherePlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmospherePlugin::shutdown()
	{

	}

}//namespace Blade

