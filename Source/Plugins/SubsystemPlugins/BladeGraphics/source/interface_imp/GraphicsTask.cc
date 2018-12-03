/********************************************************************
	created:	2010/04/09
	filename: 	GraphicsTask.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "GraphicsTask.h"
#include <utility/Profiling.h>

#include <interface/IPlatformManager.h>
#include <interface/public/graphics/IRenderView.h>
#include <interface/ITaskManager.h>

#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <FrameEvents.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <parallel/ParaStateQueue.h>

#include <interface/ISpaceCoordinator.h>
#include <interface/IRenderScheme.h>
#include <interface/IGraphicsUpdater.h>
#include "View.h"
#include "../internal/GraphicsConfig.h"


namespace Blade
{
#if MULTITHREAD_CULLING
	class GraphicsCullingTask : public IDispatchableTask
	{
	public:
		const TString& getName() const
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

			static const TString NAME = BTString("GraphicsTask::Culling");
			return NAME;

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
		}
		virtual void run()
		{
			//BLADE_LW_PROFILING(CULL);
			mTask->mUpdater->update(SS_PRE_CULLING);
			mTask->mRenderScheme->updateCulling();
			mTask->mUpdater->update(SS_POST_CULLING);
		}
		GraphicsCullingTask() { mTask = NULL; }
		GraphicsCullingTask(GraphicsTask* task) :mTask(task) {}
		GraphicsTask* mTask;
	};
#endif

	const ITask::Type GraphicsTask::TYPE = ITask::Type(BTString("GPU"), ITask::TA_MAIN);

	//////////////////////////////////////////////////////////////////////////
	GraphicsTask::GraphicsTask(const TString& name,IRenderDevice* device,IRenderTarget* target,ETaskPriority priority/* = TP_NORMAL */,index_t priorityID/* = 0*/)
		:TaskBase(name,priority, (uint32)priorityID)
		,mRenderDevice(device)
		,mRenderTarget(target)
		,mRenderScheme(NULL)
		,mRenderView(NULL)
		,mMainCamera(NULL)
		,mQueue(NULL)
		,mUpdater(NULL)
		,mCoordinator(NULL)
		,mLightManager(NULL)
		,mCameraManager(NULL)
		,mController(NULL)
		,mTaskEnQueued(false)
		,mInited(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsTask::~GraphicsTask()
	{
		if(mTaskEnQueued)
		{
			//BLADE_EXCEPT(EXCE_STATE_ERROR, BTString("task still in running state, cannot destroy, remove it from task manager first.") );
			assert(false && "task still in running state, cannot destroy, remove it from task manager first.");
		}

		if(mController != NULL )
			mController->onTaskDestroy(this);
	}

	/************************************************************************/
	/* ITask class interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const ITask::Type&	GraphicsTask::getType() const
	{
		return TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::run()
	{
		if( !mInited )
			return;
		//BLADE_LW_PROFILING_FUNCTION();

		//graphics task working in fixed thread (async mode)
		static Thread::ID id = Thread::getCurrentID();
		mController->updateThreadID(id);

		uint32 time = 0;
		bool ready = mRenderTarget->isReady();
		bool sleep = !mController->isTaskRunNeeded(this, time);
		if (sleep || !ready)
		{
			if (sleep && time > 0)
				IPlatformManager::getSingleton().sleep(time);
			return;
		}

		IEventManager& evtm = IEventManager::getSingleton();

#if MULTITHREAD_CULLING && 0
		//note: perform async culling before swapBuffer, is almost the same as perform culling after swapBuffer, or even worse.
		//since swapBuffer is just simply waiting GPU to finish works.
		//if not multi threaded, and GPU has work load, culling will take usage of CPU and wait GPU to finish.
		//if multi threaded, it is almost the same thing, except that the task will block on swapBuffer instead of working on culling.
		//and, blade's batch combination may not work while swapBuffer
		//because batch combiners will operate on GPU to modify vertex/index buffers, and swapBuffer will operate on GPU too.
		//this will need  API with multi thread support(i.e. d3d9 simply with D3DCREATE_MULTITHREADED, or d3d11/vulkan).
		//swap buffer of last frame
		{
			//BLADE_LW_PROFILING(CULL_SWAP);
			GraphicsCullingTask task(this);
			IDispatchableTask::Handles handles;
			IDispatchableTask* ptask = &task;
			ITaskManager::getSingleton().dispatchTasks(&ptask, 1, &handles);
			mRenderTarget->swapBuffers();
			handles.sync();
		}
#else
		{
			//BLADE_LW_PROFILING(CULL);
			mUpdater->update(SS_PRE_CULLING);
			mRenderScheme->updateCulling();
			mUpdater->update(SS_POST_CULLING);
		}

		//swap buffer of last frame
		{
			//BLADE_LW_PROFILING(SWAP);
			mRenderTarget->swapBuffers();
		}
#endif

		mUpdater->update(SS_PRE_RENDER);

		evtm.dispatchEvent(FrameBeginEvent(mRenderTarget));
		//render current frame
		{
			//BLADE_LW_PROFILING(RENDER);
			mRenderDevice->beginScene();
			mRenderScheme->render(mRenderDevice, false);
			mRenderDevice->endScene();
		}
		evtm.dispatchEvent(FrameEndEvent(mRenderTarget));

		mUpdater->update(SS_POST_RENDER);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::update()
	{
		if( !mInited )
			return;

		uint32 time = 0;
		bool ready = mRenderTarget->isReady();
		bool sleep = !mController->isTaskRunNeeded(this, time);
		if (sleep || !ready)
		{
			if (sleep && time > 0)
				IPlatformManager::getSingleton().sleep(time);
			return;
		}

		static Thread::ID id = Thread::getCurrentID();
		mController->updateThreadID(id);

		mUpdater->update(SS_ASYNC_UPDATE);

		//update element/content transforms from other subsystem
		mQueue->update();	//camera transform may change after update
		//update view matrix for next culling & draw
		mMainCamera->updateMatrix();
		mCoordinator->update();	//update space contents due to element/content transforms change
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::onAccepted()
	{
		mTaskEnQueued = true;
		if(mController != NULL )
			mController->onTaskAccepted(this);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::onRemoved()
	{
		mTaskEnQueued = false;
		if(mController != NULL )
			mController->onTaskRemoved(this);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::initialize(const GRAPHICS_TASK_DESC& taskDesc)
	{
		if( !mInited )
		{
			mQueue = taskDesc.mQueue;
			mUpdater = taskDesc.mUpdater;
			mRenderView = taskDesc.mRenderView;
			View* view = static_cast<View*>(mRenderView);
			mRenderScheme = view->getAttachedRenderScheme();
			mMainCamera = taskDesc.mRenderCam;
			mCoordinator = taskDesc.mCoordinator;
			mLightManager = taskDesc.mLightManager;
			mCameraManager = taskDesc.mCameraManager;
			mController = taskDesc.mTaskController;

			mRenderScheme->setupRender( mRenderDevice, mRenderTarget, view, mMainCamera,
				mLightManager, mCameraManager);

			mInited = true;
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	GraphicsTask::getRenderTarget() const
	{
		return mRenderTarget;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::setRenderTarget(IRenderTarget* target)
	{
		mRenderTarget = target;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::setCoordinator(ISpaceCoordinator* coordinator)
	{
		mCoordinator = coordinator;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::setRenderScheme(IRenderScheme* scheme)
	{
		mRenderScheme = scheme;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::mainLoopUpdate()
	{
		if( mInited )
		{
			mController->updateThreadID( Thread::INVALID_ID );

			mUpdater->update(SS_MAIN_UPDATE);

			View* view = static_cast<View*>(mRenderView);
			IRenderScheme* scheme = view->getAttachedRenderScheme();
			if( scheme != mRenderScheme )
			{
				mRenderScheme = scheme;
				if( mRenderScheme != NULL )
					mRenderScheme->setupRender(mRenderDevice, mRenderTarget, view, mMainCamera,
					mLightManager, mCameraManager);
			}

			//update view matrix for next culling & draw
			mMainCamera->updateMatrix();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::syncDraw(ICamera* cameraOverride, IRenderView* viewOverride, bool swap)
	{
		IEventManager::getSingleton().dispatchEvent(FrameBeginEvent(mRenderTarget));

		//save old data
		mQueue->update();
		mUpdater->update(SS_PRE_CULLING);
		mRenderScheme->updateCulling(cameraOverride);
		mUpdater->update(SS_POST_CULLING);

		mUpdater->update(SS_PRE_RENDER);
		
		if(mRenderTarget != NULL && mRenderTarget->isReady())
		{
			mRenderDevice->beginScene();
			mRenderScheme->render( mRenderDevice, swap, NULL, static_cast<IGraphicsView*>(viewOverride), cameraOverride);
			mRenderDevice->endScene();
		}
		mUpdater->update(SS_POST_RENDER);

		IEventManager::getSingleton().dispatchEvent(FrameEndEvent(mRenderTarget));
	}


	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::syncDraw(IRenderTarget* target, ICamera* cameraOverride, IRenderView* viewOverride)
	{
		FrameBeginEvent fbs(mRenderTarget);
		IEventManager::getSingleton().dispatchEvent(fbs);

		assert(target != NULL);
		mQueue->update();
		mUpdater->update(SS_PRE_CULLING);
		mRenderScheme->updateCulling(cameraOverride);
		mUpdater->update(SS_POST_CULLING);

		mRenderDevice->beginScene();
		mUpdater->update(SS_PRE_RENDER);
		mRenderScheme->render(mRenderDevice, true, target, static_cast<IGraphicsView*>(viewOverride), cameraOverride);
		mUpdater->update(SS_POST_RENDER);
		mRenderDevice->endScene();

		FrameEndEvent fes(mRenderTarget);
		IEventManager::getSingleton().dispatchEvent(fes);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GraphicsTask::setTargetCameraView(IRenderTarget* target, ICamera* camera, IRenderView* view)
	{
		assert(target != NULL);
		if( camera == NULL )
			camera = mMainCamera;
		if( view == NULL )
			view = mRenderView;

		mRenderTarget = target;
		mRenderScheme->setupRender( mRenderDevice, mRenderTarget, static_cast<View*>(view), camera,
			mLightManager, mCameraManager);
	}
	
}//namespace Blade