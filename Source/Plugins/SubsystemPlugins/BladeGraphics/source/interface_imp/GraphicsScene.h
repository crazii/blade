/********************************************************************
	created:	2010/04/09
	filename: 	GraphicsScene.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsScene_h__
#define __Blade_GraphicsScene_h__
#include <Handle.h>
#include <utility/BladeContainer.h>

#include <interface/public/ITask.h>
#include <interface/IRenderScene.h>
#include <interface/ISpaceCoordinator.h>
#include <interface/IEffectManager.h>
#include <parallel/ParaStateQueue.h>

#include "RenderHelper/HelperRenderType.h"
#include "RenderHelper/AABBRenderer.h"
#include "MaterialLODUpdater.h"

namespace Blade
{
	class IRenderScheme;
	class GraphicsTaskController;

	class GraphicsScene : public IRenderScene, public Allocatable
	{
	public:
		GraphicsScene();
		~GraphicsScene();
		/************************************************************************/
		/* ISerializable interface                                                                     */
		/************************************************************************/
		/** @brief called after loading data */
		virtual void			postProcess(const ProgressNotifier& notifier);

		/************************************************************************/
		/* IScene class interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getSceneType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			getAvailableElementTypes(ElementTypeList& elemList) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IElement*		createElement(const TString& type);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onAttachToStage(IStage* pstage);


		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			onDetachFromStage(IStage* pstage);

		/************************************************************************/
		/* IGraphicsScene interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual IGraphicsSpaceCoordinator*	getSpaceCoordinator() const	{BLADE_TS_VERIFY(TS_MAIN_SYNC);return mCoordinator;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsEffectManager*	getEffectManager() const	{BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();return mEffectMananger;}

		/** @brief internal method without state check */
		inline IEffectManager*	getEffectManagerImpl() const		{return mEffectMananger;}

		/*
		@describe the space should be setup before calling this
		@param
		@return
		*/
		virtual	bool		initialize(IGraphicsView* mainView, const TString& coordinator = TString::EMPTY, const TString& defSpace = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsView*	getMainView() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsCamera*	getMainCamera() const
		{
			BLADE_TS_VERIFY(TS_MAIN_SYNC);
			return this->GraphicsScene::getMainCameraImpl();
		}

		/*
		@describe get the default render window of the scene
		@param 
		@return 
		*/
		virtual IRenderWindow*	getDefaultRenderWindow() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IRenderWindow*	getActiveRenderWindow() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		setActiveRenderWindow(IRenderWindow* window = NULL, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL);

		/*
		@describe
		@param
		@return
		*/
		virtual HELEMENT	createGraphicsElement(const TString& elemType, const TString& spaceName = TString::EMPTY );

		/*
		@describe
		@param [in] separated create the space as separated, so cameras in other spaces cannot see object it it
		and cameras in this space cannot see objects in other spaces
		@return
		*/
		virtual bool		createSpace(const TString& spaceName,const TString& spaceType,
			const Vector3& maxSize, bool separated = false);

		/*
		@describe
		@param
		@return
		*/
		virtual bool		removeSpace(const TString& InstanceName);


		/*
		@describe
		@param
		@return
		*/
		virtual HTASK		createExtraTask(const TString& name, IRenderTarget* target, IGraphicsView* mainView, IGraphicsCamera* mainCamera,
			ITask::ETaskPriority priorityGroup = ITask::TP_HIGH, index_t priorityID = -1);

		/*
		@describe whether this scene is drawing on each frame automatically
		@param 
		@return 
		*/
		virtual void		setRealtimeDrawing(bool realtime);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		isRealtimeDrawing() const;

		/*
		@describe redraw the scene at once, mainly used for Editor style paint/refresh
		@param the camera & view override, specify NULL to use scene default
		@return 
		*/
		virtual bool		drawScene(IWindow* window = NULL, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		drawScene(IRenderTarget* target, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		drawScene(IImage* target, IGraphicsCamera* camera = NULL, IGraphicsView* view = NULL);

		/**
		@describe query the all elements, using implementation defined query method
		@param [in] spaceName: the space inside which to perform a ray cast.\n
		default:EMPTY means all spaces within the scene's coordinator
		"Default" means only the coordinator itself, no other sub scenes
		@return
		*/
		virtual bool		queryElements(SpaceQuery& query, const TString& spaceName = TString::EMPTY) const;

		/**
		@describe
		@param
		@return
		*/
		virtual IElement*	queryNearestElement(SpaceQuery& query, const TString& spaceName = TString::EMPTY) const;

		/************************************************************************/
		/* IRenderScene interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual bool					setupRenderDevice(IRenderDevice* device);

		/*
		@describe synchronous update
		@param 
		@return 
		*/
		virtual void					onMainLoopUpdate();

		/*
		@describe
		@param
		@return
		*/
		virtual bool					isRenderTargetUsed(IRenderTarget* target) const;

		/*
		@describe
		@param
		@return
		*/
		virtual void					useRenderTarget(IRenderTarget* target);

		/*
		@describe
		@param
		@return
		*/
		virtual void					unuseRenderTarget(IRenderTarget* target);

		/*
		@describe get current render scheme for this scene (per scene)
		@param 
		@return 
		*/
		virtual IRenderScheme*			getRenderScheme() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IStage*					getStage() const		{return mStage;}

		/*
		@describe
		@param
		@return
		*/
		virtual ParaStateQueue*			getStateQueue() const	{return mStateQueue;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IGraphicsUpdater*		getUpdater() const		{return mUpdater;}

		/*
		@describe
		@param
		@return
		*/
		virtual IAABBRenderer*			getAABBRenderer() const	{return mAABBRenderer;}

		/*
		@describe
		@param
		@return
		*/
		virtual ILightManager*			getLightManager() const	{return mLightManager;}

		/*
		@describe
		@param
		@return
		*/
		virtual ICameraManager*	getCameraManager() const		{return mCameraManager;}

		/*
		@describe
		@param
		@return
		*/
		virtual IMaterialLODUpdater*	getMaterialLODUpdater() const	{return mLODUpdater;}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual ICamera*		getMainCameraImpl() const
		{
			if (mTask == NULL)
				return NULL;
			return mMainCamera;
		}

		/** @brief internal method without state check */
		virtual ISpaceCoordinator*	getSpaceCoordinatorImpl() const { return mCoordinator; }

	protected:

		/** @brief  */
		bool		setupSpace(const TString& coordinator = TString::EMPTY, const TString& space = TString::EMPTY);

		typedef	Set<IRenderTarget*>		RenderTargetList;
		RenderTargetList	mUsedRenderTargets;

		HTASK				mTask;
		HPARASTATEQUEUE		mStateQueue;
		GraphicsTaskController* mTaskController;
		IRenderWindow*		mDefaultRenderWindow;
		IGraphicsUpdater*	mUpdater;

		IRenderDevice*		mDevice;
		IStage*				mStage;
		ISpaceCoordinator*	mCoordinator;

		AABBRenderer*		mAABBRenderer;
		ILightManager*		mLightManager;
		ICamera*			mMainCamera;
		ICameraManager*		mCameraManager;
		IEffectManager*		mEffectMananger;
		MaterialLODUpdater*	mLODUpdater;
		bool				mInited;

		//data-binding
		COORDINATOR_DESC	mCoordinatorDesc;
		const TString&		getCoordinatorType(index_t) const { return mCoordinatorDesc.type; }
		bool				setCoordinatorType(index_t, const TString& type) { mCoordinatorDesc.type = type; return true; }
		const TString&		getDefaultCoordinatorSpaceType(index_t) const { return mCoordinatorDesc.defaultSpace; }
		bool				setDefaultCoordinatorSpaceType(index_t, const TString& space) { mCoordinatorDesc.defaultSpace = space; return true; }
		friend class GraphicsSystemPlugin;
	};//class GraphicsScene

	//template class BLADE_GRAPHICS_API Handle<GraphicsScene>;
	//typedef Handle<GraphicsScene> HGRAPHICSSCENE;
	
}//namespace Blade



#endif //__Blade_GraphicsScene_h__