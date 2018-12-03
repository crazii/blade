/********************************************************************
	created:	2009/02/28
	filename: 	WindowSystem.h
	author:		Crazii
	
	purpose:	IWindowSystem implementation(header)
*********************************************************************/
#ifndef __Blade_WindowSystem_h__
#define __Blade_WindowSystem_h__
#include <windowsysteminterface/IWindowSystem.h>
#include <utility/BladeContainer.h>
#include <interface/IConfig.h>
#include "WindowEventDispatcher.h"

namespace Blade
{
	class WindowSystem : public IWindowSystem, public Singleton<WindowSystem>
	{
	public:
		static const TString WINDOW_SYSTEM_NAME;
	public:
		//ctor & dector
		WindowSystem();
		virtual ~WindowSystem();

		/************************************************************************/
		/*SubSystem spec Interface                                                                      */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName();


		/*
		@describe this would be called when added to Framework
		@param 
		@return 
		*/
		virtual void			install();

		/*
		@describe
		@param
		@return
		*/
		virtual void			uninstall();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			update();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*			createScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			destroyScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*			getScene(const TString& name) const;

		/************************************************************************/
		/*WindowService spec Interface                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t				getNumWindows() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		getWindow(index_t windowindex) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		getWindow(const TString& name) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				registerEventHandler(IWindowEventHandler *handler);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				removeEventHandler(IWindowEventHandler *handler);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		createWindow(const TString& winname);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				destroyWindow(const TString& winname);

		/*
		@describe 
		@param 
		@return 
		@remark uses NamedPointer to make the pointer explicit
		*/
		virtual const HWINDOW&		attachWindow(uintptr_t internalImpl);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				setAppWindow(const HWINDOW&  root);
		
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const HWINDOW&		getAppWindow() const;

		/*
		@describe set whether auto creating window in service initialization
		@param 
		@return 
		*/
		virtual void				setAutoCreatingWindow(bool autoWindow);

		/*
		@describe get whether auto creating window in service initialization
		@param 
		@return 
		*/
		virtual bool				isAutoCreatingWindow() const;

		/**
		@describe override window implementation type. by default window device implementation are configured through config dialog,
		but some plugin may override the implementation
		@param
		@return
		*/
		virtual void				setWindowTypeOverride(const TString& winType);

		/**
		@describe 
		@param
		@return
		*/
		virtual const TString&		getWindowType() const;
		
	protected:
		/** @brief generate config options */
		void			genterateOptions();

		typedef TStringMap< HWINDOW >		WindowList;

		WindowList				mWindows;
		HWINDOW					mRootWindow;
		HCONFIG					mWinOption;
		WindowEventDispatcher	mHandlers;
		TString					mWindowType;

		bool					mAutoCreateWindow;
	};//class WindowSystem
	
}//namespace Blade

#endif // __Blade_WindowSystem_h__