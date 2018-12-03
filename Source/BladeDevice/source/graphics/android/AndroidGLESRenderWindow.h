/********************************************************************
	created:	2014/12/20
	filename: 	AndroidGLESRenderWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_AndroidGLESRenderWindow_h__
#define __Blade_AndroidGLESRenderWindow_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <graphics/GLES/GLESRenderWindow.h>
#include <interface/public/graphics/PixelFormat.h>


namespace Blade
{
	class GLESContext;

	class AndroidGLESRenderWindow : public GLESRenderWindow, public Allocatable
	{
	public:
		AndroidGLESRenderWindow(IWindow* window, IRenderDevice* device, GLESContext* context);
		~AndroidGLESRenderWindow();

		/************************************************************************/
		/* IRenderTarget interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual bool			setDepthBuffer(const HTEXTURE& hDethBuffer);

		/*
		@describe if a output buffer is specified,return it ,otherwise return built in buffer
		@param 
		@return 
		*/
		virtual bool			setColorBuffer(index_t index, const HTEXTURE& buffer);

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
		virtual bool			swapBuffers(IImage* destImage, IRenderView* sourceView = NULL) { BLADE_EXCEPT(EXC_UNKNOWN, BTString("not implemented yet."));}

	protected:
		/************************************************************************/
		/* IWindowEventHandler interface                                                           */
		/************************************************************************/
		/*
		@describe called when window created(re-created)
		@param
		@return
		*/
		virtual void onWindowCreated(IWindow* pwin);

		/*
		@describe called before destroy
		@param 
		@return 
		*/
		virtual void onWindowDestroy(IWindow* pwin);

	public:

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

	protected:
	};
	
}//namespace Blade

#endif//BLADE_PLATFORM
#endif // __Blade_AndroidGLESRenderWindow_h__