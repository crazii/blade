/********************************************************************
	created:	2011/05/14
	filename: 	DInput8Keyboard.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <interface/public/window/IWindow.h>
#include <input/windows/DInput8Keyboard.h>
#include "DInput8KeyMap.h"

#define SAMPLE_BUFFER_SIZE (16)

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)// conditional expression is constant
#endif

namespace Blade
{
	//
	DirectInputKeyboard::SharedDeviceMap DirectInputKeyboard::msSharedDevices;


	//////////////////////////////////////////////////////////////////////////
	DirectInputKeyboard::DirectInputKeyboard()
		:mWinHandle(NULL)
		,mUsedWinHandle(NULL)
		,mShareDevice(false)
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	DirectInputKeyboard::~DirectInputKeyboard()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputKeyboard::open()
	{
		if( !::IsWindow(mWinHandle) )
			return false;

		HWND theHandle = mWinHandle;

		//assume a top level window
		while(::GetParent(theHandle) != NULL)
			theHandle = ::GetParent(theHandle);
		mUsedWinHandle = theHandle;

		if( theHandle != mWinHandle )
		{
			mShareDevice = true;
			mDI8Device = msSharedDevices.addRef(theHandle);
			if(mDI8Device != NULL)
				return true;
		}

		mShareDevice = false;
		bool result = this->createDI8Keyboard();
		if( !result )
			return false;

		HRESULT hr = mDI8Device->SetCooperativeLevel(theHandle,DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}

		hr = mDI8Device->SetDataFormat(&c_dfDIKeyboard);
		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}

		DIPROPDWORD dipdw;
		// the header
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		// the data
		dipdw.dwData            = SAMPLE_BUFFER_SIZE;

		//set buffer size
		hr = mDI8Device->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph);

		if( FAILED(hr) )
		{
			this->releaseDI8Device();
			return false;
		}

		hr = mDI8Device->Acquire();

		if( SUCCEEDED(hr) )
		{
			if( theHandle != mWinHandle )
			{
				result = msSharedDevices.add(theHandle,mDI8Device);
				assert( result );
				BLADE_UNREFERENCED(result);
			}
			return true;
		}
		else
		{
			this->releaseDI8Device();
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputKeyboard::isOpen() const
	{
		return ::IsWindow(mWinHandle) && mDI8Device != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	DirectInputKeyboard::close()
	{
		if(mShareDevice)
		{
			bool ret = msSharedDevices.removeRef(mUsedWinHandle, mDI8Device);
			assert(ret); BLADE_UNREFERENCED(ret);
			mDI8Device = NULL;
		}

		mWinHandle = NULL;
		mUsedWinHandle = NULL;
		this->releaseDI8Device();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputKeyboard::reset()
	{
		std::memset(mKeyState,KS_UP,sizeof(mKeyState));
		std::memset(mPrevKeyState,KS_UP,sizeof(mPrevKeyState));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DirectInputKeyboard::update()
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
				msSharedDevices.erase(mUsedWinHandle);
				this->releaseDI8Device();
				mUsedWinHandle = NULL;
			}
			//shared, shared window handle is valid, just stop sharing
			else
			{
				bool result = msSharedDevices.removeRef(mUsedWinHandle,mDI8Device);
				assert(result);
				BLADE_UNREFERENCED(result);
			}

			return false;
		}

		//skip update if the window is not focused
		if( ::GetFocus() != mWinHandle )
			return true;

		return this->readKeyboardData();
	}

	/************************************************************************/
	/* IKeyboard interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				DirectInputKeyboard::attach(IWindow* window)
	{
		if( window != NULL )
			mWinHandle = (HWND)(void*)window->getNativeHandle();
	}

	//////////////////////////////////////////////////////////////////////////
	bool				DirectInputKeyboard::addListener(IListener* listener)
	{
		if( listener == NULL )
			return false;
		else
			return mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				DirectInputKeyboard::removeListener(IListener* listener)
	{
		return mListeners.erase(listener) == 1;
	}

	/************************************************************************/
	/* IKeyboard interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	EKeyState			DirectInputKeyboard::getKeyState(EKeyCode code) const
	{
		assert( code >= 0 && code <= 256 );
		return mKeyState[code];
	}


	//////////////////////////////////////////////////////////////////////////
	bool				DirectInputKeyboard::isKeyPressed(EKeyCode code) const
	{
		assert( code >= 0 && code <= 256 );
		return mKeyState[code] == KS_UP && mPrevKeyState[code] == KS_DOWN;
	}

	//////////////////////////////////////////////////////////////////////////
	uint32				DirectInputKeyboard::getKeyModifier() const
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
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool				DirectInputKeyboard::readKeyboardData()
	{
		DIDEVICEOBJECTDATA keyData[SAMPLE_BUFFER_SIZE];
		bool result;
		while(true)
		{
			DWORD dwItems = 1;
			HRESULT hr = mDI8Device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), keyData, &dwItems, 0);

			if ( hr == DIERR_INPUTLOST ) 
			{
				mDI8Device->Acquire();
				result = false;
				break;
			}

			//get data failed
			if ( FAILED(hr) )
			{
				result = false;
				break;
			}
			//no data
			else if( dwItems == 0 )
			{
				result = true;
				break;
			}

			for( DWORD i = 0; i < dwItems; ++i )
			{
				//record previous state
				EKeyCode code = (EKeyCode)DI8KeyMap[keyData[i].dwOfs];

				if( code == KC_UNDEFINED )
				{
					assert( false );
					continue;
				}
				


				if ( (keyData[i].dwData & 0x80) && mKeyState[code] == KS_UP )
				{
					bool record = true;
					if( mListeners.size() > 0 )
					{
						uint32 modifierMask = modifierMask = this->getKeyModifier();
						for(ListenerList::iterator n = mListeners.begin(); n != mListeners.end(); ++n)
						{
							IKeyboard::IListener* listener = *n;
							if( !listener->onKeyDown(code,modifierMask) )
								record = false;
						}
					}

					if(record)
					{
						mPrevKeyState[ code ] = mKeyState[ code ];
						mKeyState[ code ] = KS_DOWN;
					}
				}
				else if ( !(keyData[i].dwData & 0x80) && mKeyState[code] == KS_DOWN )
				{
					bool record = true;
					if( mListeners.size() > 0 )
					{
						uint32 modifierMask = modifierMask = this->getKeyModifier();
						for(ListenerList::iterator n = mListeners.begin(); n != mListeners.end(); ++n)
						{
							IKeyboard::IListener* listener = *n;
							if( !listener->onKeyUp(code,modifierMask) )
								record = false;
						}
					}

					if(record)
					{
						mPrevKeyState[ code ] = mKeyState[ code ];
						mKeyState[ code ] = KS_UP;
					}
				}
			}//for

		}//while

		return result;
	}
	

}//namespace Blade




#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS