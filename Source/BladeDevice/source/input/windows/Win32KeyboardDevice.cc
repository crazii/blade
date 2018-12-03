/********************************************************************
	created:	2010/04/30
	filename: 	Win32KeyboardDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <interface/public/window/IWindow.h>
#include <input/windows/Win32KeyMap.h>
#include <input/windows/Win32KeyboardDevice.h>

#include <window/windows/Win32WindowMessagePump.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4312)
#pragma warning(disable:4244)
#endif

namespace Blade
{
	Win32KeyboardDevice::DeviceMap	Win32KeyboardDevice::msDeviceMap;

	//////////////////////////////////////////////////////////////////////////
	Win32KeyboardDevice::Win32KeyboardDevice()
		:mWinHandle(NULL)
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	Win32KeyboardDevice::~Win32KeyboardDevice()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool	Win32KeyboardDevice::open()
	{
		if( !::IsWindow(mWinHandle) )
			return false;

		mPrevWndProc = (WNDPROC)::GetWindowLongPtr(mWinHandle,GWLP_WNDPROC);
		::SetWindowLongPtr(mWinHandle,GWLP_WNDPROC,(LONG_PTR)Win32KeyboardDevice::KeyProc);
		msDeviceMap[mWinHandle] = this;

		Win32MessagePump::getSingleton().addMessageListener(this);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32KeyboardDevice::isOpen() const
	{
		return ::IsWindow(mWinHandle) == TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	void	Win32KeyboardDevice::close()
	{
		msDeviceMap.erase(mWinHandle);
		if( mWinHandle != NULL && ::IsWindow(mWinHandle) && mPrevWndProc != NULL )
		{
			::SetWindowLongPtr(mWinHandle,GWLP_WNDPROC,(LONG_PTR)mPrevWndProc);
			mPrevWndProc = NULL;
			mWinHandle = NULL;
			Win32MessagePump::getSingleton().removeMessageListener(this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32KeyboardDevice::reset()
	{
		std::memset(mKeyState,KS_UP,sizeof(mKeyState));
		std::memset(mPrevKeyState,KS_UP,sizeof(mPrevKeyState));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32KeyboardDevice::update()
	{
		return true;
	}

	/************************************************************************/
	/* IKeyboardDevice interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void				Win32KeyboardDevice::attach(IWindow* window)
	{
		if( window != NULL )
			mWinHandle = (HWND)(void*)window->getNativeHandle();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Win32KeyboardDevice::addListener(IListener* listener)
	{
		if( listener == NULL )
			return false;

		return mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Win32KeyboardDevice::removeListener(IListener* listener)
	{
		return mListeners.erase(listener) == 1;
	}

	/************************************************************************/
	/* IKeyboard interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	EKeyState			Win32KeyboardDevice::getKeyState(EKeyCode code) const
	{
		assert( code >= 0 && code <= 256 );
		return (EKeyState)mKeyState[code];
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Win32KeyboardDevice::isKeyPressed(EKeyCode code) const
	{
		assert( code >= 0 && code <= 256 );
		return mKeyState[code] == KS_UP && mPrevKeyState[code] == KS_DOWN;
	}

	//////////////////////////////////////////////////////////////////////////
	uint32				Win32KeyboardDevice::getKeyModifier() const
	{
		uint32 modifierMask = 0;
		if( this->mKeyState[ KC_CTRL ] == KS_DOWN )
			modifierMask |= KM_CTRL;
		if( this->mKeyState[ KC_ALT ] == KS_DOWN )
			modifierMask |= KM_ALT;
		if( this->mKeyState[ KC_SHIFT ] == KS_DOWN )
			modifierMask |= KM_SHIFT;

		return modifierMask;
	}

	/************************************************************************/
	/* IWindowMessagePump::IMessageListener interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void Win32KeyboardDevice::preProcessMessages()
	{
		std::memcpy(mPrevKeyState, mKeyState, sizeof(mPrevKeyState) );
	}

	//////////////////////////////////////////////////////////////////////////
	LRESULT CALLBACK	Win32KeyboardDevice::KeyProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam )
	{
		DeviceMap::iterator i = msDeviceMap.find(hwnd);
		if( i == msDeviceMap.end() || i->second->mPrevWndProc == NULL )
			BLADE_EXCEPT(EXC_NEXIST,BTString("unkown error,window handle not registered.") );

		Win32KeyboardDevice* key = i->second;

		switch( uMsg )
		{
		case WM_KEYDOWN:
			{
				EKeyCode code = KeyMap[wParam];
				if( key->mKeyState[ code ] == KS_UP )
				{

					bool record = true;
					if( key->mListeners.size() > 0 )
					{
						uint32 modifierMask = key->getKeyModifier();
						for(Win32KeyboardDevice::ListenerList::iterator n = key->mListeners.begin(); n != key->mListeners.end(); ++n)
						{
							IKeyboard::IListener* listener = *n;
							if( !listener->onKeyDown(code,modifierMask) )
								record = false;

							//listener may call exit(such as a close hot key processing),so we don't continue
							if( !::IsWindow(hwnd) )
								return 0;
						}
					}

					if( record )
					{
						key->mKeyState[ code ] = KS_DOWN;
						key->mPrevKeyState[ code ] = (lParam&(1<<30))?KS_DOWN:KS_UP;
					}

				}
			}
			break;
		case WM_KEYUP:
			{
				EKeyCode code = KeyMap[wParam];
				if( key->mKeyState[ code ] == KS_DOWN )
				{

					bool record = true;
					if( key->mListeners.size() > 0 )
					{
						uint32 modifierMask = key->getKeyModifier();
						for(Win32KeyboardDevice::ListenerList::iterator n = key->mListeners.begin(); n != key->mListeners.end(); ++n)
						{
							IKeyboard::IListener* listener = *n;
							if( !listener->onKeyUp(code,modifierMask) )
								record = false;

							if( !::IsWindow(hwnd) )
								return 0;
						}
					}

					if( record )
					{
						key->mKeyState[ code ] = KS_UP;
						key->mPrevKeyState[ code ] = KS_DOWN;
					}

				}
			}
			break;
		case WM_SETFOCUS: case WM_KILLFOCUS:
			{
				key->reset();
			}
			break;
		}

		return key->mPrevWndProc(hwnd,uMsg,wParam,lParam);
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS