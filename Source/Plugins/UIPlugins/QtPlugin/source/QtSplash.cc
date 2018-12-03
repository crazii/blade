/********************************************************************
	created:	2016/07/24
	filename: 	QtSplash.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtSplash.h>
#include <interface/IEnvironmentManager.h>
#include <ConstDef.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QtSplash::QtSplash()
		:mSplash(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	QtSplash::~QtSplash()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void QtSplash::initialize(const TString& softInfo)
	{
		mInfoString = softInfo;
		mImagePath = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::STARTUP_IMAGE);
	}

	//////////////////////////////////////////////////////////////////////////
	void QtSplash::show(bool bShow)
	{
		if(bShow)
		{
			mSplash = QT_NEW QtSplashWindow();
			mSplash->show(mInfoString, mImagePath);
		}
		else if(mSplash != NULL)
		{
			mSplash->close();
			delete mSplash;
			mSplash = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtSplash::outputProgress(const TString& detail)
	{
		if(mSplash != NULL)
			mSplash->setDetailText(detail);
		IWindowMessagePump::getSingleton().processMessage();
	}

}//namespace Blade