/********************************************************************
	created:	2010/06/12
	filename: 	TerrainEditorPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <BladeTerrainEditor.h>
#include "TerrainEditorPlugin.h"
#include <ConfigTypes.h>
#include <interface/IFramework.h>
#include <interface/IPlatformManager.h>
#include <interface/IStageConfig.h>
#include <interface/IPage.h>
#include <interface/IMediaLibrary.h>
#include <interface/public/IEditable.h>
#include <databinding/DataSourceWriter.h>

#include <interface/IWorldEditableManager.h>
#include <interface/ITerrainConfigManager.h>

#include <TerrainUtility.h>

#include "interface_imp/TerrainPageEditable.h"
#include "interface_imp/TerrainToolManager.h"
#include "interface_imp/TerrainEditorElement.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("Editor_TerrainEditorPlugin");
	BLADE_DEF_PLUGIN(TerrainEditorPlugin);

	//////////////////////////////////////////////////////////////////////////
	class TerrainTextureFilter : public IMediaLibrary::IMediaFilter, public StaticAllocatable
	{
	public:
		/** @brief filter for media files */
		/* @remark return true means the input file is valid  */
		virtual bool	filterFile(const TString& file) const
		{
			return !TerrainUtility::isNormalTexture(file) && !TerrainUtility::isSpecularTexture(file);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	TerrainEditorPlugin::TerrainEditorPlugin()
		:PluginBase(PLUGIN_NAME)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainEditorPlugin::~TerrainEditorPlugin()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("TerrainEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsTerrain"));
		dependencyList.push_back(BTString("WorldEditor"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorPlugin::install()
	{
		NameRegisterFactory(TerrainEditorElement, EditorElement, TerrainEditorElement::TERRAIN_EDITOR_ELEMENT);
		NameRegisterFactory(TerrainPageEditable, IEditable, TerrainPageEditable::TERRAIN_PAGE_EDITABLE_NAME );
		NameRegisterFactory(TerrainEditable, IEditable, TerrainEditable::TERRAIN_EDITABLE_NAME );

		//register terrain editor element data source (no extra data)
		DataSourceWriter<TerrainEditorElement> writer( TerrainEditorElement::TERRAIN_EDITOR_ELEMENT, DS_SIGNATURE(EditorElement) );

		IStageConfig::getSingleton().addLayerElement(IPage::DEFAULT_LAYER, TerrainEditorElement::TERRAIN_EDITOR_ELEMENT);
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorPlugin::uninstall()
	{
		IWorldEditableManager::getInterface().unregisterEditableInfo( BTLang(BLANG_TERRAIN) );
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorPlugin::initialize()
	{
		//register media type
		IMediaLibrary::MEDIA_TYPE TerrainTextureMediaType;
		TerrainTextureMediaType.mType = TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT;
		TerrainTextureMediaType.mIndex = 0;
		TerrainTextureMediaType.mIsPlayable = false;
		TerrainTextureMediaType.mSharedIcon = false;
		TerrainTextureMediaType.mPreviewer.bind( BLADE_FACTORY_CREATE(IMediaLibrary::IMediaPreviewer,BTString("ImagePreivewer")) );
		TerrainTextureMediaType.mFilter.bind( BLADE_NEW TerrainTextureFilter() );
		TerrainTextureMediaType.mExtensions.reserve(2);
		TerrainTextureMediaType.mExtensions.push_back( BTString("dds") );
		TerrainTextureMediaType.mExtensions.push_back( BTString("png") );
		index_t index = IMediaLibrary::getSingleton().registerMediaType(TerrainTextureMediaType, BTString("image:terrain_textures") );
		IMediaLibrary::getSingleton().addMediaFilePath(index, BTString("image:") );

		//enable edit mode
		ITerrainConfigManager::getInterface().setEditingMode(true);

		//setup tools
		TerrainToolManager::getSingleton().initalize();

		//register editable types into world editable manager

		ConfigGroup* group = BLADE_NEW ConfigGroup(BTString("Terrain Config"));
		mTerrainGlobalConfig.bind( group );
		group->addSubConfig(ITerrainConfigManager::getInterface().getTerrainCreationConfig() );
		group->addSubConfig(ITerrainConfigManager::getInterface().getTerrainGlobalConfig() );

		ED_INFO info;
		info.mClassName = TerrainPageEditable::TERRAIN_PAGE_EDITABLE_NAME;
		info.mConfigOnCreation = ITerrainConfigManager::getInterface().getTerrainCreationConfig();
		info.mModalConfig = true;
		info.mEditableName = BTLang(BLANG_TERRAIN);
		info.mGlobalConfig = mTerrainGlobalConfig;
		info.mInstanceLimit = 1;
		info.mType = ITerrainConfigManager::getInterface().getTerrainGraphicsType();
		info.mNeedAppFlag = true;
		info.mNeedMenuCreation = true;
		info.mGlobalConfigNotify = Delegate(ITerrainConfigManager::getInterfacePtr(), &ITerrainConfigManager::notifyGlobalConfigChanged);
		IWorldEditableManager::getInterface().registerEditableInfo(info);
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainEditorPlugin::shutdown()
	{
		TerrainToolManager::getSingleton().shutdown();
		mTerrainGlobalConfig.clear();
	}

}//namespace Blade