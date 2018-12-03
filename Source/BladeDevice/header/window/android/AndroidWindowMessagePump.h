/********************************************************************
	created:	2013/12/28
	filename: 	AndroidWindowMessagePump.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AndroidWindowMessagePump_h__
#define __Blade_AndroidWindowMessagePump_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <interface/public/window/IWindowMessagePump.h>
#include <Singleton.h>
#include <utility/BladeContainer.h>

#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>

namespace Blade
{

	class AndroidMessagePump : public IWindowMessagePump, public Singleton<AndroidMessagePump>
	{
	public:
		using Singleton<AndroidMessagePump>::getSingleton;
		using Singleton<AndroidMessagePump>::getSingletonPtr;
	public:
		AndroidMessagePump();
		~AndroidMessagePump();

		/************************************************************************/
		/* IWindowMessagePump interface                                                                     */
		/************************************************************************/
		/**
		@describe get global keyboard, can be NULL
		@param
		@return
		*/
		virtual IKeyboardDevice*	getGlobalKeyboard() const {return NULL;}

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

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		static int8_t	androidThreadMessage(struct android_app* android_app);

	protected:

		typedef Set<IMessageListener*>		MessageListenerSet;
		MessageListenerSet		mMessageListeners;

	};//class AndroidMessagePump
	
}//namespace Blade

#endif//BLADE_PLATFORM
#endif //  __Blade_AndroidWindowMessagePump_h__