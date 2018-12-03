/********************************************************************
	created:	2011/05/12
	filename: 	FrameworkExports.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/CommonState.h>
#include <interface/public/IStartupOutput.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/public/graphics/IGraphicsType.h>
#include <interface/public/ui/IIconManager.h>
#include <interface/public/ui/IUIMainWindow.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <interface/public/ui/IUIWidget.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/IConfig.h>
#include <interface/public/IScene.h>
#include <interface/public/ITask.h>
#include <interface/IEntity.h>
#include <ConstDef.h>
#include <databinding/DataSourceWriter.h>

#include <BladeFramework_blang.h>

#ifdef PAGE_SIZE	//*nix OS macro
#undef PAGE_SIZE
#endif

namespace Blade
{
	//task state enumeration
	size_t BLADE_TASK_STATE = TS_MAIN_SYNC;
	const volatile size_t* GLOBAL_TASK_STATE = &BLADE_TASK_STATE;

	namespace CommonState
	{
		const TString POSITION = BTString(BLANG_POSITION);
		const TString ROTATION = BTString(BLANG_ROTATION);
		const TString SCALE = BTString(BLANG_SCALE);
		const TString BOUNDING = BTString(BLANG_BOUNDING);
	}//namespace CommonState

	//entity resource string
	const TString IEntity::ENTITY_RESOURCE_TYPE = BTString("Entity Resource");
	const TString IEntity::ENTITY_RESOURCE_EXTENSION = BTString("entity");

	/************************************************************************/
	/* UI events                                                                     */
	/************************************************************************/
	const TString MediaViewEvent::NAME = BTString("UI::MediaView");
	const TString EntitySlectedEvent::NAME = BTString("UI::EntitySelected");
	const TString ConfigurableSlectedEvent::NAME = BTString("UI::ConfigurableSelected");
	const TString OptionHighLightEvent::NAME = BTString("UI::OptionHighLighted");
	const TString ViewportCameraConfigEvent::NAME = BTString("UI::ViewportCameraConfigChanged");

	const TString IUILayoutManager::WORKING_AREA = BTString("Main");
	const TString IViewportWindow::VIEWPORT_WINDOW_TYPE = BTString("Viewport");
	const TString ITableWindow::TABLE_WINDOW_TYPE = BTString("Table");
}

#include <interface/public/window/IWindowService.h>
#include <interface/public/ui/IUIService.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/logic/ILogicService.h>
#include "interface_imp/TimeSourceManager.h"
#include "interface_imp/EntityResource.h"
#include "interface_imp/EntitySerializer.h"
#include "StageResource.h"
#include "StageSerializer.h"
#include "resource/ResourceManager.h"
#include "resource/ZipFileDevice.h"
#include "resource/BPKFileDevice.h"
#include "interface_imp/TaskManager.h"
#include "interface_imp/TBBTaskWorker.h"
#include "interface_imp/DefaultTaskWorker.h"
#include "interface_imp/EventManager.h"
#include "interface_imp/Framework.h"
#include "interface_imp/EnvironmentManager.h"
#include "interface_imp/logic/LogicWorldManager.h"
#include "interface_imp/StageConfig.h"
#include "parallel/ParaStateDef.cc"
#include "interface_imp/ConfigManager.h"
#include "databinding/DataSourceDataBase.h"
#include "interface_imp/HotkeyManager.h"
#include "interface_imp/MediaLibrary.h"
#include "interface_imp/MediaPreviewers.h"
#include "ui/ViewportManager.h"

namespace Blade
{
	template class Factory<IScene>;
	template class Factory<ISerializable>;
	template class Factory<IConfigManager>;
	template class Factory<IEnvironmentManager>;
	template class Factory<IResource>;
	template class Factory<IResourceManager>;
	template class Factory<ISerializer>;
	template class Factory<IWindowService>;
	template class Factory<IUIService>;
	template class Factory<IGeometryService>;
	template class Factory<IGraphicsService>;
	template class Factory<ILogicService>;
	template class Factory<ITimeService>;
	template class Factory<IStageConfig>;
	template class Factory<IEventManager>;
	template class Factory<ISubsystem>;
	template class Factory<ITaskManager>;
	template class Factory<IHotkeyManager>;

	template class Factory<IUIWindow>;
	template class Factory<IUIMainWindow>;
	template class Factory<IUIWidget>;
	template class Factory<IIconManager>;
	template class Factory<IMenuManager>;

	template class ParaStateT<bool>;
	template class ParaStateT<int>;
	template class ParaStateT<scalar>;
	template class ParaStateT<uint>;
	template class ParaStateT<TString>;

	template class ParaStateT<Vector3>;
	template class ParaStateT<Vector4>;
	template class ParaStateT<Color>;
	template class ParaStateT<Quaternion>;
	template class ParaStateT<AABB>;

	template class Factory<ITaskWorker>;
	template class Factory<IStartupOutput>;

	/************************************************************************/
	/* initialization, especially for static library build(further support)                                                                     */
	/************************************************************************/
	bool initializeFramework()
	{
		//singletons
		RegisterSingleton(ConfigManager,IConfigManager);
		RegisterSingleton(TimeSourceManager, ITimeService);

		//entity resource
		NameRegisterFactory(EntityResource, IResource, IEntity::ENTITY_RESOURCE_TYPE);

		//entity serializer
		NameRegisterFactory(EntitySerializer,ISerializer, IEntity::ENTITY_RESOURCE_TYPE );
		NameRegisterFactory(EntitySerializerBinary,ISerializer, EntityResource::ENTITY_SERIALIZER_TYPE_BINARY);
		NameRegisterFactory(EntitySerializerXML,ISerializer, EntityResource::ENTITY_SERIALIZER_TYPE_XML);

		NameRegisterFactory(EntitySerializerImpl_Binary, IEntitySerializerImpl, EntityResource::ENTITY_SERIALIZER_TYPE_BINARY);
		NameRegisterFactory(EntitySerializerImpl_XML, IEntitySerializerImpl, EntityResource::ENTITY_SERIALIZER_TYPE_XML);

		//stage resource
		NameRegisterFactory(StageResource,IResource, STAGE_RESOURCE_TYPE);

		//stage serializer
		NameRegisterFactory(StageSerializer,ISerializer,STAGE_RESOURCE_TYPE);
		NameRegisterFactory(StageSerializerBinary,ISerializer, STAGE_SERIALIZER_TYPE_BINARY);
		NameRegisterFactory(StageSerializerXML,ISerializer, STAGE_SERIALIZER_TYPE_XML);
		NameRegisterFactory(StageSerializerImpl_Binary, IStageSerializerImpl, STAGE_SERIALIZER_TYPE_BINARY);
		NameRegisterFactory(StageSerializerImpl_XML, IStageSerializerImpl, STAGE_SERIALIZER_TYPE_XML);

		NameRegisterFactory(PageSerializer, ISerializer, PAGE_SERIALIZER_TYPE);
		NameRegisterFactory(PageSerializerBinary, ISerializer, PAGE_SERIALIZER_TYPE_BINARY);
		NameRegisterFactory(PageSerializerXML, ISerializer, PAGE_SERIALIZER_TYPE_XML);

		RegisterSingleton(ViewportManager, IViewportManager);
#if BLADE_ENABLE_ZIPFILE
		NameRegisterFactory(ZipFileDevice, IFileDevice, ZipFileDevice::ZIP_FILE_TYPE);
#endif
		NameRegisterFactory(BPKFileDevice, IFileDevice, BPKFileDevice::BPK_FILE_TYPE);
		
		//singletons
		RegisterSingleton(TaskManager,ITaskManager);
#if BLADE_TASK_USE_TBB 
		NameRegisterSingleton(TBBTaskWorker,ITaskWorker, BTString("TBB") );
#endif
		NameRegisterSingleton(DefaultTaskWorker, ITaskWorker, BTString("Default"));
		RegisterSingleton(EventManager,IEventManager);
		RegisterSingleton(Framework,IFramework);
		RegisterSingleton(ResourceManager,IResourceManager);
		RegisterSingleton(EnvironmentManager,IEnvironmentManager);
		RegisterSingleton(LogicWorldManager,ILogicWorldManager);
		RegisterSingleton(StageConfig,IStageConfig);
		RegisterSingleton(DataSourceDataBase,IDataSourceDataBase);
		RegisterSingleton(HotkeyManager, IHotkeyManager);

		RegisterSingleton(MediaLibrary, IMediaLibrary);
		NameRegisterFactory(ImagePreviewer, IMediaLibrary::IMediaPreviewer, BTString("ImagePreivewer"));

		//init tbb task worker before task manager so that on exception task manager will destruct before TBB task worker
#if BLADE_TASK_USE_TBB 
		SingletonInitializer<TBBTaskWorker> TBBInitHelper;
#endif
		SingletonInitializer<TaskManager> TaskManagerInit;
		SingletonInitializer<ResourceManager> ResourceManagerInit;	//resource manager's destruction perform an task clean up which depend on task manager 


		IResourceManager::getSingleton().registerFileExtension(STAGE_RESOURCE_TYPE, BTString("bstage"));
		IResourceManager::getSingleton().registerFileExtension(STAGE_RESOURCE_TYPE, BTString("bpage"));

		IResourceManager::getSingleton().registerFileExtension( IEntity::ENTITY_RESOURCE_TYPE, IEntity::ENTITY_RESOURCE_EXTENSION );
#if BLADE_ENABLE_ZIPFILE
		IResourceManager::getSingleton().registerDeviceExtension( ZipFileDevice::ZIP_FILE_TYPE, ZipFileDevice::ZIP_FILE_EXT );
#endif
		IResourceManager::getSingleton().registerDeviceExtension( BPKFileDevice::BPK_FILE_TYPE, BPKFileDevice::BPK_FILE_EXT );
		IResourceManager::getSingleton().registerDeviceExtension( BPKFileDevice::BPK_FILE_TYPE, BTString("obb") );//android obb

		IResourceManager::getSingleton().registerDeviceExtension( BTString("MemoryFileDevice"), BTString("mem") );

		{
			DataSourceWriter<Stage> writer(STAGE_BINDING_CONFIG);
			const TString STAGE_TITLE = BTString("title");
			const TString PAGE_SIZE = BXLang(BLANG_PAGE_SIZE);
			const TString PAGE_COUNT = BXLang(BLANG_PAGE_COUNT);
			const TString PAGE_VISIBLE_COUNT = BXLang(BLANG_VISIBLE_PAGE_COUNT);

			//version 0.1
			writer.beginVersion(Version(0, 1));
			{
				writer << STAGE_TITLE << Variant(TString::EMPTY) << CAF_NON_VISIBLE << DS_GETTER_SETTER(&Stage::getTitle, &Stage::setTitle);
				writer << PAGE_SIZE << ConfigAtom::Range(128u, 1024u, 128u, 512u) << DS_GETTER_SETTER(&Stage::getPageSize, &Stage::setPageSize);
				writer << PAGE_COUNT << ConfigAtom::Range(1u, (uint32)IPage::MAX_PAGE_COUNT, 1u, uint32(1)) << DS_GETTER_SETTER(&Stage::getPageCount, &Stage::setPageCount);
				//odd step or terrain complains for now
				writer << PAGE_VISIBLE_COUNT << ConfigAtom::Range(1u, (uint32)IPage::MAX_VISIBLE_PAGE, 2u, uint32(1)) << DS_GETTER_SETTER(&Stage::getVisbiblePageCount, &Stage::setVisbiblePageCount);
			}
			writer.endVersion();
		}


		{
			DataSourceWriter<Entity> writer(ENTITY_BNDING_CONFIG);

			//version 0.1
			writer.beginVersion(Version(0, 1));
			{
				writer << ENTITY_NAME << Variant(TString::EMPTY) << CAF_NONE_CONFIG << DS_GETTER_SETTER(&Entity::getName);
				writer << ENTITY_PAGES << Variant(uint16(0)) << CAF_NON_VISIBLE << DS_GETTER_SETTER(&Entity::getMaxPages, &Entity::setMaxPages);
			}
			writer.endVersion();
		}

		return true;
	}
	
}//namespace Blade
