/********************************************************************
	created:	2013/04/23
	filename: 	Viewport.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Viewport_h__
#define __Blade_Viewport_h__
#include <utility/BladeContainer.h>
#include <math/Box2i.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/ui/IViewport.h>
#include <interface/public/ui/IViewportManager.h>
#include <interface/public/graphics/IGraphicsService.h>

#define USE_RENDER_WINDOW 0
#if USE_RENDER_WINDOW
#include <interface/public/graphics/IRenderWindow.h>
#endif

namespace Blade
{
	class ILogicWorld;

	typedef struct SViewportData : public NonCopyable
	{
		ILogicWorld*		mWorld;
		CameraActor*		mCamera;
		IGraphicsView*		mView;
		bool				mAutoDelete;

		SViewportData()	{mCamera = NULL; mView = NULL; mAutoDelete = false;}

		~SViewportData()	{this->clear();}
		void				clear();
		bool				empty()	{return mCamera == NULL && mView == NULL;}
	}VIEWPORT_DATA;

	class Viewport : public IViewport, public NonAssignable, public Allocatable
	{
	public:
		Viewport(const VIEWPORT_DESC& desc, IViewportSet* set, index_t index);
		~Viewport();

		/************************************************************************/
		/* IViewport interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual index_t				getIndex() const		{return mIndex;}

		/*
		@describe get the controllable camera instance of the view port
		@param 
		@return 
		*/
		virtual CameraActor*		getCameraActor() const	{return mData.mCamera;}

		/*
		@describe the the view of the view port
		@param 
		@return 
		*/
		virtual IGraphicsView*		getView() const		{return mData.mView;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setWindow(IWindow* window)
		{
			assert( mWindow == NULL && window != NULL);
			mWindow = window;
#if USE_RENDER_WINDOW
			mRenderWindow = IGraphicsService::getSingleton().createRenderWindow(mWindow);
			mWindow->addEventHandler(mRenderWindow);
#endif
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IWindow*			getWindow() const	{return mWindow;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const VIEWPORT_DESC&	getDesc() const	{return mDesc;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		setRealtime(bool realtime);

		/*
		@describe 
		@param
		@return
		*/
		virtual void		setMaximized(bool maximized);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		drawViewport()	{ mSet->drawViewport(mIndex); }

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		activate()		{return (mDesc.mActive = mSet->setActiveViewport(mIndex));}
		
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		VIEWPORT_DATA&		getViewportDataRef()	{return mData;}
		/** @brief  */
		void				updateViewCamera();
		/** @brief get the rect in back buffer coordinates */
		Box2i				getViewportRect(IRenderTarget* target) const;
		/** @brief  */
		void				setAcitve(bool active)	{mDesc.mActive = active;}

#if USE_RENDER_WINDOW
		IRenderWindow*		getRenderWindow() const { return mRenderWindow; }
#endif

	protected:
		const VIEWPORT_DESC mDesc;
		VIEWPORT_DATA		mData;
		IWindow*			mWindow;
#if USE_RENDER_WINDOW
		IRenderWindow*		mRenderWindow;
#endif
		index_t				mIndex;
		IViewportSet*		mSet;
	};//class Viewport


	class ViewportSet : public IViewportSet, public Allocatable
	{
	public:
		ViewportSet(const VIEWPORT_LAYOUT& layout);
		~ViewportSet();

		/************************************************************************/
		/* IViewportSet interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual bool		initialize(ILogicWorld* world, const TString& cameraNamePrefix);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getViewportCount() const	{return mViewports.size();}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IViewport*	getViewport(index_t index) const
		{
			return index < mViewports.size() ? mViewports[index] : NULL;
		}

		/*
		@describe get the only one active view port
		the active view port is update (drawing) in real time.
		@param 
		@return 
		*/
		virtual IViewport*	getActiveViewport() const
		{
			assert(mActiveViewport < mViewports.size());
			return mViewports[mActiveViewport];
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual index_t		getActiveViewportIndex() const
		{
			return mActiveViewport;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		setActiveViewport(index_t index);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		resetLayout(const VIEWPORT_LAYOUT* layout = NULL);

		/*
		@describe manually draw the view port
		@param 
		@return false if no view port found with the index, or the view port is auto-updated (active & real-time)
		*/
		virtual bool		drawViewport(index_t index);

	protected:
		/** @brief  */
		void				setupCameraView(Viewport* viewport);
		/** @brief  */
		void				cleanup();

		typedef Vector<Viewport*> ViewportList;

		ViewportList	mViewports;
		index_t			mActiveViewport;
		const VIEWPORT_LAYOUT* mLayout;
		ILogicWorld*	mWorld;
		TString			mCameraPrefix;
	};//class ViewportSet
	
}//namespace Blade

#endif//__Blade_Viewport_h__