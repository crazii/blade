/********************************************************************
	created:	2010/04/13
	filename: 	IRenderWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderWindow_h__
#define __Blade_IRenderWindow_h__
#include <interface/public/window/IWindowEventHandler.h>
#include "IRenderTarget.h"

namespace Blade
{
	class IRenderView;
	class IImage;

	class IRenderWindow : public IRenderTarget , public IWindowEventHandler
	{
	public:
		virtual ~IRenderWindow()	{}

		/**
		@describe IRenderTarget function to avoid warning
		@param 
		@return 
		*/
		virtual bool			swapBuffers() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			getFullScreen() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isVisible() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IWindow*		getWindow() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getWindowWidth() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t			getWindowHeight() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			swapBuffers(IWindow* destWindow, IRenderView* sourceView = NULL) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			setSwapBufferOverride(IWindow* ovrWin, IRenderView* ovrView = NULL) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool			swapBuffers(IImage* destImage, IRenderView* sourceView = NULL) = 0;

	};//class IRenderWindow

	
}//namespace Blade


#endif //__Blade_IRenderWindow_h__