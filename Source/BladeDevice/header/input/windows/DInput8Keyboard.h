/********************************************************************
	created:	2011/05/14
	filename: 	DInput8Keyboard.h
	author:		Crazii
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#ifndef __Blade_DInput8Keyboard_h__
#define __Blade_DInput8Keyboard_h__
#include <interface/public/input/IKeyboardDevice.h>
#include "DInput8Base.h"

namespace Blade
{

	class DirectInputKeyboard : public IKeyboardDevice , public DInput8Base , public Allocatable
	{
	public:
		DirectInputKeyboard();
		~DirectInputKeyboard();

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
		/* IKeyboard interface                                                                     */
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
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		bool				readKeyboardData();

	protected:
		typedef Set<IListener*>		ListenerList;

		HWND		mWinHandle;
		HWND		mUsedWinHandle;
		ListenerList	mListeners;
		EKeyState		mKeyState[256];
		EKeyState		mPrevKeyState[256];
		bool			mShareDevice;


		static SharedDeviceMap msSharedDevices;

	};//class DirectInputKeyboard
	

}//namespace Blade

#endif // __Blade_DInput8Keyboard_h__

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS