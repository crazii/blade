/********************************************************************
	created:	2011/05/14
	filename: 	DInput8Mouse.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <input/windows/DInput8Mouse.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)// conditional expression is constant
#endif

#define SHARE_DEVICE 1
#define BUFFER_MODE 0

#define SAMPLE_BUFFER_SIZE  (128)
#define CLICK_MOVE_TOLERANCE (2)
#define DBCLICK_DELTA		(500)

namespace Blade
{

	DirectInputMouse::SharedDeviceMap	DirectInputMouse::msSharedDevice;

	//////////////////////////////////////////////////////////////////////////
	DirectInputMouse::DirectInputMouse()
		:mUsedWinHandle(NULL)
		,mPreCaptureHandle(NULL)
		,mMovement(0,0,0)
		,mLastTimeStamp(0)
		,mTimeStamp(0)
		,mShareDevice(false)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	DirectInputMouse::~DirectInputMouse()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputMouse::open()
	{
		if( !::IsWindow(mWinHandle) )
			return false;

		HWND theHandle = mWinHandle;

		//assume a top level window
		while(::GetParent(theHandle) != NULL)
			theHandle = ::GetParent(theHandle);
		mUsedWinHandle = theHandle;

#if SHARE_DEVICE
		if( theHandle != mWinHandle )
		{
			mShareDevice = true;
			mDI8Device = msSharedDevice.addRef(theHandle);
			if(mDI8Device != NULL)
				return true;
		}
#endif
		mShareDevice = false;

		bool result = this->createDI8Mouse();
		if( !result )
			return false;

		HRESULT hr = mDI8Device->SetCooperativeLevel(theHandle,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}

		hr = mDI8Device->SetDataFormat(&c_dfDIMouse);
		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		//set buffer size
#if BUFFER_MODE
		dipdw.dwData            = SAMPLE_BUFFER_SIZE;
#else
		dipdw.dwData            = 0;
#endif
		hr = mDI8Device->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph);
		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}
		//set axis mode
		dipdw.dwData = DIPROPAXISMODE_REL;
		hr = mDI8Device->SetProperty(DIPROP_AXISMODE,&dipdw.diph);
		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}

		hr = mDI8Device->Acquire();

		if( SUCCEEDED(hr) )
		{
#if SHARE_DEVICE
			if( theHandle != mWinHandle )
			{
				result = msSharedDevice.add(theHandle,mDI8Device);
				assert(result);
				BLADE_UNREFERENCED(result);
			}
#endif
			return true;
		}
		else
		{
			this->releaseDI8Device();
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputMouse::isOpen() const
	{
		return ::IsWindow(mWinHandle) && mDI8Device != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	DirectInputMouse::close()
	{
#if SHARE_DEVICE
		if(mShareDevice)
		{
			bool ret = msSharedDevice.removeRef(mUsedWinHandle, mDI8Device);
			assert(ret); BLADE_UNREFERENCED(ret);
			mDI8Device = NULL;
		}
#endif
		mWinHandle = NULL;
		mUsedWinHandle = NULL;
		this->releaseDI8Device();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputMouse::reset()
	{
		mMovement.x = mMovement.y = mMovement.z = 0;
		mLastTimeStamp = mTimeStamp = 0;

		for( int i = MSB_BEGIN; i < MSB_COUNT; ++i )
		{
			mPrevButtonState[i] = mButtonState[i] = MBS_UP;
			mHistroy[i].clear();
		}
		//this->show();

		//flush buffer
		if(mDI8Device != NULL)
		{
			DWORD dwItems = INFINITE; 
			mDI8Device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwItems, 0);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputMouse::update()
	{
		if( mDI8Device == NULL )
			return false;

		if( !::IsWindow(mWinHandle) )
		{
			if( mUsedWinHandle == NULL )
				//not shared, release
				this->releaseDI8Device();
			else if( !::IsWindow(mUsedWinHandle)  )
				//shared, and shared window handle is invalid
			{
				msSharedDevice.erase(mUsedWinHandle);
				this->releaseDI8Device();
				mUsedWinHandle = NULL;
			}
				//shared, shared window handle is valid, just stop sharing
			else
			{
				bool result = msSharedDevice.removeRef(mUsedWinHandle,mDI8Device);
				assert(result);
				BLADE_UNREFERENCED(result);
				mDI8Device = NULL;
			}

			return false;
		}

		//skip update if the window is not focused
		if (::GetFocus() != mWinHandle)
		{
			this->reset();
			return true;
		}

		if(!msCursorVisible)
		{
			RECT rect;
			::GetWindowRect(mWinHandle,&rect);
			::SetCursorPos((rect.left+rect.right)/2, (rect.top+rect.bottom)/2);
		}

		return this->readMouseData();
	}

	/************************************************************************/
	/* IMouse Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	POINT3I			DirectInputMouse::getMouseMovement() const
	{
		return mMovement;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			DirectInputMouse::readMouseData()
	{
		//init data
		mMovement.x = mMovement.y = mMovement.z = 0;

		bool result = false;
#if BUFFER_MODE
		while(true)
		{
			//generate click/double click event
			for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
			{
				if( mHistroy[i].size() == mHistroy[i].capacity() )
					this->updateMouseButtonEvent(i,mHistroy[i]);
			}

			DIDEVICEOBJECTDATA od;
			DWORD dwElements = 1;   // number of items to be retrieved
			HRESULT hr = mDI8Device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);

			//get data failed
			if ( FAILED(hr) )
			{
				mDI8Device->Acquire();
				return false;
			}

			//no data
			else if( dwElements == 0 )
			{
				result = true;
				break;
			}

			int button = -1;
			DWORD buttonState = 0;
			DWORD time = 0;

 			switch (od.dwOfs) 
			{
				// Mouse horizontal motion
			case DIMOFS_X: 
				{
					mMovement.x += (int)od.dwData;

					//if there is any movement, flush history
					if( !Math::Equal<int>((int)od.dwData,0,CLICK_MOVE_TOLERANCE) )
					{
						for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
							this->updateMouseButtonEvent(i,mHistroy[i]);
					}
				}
				break;

				// Mouse vertical motion
			case DIMOFS_Y: 
				{
					mMovement.y += (int)od.dwData;
					if( !Math::Equal<int>((int)od.dwData,0,CLICK_MOVE_TOLERANCE) )
					{
						for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
							this->updateMouseButtonEvent(i,mHistroy[i]);
					}
				}
				break; 
			case DIMOFS_Z:
				{
					mMovement.z += (int)od.dwData;

					if( !Math::Equal<int>((int)od.dwData,0,CLICK_MOVE_TOLERANCE) )
					{
						for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
							this->updateMouseButtonEvent(i,mHistroy[i]);
					}
				}
				break;

				// left button
			case DIMOFS_BUTTON0:
					button = MSB_LEFT;
					buttonState = od.dwData;
					time = od.dwTimeStamp;
				break;
				// right button
			case DIMOFS_BUTTON1:
				button = MSB_RIGHT;
				buttonState = od.dwData;
				time = od.dwTimeStamp;
				break;
				// middle button
			case DIMOFS_BUTTON3:
				button = MSB_MIDDLE;
				buttonState = od.dwData;
				time = od.dwTimeStamp;
				break;
			default:
				break;
			}//switch
#else
		DIMOUSESTATE mouseState;
		HRESULT hr = mDI8Device->GetDeviceState(sizeof(mouseState), &mouseState);

		if ( FAILED(hr) )
		{
			//lost focus?
			mDI8Device->Acquire();
			for(int i = MSB_BEGIN; i < MSB_COUNT; ++i)
			{
				if( mButtonState[i] == MBS_DOWN )
					std::for_each(mListeners.begin(), mListeners.end(), Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonUp), (MSBUTTON)i));
			}
			return false;
		}
		result = true;

		mMovement.x = mouseState.lX;
		mMovement.y = mouseState.lY;
		mMovement.z = mouseState.lZ;

		for(int i = MSB_BEGIN; i < MSB_COUNT; ++i)
		{
			int button = i;
			DWORD buttonSate = mouseState.rgbButtons[i];
			DWORD time = ::GetTickCount();
#endif
			if (button != -1)
			{
				assert(button >= MSB_BEGIN && button < MSB_COUNT);
				mPrevButtonState[button] = mButtonState[button];
				ButtonHistory& history = mHistroy[button];
				bool buttonDown = (buttonSate & 0x80) != 0;

				bool inScope = true;
				POINT2I pos = this->getMousePosition();
				if ((pos.x <= 0 || pos.y <= 0 || pos.x >= (int)mWinWidth || pos.y >= (int)mWinHeight))
					inScope = false;
				else
				{
					POINT pt;
					::GetCursorPos(&pt);
					inScope = ::WindowFromPoint(pt) == mWinHandle;
				}
				HWND hWndCaptture = ::GetCapture();
				inScope = inScope || hWndCaptture == mWinHandle;

				if( !inScope)
					mHistroy->clear();
				else if (buttonDown && mPrevButtonState[button] == MBS_UP)
				{
					mLastTimeStamp = mTimeStamp;
					mTimeStamp = time;
					bool bKeepData = std::for_each(mListeners.begin(), mListeners.end(),
						Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonDown), (MSBUTTON)button)).getResult();

					if (bKeepData)
					{
						mButtonState[button] = MBS_DOWN;
						history.push_back(MBS_DOWN);
					}

					if (::GetCapture() != mWinHandle)
						mPreCaptureHandle = ::SetCapture(mWinHandle);
				}
				else if (!buttonDown && mPrevButtonState[button] == MBS_DOWN)
				{
					bool bKeepData = std::for_each(mListeners.begin(), mListeners.end(),
						Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonUp), (MSBUTTON)button)).getResult();

					if (bKeepData)
					{
						mButtonState[button] = MBS_UP;
						if (history.size() > 0)
						{
							if (history[history.size() - 1] == MBS_DOWN)
								history.push_back(MBS_UP);
						}
					}

					if (mPreCaptureHandle != NULL && mPreCaptureHandle != mWinHandle)
						::SetCapture(mPreCaptureHandle);
					else
						::ReleaseCapture();
					mPreCaptureHandle = NULL;
				}
			}
		}

		//check accumulation and flush history
		if( !Math::Equal<int16>( (int16)mMovement.x,0,CLICK_MOVE_TOLERANCE)
			|| !Math::Equal<int16>( (int16)mMovement.y,0,CLICK_MOVE_TOLERANCE)
			|| !Math::Equal<int16>( (int16)mMovement.y,0,CLICK_MOVE_TOLERANCE) )
		{
			for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
				this->updateMouseButtonEvent(i,mHistroy[i]);
		}
		else
		{
			bool bUpdateAndClear = false;
			int count = 0;
			for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
			{
				if( mHistroy[i].size() == 1)
					++count;
				else if( mHistroy[i].size() >= 2 )
				{
					bUpdateAndClear = true;
					break;
				}
			}

			//update the valid button and clear the others
			if( bUpdateAndClear || count >= 2 )
			{
				for( int i = MSB_BEGIN; i < MSB_COUNT; ++i)
					this->updateMouseButtonEvent(i,mHistroy[i]);
			}
		}

		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	void			DirectInputMouse::updateMouseButtonEvent(int buttonIndex,ButtonHistory& history)
	{
		assert( buttonIndex >= MSB_BEGIN && buttonIndex < MSB_COUNT );

		MSBUTTON index = (MSBUTTON)buttonIndex;

		if( history.size() == 0 )
		{

		}
		else if( history.size() == 1 )
			history.clear();
		else if( history.size() == 3 )
		{
			if( mTimeStamp - mLastTimeStamp < DBCLICK_DELTA )
			{
				std::for_each(mListeners.begin(),mListeners.end(), 
					std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonDBClick),index ) );
			}
			history.clear();
		}
		else
		{
			if( history[0] == MBS_DOWN && history[1] == MBS_UP )
			{
				std::for_each(mListeners.begin(),mListeners.end(), 
					std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonClick),index) );

				history.clear();
				//add down state for double click check
				history.push_back(MBS_DOWN);
			}
			else if( history[0] == MBS_DOWN && history[1] == MBS_DOWN )
			{
				if( mTimeStamp - mLastTimeStamp < DBCLICK_DELTA )
				{
					std::for_each(mListeners.begin(),mListeners.end(), 
						std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonDBClick),index ) );
				}

				history.clear();
			}
			else
				assert(false);
		}
	}	

}//namespace Blade


#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS