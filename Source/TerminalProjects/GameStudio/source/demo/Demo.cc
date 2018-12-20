/********************************************************************
	created:	2015/03/16
	filename: 	Demo.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <GameFSM.h>
#include <math/AxisAlignedBox.h>
#include <interface/public/window/IWindow.h>
#include <interface/IFramework.h>
#include <interface/IEntity.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/ITimeService.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/IPage.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/logic/ILogicService.h>
#include <interface/public/logic/ILogicWorldManager.h>
#include <EntityResourceDesc.h>
#include <Application.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include <App.h>
#include <ClientConfig.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static const TString ANIMDEMO_STATE = BTString("Animation Demo");
	class AnimDemoState : public IGameState, public Allocatable
	{
	public:
		AnimDemoState();
		~AnimDemoState();

		/** @brief get the shown name of the state */
		virtual const TString& getName() const
		{
			return ANIMDEMO_STATE;
		}

		/** @brief  */
		virtual void	initialize();

		/** @brief  */
		virtual void	shutdown();

		/** @brief  */
		virtual void	run();

		/** @brief  */
		virtual void	update();

		/** @brief  */
		virtual void	syncUpdate();

	protected:
		IStage*		mStage;
		IEntity*	mEntity;
		IEntity*	mLightEntity;
	};//class AnimDemoState

	const TString ELEMENT_NAME = BTString("Model");
	//////////////////////////////////////////////////////////////////////////
	AnimDemoState::AnimDemoState()
	{
		mStage = NULL;
		mEntity = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	AnimDemoState::~AnimDemoState()
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	AnimDemoState::initialize()
	{
		//init default world
		ILogicWorld* world = DEF_WORLD_INFO.world;
		IStage* stage = DEF_WORLD_INFO.winfo.stage;
		stage->instantProcess();	//this should be called if stage is not loaded from file

		WORLD_INFO winfo = DEF_WORLD_INFO.winfo;
		winfo.worldResourcePath = BTString("character:");
		DEF_WORLD_INFO.world->initialize(winfo, Vector3(0, 100, 0));

		//load model file & setup view & camera:
		//create entity & graphics model element
		IGraphicsScene* graphicsScene = winfo.graphicsScene;
		IEntity* entity = stage->createEntity(BTString("demoEntity"));

		//create geometry
		IGeometryScene* geomScene = winfo.geomScene;
		IElement* geomElem = geomScene->createElement(GeomConsts::GEOMETRY_ELEMENT);
		entity->addElement(BTString("Geometry"), HELEMENT(geomElem));
		IGeometry* geom = geomElem->getInterface(IID_GEOMETRY());

		HELEMENT element = graphicsScene->createGraphicsElement(BTString(BLANG_MODEL_ELEMENT));
		entity->addElement(ELEMENT_NAME, element);

		//load model file
		EntityResourceDesc desc;
		desc.addElementResource( ELEMENT_NAME, BTString("bloodelf.blm") );
		stage->loadEntitySync(entity, &desc);
		
		const AABB& aabb = geom->getLocalBounding();

		scalar radius = aabb.getHalfSize().getLength();
		Vector3 center = aabb.getCenter();
		//scalar limit = (scalar)200/2;
		//if( limit < radius )
		//{
		//	scalar scale = limit/radius;
		//	geom->setGlobalScale( Vector3::UNIT_ALL*scale );
		//	radius *= scale;
		//}

		CameraActor* camera = world->getWorldCamera();
		scalar fov = camera->getController()->getFOV();
		//TODO: camera aspect ratio feedback
		IWindow* window = stage->getWindow();
		scalar aspect = (scalar)window->getWidth()/(scalar)window->getHeight();
		//if( aspect < 1.0f )
		//	aspect = 1.0f/aspect;
		scalar distance = radius*aspect/::tan(fov/2);

		Vector3 lookdir = Vector3(-1, 0, -0.5).getNormalizedVector();
		camera->getGeometry()->setDirection(lookdir);
		Vector3 cameraPos = center - lookdir*distance;
		camera->getGeometry()->setGlobalPosition(cameraPos);
		if( distance+radius > camera->getController()->getFarDistance() )
			camera->getController()->setFarDistance(distance+radius);

		//mWorld->getWorldView()->setBackgroundColor( Color::BLUE );
		
		mEntity = entity;

		HELEMENT hLight = graphicsScene->createGraphicsElement(BTString(BLANG_LIGHT_ELEMENT));
		mLightEntity = stage->createEntity(BTString("dirLight"));
		mLightEntity->addElement(BTString("light"), hLight);
		stage->loadEntity(mLightEntity);
		mStage = stage;
	}

	//////////////////////////////////////////////////////////////////////////
	void	AnimDemoState::shutdown()
	{
		DEF_WORLD_INFO.world->deinitialize();
		mStage->destroyEntity( mEntity );
		mStage->destroyEntity(mLightEntity);

		mEntity = NULL;
		mLightEntity = NULL;
		mStage = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	AnimDemoState::run()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void	AnimDemoState::update()
	{
		IGeometryService::getSingleton().updateScenes();
	}

	//////////////////////////////////////////////////////////////////////////
	void	AnimDemoState::syncUpdate()
	{

	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static const TString SCENEDEMO_STATE = BTString("Scene Demo");
	class SceneDemoState : public IGameState, public Allocatable
	{
	public:
		SceneDemoState();
		~SceneDemoState();

		/** @brief get the shown name of the state */
		virtual const TString& getName() const
		{
			return SCENEDEMO_STATE;
		}

		/** @brief  */
		virtual void	initialize();

		/** @brief  */
		virtual void	shutdown();

		/** @brief  */
		virtual void	run();

		/** @brief  */
		virtual void	update();

		/** @brief  */
		virtual void	syncUpdate();

		CameraActor* mCamera;
		ILogicWorld* mWorld;
	};//class SceneDemoState


	//////////////////////////////////////////////////////////////////////////
	SceneDemoState::SceneDemoState()
	{
		mCamera = NULL;
		mWorld = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	SceneDemoState::~SceneDemoState()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void	SceneDemoState::initialize()
	{
		ILogicWorld* world = DEF_WORLD_INFO.world;
		//IStage* stage = DEF_WORLD_INFO.winfo.stage;

		//load scene
		TString worldFolder = BTString("media:/worlds/sponza");
		IResourceManager::getSingleton().registerScheme(BTString("sponza"), worldFolder, false);

		DEF_WORLD_INFO.winfo.worldResourcePath = BTString("sponza:");
		world->initialize(DEF_WORLD_INFO.winfo, BTString("sponza:/sponza.bstage"), NULL);
		mCamera = world->getWorldCamera();
		mWorld = world;
	}

	//////////////////////////////////////////////////////////////////////////
	void	SceneDemoState::shutdown()
	{
		mWorld->deinitialize();
		IResourceManager::getSingleton().unRegisterScheme(BTString("sponza"));
		mCamera = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	SceneDemoState::run()
	{
		scalar time = ITimeService::getSingleton().getTimeSource().getTimeThisLoop();
		if( time < 1.0f )
			mCamera->getGeometryNode()->yaw(0.125f*time, GS_WORLD);
	}

	//////////////////////////////////////////////////////////////////////////
	void	SceneDemoState::update()
	{
		IGeometryService::getSingleton().updateScenes();
	}

	//////////////////////////////////////////////////////////////////////////
	void	SceneDemoState::syncUpdate()
	{
		mWorld->update(*mCamera);
	}

	//////////////////////////////////////////////////////////////////////////
	void initDemoStates()
	{
		//set demo state as init state
		NameRegisterFactory(AnimDemoState, IGameState, IGameState::INIT_STATE);
	}
	
	
}//namespace Blade