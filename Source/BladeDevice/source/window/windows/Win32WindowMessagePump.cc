/********************************************************************
	created:	2011/05/07
	filename: 	Win32WindowMessagePump.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <window/windows/Win32WindowMessagePump.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Win32MessagePump::Win32MessagePump()
	{
		this->addMessageListener(Win32GlobalKeyboardDevice::getSingletonPtr());
	}

	//////////////////////////////////////////////////////////////////////////
	Win32MessagePump::~Win32MessagePump()
	{
		this->removeMessageListener(Win32GlobalKeyboardDevice::getSingletonPtr());
	}

	/************************************************************************/
	/* IWindowMessagePump interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	Win32MessagePump::processMessage()
	{
		bool idle = false;

		std::for_each(mMessageListeners.begin(),mMessageListeners.end(), std::mem_fun(&IMessageListener::preProcessMessages) );

		MSG msg;
		//update all other window / attached window msg
		while( ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE )
		{
			bool skip = false;
			for(MessageListenerSet::iterator i = mMessageListeners.begin(); i != mMessageListeners.end(); ++i )
			{
				IMessageListener* listener = *i;
				if( !listener->preProcessMessage(&msg) )
					skip = true;

				//check idle message
				if( !idle )
					idle = listener->isIdle();
			}

			if( !skip )
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			std::for_each(mMessageListeners.begin(),mMessageListeners.end(),
				std::bind2nd( std::mem_fun(&IMessageListener::postProcessMessage),&msg) );

		}

		//idle update (make it the same way MFC does)
		//important: idle update must happen right after all messages are processed
		bool SomeoneCares = true;
		while( SomeoneCares && idle && ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == FALSE )
		{
			SomeoneCares = false;
			for(MessageListenerSet::iterator i = mMessageListeners.begin(); i != mMessageListeners.end(); ++i )
			{
				IMessageListener* listener = *i;
				bool listenerContinue = true;
				if( listener->onIdleUpdate(listenerContinue) )
				{
					if( !listenerContinue )
						idle = false;

					SomeoneCares = true;
				}
			}
		}

		std::for_each(mMessageListeners.begin(),mMessageListeners.end(), std::mem_fun(&IMessageListener::postProcessMessages) );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32MessagePump::addMessageListener(IMessageListener* listener)
	{
		if( listener != NULL )
			return mMessageListeners.insert(listener).second;
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Win32MessagePump::removeMessageListener(IMessageListener* listener)
	{
		if( listener != NULL )
			return mMessageListeners.erase(listener) == 1;
		else
			return false;
	}

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS