/********************************************************************
	created:	2010/04/12
	filename: 	WindowEventDispatcher.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowEventDispatcher_h__
#define __Blade_WindowEventDispatcher_h__
#include <interface/public/window/IWindowEventHandler.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class WindowEventDispatcherBase  : public IWindowEventHandler
	{
	protected:
		virtual ~WindowEventDispatcherBase() = 0;
	};

	class WindowEventDispatcher : public WindowEventDispatcherBase
	{
	public:
		WindowEventDispatcher();
		virtual ~WindowEventDispatcher();

		/************************************************************************/
		/* IWindowEventHandler interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void onWindowShow(IWindow* pwin);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void onWindowHide(IWindow* pwin);

		/*
		@describe 
		@param 
		@return whether allow resize
		*/
		virtual bool onWindowResizing(IWindow* pwin,size_t width,size_t height);

		/*
		@describe 
		@param 
		@return
		@remark pwin 's width and pwin 's height is 0 indicates it is minimized
		*/
		virtual void onWindowResized(IWindow* pwin);


		/*
		@describe obtain focus
		@param 
		@return 
		*/
		virtual void onWindowActivate(IWindow* pwin);

		/*
		@describe lost focus
		@param 
		@return 
		*/
		virtual void onWindowDeactivate(IWindow* pwin);

		/*
		@describe 
		@param 
		@return indicates whether close window indeed
		*/
		virtual bool onWindowClose(IWindow* pwin);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void onWindowDestroy(IWindow* pwin);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		
		/*
		@describe 
		@param 
		@return 
		*/
		bool		addEventHandler(IWindowEventHandler*	handler);

		/*
		@describe 
		@param 
		@return 
		*/
		bool		removeEventHandler(IWindowEventHandler* handler);

	protected:
		typedef Set<IWindowEventHandler*>	WinHandlerList;

		WinHandlerList		mHandlerList;
	};//class WindowEventDispatcher
	
}//namespace Blade


#endif //__Blade_WindowEventDispatcher_h__