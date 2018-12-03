/********************************************************************
	created:	2010/04/12
	filename: 	WindowEventDispatcher.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "WindowEventDispatcher.h"
#include <interface/public/window/Helpers.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	WindowEventDispatcherBase::~WindowEventDispatcherBase()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WindowEventDispatcher::WindowEventDispatcher()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WindowEventDispatcher::~WindowEventDispatcher()
	{

	}

	/************************************************************************/
	/* IWindowEventHandler interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void WindowEventDispatcher::onWindowShow(IWindow* pwin)
	{
		for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinShow(pwin) );
	}


	//////////////////////////////////////////////////////////////////////////
	void WindowEventDispatcher::onWindowHide(IWindow* pwin)
	{
		for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinHide(pwin) );
	}


	//////////////////////////////////////////////////////////////////////////
	bool WindowEventDispatcher::onWindowResizing(IWindow* pwin,size_t width,size_t height)
	{
		return for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinResizing(pwin,width,height) ).mAllowResize;
	}


	//////////////////////////////////////////////////////////////////////////
	void WindowEventDispatcher::onWindowResized(IWindow* pwin)
	{
		for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinResized(pwin) );
	}


	//////////////////////////////////////////////////////////////////////////
	void WindowEventDispatcher::onWindowActivate(IWindow* pwin)
	{
		for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinActivate(pwin) );
	}


	//////////////////////////////////////////////////////////////////////////
	void WindowEventDispatcher::onWindowDeactivate(IWindow* pwin)
	{
		for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinDeactivate(pwin) );
	}


	//////////////////////////////////////////////////////////////////////////
	bool WindowEventDispatcher::onWindowClose(IWindow* pwin)
	{
		
		return for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinClose(pwin) ).getResult();
	}


	//////////////////////////////////////////////////////////////////////////
	void WindowEventDispatcher::onWindowDestroy(IWindow* pwin)
	{
		for_each(mHandlerList.begin(),mHandlerList.end(),FnOnWinDestroy(pwin) );
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/


	//////////////////////////////////////////////////////////////////////////
	bool		WindowEventDispatcher::addEventHandler(IWindowEventHandler*	handler)
	{
		return mHandlerList.insert(handler).second;
	}


	//////////////////////////////////////////////////////////////////////////
	bool		WindowEventDispatcher::removeEventHandler(IWindowEventHandler* handler)
	{
		return mHandlerList.erase(handler) == 1;
	}

}//namespace Blade