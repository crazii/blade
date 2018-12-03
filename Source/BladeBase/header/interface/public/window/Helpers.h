/********************************************************************
	created:	2015/09/17
	filename: 	Helpers.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Helpers_h__
#define __Blade_Helpers_h__
#include <utility/Functors.h>
#include "IWindowEventHandler.h"

namespace Blade
{

	/************************************************************************/
	/* for_each utility                                                                     */
	/************************************************************************/
	/** @brief  */
	struct FnOnWinEvent : public std::binder2nd< std::mem_fun1_t<void,IWindowEventHandler,IWindow*> >
	{
		typedef std::mem_fun1_t<void,IWindowEventHandler,IWindow*> fnType;
	public:
		FnOnWinEvent(IWindow* window,void (IWindowEventHandler::*func)(IWindow*) )
			:std::binder2nd< fnType >( fnType(func),window)
		{}
	};

	struct FnOnWinEventResult : public FnBinder2ndBool< FnMemFun1Bool<IWindowEventHandler,IWindow*> >
	{
		typedef FnMemFun1Bool<IWindowEventHandler,IWindow*> fnType;
	public:
		FnOnWinEventResult(IWindow* window,bool (IWindowEventHandler::*func)(IWindow*) )
			:FnBinder2ndBool< fnType >( fnType(func),window)
		{
		}
	};


	/** @brief  */
	struct FnOnWinClose : public FnOnWinEventResult
	{
		FnOnWinClose(IWindow* window)
			:FnOnWinEventResult(window,&IWindowEventHandler::onWindowClose )
		{}
	};

	/** @brief  */
	struct FnOnWinShow : public FnOnWinEvent
	{
		FnOnWinShow(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowShow )
		{}
	};

	/** @brief  */
	struct FnOnWinHide : public FnOnWinEvent
	{
		FnOnWinHide(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowHide )
		{}
	};

	/** @brief  */
	struct FnOnWinResizing
	{
		FnOnWinResizing(IWindow* window,size_t x,size_t y )
			:mWindow(window)
			,mWidth(x)
			,mHeight(y)
			,mAllowResize(true)
		{}

		void operator()(IWindowEventHandler* handler)
		{
			mAllowResize = handler->onWindowResizing(mWindow,mWidth,mHeight);
		}

		IWindow*	mWindow;
		size_t		mWidth;
		size_t		mHeight;
		bool		mAllowResize;
	};

	/** @brief  */
	struct FnOnWinResized : public FnOnWinEvent
	{
		FnOnWinResized(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowResized )
		{}
	};


	/** @brief  */
	struct FnOnWinDeactivate : public FnOnWinEvent
	{
		FnOnWinDeactivate(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowDeactivate )
		{}
	};


	struct FnOnWinActivate : public FnOnWinEvent
	{
		FnOnWinActivate(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowActivate )
		{}
	};

	/** @brief  */
	struct FnOnWinCaptionChange : public FnOnWinEvent
	{
		FnOnWinCaptionChange(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowCaptionChanged )
		{}
	};

	struct FnOnWinCreate : public FnOnWinEvent
	{
		FnOnWinCreate(IWindow* window)
			:FnOnWinEvent(window, &IWindowEventHandler::onWindowCreated )
		{}
	};

	struct FnOnWinDestroy : public FnOnWinEvent
	{
		FnOnWinDestroy(IWindow* window)
			:FnOnWinEvent(window,&IWindowEventHandler::onWindowDestroy )
		{}
	};
	
}//namespace Blade


#endif // __Blade_Helpers_h__