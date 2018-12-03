/********************************************************************
	created:	2010/04/30
	filename: 	Win32MouseDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <interface/public/window/IWindow.h>

#include <input/windows/Win32MouseDevice.h>
#include <window/windows/Win32WindowMessagePump.h>
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4312)
#pragma warning(disable:4244)
#endif

#define DRAG_CLICK_TOLERANCE (1)

namespace Blade
{
	Win32MouseDevice::DeviceMap	Win32MouseDevice::msDeviceMap;

	//////////////////////////////////////////////////////////////////////////
	Win32MouseDevice::Win32MouseDevice()
		:mPrevWndProc(NULL)
		,mPreCaptureWindow(NULL)
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	Win32MouseDevice::~Win32MouseDevice()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	Win32MouseDevice::open()
	{
		if( !::IsWindow(mWinHandle) )
			return false;

		mPrevWndProc = (WNDPROC)::GetWindowLongPtr(mWinHandle,GWLP_WNDPROC);
		::SetWindowLongPtr(mWinHandle,GWLP_WNDPROC,(LONG_PTR)Win32MouseDevice::MouseProc);
		msDeviceMap[mWinHandle] = this;

		Win32MessagePump::getSingleton().addMessageListener(this);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32MouseDevice::isOpen() const
	{
		return ::IsWindow(mWinHandle) == TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	void	Win32MouseDevice::close()
	{
		msDeviceMap.erase(mWinHandle);
		if( mWinHandle != NULL && mPrevWndProc != NULL )
		{
			::SetWindowLongPtr(mWinHandle,GWLP_WNDPROC,(LONG_PTR)mPrevWndProc);
			mPrevWndProc = NULL;
			mWinHandle = NULL;
		}

		Win32MessagePump::getSingleton().removeMessageListener(this);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32MouseDevice::reset()
	{
		for( int i = MSB_BEGIN; i < MSB_COUNT; ++i )
			mPrevButtonState[i] = mButtonState[i] = MBS_UP;

		this->show();
		mPrevMousePos = this->getMousePosition();
		mIsDragging = false;
		mMouseMovement = POINT3I::ZERO;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32MouseDevice::update()
	{
		if( !::IsWindow(mWinHandle) )
			return false;

		if( !msCursorVisible )
		{
			this->setGlobalMousePosition(msMouseHidePos);
			POINT p;
			p.x = msMouseHidePos.x;
			p.y = msMouseHidePos.y;
			::ScreenToClient(mWinHandle, &p);

			mMousePos.x = p.x;
			mMousePos.y = p.y;
		}
		return true;
	}


	/************************************************************************/
	/* IMouse Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	POINT3I			Win32MouseDevice::getMouseMovement() const
	{
		return mMouseMovement;
	}

	/************************************************************************/
	/* IWindowMessagePump::IMessageListener interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void Win32MouseDevice::preProcessMessages()
	{
		mMouseMovement = POINT3I::ZERO;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Win32MouseDevice::checkDragging(bool testMove/* = false*/)
	{
		if( testMove )
		{
			if( !Math::Equal<short>((short)mMouseMovement.x, 0, DRAG_CLICK_TOLERANCE) || !Math::Equal<short>((short)mMouseMovement.y, 0, DRAG_CLICK_TOLERANCE) )
			{
				if( this->isButtonDown(MSB_LEFT) || this->isButtonDown(MSB_RIGHT) || this->isButtonDown(MSB_MIDDLE) )
					mIsDragging = true;
			}
		}
		else if( this->isButtonDown(MSB_LEFT) || this->isButtonDown(MSB_RIGHT) || this->isButtonDown(MSB_MIDDLE) )
				mIsDragging = true;
		else
			mIsDragging = false;
	}

	//////////////////////////////////////////////////////////////////////////
	LRESULT CALLBACK	Win32MouseDevice::MouseProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam )
	{
		DeviceMap::iterator i = msDeviceMap.find(hwnd);
		if( i == msDeviceMap.end() || i->second->mPrevWndProc == NULL )
		{
			BLADE_EXCEPT(EXC_NEXIST,BTString("unkown error,window handle not registered.") );
		}

		Win32MouseDevice* mouse = i->second;
		//call default window proc
		LRESULT result =  mouse->mPrevWndProc(hwnd,uMsg,wParam,lParam);

		int button = -1;
		bool buttonDown = false;

		switch( uMsg )
		{
		case WM_MOUSEMOVE:
			{
				mouse->mPrevMousePos = mouse->mMousePos;

				WORD xPos = LOWORD(lParam); 
				WORD yPos = HIWORD(lParam); 

				mouse->mMousePos.x = (short)xPos;
				mouse->mMousePos.y = (short)yPos;

				POINT2I movement = mouse->mMousePos - mouse->mPrevMousePos;
				mouse->mMouseMovement.x += movement.x;
				mouse->mMouseMovement.y += movement.y;
				mouse->checkDragging(true);
				if( ::GetCapture() == mouse->mWinHandle && 
					!(mouse->isButtonDown(MSB_LEFT) || mouse->isButtonDown(MSB_RIGHT) || mouse->isButtonDown(MSB_MIDDLE)) )
				{
					mouse->releaseCapture();
				}
			}
			break;
		case WM_MOUSEWHEEL:
			{
				HWND hwndFocus = GetFocus();
				if( hwnd == hwndFocus )
				{
					//short wKeys = GET_KEYSTATE_WPARAM(wParam);
					short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

					mouse->mPrevMousePos = mouse->mMousePos;
					mouse->mMouseMovement.z = zDelta;
				}
			}
			break;
		case WM_LBUTTONDOWN:
			{
				button = MSB_LEFT;
				buttonDown = true;
			}
			break;
		case WM_LBUTTONUP:
			{
				button = MSB_LEFT;
				buttonDown = false;
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				bool bNotify = std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
					Fn::BoolBind2nd( Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonDown),MSB_LEFT) ).getResult();

				if( bNotify )
				{
					std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
						std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonDBClick),MSB_LEFT) );
				}
			}
			break;
		case WM_RBUTTONDOWN:
			{
				button = MSB_RIGHT;
				buttonDown = true;
			}
			break;
		case WM_RBUTTONUP:
			{
				button = MSB_RIGHT;
				buttonDown = false;
			}
			break;
		case WM_RBUTTONDBLCLK:
			{
				bool bNotify = std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
					Fn::BoolBind2nd( Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonDown),MSB_RIGHT) ).getResult();

				if( bNotify )
				{
					std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
						std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonDBClick),MSB_RIGHT) );
				}
			}
			break;
		case WM_MBUTTONDOWN:
			{
				button = MSB_RIGHT;
				buttonDown = true;
			}
			break;
		case WM_MBUTTONUP:
			{
				button = MSB_RIGHT;
				buttonDown = false;
			}
			break;
		case WM_MBUTTONDBLCLK:
			{
				bool bNotify = std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
					Fn::BoolBind2nd( Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonDown),MSB_MIDDLE) ).getResult();

				if( bNotify )
				{
					std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
						std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonDBClick),MSB_MIDDLE) );
				}
			}
			break;
		case WM_KILLFOCUS:
			{
				//
				if( mouse->isButtonDown(MSB_LEFT) )
				{
					std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
						Fn::BoolBind2nd( Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonUp),MSB_LEFT) ).getResult();
				}

				if( mouse->isButtonDown(MSB_RIGHT) )
				{
					std::for_each(mouse->mListeners.begin(),mouse->mListeners.end(), 
						Fn::BoolBind2nd( Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonUp),MSB_RIGHT) ).getResult();
				}

				mouse->reset();

				mouse->releaseCapture();

				if( Win32MouseDevice::msCursorVisible )
					Win32MouseDevice::showCursor();
			}
			break;
		case WM_SETFOCUS:
			{
				POINT p;
				//mouse->mPreCaptureWindow = ::SetCapture( mouse->mWinHandle );
				::GetCursorPos(&p);
				::ScreenToClient(mouse->mWinHandle,&p);
				mouse->mPrevMousePos.x = p.x;
				mouse->mPrevMousePos.y = p.y;
				mouse->mMousePos = mouse->mPrevMousePos;// = *(POINT2I*)&p;

				if( !Win32MouseDevice::msCursorVisible )
					::ShowCursor(FALSE);
			}
			break;
		}

		if (button != -1)
		{
			assert(button >= MSB_BEGIN && button < MSB_COUNT);
			if (buttonDown)
			{
				bool bKeepData = std::for_each(mouse->mListeners.begin(), mouse->mListeners.end(),
					Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonDown), MSBUTTON(button))).getResult();

				if (bKeepData)
				{
					mouse->mPrevButtonState[button] = mouse->mButtonState[button];
					mouse->mButtonState[button] = MBS_DOWN;
				}
				
				mouse->setCapture();
			}
			else
			{
				bool bKeepData = std::for_each(mouse->mListeners.begin(), mouse->mListeners.end(),
					Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonUp), MSBUTTON(button))).getResult();

				if (bKeepData)
				{
					mouse->mPrevButtonState[button] = mouse->mButtonState[button];
					mouse->mButtonState[button] = MBS_UP;
				}

				if (!mouse->mIsDragging)
				{
					if (mouse->mPrevButtonState[button] == MBS_DOWN)
					{
						std::for_each(mouse->mListeners.begin(), mouse->mListeners.end(),
							std::bind2nd(std::mem_fun(&IMouse::IListener::onMouseButtonClick), MSBUTTON(button)));
					}
				}

				mouse->checkDragging();
				mouse->releaseCapture();
			}
		}

		return result;
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS