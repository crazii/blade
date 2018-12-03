/********************************************************************
	created:	2011/05/07
	filename: 	Win32GlobalKeyboardDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <input/windows/Win32KeyMap.h>
#include <input/windows/Win32GlobalKeyboardDevice.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Win32GlobalKeyboardDevice::Win32GlobalKeyboardDevice()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Win32GlobalKeyboardDevice::~Win32GlobalKeyboardDevice()
	{
		
	}

	/************************************************************************/
	/* IKeyboard interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	EKeyState			Win32GlobalKeyboardDevice::getKeyState(EKeyCode code) const
	{
		return mKey.getKeyState(code);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Win32GlobalKeyboardDevice::isKeyPressed(EKeyCode code) const
	{
		return mKey.isKeyPressed(code);
	}

	//////////////////////////////////////////////////////////////////////////
	uint32				Win32GlobalKeyboardDevice::getKeyModifier() const
	{
		return mKey.getKeyModifier();
	}

	/************************************************************************/
	/* IKeyboardDevice interface                                                                     */
	/************************************************************************/


	//////////////////////////////////////////////////////////////////////////
	void				Win32GlobalKeyboardDevice::attach(IWindow* window)
	{
		BLADE_UNREFERENCED(window);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Win32GlobalKeyboardDevice::addListener(IListener* listener)
	{
		return mKey.addListener(listener);
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Win32GlobalKeyboardDevice::removeListener(IListener* listener)
	{
		return mKey.removeListener(listener);
	}

	/************************************************************************/
	/* IWindowMessagePump::IMessageListener                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool Win32GlobalKeyboardDevice::preProcessMessage(void* msg)
	{
		MSG* pMSG = (MSG*)msg;
		UINT uMsg;
		WPARAM wParam;
		LPARAM lParam;
		uMsg = pMSG->message;
		wParam = pMSG->wParam;
		lParam = pMSG->lParam;

		Win32KeyboardDevice* key = &mKey;
		key->preProcessMessage(msg);

		bool record = true;

		switch( uMsg )
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			{
				uint32 modifierMask = 0;

				if( key->mListeners.size() > 0 )
				{
					modifierMask = key->getKeyModifier();
					for(Win32KeyboardDevice::ListenerList::iterator n = key->mListeners.begin(); n != key->mListeners.end(); ++n)
					{
						IKeyboard::IListener* listener = *n;
						if( !listener->onKeyDown(KeyMap[wParam],modifierMask) )
							record = false;
					}
				}

				if( record )
				{
					key->mKeyState[ KeyMap[wParam] ] = KS_DOWN;
					key->mPrevKeyState[ KeyMap[wParam] ] = (char)( (lParam&(1<<30))?KS_DOWN:KS_UP);
				}
				else
					key->reset();
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
				uint32 modifierMask = 0;

				if( key->mListeners.size() > 0 )
				{
					modifierMask = key->getKeyModifier();
					for(Win32KeyboardDevice::ListenerList::iterator n = key->mListeners.begin(); n != key->mListeners.end(); ++n)
					{
						IKeyboard::IListener* listener = *n;
						if( !listener->onKeyUp(KeyMap[wParam],modifierMask) )
							record = false;
					}
				}

				if( record )
				{
					key->mKeyState[ KeyMap[wParam] ] = KS_UP;
					key->mPrevKeyState[ KeyMap[wParam] ] = KS_DOWN;
				}
				else
					key->reset();
			}
			break;
		case WM_SYSCHAR:
		case WM_ACTIVATEAPP:
		case WM_ACTIVATE:
		case WM_MOUSEACTIVATE:
		case WM_SETFOCUS:
			key->reset();
			break;
		default:
			break;
		}
		return record;
	}

}//namespace Blade



#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

