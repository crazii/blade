/********************************************************************
	created:	2011/05/07
	filename: 	Win32GlobalKeyboardDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Win32GlobalKeyboardDevice_h__
#define __Blade_Win32GlobalKeyboardDevice_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <Singleton.h>
#include <interface/public/input/IKeyboardDevice.h>
#include "Win32KeyboardDevice.h"


namespace Blade
{
	class Win32GlobalKeyboardDevice : public IKeyboardDevice, public IWindowMessagePump::IMessageListener, public Singleton<Win32GlobalKeyboardDevice>
	{
	public:
		Win32GlobalKeyboardDevice();
		~Win32GlobalKeyboardDevice();

		/************************************************************************/
		/* IKeyboard interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual EKeyState			getKeyState(EKeyCode code) const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool				isKeyPressed(EKeyCode code) const;

		/*
		@describe 
		@param 
		@return EKeyModifier
		*/
		virtual uint32				getKeyModifier() const;

		/************************************************************************/
		/* IKeyboardDevice interface                                                                     */
		/************************************************************************/

		/** @brief open a device */
		virtual bool	open()	{return true;}

		/** @brief check if it is open */
		virtual bool	isOpen() const {return true;}

		/** @brief close the device */
		virtual void	close()	 {}

		/** @brief reset the device */
		virtual bool	reset()	 {return mKey.reset();}

		/** @brief update data(I/O) */
		virtual bool	update() {return mKey.update();}

		/*
		@describe
		@param
		@return
		*/
		virtual void				attach(IWindow* window);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				addListener(IListener* listener);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool				removeListener(IListener* listener);

		/************************************************************************/
		/* IWindowMessagePump::IMessageListener                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool preProcessMessage(void* msg);
		/** @brief  */
		virtual void postProcessMessage(void* msg)			{mKey.postProcessMessage(msg);}
		/** @brief  */
		virtual bool isIdle()								{return mKey.isIdle();}
		/** @brief return true if need continue update */
		virtual bool onIdleUpdate(bool& bContinueUpdate)	{return mKey.onIdleUpdate(bContinueUpdate);}
		/** @brief called on each loop before processing all messages */
		virtual void preProcessMessages()					{ mKey.preProcessMessages();}
		/** @brief called on each loop after processing all messages */
		virtual void postProcessMessages()					{ mKey.postProcessMessages();}

	protected:
		Win32KeyboardDevice		mKey;
	};
	

}//namespace Blade




#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif // __Blade_Win32GlobalKeyboardDevice_h__


