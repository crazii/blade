/********************************************************************
	created:	2010/04/02
	filename: 	GraphicsSystemPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GraphicsSystemPlugin.h"
#include <BladeGraphics.h>
#include "interface_imp/GraphicsSystem.h"
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/IEventManager.h>

#include <interface/IRenderTypeManager.h>
#include <interface/IStageConfig.h>
#include <TextureResource.h>

#include <Element/GraphicsElement.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <Element/LightElement.h>
#include <Element/CameraElement.h>
#include <databinding/DataSourceWriter.h>
#include <interface/public/CommonState.h>
#include <BladeBase_blang.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include <BladeFramework_blang.h>

#include "interface_imp/GraphicsSystem.h"
#include "interface_imp/Material/MaterialManager.h"
#include "interface_imp/RenderScheme/RenderSchemeManager.h"

#include "interface_imp/DefaultSpace.h"
#include "interface_imp/QuadtreeSpace/QuadtreeSpace.h"
#include "interface_imp/DefaultSpaceCoordinator.h"
#include "interface_imp/ShaderVariables/ShaderVariableSource.h"

#include "interface_imp/Serializers/TextureResourceSerializer.h"
#include "interface_imp/Serializers/MaterialSerializer.h"
#include "interface_imp/Serializers/ShaderResourceSerializer.h"
#include "interface_imp/Serializers/BinaryShaderSerializer.h"
#include "interface_imp/Serializers/HybridShaderSerializer.h"
#include "interface_imp/Material/DefaultMaterial.h"
#include "interface_imp/RenderHelper/HelperRenderType.h"
#include "interface_imp/GraphicsScene.h"
#include "interface_imp/Effect/EffectManager.h"
#include "interface_imp/RenderTypeManager.h"

//effects
#include "interface_imp/Effect/BoundingEffect.h"
#include "interface_imp/Effect/DecalEffect.h"
#include "interface_imp/Effect/BrushDecalEffect.h"
#include "interface_imp/Effect/EditGizmoEffect.h"
#include "interface_imp/Effect/HUDEffect.h"
#include "interface_imp/RenderSorter.h"
#include "interface_imp/LightType.h"

//shadows
#include "interface_imp/Shadows/ShadowManager.h"
#include "interface_imp/Shadows/DefaultShadowTechnique.h"
#include "interface_imp/Shadows/LiSPSMShadowTechnique.h"
#include "interface_imp/Shadows/TSMShadowTechnique.h"

//exports
#include <interface/public/IImageEffect.h>
#include "interface_imp/RenderScheme/ImageEffectOutput.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

#define DECAL_TYPE_GEOMETRY	 0
#define DECAL_TYPE_VOLUME	 1
#define DECAL_TYPE DECAL_TYPE_GEOMETRY

namespace Blade
{
#if DECAL_TYPE == DECAL_TYPE_GEOMETRY
	const TString DecalEffect::DEFAULT_MATERIAL= BTString("geometry_decal");
	const TString BrushDecalEffect::BRUSH_DECAL_MATERIAL = BTString("geometry_brush_decal");
#else
#	error not implemented yet
#endif	
	const TString BrushDecalEffect::BRUSH_DECAL_SAMPLER = BTString("brushDiffuse");
}//namespace Blade


namespace Blade
{
	//template instantiations
	template class Factory<GraphicsElement>;
	template class Factory<IGraphicsEffect>;
	template class Factory<IGraphicsSystem>;
	template class Factory<IMaterialManager>;
	template class Factory<IRenderScene>;
	template class Factory<IRenderSchemeManager>;
	template class Factory<IRenderTypeManager>;
	template class Factory<IShaderVariableSource>;
	template class Factory<ISpace>;
	template class Factory<ISpaceCoordinator>;
	template class FixedVector<MATERIALLOD,MATERIALLOD::MAX_LOD>;
	template class Factory<IImageEffect>;
	template class Factory<IRenderSorter>;

	static const TString PLUGIN_NAME = BTString("BladeGraphics");
	BLADE_DEF_PLUGIN(GraphicsSystemPlugin);

	static const TString SCENE_SPACE_TYPE = BXLang(BLANG_SPACE_TYPE);
	static const TString SCENE_COORD_TYPE = BXLang(BLANG_COORDINATOR_TYPE);

	static const TString ELEMENT_POSITION = BXLang(BLANG_POSITION);
	static const TString ELEMENT_SCALE = BXLang(BLANG_SCALE);
	static const TString ELEMENT_ROTATION = BXLang(BLANG_ROTATION);

	namespace Impl
	{

		class EmptyContent : public SpaceContent, public Allocatable
		{
		public:
			EmptyContent() { mSpaceFlags.raiseBits(CSF_VIRTUAL); }
			//TODO: visible renderable object
			virtual void			updateRender(IRenderQueue* /*buffer*/) {}
		};

		class EmptyGraphicsElement : public GraphicsElement, public Allocatable
		{
		public:
			EmptyGraphicsElement() :GraphicsElement(GraphicsConsts::EMPTY_ELEMENT_TYPE) {}
			~EmptyGraphicsElement() { BLADE_DELETE mContent; }

			/** @brief  */
			virtual void			onInitialize()
			{
				mContent = BLADE_NEW EmptyContent();
				this->activateContent();
			}
		};

	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	GraphicsSystemPlugin::GraphicsSystemPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsSystemPlugin::~GraphicsSystemPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsSystemPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("GraphicsService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsSystemPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("RenderDevice"));
		dependencyList.push_back(BTString("WindowService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsSystemPlugin::install()
	{
		IEventManager::getSingleton().registerEvent(RenderDeviceReadyEvent::NAME);
		IEventManager::getSingleton().registerEvent(RenderDeviceCloseEvent::NAME);
		IEventManager::getSingleton().registerEvent(FrameBeginEvent::NAME);
		IEventManager::getSingleton().registerEvent(FrameEndEvent::NAME);

		RegisterSingleton(GraphicsSystem,IGraphicsSystem);
		RegisterSingleton(MaterialManager,IMaterialManager);
		RegisterSingleton(RenderSchemeManager,IRenderSchemeManager);

		//NameRegisterFactory(GraphicsSystem,ISubsystem,GraphicsSystemName);
		//register as singleton
		NameRegisterSingleton(GraphicsSystem,ISubsystem,GraphicsSystem::GRAPHICS_SYSTEM_NAME);
		RegisterSingleton(RenderTypeManager,IRenderTypeManager);

		NameRegisterFactory(QuadtreeSpace, ISpace, BXLang(BLANG_QUADTREE_SPACE));	//first is default
		NameRegisterFactory(DefaultSpace, ISpace, BXLang(BLANG_DEFAULT_SPACE) );

		NameRegisterFactory(DefaultSpaceCoordinator,ISpaceCoordinator,BXLang(BLANG_DEFAULT_COORDINATOR) );
		NameRegisterFactory(GraphicsScene, IRenderScene, IRenderScene::GRAPHICS_SCENE_TYPE);
		NameRegisterFactory(GraphicsScene,IScene,IRenderScene::GRAPHICS_SCENE_TYPE);

#if DECAL_TYPE == DECAL_TYPE_GEOMETRY
		NameRegisterFactory(DecalEffect,IGraphicsEffect,IDecalEffect::TYPE);
		NameRegisterFactory(BrushDecalEffect,IGraphicsEffect,IBrushDecalEffect::TYPE);
#else
#error not implemented yet
#endif
		NameRegisterFactory(BoundingEffect, IGraphicsEffect, IBoundingEffect::TYPE);
		NameRegisterFactory(EditGizmoEffect, IGraphicsEffect, IEditGizmoEffect::TYPE);
		NameRegisterFactory(HUDEffect, IGraphicsEffect, IHUDEffect::TYPE);

		NameRegisterFactory(Impl::EmptyGraphicsElement, GraphicsElement, GraphicsConsts::EMPTY_ELEMENT_TYPE );
		NameRegisterFactory(LightElement,GraphicsElement, BTString(BLANG_LIGHT_ELEMENT));
		NameRegisterFactory(CameraElement,GraphicsElement, BTString(BLANG_CAMERA_ELEMENT) );
		NameRegisterFactory(MainCameraElement, GraphicsElement, MainCameraElement::MAIN_CAMERA_ELEMENT);
		NameRegisterFactory(ImageEffect_Blit, IImageEffect, ImageEffect_Blit::BLIT_NAME);

		{
			NameRegisterFactory(TextureResource, IResource, TextureResource::TEXTURE_RESOURCE_TYPE);
			NameRegisterFactory(Texture1DSerializer, ISerializer, TextureResource::TEXTURE_1D_SERIALIZER);
			NameRegisterFactory(Texture2DSerializer, ISerializer, TextureResource::TEXTURE_2D_SERIALIZER);
			NameRegisterFactory(Texture3DSerializer, ISerializer, TextureResource::TEXTURE_3D_SERIALIZER);
			NameRegisterFactory(TextureCubeSerializer, ISerializer, TextureResource::TEXTURE_CUBE_SERIALIZER);
			NameRegisterFactory(Texture2DSerializer, ISerializer, TextureResource::TEXTURE_RESOURCE_TYPE);
		}

		{
			//only support 2 file formats:
			IResourceManager::getSingleton().registerFileExtension( TextureResource::TEXTURE_RESOURCE_TYPE, BTString("dds") );
			IResourceManager::getSingleton().registerFileExtension( TextureResource::TEXTURE_RESOURCE_TYPE, BTString("png") );
			IResourceManager::getSingleton().addSearchPath( TextureResource::TEXTURE_RESOURCE_TYPE, BTString("image:/"), true );

			NameRegisterFactory(DefaultMaterial,IResource, Material::MATERIAL_RESOURCE_TYPE);
			NameRegisterFactory(MaterialSerializer, ISerializer, Material::MATERIAL_RESOURCE_TYPE);
			IResourceManager::getSingleton().registerFileExtension( Material::MATERIAL_RESOURCE_TYPE, BTString("bmt") );
		}

		{
			NameRegisterFactory(ShaderResource,IResource, ShaderResource::SHADER_RESOURCE_TYPE);
			NameRegisterFactory(BinaryShaderSerializer,ISerializer, ShaderResource::BINARY_SHADER_SERIALIZER);
			NameRegisterFactory(ShaderResourceSerializer,ISerializer, ShaderResource::TEXT_SHADER_SERIALIZER);
			NameRegisterFactory(HybridShaderSerializer,ISerializer, ShaderResource::HYBRID_SHADER_SERIALIZER);
			{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
				NameRegisterFactory(BinaryShaderSerializer,ISerializer, ShaderResource::SHADER_RESOURCE_TYPE);
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
				//use hybrid shader as default
				//pre-compile shader into binary at startup, and use binary shader as default
				//NameRegisterFactory(HybridShaderSerializer, ISerializer, ShaderResource::SHADER_RESOURCE_TYPE);
				NameRegisterFactory(BinaryShaderSerializer, ISerializer, ShaderResource::SHADER_RESOURCE_TYPE);
#else
#error
#endif
			}
		}
		RegisterSingleton(ShaderVariableSource,IShaderVariableSource);

		NameRegisterFactory(Front2BackRenderSorter, IRenderSorter, BTString("FrontToBack"));
		NameRegisterFactory(Back2FrontRenderSorter, IRenderSorter, BTString("BackToFront"));
		NameRegisterFactory(GeometrySorter, IRenderSorter, BTString("Geometries"));
		NameRegisterFactory(TextureRenderSorter, IRenderSorter, BTString("Textures"));
		NameRegisterFactory(DefaultShadowTechnique, IShadowTechnique, BTString("Default"));
		NameRegisterFactory(LiSPSMShadowTechnique, IShadowTechnique, BTString("LiSPSM"));
		NameRegisterFactory(TSMShadowTechnique, IShadowTechnique, BTString("TSM"));
		NameRegisterFactory(ShadowManager, IRenderSchemeOperation, ShadowManager::SHADOW_MANAGER_TYPE);

		IRenderTypeManager::getSingleton().addRenderType( HelperRenderType::getSingletonPtr() );
		IRenderTypeManager::getSingleton().addRenderType( &EffectManager::getEffectRenderType() );
		IRenderTypeManager::getSingleton().addRenderType( &LightRenderManager::getSingleton().getLightRenderType() );
		//built in type flags
		GraphicsSystem::getSingleton().generateGraphicsTypeAppFlag(HelperRenderType::getSingleton());
		GraphicsSystem::getSingleton().generateGraphicsTypeAppFlag(LightRenderManager::getSingleton().getLightRenderType());


		{
			DataSourceWriter<GraphicsScene> writer(IRenderScene::GRAPHICS_SCENE_TYPE);
			//version 0.1
			writer.beginVersion(Version(0, 1));
			{
				writer << SCENE_SPACE_TYPE << SpaceFactory::getSingleton() << DS_GETTER_SETTER(&GraphicsScene::getDefaultCoordinatorSpaceType, &GraphicsScene::setDefaultCoordinatorSpaceType);
				writer << SCENE_COORD_TYPE << CoordinatorFactory::getSingleton() << DS_GETTER_SETTER(&GraphicsScene::getCoordinatorType, &GraphicsScene::setCoordinatorType);
			}
			writer.endVersion();
		}

		{
			//element config
			DataSourceWriter<GraphicsElement> writer(GraphicsConsts::EMPTY_ELEMENT_TYPE);
			writer.beginVersion(Version(0, 1));
			{
				writer << CommonState::POSITION << CAF_NONE_CONFIG << POINT3::ZERO << &GraphicsElement::onConfigChange << GraphicsElement::getPositioAddr<GraphicsElement>();
				writer << CommonState::ROTATION << CAF_NONE_CONFIG << Quaternion::IDENTITY << &GraphicsElement::onConfigChange << GraphicsElement::getRotationAddr<GraphicsElement>();
				writer << CommonState::SCALE << CAF_NONE_CONFIG << Vector3::UNIT_ALL << &GraphicsElement::onConfigChange << GraphicsElement::getScaleAddr<GraphicsElement>();
				writer << GraphicsConsts::LOCAL_BOUNDING << CAF_NONE_CONFIG << Box3::EMPTY << GraphicsElement::getBoundingAddr<GraphicsElement>();
				writer << GraphicsConsts::CONFIG_SPACE_MASK << CAF_NON_VISIBLE << (uint32)ISpace::INVALID_PARTITION << DS_GETTER_SETTER(&GraphicsElement::getSpacePartitionMask, &GraphicsElement::setSpacePartitionMask);
				writer << GraphicsConsts::DYNAMIC_TAG << CAF_NORMAL << DS_GETTER_SETTER(&GraphicsElement::getDynamic, &GraphicsElement::setDynamic);
			}
			writer.endVersion();
		}

#if 0
		{
			DataSourceWriter<CameraElement> writer(BTString(BLANG_CAMERA_ELEMENT), GraphicsConsts::EMPTY_ELEMENT_TYPE);
			writer.beginVersion(Version(0, 1));
			writer.endVersion();
		}
		{
			DataSourceWriter<MainCameraElement> writer(MainCameraElement::MAIN_CAMERA_ELEMENT, GraphicsConsts::EMPTY_ELEMENT_TYPE);
			writer.beginVersion(Version(0, 1));
			writer.endVersion();
		}
#endif

		{
			DataSourceWriter<LightElement> writer(LightElement::LIGHT_TYPE, GraphicsConsts::EMPTY_ELEMENT_TYPE);
			writer.beginVersion(Version(0, 1));

			//shared data
			writer << BXLang(BLANG_DIFFUSE) << Color::WHITE << DS_GETTER_SETTER(&LightElement::getLightDiffuse, &LightElement::setLightDiffuse);
			writer << BXLang(BLANG_SPECULAR) << Color::WHITE << DS_GETTER_SETTER(&LightElement::getLightSpecular, &LightElement::setLightSpecular);

			static const TString LIGHT_TYPE_DIRECTIONAL = BXLang("Directional Light");
			static const TString LIGHT_TYPE_POINT = BXLang("Point Light");
			static const TString LIGHT_TYPE_SPOT = BXLang("Spot Light");

			TStringParam typeStrings;
			typeStrings.push_back(LIGHT_TYPE_DIRECTIONAL);
			typeStrings.push_back(LIGHT_TYPE_POINT);
			typeStrings.push_back(LIGHT_TYPE_SPOT);
			DataSourceWriter<LightElement> subTypeWriter = (writer << BXLang(BLANG_TYPE) << typeStrings << DS_USE_INDEX(0) << DS_GROUP<LightElement>(DS_GETTER_SETTER(&LightElement::getLightType, &LightElement::setLightType)));

			subTypeWriter[LIGHT_TYPE_DIRECTIONAL] << BXLang("Direction") << CAF_READ << Vector3::NEGATIVE_UNIT_Z << DS_GETTER_SETTER(&LightElement::getLightDirection);

			subTypeWriter[LIGHT_TYPE_POINT] << BXLang(BLANG_RANGE) << CAF_READWRITE << 0.0f << DS_GETTER_SETTER(&LightElement::getLightRange, &LightElement::setLightRange);
			subTypeWriter[LIGHT_TYPE_POINT] << BXLang(BLANG_ATTENUATION) << ConfigAtom::Range(0.001f, 1.0f, 0.001f, 1.0f)
				<< DS_GETTER_SETTER(&LightElement::getLightAttenuation, &LightElement::setLightAttenuation);

			subTypeWriter[LIGHT_TYPE_SPOT] << BXLang(BLANG_RANGE) << CAF_READWRITE << 0.0f << DS_GETTER_SETTER(&LightElement::getLightRange, &LightElement::setLightRange);
			subTypeWriter[LIGHT_TYPE_SPOT] << BXLang(BLANG_ATTENUATION) << ConfigAtom::Range(0.001f, 1.0f, 0.001f, 1.0f)
				<< DS_GETTER_SETTER(&LightElement::getLightAttenuation, &LightElement::setLightAttenuation);

			//directions should be adjusted by gizmo rotation helper
			//serialization is done through static geometry
			subTypeWriter[LIGHT_TYPE_SPOT] << BXLang("Direction") << CAF_READ << Vector3::NEGATIVE_UNIT_Z << DS_GETTER_SETTER(&LightElement::getLightDirection);
			subTypeWriter[LIGHT_TYPE_SPOT] << BXLang(BLANG_INNER_ANGLE) << ConfigAtom::Range(0.0f, 170.0f, 0.001f, 30.0f)
				<< DS_GETTER_SETTER(&LightElement::getLightInnerAngle, &LightElement::setLightInnerAngle);

			subTypeWriter[LIGHT_TYPE_SPOT] << BXLang(BLANG_OUTER_ANGLE) << CAF_READWRITE << ConfigAtom::Range(0.0f, 170.0f, 0.001f, 90.0f)
				<< DS_GETTER_SETTER(&LightElement::getLightOuterAngle, &LightElement::setLightOuterAngle);
			subTypeWriter.endVersion();

			writer.endVersion();
		}

		{
			static const TString SHADOW_QUALITY = BTString("Shadow Quality");
			static const TString SHADOW_TECH = BTString("Technique");
			static const TString SHADOW_CASCADE = BTString("CSM Cascade Count");
			static const TString SHADOW_AUTO_DIST = BTString("CSM Auto Distance");
			static const TString SHADOW_DISTRATE = BTString("CSM Distance Factor");
			static const TString SHADOW_DIST = BTString("CSM Distance");

			DataSourceWriter<ShadowManager> writer(ShadowManager::SHADOW_MANAGER_TYPE);
			writer.beginVersion(Version(0, 2));

			TStringParam qualities;
			qualities.push_back(BTString(BLANG_LOW));
			qualities.push_back(BTString(BLANG_MIDDLE));
			qualities.push_back(BTString(BLANG_HIGH));
			writer << SHADOW_QUALITY << CUIH_RANGE << qualities << DS_USE_INDEX(uint32(1u)) << &ShadowManager::mQuality;
			writer << SHADOW_TECH << CAF_NORMAL_DEVMODE << Factory<IShadowTechnique>::getSingleton() << &ShadowManager::notifyConfigChanges << &ShadowManager::mTechniqueName;
			writer << SHADOW_CASCADE << CAF_NORMAL_DEVMODE << ConfigAtom::Range(uint32(1), uint32(MAX_SHADOW_CASCADE), uint32(1), uint32(MAX_SHADOW_CASCADE)) << &ShadowManager::notifyConfigChanges << &ShadowManager::mCascadeCount;
			{
				DataSourceWriter<ShadowManager> autoDistWriter = (writer << SHADOW_AUTO_DIST << CAF_NORMAL_DEVMODE << Variant(true) << DS_GROUP<ShadowManager>(&ShadowManager::mAutoDisance));
				autoDistWriter[BTString("TRUE")] << SHADOW_DISTRATE << CAF_NORMAL_DEVMODE << ConfigAtom::Range(0.5f, 1.0f, 0.001f, SHAODW_AUTO_SPLIT_FACTOR) << &ShadowManager::mAutoSplitFactor;
				Variant values[MAX_SHADOW_CASCADE] = { SHADOW_MANUAL_SPLIT_DISTANCE[0], SHADOW_MANUAL_SPLIT_DISTANCE[1], SHADOW_MANUAL_SPLIT_DISTANCE[2], SHADOW_MANUAL_SPLIT_DISTANCE[3] };
				autoDistWriter[BTString("FALSE")] << SHADOW_DIST << CAF_NORMAL_DEVMODE << ConfigAtom::Range(0.0f, SHADOW_MANUAL_SPLIT_DISTANCE[MAX_SHADOW_CASCADE - 1], 0.001f) << DS_ARRAY_VALUES(values) << &ShadowManager::mSplitDistance;
				autoDistWriter.endVersion();
			}

			writer.endVersion();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsSystemPlugin::uninstall()
	{
		IRenderTypeManager::getSingleton().removeRenderType(HelperRenderType::getSingletonPtr());
		IRenderTypeManager::getSingleton().removeRenderType(&EffectManager::getEffectRenderType());
		IRenderTypeManager::getSingleton().removeRenderType(&LightRenderManager::getSingleton().getLightRenderType());

		IEventManager::getSingleton().unRegisterEvent(RenderDeviceReadyEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(RenderDeviceCloseEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(FrameBeginEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(FrameEndEvent::NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsSystemPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				GraphicsSystemPlugin::shutdown()
	{
		ShaderVariableSource::getSingleton().shutdown();
	}
	
}//namespace Blade