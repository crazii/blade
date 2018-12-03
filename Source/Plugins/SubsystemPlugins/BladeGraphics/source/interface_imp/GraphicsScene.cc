/********************************************************************
	created:	2010/04/09
	filename: 	GraphicsScene.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IRenderWindow.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderView.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/GraphicsInterface_blang.h>

#include <interface/IStage.h>
#include <interface/ITaskManager.h>
#include <interface/IGraphicsSystem.h>
#include <interface/IRenderSchemeManager.h>

#include <BladeGraphics.h>
#include <BladeGraphics_blang.h>
#include <Element/GraphicsElement.h>
#include <SpaceContent.h>

#include "GraphicsScene.h"
#include "GraphicsTask.h"
#include "GraphicsTaskController.h"
#include "GraphicsUpdater.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "Element/CameraElement.h"
#include "MaterialLODUpdater.h"
#include "View.h"
#include "Effect/EffectManager.h"

namespace Blade
{
	const TString IRenderScene::GRAPHICS_SCENE_TYPE = BXLang(BLANG_GRAPHICS_SCENE);

	//////////////////////////////////////////////////////////////////////////
	GraphicsScene::GraphicsScene()
		:mDevice(NULL)
		,mStage(NULL)
		,mCoordinator(NULL)
		,mAABBRenderer(NULL)
		,mEffectMananger(NULL)
		,mLODUpdater(NULL)
		,mInited(false)
	{
		mTaskController = BLADE_NEW GraphicsTaskController(this);
		mDefaultRenderWindow = NULL;
		mLightManager = BLADE_NEW LightManager();
		mMainCamera = NULL;
		mCameraManager = BLADE_NEW CameraManager();
		mEffectMananger = BLADE_NEW EffectManager(this);
		mAABBRenderer = BLADE_NEW AABBRenderer();
		mUpdater = BLADE_NEW GraphicsUpdater();
		mLODUpdater = BLADE_NEW MaterialLODUpdater(NULL);

		mCoordinatorDesc.type = BXLang(BLANG_DEFAULT_COORDINATOR);
		mCoordinatorDesc.defaultSpace = BXLang(BLANG_DEFAULT_SPACE);
		mCoordinatorDesc.pageSize = 512;
		mCoordinatorDesc.visiblePage = 1;
	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsScene::~GraphicsScene()
	{
		if( mStage != NULL )
			mStage->removeScene(this);

		//explicit destroy it,or listener will not work
		mTask.clear();

		BLADE_DELETE mAABBRenderer;	//AABB content destruct before spaces(coordinator)
		BLADE_DELETE mLightManager;
		BLADE_DELETE mTaskController;
		BLADE_DELETE mCameraManager;
		BLADE_DELETE mLODUpdater;
		BLADE_DELETE mEffectMananger;
		BLADE_DELETE mUpdater;
		BLADE_DELETE mCoordinator;
	}

	/************************************************************************/
	/* ISerializable interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GraphicsScene::postProcess(const ProgressNotifier& /*callback*/)
	{
		this->setupSpace();
	}

	/************************************************************************/
	/* IScene class interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	GraphicsScene::getSceneType() const
	{
		//BLADE_TS_VERIFY( TS_MAIN_SYNC );

		return GRAPHICS_SCENE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsScene::getAvailableElementTypes(ElementTypeList& elemList) const
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		size_t count = GraphicsElementFactory::getSingleton().getNumRegisteredClasses();
		for( size_t i = 0; i < count; ++i)
		{
			const TString& name = GraphicsElementFactory::getSingleton().getRegisteredClass(i);
			elemList.push_back( name );
		}
	}


	//////////////////////////////////////////////////////////////////////////
	IElement*		GraphicsScene::createElement(const TString& type)
	{
		HELEMENT hElem = this->createGraphicsElement(type, TString::EMPTY);
		return hElem.unbind();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsScene::onAttachToStage(IStage* pstage)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );
		mStage = pstage;

		if( mStage == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("cannot be attached to NULL stage.") );

		IRenderWindow* window = mDevice->getRenderWindow( pstage->getWindow()->getUniqueName() );
		assert( window == NULL );

		if( window == NULL )
		{
			window = IGraphicsSystem::getSingleton().createRenderWindow( pstage->getWindow() );
			//register window handler
			pstage->getWindow()->addEventHandler( window );
		}

		GraphicsTask* graphicsTask;
		if( mTask == NULL )
		{
			ScopedLock(mTask.getLock());
			if( mTask == NULL )
			{
				graphicsTask = BLADE_NEW GraphicsTask(mStage->getName()+TEXT("'s GraphicsTask"), mDevice, window);
				mTask.bind(graphicsTask);
			}
			else
				graphicsTask = static_cast<GraphicsTask*>(mTask);
		}
		else
		{
			graphicsTask = static_cast<GraphicsTask*>(mTask);
			graphicsTask->setRenderTarget( window );
		}
		assert(mDefaultRenderWindow == NULL);
		mDefaultRenderWindow = window;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsScene::onDetachFromStage(IStage* pstage)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );
		if( mTask != NULL && static_cast<GraphicsTask*>(mTask)->isInitialized() )
			ITaskManager::getSingleton().removeTask(mTask);

		//unregister window handler for render window
		IWindow* window = pstage->getWindow();
		if( window != NULL )
		{
			const TString& name = window->getUniqueName();
			IRenderWindow* renderWindow = mDevice->getRenderWindow( name );
			window->removeEventHandler(renderWindow);
		}
		assert( mDefaultRenderWindow != NULL );
		if(mDefaultRenderWindow != NULL )
			mDevice->destroyRenderTarget( mDefaultRenderWindow->getName() );
		mStage = NULL;
	}


	/************************************************************************/
	/* IGraphicsScene interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::initialize(IGraphicsView* mainView, const TString& coordinator/* = TString::EMPTY*/, const TString& defSpace/* = TString::EMPTY*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );
		assert( mainView != NULL);
		if( mInited )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("repeat action to initialize."));
		mInited = true;

		this->setupSpace(coordinator, defSpace);

		if( mainView == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("view cannot be empty."));
		View* view = static_cast<View*>(mainView);
		//notify scheme loss
		if( view->getAttachedRenderScheme() != NULL )
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("view is already used in another scene is not supported.") );

		const TString& schemeName = mainView->getRenderScheme();
		IRenderScheme* scheme = IRenderSchemeManager::getSingleton().createRenderScheme(schemeName);
		if( scheme == NULL )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("unable to create render scheme for view.") );
		scheme->setProfile( IGraphicsSystem::getSingleton().getCurrentProfile() );
		view->attachRenderScheme(scheme, true);

		//create main camera
		if (mMainCamera == NULL)
		{
			IEntity* entity = mStage->createEntity(BTString("MAIN_CAMERA"));
			HELEMENT hCam = this->createGraphicsElement(MainCameraElement::MAIN_CAMERA_ELEMENT, TString::EMPTY);
			entity->addElement(BTString("camera"), hCam);
			mMainCamera = static_cast<ICamera*>(entity->getInterface(IID_CAMERA()));
			mStage->loadEntity(entity);
		}

		//camera pointers set by app/other systems must be a GraphicsCamera wrapper
		ICamera* cam = mMainCamera;

		mAABBRenderer->initialize(this, mUpdater);
		mUpdater->addForUpdate( static_cast<EffectManager*>(mEffectMananger) );
		mLODUpdater->setCamera(cam);
		mUpdater->addForIntervalUpate(mLODUpdater, mLODUpdater->getUpdateInterval());
		
		if( mTask != NULL )
		{
			GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
			GRAPHICS_TASK_DESC desc;

			desc.mQueue = mStateQueue;
			desc.mUpdater = mUpdater;
			desc.mRenderView = view;
			desc.mRenderCam = cam;
			desc.mLightManager = mLightManager;
			desc.mCameraManager = mCameraManager;
			desc.mCoordinator = mCoordinator;
			desc.mTaskController = mTaskController;
		
			task->initialize( desc );
			ITaskManager::getSingleton().addTask(task);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsView*	GraphicsScene::getMainView() const
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mTask == NULL )
			return NULL;

		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		return static_cast<IGraphicsView*>( task->getRenderView() );
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*	GraphicsScene::getDefaultRenderWindow() const
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		return mDefaultRenderWindow;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*	GraphicsScene::getActiveRenderWindow() const
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mDevice == NULL || mTask == NULL)
			return NULL;
		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		return static_cast<IRenderWindow*>( task->getRenderTarget() );
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::setActiveRenderWindow(IRenderWindow* window/* = NULL*/, IGraphicsCamera* camera/* = NULL*/, IGraphicsView* view/* = NULL*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mDevice == NULL || mTask == NULL)
			return false;
		if( window == NULL )
			window = mDefaultRenderWindow;

		if( camera == NULL )
			camera = mMainCamera;

		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		task->setTargetCameraView(window, static_cast<ICamera*>(camera), view);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	HELEMENT	GraphicsScene::createGraphicsElement(const TString& elemType, const TString& spaceName/* = TString::EMPTY */)
	{
		//BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mCoordinator == NULL )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("coordinator not created,create it first.") );

		//type not registered, maybe modules not loaded
		if (!GraphicsElementFactory::getSingleton().isClassRegistered(elemType))
			return HELEMENT::EMPTY;

		ISpace* spaceman;
		if( spaceName == TString::EMPTY )
			spaceman = mCoordinator;
		else
		{
			spaceman = mCoordinator->getSpace(spaceName);
			if( spaceman == NULL )
				BLADE_EXCEPT(EXC_NEXIST,BTString("specific space name not found.") );
		}

		GraphicsElement* elem = BLADE_FACTORY_CREATE(GraphicsElement, elemType );
		elem->initialize(this, spaceman);

		if (elemType == MainCameraElement::MAIN_CAMERA_ELEMENT)
		{
			assert(mMainCamera == NULL);
			mMainCamera = static_cast<ICamera*>(elem->IElement::getInterface(IID_CAMERA()));
			assert(mMainCamera != NULL);
		}
		return HELEMENT(elem);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::createSpace(const TString& InstanceName,const TString& spaceType,
		const Vector3& maxSize, bool separated/* = false*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mCoordinator == NULL )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("coordinator not created,create it first.") );

		return mCoordinator->addSpace(spaceType, InstanceName, maxSize, separated) != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::removeSpace(const TString& InstanceName)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mCoordinator == NULL )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("coordinator not created,create it first.") );

		return mCoordinator->removeSpace(InstanceName);
	}


	//////////////////////////////////////////////////////////////////////////
	HTASK		GraphicsScene::createExtraTask(const TString& name, IRenderTarget* target, IGraphicsView* mainView, IGraphicsCamera* mainCamera,
		ITask::ETaskPriority priorityGroup/* = ITask::TP_HIGH*/, index_t priorityID/* = -1*/)
	{
		if (!mInited)
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("scene not initialize,initialize it first."));

		View* view = static_cast<View*>(mainView);
		//notify scheme loss
		if (view->getAttachedRenderScheme() != NULL)
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("view is already used in another scene is not supported."));

		const TString& schemeName = mainView->getRenderScheme();
		IRenderScheme* scheme = IRenderSchemeManager::getSingleton().createRenderScheme(schemeName);
		if (scheme == NULL)
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("unable to create render scheme for view."));
		scheme->setProfile(IGraphicsSystem::getSingleton().getCurrentProfile()); //TODO: custom profile
		view->attachRenderScheme(scheme, true);

		size_t SceneCount = IGraphicsService::getSingleton().getGraphicsSceneCount();
		for (index_t i = 0; i < SceneCount; ++i)
		{
			GraphicsScene* gscene = (GraphicsScene*)IGraphicsService::getSingleton().getGraphicsScene(i);
			if (gscene->isRenderTargetUsed(target))
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("render target already used in another task."));
		}

		//camera pointers set by app/other systems must be a GraphicsCamera wrapper
		//TODO: per space LOD updater
		//mLODUpdater->setCamera(internalCam);

		GraphicsTask* task = BLADE_NEW GraphicsTask(name, mDevice, target, priorityGroup, priorityID);
		GRAPHICS_TASK_DESC desc;

		desc.mQueue = mStateQueue;
		//TODO: per space updater
		desc.mUpdater = mUpdater;
		desc.mRenderView = view;
		desc.mRenderCam = static_cast<ICamera*>(mainCamera);
		desc.mLightManager = mLightManager;
		desc.mCameraManager = mCameraManager;
		desc.mCoordinator = mCoordinator;
		desc.mTaskController = mTaskController;

		task->initialize(desc);
		this->useRenderTarget(target);

		return HTASK(task);
	}

	//////////////////////////////////////////////////////////////////////////
	void		GraphicsScene::setRealtimeDrawing(bool realtime)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );
		mTaskController->setSkipRunAndUpdate( !realtime );
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::isRealtimeDrawing() const
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );
		return !mTaskController->getSkipRunAndUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::drawScene(IWindow* window/* = NULL*/,IGraphicsCamera* camera/* = NULL*/, IGraphicsView* view/* = NULL*/)
	{
		if( !BLADE_TS_CHECK( TS_MAIN_SYNC ) )
		{
			assert(false);
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("main sync state needed") );
		}

		if( mTask == NULL )
			return false;

		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		if( camera == NULL )
			camera = mMainCamera;
		if( view == NULL )
			view = static_cast<IGraphicsView*>( task->getRenderView() );

		task->syncDraw( static_cast<ICamera*>(camera), view, false);

		IRenderWindow* rw = static_cast<IRenderWindow*>( task->getRenderTarget() );
		return rw->swapBuffers(window, view);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::drawScene(IImage* target, IGraphicsCamera* camera/* = NULL*/, IGraphicsView* view/* = NULL*/)
	{
		if (!BLADE_TS_CHECK(TS_MAIN_SYNC))
		{
			assert(false);
			BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("main sync state needed"));
		}

		if (mTask == NULL)
			return false;

		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		if (camera == NULL)
			camera = mMainCamera;
		if (view == NULL)
			view = static_cast<IGraphicsView*>(task->getRenderView());

		task->syncDraw(static_cast<ICamera*>(camera), view, false);

		IRenderWindow* rw = static_cast<IRenderWindow*>(task->getRenderTarget());
		return rw->swapBuffers(target, view);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::drawScene(IRenderTarget* target, IGraphicsCamera* camera/* = NULL*/, IGraphicsView* view/* = NULL*/)
	{
		BLADE_TS_VERIFY( TS_MAIN_SYNC );

		if( mTask == NULL || target == NULL || target->getColorBufferCount() == 0)
			return false;

		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		if( camera == NULL )
			camera = mMainCamera;
		if( view == NULL )
			view = static_cast<IGraphicsView*>( task->getRenderView() );

		task->syncDraw(target, static_cast<ICamera*>(camera), view);
		return target->swapBuffers();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::queryElements(SpaceQuery& query, const TString& spaceName/* = TString::EMPTY*/) const
	{
		//update state will modify elements/contents so query is unavailable from outside
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		bool result;
		int type = (query.getFilter() == SpaceQuery::TF_3D) ? (CSF_CONTENT | CSF_ELEMENT) : CSF_ONSCREEN;
		scalar limit = query.getDistanceLimit();

		if( spaceName == TString::EMPTY )
			result = mCoordinator->queryContents(query, EContentSpaceFlag(type), limit);
		else
			result = mCoordinator->getSpace(spaceName)->queryContents(query, EContentSpaceFlag(type), limit);

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	IElement*	GraphicsScene::queryNearestElement(SpaceQuery& query, const TString& spaceName/* = TString::EMPTY*/) const
	{
		//update state will modify elements/contents so query is unavailable from outside
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		query.setQueryNearest(true);
		if( this->queryElements(query, spaceName) )
		{
			query.sort();
			ScreenQuery::RESULT* result = query.getResult(0);
			IElement* element = result->element;
			scalar distance = result->distance;
			query.clear();
			query.push_back(distance, element);
			return element;
		}
		return NULL;
	}

	/************************************************************************/
	/* IRenderScene interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool					GraphicsScene::setupRenderDevice(IRenderDevice* device)
	{
		mDevice = device;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void					GraphicsScene::onMainLoopUpdate()
	{
		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		if( task != NULL )
			task->mainLoopUpdate();
	}

	//////////////////////////////////////////////////////////////////////////
	bool					GraphicsScene::isRenderTargetUsed(IRenderTarget* target) const
	{
		return mUsedRenderTargets.find(target) != mUsedRenderTargets.end();
	}

	//////////////////////////////////////////////////////////////////////////
	void					GraphicsScene::useRenderTarget(IRenderTarget* target)
	{
		mUsedRenderTargets.insert(target);
	}

	//////////////////////////////////////////////////////////////////////////
	void					GraphicsScene::unuseRenderTarget(IRenderTarget* target)
	{
		mUsedRenderTargets.erase(target);
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderScheme*			GraphicsScene::getRenderScheme() const
	{
		GraphicsTask* task = static_cast<GraphicsTask*>(mTask);
		if( task != NULL )
			return task->getRenderScheme();
		else
			return NULL;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		GraphicsScene::setupSpace(const TString& coordinator/* = TString::EMPTY*/, const TString& space/* = TString::EMPTY*/)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		if (mCoordinator != NULL || mStateQueue != NULL)
			return false;

		const IPagingManager::DESC& desc = mStage->getPagingManager()->getDesc();
		mCoordinatorDesc.pageSize = desc.mPageSize;
		mCoordinatorDesc.visiblePage = desc.mVisiblePages;

		if(!coordinator.empty())
			mCoordinatorDesc.type = coordinator;
		if(!space.empty())
			mCoordinatorDesc.defaultSpace = space;

		mCoordinator = BLADE_FACTORY_CREATE(ISpaceCoordinator, mCoordinatorDesc.type);
		mCoordinator->initialize(&mCoordinatorDesc);

		mStage->getPagingManager()->addListener(mCoordinator);

		//since the space is set up, now element can created,
		//and we need setup state queue to make element creation available
		mStateQueue.bind(BLADE_NEW ParaStateQueue());

		static_cast<LightManager*>(mLightManager)->setSpace(mCoordinator);
		static_cast<CameraManager*>(mCameraManager)->setSpace(mCoordinator);
		return true;
	}
	
}//namespace Blade