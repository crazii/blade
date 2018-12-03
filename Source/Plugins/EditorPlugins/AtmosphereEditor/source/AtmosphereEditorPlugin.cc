/********************************************************************
created:	2011/09/03
filename: 	AtmosphereEditorPlugin.cc
author:		Crazii
purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeAtmosphereEditor.h>
#include "AtmosphereEditorPlugin.h"

#include <interface/IPlatformManager.h>
#include <interface/IStageConfig.h>
#include <interface/IMediaLibrary.h>
#include <interface/public/IEditable.h>

#include <interface/IWorldEditableManager.h>

#include <interface/IAtmosphere.h>
#include <BladeAtmosphereEditor_blang.h>
#include "AtmosphereEditable.h"


namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_AtmosphereEditorPlugin");
	BLADE_DEF_PLUGIN(AtmosphereEditorPlugin);

	//////////////////////////////////////////////////////////////////////////
	AtmosphereEditorPlugin::AtmosphereEditorPlugin()
		:PluginBase(PLUGIN_NAME)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	AtmosphereEditorPlugin::~AtmosphereEditorPlugin()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmosphereEditorPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("AtmosphereEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmosphereEditorPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsAtmosphere"));
		dependencyList.push_back(BTString("WorldEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmosphereEditorPlugin::install()
	{
		NameRegisterFactory(AtmosphereEditable,IEditable,AtmosphereEditable::ATMOSPHERE_EDITABLE_NAME );
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmosphereEditorPlugin::uninstall()
	{
		IWorldEditableManager::getInterface().unregisterEditableInfo( BTLang(BLANG_ATMOSPHERE) );
	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmosphereEditorPlugin::initialize()
	{
		//register 
		ED_INFO info;
		info.mClassName = AtmosphereEditable::ATMOSPHERE_EDITABLE_NAME;
		info.mConfigOnCreation = IAtmosphere::getInterface().getAtmosphereConfig();
		info.mModalConfig = false;
		info.mEditableName = BTLang(BLANG_ATMOSPHERE);
		info.mGlobalConfig = IAtmosphere::getInterface().getAtmosphereConfig();
		info.mInstanceLimit = 1;
		info.mType = IAtmosphere::getInterface().getAtmosphereType();
		info.mNeedAppFlag = false;
		info.mNeedMenuCreation = true;
		info.mGlobalConfigNotify = Delegate::EMPTY;

		//register media type
		IMediaLibrary::MEDIA_TYPE AtmosphereTextureMediaType;
		AtmosphereTextureMediaType.mType = AtmosphereConsts::ATMOSPHERE_SKY_TEXTURE_IMAGE;
		AtmosphereTextureMediaType.mIndex = 0;
		AtmosphereTextureMediaType.mIsPlayable = false;
		AtmosphereTextureMediaType.mSharedIcon = false;
		AtmosphereTextureMediaType.mPreviewer.bind( BLADE_FACTORY_CREATE(IMediaLibrary::IMediaPreviewer,BTString("ImagePreivewer")) );
		AtmosphereTextureMediaType.mExtensions.reserve(2);
		AtmosphereTextureMediaType.mExtensions.push_back( BTString("dds") );
		AtmosphereTextureMediaType.mExtensions.push_back( BTString("png") );
		IMediaLibrary::getSingleton().registerMediaType( AtmosphereTextureMediaType, BTString("media:image/sky_textures") );

		AtmosphereTextureMediaType.mType = AtmosphereConsts::ATMOSPHERE_SKYBOX_TEXTURE_IMAGE;
		IMediaLibrary::getSingleton().registerMediaType(AtmosphereTextureMediaType, BTString("media:image/sky_textures/cube"));

		IWorldEditableManager::getInterface().registerEditableInfo(info);

	}

	//////////////////////////////////////////////////////////////////////////
	void				AtmosphereEditorPlugin::shutdown()
	{

	}

}//namespace Blade