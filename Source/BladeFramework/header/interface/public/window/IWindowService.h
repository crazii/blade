/********************************************************************
	created:	2009/03/07
	filename: 	IWindowService.h
	author:		Crazii
	
	purpose:	IWindow service interface
*********************************************************************/
#ifndef __Blade_IWindowService_h__
#define __Blade_IWindowService_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/window/IWindow.h>
#include <interface/public/window/IWindowMessagePump.h>

namespace Blade
{
	class IWindowEventHandler;

	class IWindowService : public InterfaceSingleton<IWindowService>
	{
	public:
		virtual ~IWindowService()	{}
		/**
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getNumWindows() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		getWindow(index_t windowindex) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		getWindow(const TString& name) const = 0;

		/**
		@describe add handler to all window
		@param 
		@return 
		*/
		virtual bool				registerEventHandler(IWindowEventHandler *handler) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				removeEventHandler(IWindowEventHandler *handler) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		createWindow(const TString& winname) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				destroyWindow(const TString& winname) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		attachWindow(uintptr_t internalImpl) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				setAppWindow(const HWINDOW&  root) = 0;
		
		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		getAppWindow() const = 0;

		/**
		@describe set whether auto creating window in service initialization
		@param 
		@return 
		*/
		virtual void				setAutoCreatingWindow(bool autoWindow) = 0;

		/**
		@describe get whether auto creating window in service initialization
		@param 
		@return 
		*/
		virtual bool				isAutoCreatingWindow() const = 0;

		/**
		@describe override window implementation type. by default window device implementation are configured through config dialog,
		but some plugin may override the implementation
		@param
		@return
		*/
		virtual void				setWindowTypeOverride(const TString& winType) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const TString&		getWindowType() const = 0;

	};//class IWindowService

	extern template class BLADE_FRAMEWORK_API Factory<IWindowService>;

}//namespace Blade

#endif // __Blade_IWindowService_h__