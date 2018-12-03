/********************************************************************
	created:	2009/03/22
	filename: 	IWindowEventHandler.h
	author:		Crazii
	
	purpose:	IWindow event handler
*********************************************************************/
#ifndef __Blade_IWindowEventHandler_h__
#define __Blade_IWindowEventHandler_h__
#include <BladeBase.h>

namespace Blade
{

	class IWindow;

	class BLADE_BASE_API IWindowEventHandler
	{
	public:
		virtual ~IWindowEventHandler()				{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void onWindowShow(IWindow* pwin)	{BLADE_UNREFERENCED(pwin);}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void onWindowHide(IWindow* pwin)	{BLADE_UNREFERENCED(pwin);}

		/**
		@describe 
		@param 
		@return whether allow resize
		*/
		virtual bool onWindowResizing(IWindow* /*pwin*/,size_t /*width*/,size_t /*height*/)	{return true;}

		/**
		@describe 
		@param 
		@return
		@remark pwin 's width and pwin 's height is 0 indicates it is minimized
		*/
		virtual void onWindowResized(IWindow* pwin)		{BLADE_UNREFERENCED(pwin);}


		/**
		@describe obtain focus
		@param 
		@return 
		*/
		virtual void onWindowActivate(IWindow* pwin)	{BLADE_UNREFERENCED(pwin);}

		/**
		@describe lost focus
		@param 
		@return 
		*/
		virtual void onWindowDeactivate(IWindow* pwin)	{BLADE_UNREFERENCED(pwin);}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void onWindowCaptionChanged(IWindow* pwin)	{BLADE_UNREFERENCED(pwin);}

		/**
		@describe 
		@param 
		@return indicates whether close window indeed
		*/
		virtual bool onWindowClose(IWindow* pwin)		{BLADE_UNREFERENCED(pwin);return true;}

		/**
		@describe called when window created(re-created)
		@param
		@return
		*/
		virtual void onWindowCreated(IWindow* pwin)		{BLADE_UNREFERENCED(pwin);}

		/**
		@describe called on destroy
		@param 
		@return 
		*/
		virtual void onWindowDestroy(IWindow* pwin)		{BLADE_UNREFERENCED(pwin);}

		///*
		//@describe 
		//@param 
		//@return 
		//*/
		//virtual void onInputEvent(IWindow*,int type,int code) = 0;

	};//class IWindowEventHandler
	
}//namespace Blade

#endif // __Blade_IWindowEventHandler_h__