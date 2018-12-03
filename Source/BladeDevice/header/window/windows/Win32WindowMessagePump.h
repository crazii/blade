/********************************************************************
	created:	2011/05/07
	filename: 	Win32WindowMessagePump.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Win32WindowMessagePump_h__
#define __Blade_Win32WindowMessagePump_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <Singleton.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <utility/BladeContainer.h>
#include <input/windows/Win32GlobalKeyboardDevice.h>

namespace Blade
{
	class Win32MessagePump : public IWindowMessagePump, public Singleton<Win32MessagePump>
	{
	public:
		using Singleton<Win32MessagePump>::getSingleton;
		using Singleton<Win32MessagePump>::getSingletonPtr;
	public:
		Win32MessagePump();
		~Win32MessagePump();

		/************************************************************************/
		/* IWindowMessagePump interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual IKeyboardDevice*	getGlobalKeyboard() const {return Win32GlobalKeyboardDevice::getSingletonPtr();}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	processMessage();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	addMessageListener(IMessageListener* listener);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeMessageListener(IMessageListener* listener);

	protected:
		typedef Set<IMessageListener*>		MessageListenerSet;

		MessageListenerSet		mMessageListeners;
	};//class Win32MessagePump
	

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif // __Blade_Win32WindowMessagePump_h__