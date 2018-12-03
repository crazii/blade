/********************************************************************
	created:	2010/05/19
	filename: 	BladeTerrainPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeTerrainPlugin.h"
#include <BladeTerrain.h>

#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/IStageConfig.h>
#include <interface/public/CommonState.h>
#include <interface/IRenderTypeManager.h>
#include <databinding/DataSourceWriter.h>
#include <interface/IGraphicsSystem.h>

#include <BladeTerrain_blang.h>
#include "TerrainElement.h"
#include "TerrainConfigManager.h"
#include "TerrainResource.h"
#include "TerrainSerializer.h"
#include "batch_combine/TerrainBatchCombiner.h"
#include "index_generator/OptimizedIndexGenerator.h"
#include "index_generator/PreciseIndexGenerator.h"

namespace Blade
{
	static const TString	PLUGIN_NAME = BTString("BladeTerrain");
	BLADE_DEF_PLUGIN(TerrainPlugin);

	/************************************************************************/
	/* template instantiations                                                                     */
	/************************************************************************/
	template class Factory<ITerrainConfigManager>;
	template class Factory<ITerrainBatchCombiner>;
	template class Factory<IIndexGenerator>;

	//////////////////////////////////////////////////////////////////////////
	TerrainPlugin::TerrainPlugin()
		:PluginBase(PLUGIN_NAME)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainPlugin::~TerrainPlugin()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GraphicsTerrain"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPlugin::install()
	{
		NameRegisterFactory(FullBatchCombiner, ITerrainBatchCombiner, BXLang(BLANG_FULL_COMBINE));
		NameRegisterFactory(EmptyBatchCombiner, ITerrainBatchCombiner, BXLang(BLANG_NO_COMBINE));
		NameRegisterFactory(SceneBatchombiner, ITerrainBatchCombiner, BXLang(BLANG_SCENE_PASS_COMBINE));
		NameRegisterFactory(OptimizedIndexGenerator, IIndexGenerator, BXLang(BLANG_LOW) );
		NameRegisterFactory(PreciseIndexGenerator, IIndexGenerator, BXLang(BLANG_HIGH) );

		RegisterSingleton(TerrainConfigManager,ITerrainConfigManager);

		NameRegisterFactory(TerrainResource, IResource, TerrainResource::TYPE);
		NameRegisterFactory(TerrainSerializer,ISerializer,TerrainResource::TYPE);

		NameRegisterFactory(TerrainElement, GraphicsElement, TerrainConsts::TERRAIN_ELEMENT);

		IResourceManager::getSingleton().addSearchPath( TextureResource::TEXTURE_RESOURCE_TYPE, BTString("image:/terrain_textures/") );

		IResourceManager::getSingleton().registerFileExtension( TerrainResource::TYPE, BTString("blt") );

		IRenderTypeManager::getSingleton().addRenderType(&TerrainConfigManager::getRenderType());

		//global config is configured through config panel, and better not save
		//IStageConfig::getSingleton().getSerializableMap().addSerializable(TerrainConfigManager::GLOBAL_CONFIG_TYPENAME, &TerrainConfigManager::getSingleton().mTerrainConfig );
		IStageConfig::getSingleton().getSerializableMap().addSerializable(TerrainConfigManager::CREATION_CONFIG_TYPENAME, &TerrainConfigManager::getSingleton().mTerrainInfo );
		//one terrain tile per page
		IStageConfig::getSingleton().addLayerElement(IPage::DEFAULT_LAYER, TerrainConsts::TERRAIN_ELEMENT);

		//mConfig = TerrainConfigManager::getSingleton().mGlobalConfig;
		//IGraphicsSystem::getSingleton().addAdvanceConfig(mConfig);
		IGraphicsSystem::getSingleton().addAdvanceConfig(TerrainConfigManager::getSingleton().mGlobalConfig);
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPlugin::uninstall()
	{
		IRenderTypeManager::getSingleton().removeRenderType(TerrainConfigManager::getRenderType().getName());
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPlugin::initialize()
	{
		RegisterFactory(TerrainElement, GraphicsElement);

		const TString TERRAIN_INDEX_X = BXLang(BLANG_INDEX_X);
		const TString TERRAIN_INDEX_Z = BXLang(BLANG_INDEX_Z);
		const TString TERRAIN_TEXTURE = BXLang(BLANG_TEXTURES);

		//element config
		DataSourceWriter<TerrainElement> writer(TerrainConsts::TERRAIN_ELEMENT/*, DS_SIGNATURE(GraphicsElement)*/);
		//version 0.1a
		writer.beginVersion(Version(0, 1, 'A'));
		{
			//read only info ( no serialization )
			writer << CommonState::POSITION << CAF_NONE_CONFIG << POINT3::ZERO << &TerrainElement::onConfigChange << GraphicsElement::getPositioAddr<TerrainElement>();
			writer << CommonState::BOUNDING << CAF_NONE_CONFIG << AABB::EMPTY << &TerrainElement::onConfigChange << GraphicsElement::getBoundingAddr<TerrainElement>();
			writer << TERRAIN_INDEX_X << CAF_NONE_CONFIG << &TerrainElement::mTileIndexX;
			writer << TERRAIN_INDEX_Z << CAF_NONE_CONFIG << &TerrainElement::mTileIndexZ;
			CONFIG_UIHINT hint(CUIH_IMAGE, TerrainConsts::TERRAIN_MEDIA_IMAGE_HINT);
			writer << TERRAIN_TEXTURE << hint << CAF_READWRITE << &TerrainElement::onTextureChange << &TerrainElement::mTileTextures;
		}
		writer.endVersion();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainPlugin::shutdown()
	{
		mConfig.clear();
		IGraphicsSystem::getSingleton().removeAdvanceConfig(TerrainConfigManager::getSingleton().mGlobalConfig);
	}

}//namespace Blade

