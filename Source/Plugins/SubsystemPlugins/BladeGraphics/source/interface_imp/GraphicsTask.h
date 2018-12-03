/********************************************************************
	created:	2010/04/09
	filename: 	GraphicsTask.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsTask_h__
#define __Blade_GraphicsTask_h__
#include <threading/Thread.h>
#include <BladeGraphics.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <TaskBase.h>

namespace Blade
{
	class GraphicsTask;
	class IRenderDevice;
	class ISpaceCoordinator;
	class ParaStateQueue;
	class IGraphicsUpdater;
	class IRenderScheme;
	class ILightManager;
	class ICameraManager;
	class IRenderView;
	class ICamera;
	class IWindow;
	class IRenderScene;

	class IGraphicsTaskController
	{
	public:
		/** @brief  */
		virtual bool	isTaskRunNeeded(GraphicsTask* ptask,uint32& sleeptime) const = 0;
		/** @brief  */
		virtual void	onTaskAccepted(GraphicsTask* ptask) = 0;
		/** @brief  */
		virtual void	onTaskRemoved(GraphicsTask* ptask) = 0;
		/** @brief  */
		virtual void	onTaskDestroy(GraphicsTask* ptask) = 0;
		/** @brief  */
		virtual void	updateThreadID(Thread::ID id) = 0;
		/** @brief  */
		virtual IRenderScene* getScene() const = 0;
	};

	typedef struct SGraphicsTaskDescriptor
	{
		ParaStateQueue*			mQueue;
		IGraphicsUpdater*		mUpdater;
		IRenderView*			mRenderView;
		ICamera*				mRenderCam;
		ISpaceCoordinator*		mCoordinator;
		ILightManager*			mLightManager;
		ICameraManager*			mCameraManager;
		IGraphicsTaskController*mTaskController;
	}GRAPHICS_TASK_DESC;

	class GraphicsTask : public TaskBase, public Allocatable
	{
	public:
		static const ITask::Type TYPE;
	public:
		GraphicsTask(const TString& name,IRenderDevice* device,IRenderTarget* target,ETaskPriority priority = TP_NORMAL,index_t priorityID = 0);
		~GraphicsTask();

		/************************************************************************/
		/* ITask class interfaces                                                                     */
		/************************************************************************/
		/*
		@describe get task main device type on which it depends,such as CPU or GPU or HDD,etc.
		@param 
		@return 
		@remark this is useful for parallel optimizing \n
		for example,a physics task maybe on CPU,or on other device like GPU (PhysX)
		*/
		virtual const ITask::Type&	getType() const;

		/*
		@describe run the ITask
		@param 
		@return 
		*/
		virtual void			run();

		/*
		@describe update the task: synchronize data
		@param 
		@return 
		*/
		virtual void			update();

		/*
		@describe
		@param
		@return
		*/
		virtual void			onAccepted();

		/*
		@describe
		@param
		@return
		*/
		virtual void			onRemoved();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		void					initialize(const GRAPHICS_TASK_DESC& taskDesc);

		/*
		@describe
		@param
		@return
		*/
		IRenderTarget*			getRenderTarget() const;

		/*
		@describe
		@param
		@return
		*/
		void					setRenderTarget(IRenderTarget* target);

		/*
		@describe
		@param
		@return
		*/
		void					setCoordinator(ISpaceCoordinator* coordinator);

		/*
		@describe
		@param
		@return
		*/
		void					setRenderScheme(IRenderScheme* scheme);

		/*
		@describe
		@param
		@return
		*/
		void					mainLoopUpdate();

		/*
		@describe 
		@param 
		@return 
		*/
		void					syncDraw(ICamera* cameraOverride, IRenderView* viewOverride, bool swap);

		/*
		@describe 
		@param 
		@return 
		*/
		void					syncDraw(IRenderTarget* target, ICamera* cameraOverride, IRenderView* viewOverride);

		/*
		@describe 
		@param 
		@return 
		*/
		void					setTargetCameraView(IRenderTarget* target, ICamera* camera, IRenderView* view);

		/** @brief  */
		inline IRenderScheme*	getRenderScheme() const
		{
			return mRenderScheme;
		}

		/** @brief  */
		inline IRenderView*		getRenderView() const
		{
			return mRenderView;
		}

		/** @brief  */
		inline ICamera*			getCamera() const
		{
			return mMainCamera;
		}

		/** @brief  */
		inline bool				isInitialized() const
		{
			return mInited;
		}

	protected:
		IRenderDevice*		mRenderDevice;
		IRenderTarget*		mRenderTarget;
		IRenderScheme*		mRenderScheme;
		IRenderView*		mRenderView;
		ICamera*			mMainCamera;

		//per-scene objects
		ParaStateQueue*		mQueue;
		IGraphicsUpdater*	mUpdater;
		ISpaceCoordinator*	mCoordinator;
		ILightManager*		mLightManager;
		ICameraManager*		mCameraManager;
		IGraphicsTaskController*	mController;
		bool				mTaskEnQueued;
		bool				mInited;

		friend class GraphicsCullingTask;
	};//class GraphicsTask
	
}//namespace Blade


#endif //__Blade_GraphicsTask_h__