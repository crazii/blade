/********************************************************************
	created:	2010/05/08
	filename: 	GameWorld.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/window/IWindow.h>

#include <interface/IStage.h>
#include <interface/IPage.h>
#include <interface/IEntity.h>
#include <interface/IResourceManager.h>
#include <interface/ITaskManager.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>
#include <interface/public/graphics/IGraphicsEffectManager.h>
#include <interface/public/graphics/IGraphicsScene.h>
#include <interface/public/graphics/IGraphicsSpaceCoordinator.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/public/geometry/IGeometryService.h>
#include <interface/public/geometry/IGeometryScene.h>
#include <interface/public/logic/ILogicService.h>

#include <interface/public/logic/ICameraController.h>
#include "GameWorld.h"
#include "CameraController.h"

//this pointer as param in member class construction
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma  warning(disable:4355)	//'this' : used in base member initializer list
#endif

namespace Blade
{
	static const TString WORLD_VIEW_NAME = BTString("WORLD_VIEW");

	static const TString CameraElementName = BTString("camera");
	static const TString CameraControllerName = BTString("controller");
	static const TString CameraGeometryName = BTString("ceometry");

	//////////////////////////////////////////////////////////////////////////
	GameWorld::GameWorld()
		:mInfo(NULL)
		,mWorldData(NULL)
		,mInitialized(false)
	{
		mInfo.geomScene = NULL;
		mInfo.graphicsScene = NULL;
		mInfo.logicScene = NULL;
		mInfo.stage = NULL;
		mInfo.camera = NULL;
		mInfo.view = NULL;
	}

	/////////////////////////////////////////////////////////////////////////////
	GameWorld::~GameWorld()
	{
		this->deinitialize();
	}

	/************************************************************************/
	/* IGameworld interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			GameWorld::setupWorldName(const TString& name)
	{
		if( mName == TString::EMPTY )
		{
			mName = name;
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameWorld::initialize(const WORLD_INFO& info, const Vector3& startPos)
	{
		if( mInitialized )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("world already initialized.") );

		mInitialized = true;
		mInfo = info;
		assert(mInfo.stage != NULL);
		assert(mInfo.graphicsScene != NULL);
		assert(mInfo.geomScene != NULL);

		mInfo.logicScene->setLogicWorld(this);

		//create view
		IGraphicsView* view = this->setupWorldView();

		//init graphics scene
		mInfo.graphicsScene->initialize(view);

		IGraphicsCamera* camera = mInfo.graphicsScene->getMainCamera();
		IEntity* entity = camera->getEntity();
		assert(entity != NULL);
		{
			entity->addElement(CameraGeometryName, HELEMENT(this->getWorldGeometryScene()->createElement(GeomConsts::GEOMETRY_ELEMENT)));
			entity->addElement(CameraControllerName, HELEMENT(this->getLogicScene()->createElement(CameraController::CAMERA_CONTROLLER_TYPE)));
			IGeometry* geom = entity->getInterface(IID_GEOMETRY());
			CameraController* camcElem = static_cast<CameraController*>(entity->getElement(CameraControllerName));
			ICameraController* camc = static_cast<ICameraController*>(camcElem);
			assert(geom != NULL && camc != NULL);
			mInfo.camera = BLADE_NEW CameraActor(entity, geom, camc);
			std::pair<ActorSet::iterator, bool> ret = mActors.insert(mInfo.camera);
			assert(ret.second); BLADE_UNREFERENCED(ret);
		}
		mInfo.camera->getGeometry()->setGlobalPosition(startPos);

		mWorldData = BLADE_NEW GameWorldData();
		mWorldData->setStartPosition(startPos);
		mWorldData->setCamera(mInfo.camera);
		mWorldData->setGraphicsScene(mInfo.graphicsScene);
		mInfo.stage->getSerializableMap().addSerializable(GameWorldData::GAME_WORLD_DATA_TYPE, mWorldData);
		mInfo.stage->getPagingManager()->loadPages(mInfo.worldResourcePath, mInfo.camera->getGeometry()->getGlobalPosition(), ProgressNotifier::EMPTY);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameWorld::initialize(const WORLD_INFO& info, const TString& worldStageFile, IProgressCallback* callback/* = NULL*/)
	{
		if( mInitialized )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("world already initialized.") );
		if( info.stage == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("empty pointer.") );

		mInitialized = true;

		//set world data in advance. those info may be used during stage loading,
		//(i.e. currently TerrainPageSerializer use those info on stage loading)
		mInfo = info;
		assert(mInfo.stage != NULL);
		assert(mInfo.graphicsScene != NULL);
		assert(mInfo.geomScene != NULL);

		mInfo.logicScene->setLogicWorld(this);

		IStage* stage = info.stage;
		mWorldData = BLADE_NEW GameWorldData();
		mWorldData->setGraphicsScene(mInfo.graphicsScene);
		stage->getSerializableMap().addSerializable(GameWorldData::GAME_WORLD_DATA_TYPE, mWorldData);

		ProgressNotifier notifier(callback);
		notifier.beginStep(0.2f);
		//load stage
		if( !stage->loadStageFile(worldStageFile, notifier) )
			BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("untable to load world stage file.") );

		//create view
		this->setupWorldView();

		//create/load camera
		IGraphicsCamera* camera = mInfo.graphicsScene->getMainCamera();
		IEntity* entity = camera->getEntity();
		assert(entity != NULL);
		{
			IGeometry* geom = entity->getInterface(IID_GEOMETRY());
			CameraController* camcElem = static_cast<CameraController*>(entity->getElement(CameraControllerName));
			ICameraController* camc = static_cast<ICameraController*>(camcElem);
			assert(geom != NULL && camc != NULL);
			mInfo.camera = BLADE_NEW CameraActor(entity, geom, camc);
			std::pair<ActorSet::iterator, bool> ret = mActors.insert(mInfo.camera);
			assert(ret.second); BLADE_UNREFERENCED(ret);
		}
		mWorldData->setCamera(mInfo.camera);

		//load pages
		notifier.beginStep(0.6f);
		stage->getPagingManager()->loadPages(mInfo.worldResourcePath, mInfo.camera->getGeometry()->getGlobalPosition(), notifier);

		//init graphics scene
		notifier.beginStep(0.2f);
		mInfo.graphicsScene->initialize(mInfo.view);
		notifier.onNotify(1.0f);

		//just in case of precision issue
		notifier.onNotify(1.0f);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GameWorld::deinitialize()
	{
		if( mInitialized )
		{
			mInitialized = false;
			this->getGraphicsScene()->getSpaceCoordinator()->setPositionReference(NULL);
			//mInfo.stage->removeConfigurable(&mWorldData);

			//destroy all cameras
			for(ActorSet::iterator i = mActors.begin(); i != mActors.end(); ++i )
			{
				this->getWorldStage()->destroyEntity( (*i)->getEntity() );
				BLADE_DELETE (*i);
			}
			mActors.clear();
			IGraphicsService::getSingleton().destroyView(mInfo.view);

			BLADE_DELETE mWorldData;
			mWorldData = NULL;
			mInfo.geomScene = NULL;
			mInfo.graphicsScene = NULL;
			mInfo.stage = NULL;
			mInfo.camera = NULL;
			mInfo.view = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	LogicActor*		GameWorld::createActor(IEntity* entity, IGeometry* geom/* = NULL*/)
	{
		if( entity == NULL )
			return NULL;	
		if (geom == NULL)
			geom = entity->getInterface(IID_GEOMETRY());
		if (geom == NULL)
			return NULL;

		LogicActor* actor = BLADE_NEW LogicActor(entity, geom);
		std::pair<ActorSet::iterator, bool> ret = mActors.insert(actor);
		if (!ret.second)
		{
			BLADE_DELETE actor;
			return NULL;
		}
		return actor;
	}

	//////////////////////////////////////////////////////////////////////////
	CameraActor*		GameWorld::createCamera(const TString& name)
	{
		IEntity* entity = this->getWorldStage()->createEntity(name);

		HELEMENT hCamera = this->getGraphicsScene()->createGraphicsElement(BTString(BLANG_CAMERA_ELEMENT));

		entity->addElement(CameraElementName, hCamera);

		HELEMENT hCameraCtrlr(this->getWorldGameScene()->createElement(BTString("CameraController")));
		entity->addElement(CameraControllerName, hCameraCtrlr);

		IElement* geomElem = this->getWorldGeometryScene()->createElement(GeomConsts::GEOMETRY_ELEMENT);
		entity->addElement(CameraGeometryName, HELEMENT(geomElem));

		IGeometry* geom = geomElem->getInterface(IID_GEOMETRY());
		//default: use fixed yaw
		geom->setFixedYawAxis(true);

		CameraController* camcElem = static_cast<CameraController*>(hCameraCtrlr);
		ICameraController* icamc = static_cast<ICameraController*>(camcElem);
		CameraActor* actor = BLADE_NEW CameraActor(entity, geom, icamc);
		std::pair<ActorSet::iterator, bool> ret = mActors.insert(actor);
		if (!ret.second)
		{
			assert(false);
			BLADE_DELETE actor;
			actor = NULL;
		}

		this->getWorldStage()->loadEntity(entity);
		return actor;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GameWorld::destroyActor(LogicActor& actor)
	{
		IEntity* entity = actor.getEntity();
		assert(entity != NULL);
		ActorSet::iterator i = mActors.find(&actor);

		if( i != mActors.end() )
		{
			this->getWorldStage()->destroyEntity(entity);
			BLADE_DELETE &actor;
			mActors.erase(i);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	LogicActor*		GameWorld::getActor(IEntity* entity) const
	{
		if( entity == NULL )
			return NULL;

		LogicActor finder(entity, NULL);
		ActorSet::const_iterator i = mActors.find(&finder);
		if( i != mActors.end() )
			return *i;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GameWorld::update(const CameraActor& camera)
	{
		assert( mInitialized );
		if( !mInitialized )
			return false;

		if( mWorldData->getCamera() != &camera)
			mWorldData->setCamera(&camera);

		if (this->getWorldStage() != NULL && this->getWorldCamera() != NULL)
			this->getWorldStage()->getPagingManager()->update(this->getWorldCamera()->getGeometry()->getGlobalPosition());

		return true;
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IGraphicsView*			GameWorld::setupWorldView()
	{
		mInfo.view = IGraphicsService::getSingleton().createView( this->getWorldName() + WORLD_VIEW_NAME );
		return mInfo.view;
	}

}//namespace Blade