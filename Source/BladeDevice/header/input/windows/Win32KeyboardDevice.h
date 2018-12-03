/********************************************************************
	created:	2010/04/29
	filename: 	Win32KeyboardDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#ifndef __Blade_Win32KeyboardDevice_h__
#define __Blade_Win32KeyboardDevice_h__
#include <interface/public/input/IKeyboardDevice.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class Win32KeyboardDevice : public IKeyboardDevice, public IWindowMessagePump::IMessageListener, public Allocatable
	{
	public:
		Win32KeyboardDevice();
		~Win32KeyboardDevice();

		/************************************************************************/
		/* IDevice interface                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update();

		/************************************************************************/
		/* IKeyboardDevice interface                                                                     */
		/************************************************************************/
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
		/* IWindowMessagePump::IMessageListener interface                                                                      */
		/************************************************************************/
		/** @brief  return true to continue message processing,
				false to skip this message */
		virtual bool preProcessMessage(void* )	{return true;}
		/** @brief  */
		virtual void postProcessMessage(void*)	{}
		/** @brief called on each loop before processing all messages */
		virtual void preProcessMessages();
		/** @brief called on each loop after processing all messages */
		virtual void postProcessMessages()		{}

	protected:
		static LRESULT CALLBACK		KeyProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam );

		typedef Set<IListener*>		ListenerList;


		HWND		mWinHandle;
		WNDPROC		mPrevWndProc;

		char		mKeyState[256];
		char		mPrevKeyState[256];

		ListenerList	mListeners;

		typedef		Map<HWND,Win32KeyboardDevice*> DeviceMap;
		static		DeviceMap	msDeviceMap;

		friend	class Win32GlobalKeyboardDevice;
	};//class Win32KeyboardDevice
	
}//namespace Blade


#endif //__Blade_Win32KeyboardDevice_h__

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS