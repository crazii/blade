/********************************************************************
	created:	2011/09/11
	filename: 	SplashOutput.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SplashOutput_h__
#define __Blade_SplashOutput_h__
#include <ConstDef.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <interface/public/IStartupOutput.h>
#include <ui/private/UISplashWindow.h>

namespace Blade
{
	class SplashOutput : public IStartupOutput, public Singleton<SplashOutput>
	{
	public:
		using Singleton<SplashOutput>::getSingleton;
		using Singleton<SplashOutput>::getSingletonPtr;
	public:
		SplashOutput()	:mSplashWindow(NULL),mShow(false)	{}
		~SplashOutput()	{this->clearup();}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void initialize(const TString& softInfo)
		{
			AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
			mStartupInfo = softInfo;
			mImage = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::STARTUP_IMAGE);
			if( mSplashWindow )
			{
				mSplashWindow->setStartupInformation( softInfo );
				mSplashWindow->setSplashImage(mImage);
			}
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void show(bool bShow)
		{
			AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

			mShow = bShow;
			if( mSplashWindow != NULL )
				mSplashWindow->ShowWindow( bShow ? SW_SHOW : SW_HIDE);
			if( !bShow ) //delete window content after hide 
				this->clearup();
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void outputProgress(const TString& detail)
		{
			AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
			if( mSplashWindow != NULL )
			{
				mSplashWindow->setInitDetails(detail);
				//process message to prevent no response
				Blade::IWindowMessagePump::getSingleton().processMessage();
			}
		}

		/** @brief  */
		inline void initialize(CUISplashWindow* window)
		{
			AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
			mSplashWindow = window;
			if( mShow && mSplashWindow != NULL )
				mSplashWindow->ShowWindow( SW_SHOW );
		}

		/** @brief  */
		inline void clearup()
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			BLADE_DELETE mSplashWindow;
			mSplashWindow = NULL;
		}

		/** @brief  */
		inline CUISplashWindow*	getSplashWindow()
		{
			return mSplashWindow;
		}

	protected:
		CUISplashWindow*	mSplashWindow;
		bool			mShow;
		TString			mStartupInfo;
		TString			mImage;
	};

	

}//namespace Blade



#endif // __Blade_SplashOutput_h__