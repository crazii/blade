/********************************************************************
created:	2017/08/31
filename: 	BladeGrassPlugin.cc
author:		Crazii
purpose:
*********************************************************************/
#include "BladeGrassPlugin.h"
#include "GrassResource.h"
#include "GrassSerializer.h"
#include "GrassElement.h"
#include "GrassConfig.h"
#include <interface/IRenderTypeManager.h>
#include <interface/IStageConfig.h>
#include <databinding/DataSourceWriter.h>
#include <interface/IGrassConfig.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

namespace Blade
{
	template class Factory<IGrassConfig>;

	static const TString	PLUGIN_NAME = BTString("BladeGrassPlugin");
	BLADE_DEF_PLUGIN(BladeGrassPlugin);

	//////////////////////////////////////////////////////////////////////////
	BladeGrassPlugin::BladeGrassPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BladeGrassPlugin::~BladeGrassPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGrassPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GraphicsGrass"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGrassPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("GraphicsService"));
		dependencyList.push_back(BTString("GraphicsModel"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGrassPlugin::install()
	{
		NameRegisterFactory(GrassElement, GraphicsElement, GrassConsts::GRASS_ELEMENT_TYPE);
		NameRegisterFactory(GrassResource, GraphicsResource, GrassConsts::GRASS_RESOURCE_TYPE);
		NameRegisterFactory(GrassSharedResource, GraphicsResource, GrassSharedResource::GRASS_SHARED_RESOURCE_TYPE);
		NameRegisterFactory(GrassSerializer, ISerializer, GrassConsts::GRASS_RESOURCE_TYPE);
		NameRegisterFactory(GrassSharedSerializer, ISerializer, GrassSharedResource::GRASS_SHARED_RESOURCE_TYPE);

		IRenderTypeManager::getSingleton().addRenderType(&static_cast<RenderType&>(GrassConfig::getSingleton().getRenderType()));
		IStageConfig::getSingleton().addPageLayer(BTString("grasses"), GrassConsts::GRASS_ELEMENT_TYPE, 32, 256.0f);

		{
			DataSourceWriter<GrassElement> writer(GrassConsts::GRASS_ELEMENT_TYPE, DS_SIGNATURE(GraphicsElement));
			writer.beginLatestVersion();
			{
				static const TString GRASS_MESH = BTString("Grass Meshes");
				static const TString GRASS_INDEX_X = BTString("Index X");
				static const TString GRASS_INDEX_Z = BTString("Index Z");
				static const TString GRASS_OFFSET_X = BTString("Offset X");
				static const TString GRASS_OFFSET_Z = BTString("Offset Z");

				writer << GRASS_INDEX_X << CAF_SERIALIZE << &GrassElement::mX;
				writer << GRASS_INDEX_Z << CAF_SERIALIZE << &GrassElement::mZ;
				writer << GRASS_OFFSET_X << CAF_SERIALIZE << &GrassElement::mOffsetX;
				writer << GRASS_OFFSET_Z << CAF_SERIALIZE << &GrassElement::mOffsetZ;

				writer << GRASS_MESH << CONFIG_ACCESS(CAF_NORMAL | CAF_COLLECTION_WRITE)
					<< CONFIG_UIHINT(CUIH_IMAGE, GrassConsts::GRASS_MODEL_HINT)
					<< &GrassElement::onConfigChange << &GrassElement::mGrassMesh;
			}
			writer.endVersion();
		}
		
		{
			const TString TARGET_FLAGS = BTString("Target Type Flags");

			DataSourceWriter<GrassConfig> writer(BTString("Grass"));
			writer.beginVersion(Version(0, 2, 'C'));
			writer << TARGET_FLAGS << CAF_NORMAL << &GrassConfig::mTargetFlag;
			writer.endVersion();
		}
		
		IStageConfig::getSingleton().addLayerElement(IPage::DEFAULT_LAYER, GrassConsts::GRASS_ELEMENT_TYPE);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGrassPlugin::uninstall()
	{
		IRenderTypeManager::getSingleton().removeRenderType(&static_cast<const RenderType&>(GrassConfig::getSingleton().getRenderType()));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGrassPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeGrassPlugin::shutdown()
	{

	}
}