/********************************************************************
	created:	2011/05/07
	filename: 	IWindowMessagePump.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IWindowMessagePump_h__
#define __Blade_IWindowMessagePump_h__
#include <Handle.h>
#include <Factory.h>
#include <interface/InterfaceSingleton.h>

namespace Blade
{
	class IKeyboardDevice;

	class IWindowMessagePump : public InterfaceSingleton<IWindowMessagePump>
	{
	public:
		/// @note this class is for UI usage , do not use it unless you know what you're doing
		class IMessageListener
		{
		public:
			/** @brief  return true to continue message processing,
					false to skip this message */
			virtual bool preProcessMessage(void* msg) = 0;
			/** @brief called after processing each message */
			virtual void postProcessMessage(void* msg) = 0;
			/** @brief called on each loop before processing all messages */
			virtual void preProcessMessages() = 0;
			/** @brief called on each loop after processing all messages */
			virtual void postProcessMessages() = 0;

			/** @brief  */
			virtual bool isIdle()	{return false;}
			/** @brief  return false if don't care about the idle update process*/
			virtual bool onIdleUpdate(bool& bContinueUpdate)	{BLADE_UNREFERENCED(bContinueUpdate);return false;}
		};

	public:
		virtual ~IWindowMessagePump()	{}

		/**
		@describe get global keyboard, can be NULL
		@param
		@return
		*/
		virtual IKeyboardDevice*	getGlobalKeyboard() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	processMessage() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	addMessageListener(IMessageListener* listener) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeMessageListener(IMessageListener* listener) = 0;

	};//class IWindowMessagePump

	extern template class BLADE_BASE_API Factory<IWindowMessagePump>;
	typedef Factory<IWindowMessagePump> WindowMessagePumpFactory;

}//namespace Blade



#endif // __Blade_IWindowMessagePump_h__