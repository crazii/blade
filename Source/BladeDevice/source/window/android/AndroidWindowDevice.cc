/********************************************************************
	created:	2013/12/28
	filename: 	AndroidWindowDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <interface/public/window/IWindowEventHandler.h>
#include <interface/public/window/Helpers.h>
#include <window/android/AndroidWindowDevice.h>

#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>

namespace Blade
{
	const TString AndroidWindowDevice::ANDROID_WINDOW_TYPE = BTString("AndroidNative");

	//////////////////////////////////////////////////////////////////////////
	AndroidWindowDevice::AndroidWindowDevice()
		:mNativeHandle(NULL)
		,mColorDepth(32)
		,mVisible(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	AndroidWindowDevice::~AndroidWindowDevice()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	AndroidWindowDevice::open()
	{
		if( mNativeHandle == NULL )
		{
			IWindowMessagePump::getSingleton().addMessageListener(this);

			//check if we missed the message
			//this could happen when device created after APP_CMD_INIT_WINDOW,
			//i.e. IPlatformManager::getSingleton().showMessageBox()'s internal message loop before framework initialization
			ANativeWindow* window = PlatformData::getSingleton().getAndroidApp()->window;
			if( window != NULL )
			{
				this->attachWindow((uintptr_t)(void*)window);
				std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinCreate(this) );
				std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinResized(this) );
			}
			else
			{
				//update message to obtain window
				while( mNativeHandle == NULL )
					IWindowMessagePump::getSingleton().processMessage();
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidWindowDevice::isOpen() const
	{
		return mNativeHandle != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	AndroidWindowDevice::close()
	{
		IWindowMessagePump::getSingleton().removeMessageListener(this);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidWindowDevice::reset()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidWindowDevice::update()
	{
		return true;
	}

	/************************************************************************/
	/*IWindow specs                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const  TString&	AndroidWindowDevice::getUniqueName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::setSize(size_t width,size_t height)
	{
		BLADE_UNREFERENCED(width);
		BLADE_UNREFERENCED(height);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t AndroidWindowDevice::getWidth() const
	{
		if( mNativeHandle != NULL )
			return ::ANativeWindow_getWidth(mNativeHandle);
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t AndroidWindowDevice::getHeight() const
	{
		if( mNativeHandle != NULL )
			return ::ANativeWindow_getHeight(mNativeHandle);
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::setCaption(const TString& caption)
	{
		//TODO: log: warning
		BLADE_UNREFERENCED(caption);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& AndroidWindowDevice::getCaption() const
	{
		return TString::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::setVisibility(bool visibile)
	{
		BLADE_UNREFERENCED(visibile);
	}

	//////////////////////////////////////////////////////////////////////////
	bool AndroidWindowDevice::isVisible() const
	{
		return mNativeHandle != NULL && mVisible;
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::setFullScreen(bool fullscreen)
	{
		BLADE_UNREFERENCED(fullscreen);
	}

	//////////////////////////////////////////////////////////////////////////
	bool AndroidWindowDevice::getFullScreen() const
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	int  AndroidWindowDevice::getColorDepth() const
	{
		return mColorDepth;
	}

	//////////////////////////////////////////////////////////////////////////
	uintptr_t AndroidWindowDevice::getNativeHandle() const
	{
		return (uintptr_t)mNativeHandle;
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::setParent(uintptr_t)
	{
		assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	uintptr_t AndroidWindowDevice::getParent() const
	{
		return (uintptr_t)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool AndroidWindowDevice::closeWindow()
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool AndroidWindowDevice::addEventHandler(IWindowEventHandler* handler)
	{
		return handler != NULL && mNativeHandlers.insert(handler).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool AndroidWindowDevice::removeEventHandler(IWindowEventHandler* handler)
	{
		return handler != NULL && mNativeHandlers.erase(handler) == 1;
	}

	/************************************************************************/
	/* IWindowDevice interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::attachWindow(uintptr_t internalImpl)
	{
		assert( mNativeHandle == NULL);
		ANativeWindow* window = static_cast<ANativeWindow*>( (void*)internalImpl);
		assert(window != NULL);
		mNativeHandle = window;
		mName = TStringHelper::fromUInt( uintptr_t(mNativeHandle) );
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::setColorDepth(int depth)
	{
		mColorDepth = depth;
	}

	//////////////////////////////////////////////////////////////////////////
	void AndroidWindowDevice::destroyWindow()
	{

	}


	/************************************************************************/
	/* IWindowMessagePump::IMessageListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool AndroidWindowDevice::preProcessMessage(void* msg)
	{
		android_poll_source* source = (android_poll_source*)msg;
		if( source->id == LOOPER_ID_MAIN )
		{
			int32_t cmd = source->cmd;
			switch (cmd)
			{
			case APP_CMD_INIT_WINDOW:
				// The window is being shown, get it ready.
				{
					assert( source->app->window != NULL );
					assert( mNativeHandle == NULL || mNativeHandle == source->app->window );
					if( mNativeHandle == NULL )
					{
						this->attachWindow((uintptr_t)(void*)source->app->window);

						std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinCreate(this) );
						std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinResized(this) );
					}
				}
				break;
			case APP_CMD_TERM_WINDOW:
				// The window is being hidden or closed, clean it up.
				{
					if( mNativeHandle != NULL )
					{
						//make a copy: mNativeHandlers element may be deleted on FnOnWinClose/FnOnWinDestroy
						TempSet<IWindowEventHandler*> copy(mNativeHandlers.begin(), mNativeHandlers.end());
						std::for_each(copy.begin(), copy.end(), FnOnWinClose(this) );
						std::for_each(copy.begin(), copy.end(), FnOnWinDestroy(this) );
						mNativeHandle = NULL;
					}
				}
				break;
			case APP_CMD_GAINED_FOCUS:
				{
					//note: sometimes devices APP_CMD_GAINED_FOCUS is before APP_CMD_TERM_WINDOW
					if( mNativeHandle != NULL )
					{
						std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinShow(this) );
						std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinActivate(this) );
					}
					mVisible = true;
				}
				break;
			case APP_CMD_LOST_FOCUS:
				{
					//assert( mNativeHandle != NULL );
					mVisible = false;
					if( mNativeHandle != NULL )
					{
						std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinHide(this) );
						std::for_each(mNativeHandlers.begin(), mNativeHandlers.end(), FnOnWinDeactivate(this) );
					}
				}
				break;
			}
		}
		return true;
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM