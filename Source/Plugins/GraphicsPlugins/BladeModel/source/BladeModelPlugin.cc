/********************************************************************
	created:	2013/04/01
	filename: 	BladeModelPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeModelPlugin.h"
#include <BladeModel.h>
#include <interface/IEnvironmentManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/IStageConfig.h>
#include <databinding/DataSourceWriter.h>
#include <TextureResource.h>
#include <interface/IRenderTypeManager.h>

#include "interface_imp/ModelResource.h"
#include "interface_imp/ModelSerializer.h"
#include "interface_imp/StaticModelResource.h"
#include "interface_imp/StaticModelSerializer.h"
#include "interface_imp/ModelElement.h"
#include "interface_imp/ModelConfigManager.h"

#include "interface_imp/SkeletonResource.h"
#include "interface_imp/SkeletonSerializer.h"

#include "interface_imp/ModelBuilder.h"
#include "interface_imp/AnimationBuilder.h"
#include "interface_imp/IKSolver.h"
#include "interface_imp/AnimationSorter.h"

#include <BladeBase_blang.h>
#include <BladeModel_blang.h>

namespace Blade
{
	template class Factory<IModelConfigManager>;

	static const TString PLUGIN_NAME = BTString("BladeModel");
	BLADE_DEF_PLUGIN(ModelPlugin);

	static const TString MODEL_SKELETON = BXLang(BLANG_SKELETON_FLE);
	static const TString MODEL_ANIMATIONLIST = BXLang(BLANG_ANIMATION_LIST);
	static const TString MODEL_SUBMESHLIST = BXLang(BLANG_SUBMESH_LIST);

	//////////////////////////////////////////////////////////////////////////
	ModelPlugin::ModelPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ModelPlugin::~ModelPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back( BTString("GraphicsModel") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlugin::getDependency(TStringParam& dependencyList) const
	{
		bool bDependency = IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) != BTString("tool");
		if( bDependency )
			dependencyList.push_back(BTString("GraphicsService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlugin::install()
	{
		RegisterSingleton(ModelConfigManager, IModelConfigManager);

		NameRegisterFactory(ModelResource, IResource, ModelConsts::MODEL_RESOURCE_TYPE );
		NameRegisterFactory(ModelSerializer, ISerializer, ModelConsts::MODEL_RESOURCE_TYPE );
		NameRegisterFactory(ModelSerializer, ISerializer, ModelConsts::MODEL_SERIALIZER_BINARY);
		NameRegisterFactory(ModelSerializerXML, ISerializer, ModelConsts::MODEL_SERIALIZER_XML);

		NameRegisterFactory(StaticModelResource, IResource, StaticModelResource::STATIC_MODEL_RESOURCE_TYPE);
		NameRegisterFactory(StaticModelSerializer, ISerializer, StaticModelResource::STATIC_MODEL_RESOURCE_TYPE);

		//internal implementation
		NameVersionRegisterFactory(ModelSerializer_Binary, ModelSerializerBase, 
			ModelConsts::MODEL_SERIALIZER_BINARY, IModelResource::MODEL_LATEST_SERIALIZER_VERSION );

		NameRegisterFactory(ModelElement, GraphicsElement, ModelConsts::MODEL_ELEMENT_TYPE);
		IResourceManager::getSingleton().registerFileExtension( ModelConsts::MODEL_RESOURCE_TYPE, ModelConsts::MODEL_FILE_EXT );
		IResourceManager::getSingleton().addSearchPath( ModelConsts::MODEL_RESOURCE_TYPE, BTString("media:/models/"), true);

		NameRegisterFactory(SkeletonResource, IResource, ModelConsts::SKELETON_RESOURCE_TYPE );
		IResourceManager::getSingleton().registerFileExtension( ModelConsts::SKELETON_RESOURCE_TYPE, ModelConsts::SKELETON_FILE_EXT );
		IResourceManager::getSingleton().addSearchPath( ModelConsts::SKELETON_RESOURCE_TYPE, BTString("media:/models/"), true);
		NameRegisterFactory(SkeletonSerializer, ISerializer, ModelConsts::SKELETON_RESOURCE_TYPE);

		NameRegisterFactory(ModelBuilder, IModelBuilder, IModelBuilder::MODEL_BUILDER_CLASS);
		NameRegisterFactory(AnimationBuilder, IAnimationBuilder, IAnimationBuilder::ANIMATION_BUILDER_CLASS);
		RegisterInterface(ModelBuilder, IModelBuilder);
		RegisterInterface(AnimationBuilder, IAnimationBuilder);
		
		NameRegisterFactory(IKSolver, IKSolver, ModelConsts::IK_HAND);
		NameRegisterFactory(IKSolver, IKSolver, ModelConsts::IK_FOOT);
		NameRegisterFactory(FullBodyIKSolver, IKSolver, ModelConsts::IK_FULLBODY);

		NameRegisterFactory(AnimationSorter, IRenderSorter, BTString("Animations"));

		bool bDependency = IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) != BTString("tool");
		if( bDependency )
		{
			IRenderTypeManager::getSingleton().addRenderType(&static_cast<RenderType&>(IModelConfigManager::getSingleton().getStaticModelType()));
			IRenderTypeManager::getSingleton().addRenderType(&static_cast<RenderType&>(IModelConfigManager::getSingleton().getSkinnedModelType()));
			IRenderTypeManager::getSingleton().addRenderType(&static_cast<RenderType&>(IModelConfigManager::getSingleton().getStaticModelType(true)));
			IRenderTypeManager::getSingleton().addRenderType(&static_cast<RenderType&>(IModelConfigManager::getSingleton().getSkinnedModelType(true)));

			//sub mesh config
			{
				DataSourceWriter<ModelElement::SubMeshConfig> writer(BTString("Sub mesh"));
				writer.beginVersion(Version(0, 1, 'A'));
				writer << BTString(BLANG_NAME) << DS_GETTER_SETTER(&ModelElement::SubMeshConfig::getName);
				writer << BTString("Visibility") << DS_GETTER_SETTER(&ModelElement::SubMeshConfig::isVisible, &ModelElement::SubMeshConfig::setVisible);
				writer.endVersion();
			}

			{
				//element config
				DataSourceWriter<ModelElement> writer(ModelConsts::MODEL_ELEMENT_TYPE, DS_SIGNATURE(GraphicsElement));
				//version 0.1a
				writer.beginVersion(Version(0, 1, 'A'));
				{
					//read only info ( no serialization / UI config )
					writer << MODEL_SKELETON << CAF_READ << DS_GETTER_SETTER(&ModelElement::getSkeleton);

					writer << MODEL_SUBMESHLIST << &ModelElement::onSubMeshConfigChanged << DS_GETTER_SETTER(&ModelElement::getSubMeshes);
				}
				writer.endVersion();
			}
		}

		//generic model(building) paging. TODO: more paging strategy for more types, i.e. trees, grass, etc.
		IStageConfig::getSingleton().addPageLayer(ModelConsts::MODEL_PAGE_LAYER, ModelConsts::MODEL_ELEMENT_TYPE, 16, 256.0f);
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlugin::uninstall()
	{
		bool bDependency = IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) != BTString("tool");
		if( bDependency )
		{
			IRenderTypeManager::getSingleton().removeRenderType( &static_cast<const RenderType&>(IModelConfigManager::getSingleton().getStaticModelType()) );
			IRenderTypeManager::getSingleton().removeRenderType( &static_cast<const RenderType&>(IModelConfigManager::getSingleton().getSkinnedModelType()) );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				ModelPlugin::shutdown()
	{

	}
	
}//namespace Blade