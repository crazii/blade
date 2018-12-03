/********************************************************************
	created:	2013/11/17
	filename: 	UIWindowBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/ui/UIWindowBase.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/ui/IUIService.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	UIWindowBase::UIWindowBase(const TString& type)
		:mType(type)
	{
		mIcon			= INVALID_ICON_INDEX;
		mWindow			= NULL;
		mKeyboard		= NULL;
		mMouse			= NULL;
		mUpdater		= NULL;
		mFrameworkData	= NULL;
		mUserData		= NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	UIWindowBase::~UIWindowBase()
	{
		if (mWindow != NULL)
			IWindowService::getSingleton().destroyWindow(mWindow->getUniqueName());
	}

	//////////////////////////////////////////////////////////////////////////
	bool	UIWindowBase::init(uintptr_t windowImpl, IconIndex icon, const TString& caption, bool createMouse, bool createKeyboard)
	{
		mIcon = icon;
		mWindow = IWindowService::getSingleton().attachWindow(windowImpl);
		mWindow->setCaption(caption);

		if( createMouse )
			mMouse = IUIService::getSingleton().createInputMouse(mWindow);

		if( createKeyboard )
			mKeyboard = IUIService::getSingleton().createInputKeyboard(mWindow);
		return true;
	}
	
}//namespace Blade