/********************************************************************
	created:	2010/04/10
	filename: 	D3D9RenderWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9RenderWindow_h__
#define __Blade_D3D9RenderWindow_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Resource.h"
#include <graphics/RenderWindow.h>

namespace Blade
{
	class D3D9RenderWindow : public RenderWindow, public D3D9Resource<IDirect3DSwapChain9>, public Allocatable
	{
	public:
		D3D9RenderWindow(IDirect3DSwapChain9* pSwapChain,IWindow* window,IRenderDevice* device, bool bMain);
		~D3D9RenderWindow();

		/************************************************************************/
		/* RenderWindow interface                                                          */
		/************************************************************************/
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
		virtual bool			swapBuffers(IWindow* destWindow, IRenderView* sourceView = NULL);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			swapBuffers(IImage* destImage, IRenderView* sourceView = NULL);

	protected:
		/************************************************************************/
		/* IWindowEventHandler interface                                                           */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			onWindowResizing(IWindow* pwin,size_t width,size_t height);

		/*
		@describe 
		@param 
		@return
		*/
		virtual void			onWindowResized(IWindow* pwin);

		/*
		@describe 
		@param
		@return
		*/
		virtual void			onWindowDestroy(IWindow* pwin);

		/************************************************************************/
		/*ID3D9Resource interface                                                                      */
		/************************************************************************/
	public:
		/** @brief  */
		virtual void			release();

		/** @brief  */
		virtual bool			isInDefaultPool() const;

	protected:
		/** @brief  */
		virtual void			rebuildImpl(IDirect3DDevice9* device);

		/** @brief  */
		virtual	void			onSet(IDirect3DSwapChain9* source);
	public:

		/************************************************************************/
		/* custom method                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool					isMainWindow() const;

	protected:

		bool					mIsMain;
		bool					mIsFullScreen;
		HWND					mHwnd;
	};//class D3D9RenderWindow
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#endif //__Blade_D3D9RenderWindow_h__