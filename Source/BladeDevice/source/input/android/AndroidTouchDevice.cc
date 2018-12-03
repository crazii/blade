/********************************************************************
	created:	2013/12/29
	filename: 	AndroidTouchDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <input/android/AndroidTouchDevice.h>
#include <interface/public/window/IWindow.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	AndroidTouchDevice::AndroidTouchDevice()
		:mWindow(NULL)
		,mParent(NULL)
	{
		std::memset(&mDesc, 0, sizeof(mDesc));
		mOpened = false;
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	AndroidTouchDevice::~AndroidTouchDevice()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	AndroidTouchDevice::open()
	{
		mOpened = ( mWindow != NULL || mParent != NULL ) &&
			IWindowMessagePump::getSingleton().addMessageListener(this);
		return mOpened;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidTouchDevice::isOpen() const
	{
		return mOpened;
	}

	//////////////////////////////////////////////////////////////////////////
	void	AndroidTouchDevice::close()
	{
		if( mOpened )
			IWindowMessagePump::getSingleton().removeMessageListener(this);

		mWindow = NULL;
		mParent = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidTouchDevice::reset()
	{
		mState = TS_UP;
		mData.mPoint = POINT2(0,0);
		mData.mNPoint = POINT2(0,0);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	AndroidTouchDevice::update()
	{
		return true;
	}

	/************************************************************************/
	/* ITouchView specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const ITouchView::TOUCH_DESC&	AndroidTouchDevice::getDesc() const
	{
		return mDesc;
	}

	//////////////////////////////////////////////////////////////////////////
	ITouchView::TOUCH_STATE			AndroidTouchDevice::getState() const
	{
		return mState;
	}

	//////////////////////////////////////////////////////////////////////////
	const ITouchView::TOUCH_DATA&	AndroidTouchDevice::getPosition() const
	{
		return mData;
	}

	//////////////////////////////////////////////////////////////////////////
	ITouchView*		AndroidTouchDevice::createChildView(const TOUCH_DESC& /*desc*/)
	{
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AndroidTouchDevice::deleteChildView(ITouchView* /*view*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	ITouchView*		AndroidTouchDevice::getChildView(index_t index) const
	{
		BLADE_UNREFERENCED(index);
		return NULL;
	}

	/************************************************************************/
	/* ITouchDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		AndroidTouchDevice::attach(IWindow* window)
	{
		assert(window != NULL);
		assert( mParent == NULL );
		mWindow = (ANativeWindow*)(void*)window->getNativeHandle();

		mDesc.mLeft = 0;
		mDesc.mTop = 0;
		mDesc.mWidth = ::ANativeWindow_getWidth(mWindow);
		mDesc.mHeight = ::ANativeWindow_getHeight(mWindow);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AndroidTouchDevice::addListener(IListener* listener)
	{
		return listener != NULL && mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		AndroidTouchDevice::removeListener(IListener* listener)
	{
		return listener != NULL && mListeners.erase(listener) == 1;
	}

	/************************************************************************/
	/* IWindowMessagePump::IMessageListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool AndroidTouchDevice::preProcessMessage(void* msg)
	{
		android_poll_source* source = (android_poll_source*)msg;
		if( source->id == LOOPER_ID_INPUT )
		{
			assert(mWindow != NULL);
			AInputEvent* event = source->event;
			int32_t type = ::AInputEvent_getType(event);

			if( type == AINPUT_EVENT_TYPE_MOTION )
			{
				int32_t action = AMotionEvent_getAction(event);
				size_t pointerCount = AMotionEvent_getPointerCount(event);//multi touch

				size_t index = (size_t)( (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT );
				action &= AMOTION_EVENT_ACTION_MASK;

				if( action == AMOTION_EVENT_ACTION_POINTER_DOWN )
				{
					//non-primary touch pointer 
					int32_t id = AMotionEvent_getPointerId(event, index);
					int x = round( AMotionEvent_getX(event, index) );
					int y = round( AMotionEvent_getY(event, index) );
				}
				else if( action == AMOTION_EVENT_ACTION_POINTER_UP )
				{
					//non-primary touch pointer 
					int32_t id = AMotionEvent_getPointerId(event, index);
					int x = round( AMotionEvent_getX(event, index) );
					int y = round( AMotionEvent_getY(event, index) );
				}
				else
				{
					for(size_t i = 0; i < pointerCount; ++i)
					{
						int32_t id = AMotionEvent_getPointerId(event, i);
						int x = round( AMotionEvent_getX(event, i) );
						int y = round( AMotionEvent_getY(event, i) );

						if( action == AMOTION_EVENT_ACTION_DOWN )
						{
						}
						else if( action == AMOTION_EVENT_ACTION_UP )
						{
						}
						else if( action == AMOTION_EVENT_ACTION_MOVE )
						{
						}
						else if( action == AMOTION_EVENT_ACTION_CANCEL )
						{
						}
					}//for
				}
			}
			else if( type == AINPUT_EVENT_TYPE_KEY )
			{
			}

		}
		return false;
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_ANDROID