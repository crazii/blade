/********************************************************************
	created:	2010/04/10
	filename: 	RenderWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderWindow_h__
#define __Blade_RenderWindow_h__
#include <BladeDevice.h>
#include <interface/public/graphics/RenderProperty.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/graphics/IRenderWindow.h>
#include <graphics/RenderTarget.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4275)
#endif

namespace Blade
{
	/* @remarks:
	using composite mode instead of inherit ,to avoid diamond structure. \n
	although the base class IRenderTarget is pure abstract, but there's boring warnings in MSVC.\n
	still for a common interface,just one level of inheritance is better.

	ideal inheritance:
						IRenderTarget(pure virtual)
						/			\
					   /			 \
			IRenderWindow			RenderTarget
						\			 /
						\			/
						RenderWindow
	now:
				
					IRenderTarget(pure virtual)
						/			\
						/			 \
			IRenderWindow			  \	
					|				   \
					|					\
					|					 \
			RenderWindow ----has a---->RenderTarget

	maybe this is more weird,but which is better

	*/
	class BLADE_DEVICE_API RenderWindow : public IRenderWindow
	{
	public:
		RenderWindow(IWindow* pwin, IRenderDevice* device);
		virtual ~RenderWindow()		{}


		/************************************************************************/
		/* IRenderTarget Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName() const;

		/*
		@describe
		@param
		@return
		*/
		virtual const HTEXTURE&getDepthBuffer() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setDepthBuffer(const HTEXTURE& hDethBuffer);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			setColorBuffer(index_t index, const HTEXTURE& buffer);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			setColorBufferCount(index_t /*count*/){return false;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HTEXTURE&getColorBuffer(index_t index) const;

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			getViewWidth() const	{return this->getWindowWidth();}

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t			getViewHeight() const	{return this->getWindowHeight();}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		setViewWidth(size_t /*width*/)	{return false;}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		setViewHeight(size_t /*height*/){return false;}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool		setViewRect(int32 left, int32 top, int32 width, int32 height);

		/*
		@describe
		@param
		@return
		*/
		virtual size_t			getColorBufferCount() const;

		/*
		@describe
		@param
		@return
		*/
		virtual IListener*		setListener(IListener* listener);

		/*
		@describe
		@param
		@return
		*/
		virtual IListener*		getListener() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool			isReady() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			swapBuffers();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setSwapBufferOverride(IWindow* ovrWin, IRenderView* ovrView = NULL);

		/************************************************************************/
		/* IRenderWindow interface                                                                     */
		/************************************************************************/

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool					getFullScreen() const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool					isVisible() const;

		/*
		@describe 
		@param 
		@return 
		*/
		IWindow*				getWindow() const;

		/*
		@describe 
		@param 
		@return 
		*/
		size_t					getWindowWidth() const;

		/*
		@describe 
		@param 
		@return 
		*/
		size_t					getWindowHeight() const;

		/*
		@describe
		@param
		@return
		*/
		HTEXTURE&				getDepthBuffer();

		/** @brief  */
		const HTEXTURE&	getBackBuffer() const{return mBackBuffer;}

		/** @brief  */
		HTEXTURE&		getBackBuffer()		{return mBackBuffer;}

	protected:
		RenderTarget			mRenderTarget;
		IWindow*				mWindow;
		HTEXTURE				mBackBuffer;

		IWindow*				mOverrideWindow;
		IRenderView*			mOVerrideView;
	};//class RenderWindow

}//namespace Blade

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif


#endif //__Blade_RenderWindow_h__