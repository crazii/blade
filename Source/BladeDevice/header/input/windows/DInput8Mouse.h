/********************************************************************
	created:	2011/05/14
	filename: 	DInput8Mouse.h
	author:		Crazii
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#ifndef __Blade_DInput8Mouse_h__
#define __Blade_DInput8Mouse_h__
#include <utility/FixedArray.h>
#include "Win32MouseBase.h"
#include "DInput8Base.h"


namespace Blade
{
	class DirectInputMouse : public Win32MouseBase , public DInput8Base , public Allocatable
	{
	protected:
		//recent 3 button status
		static const int RECENT_BUTTON_HISTORY_COUNT = 3;
		typedef FixedVector<MBSTATE,RECENT_BUTTON_HISTORY_COUNT>	ButtonHistory;
	public:
		DirectInputMouse();
		~DirectInputMouse();

		/************************************************************************/
		/* IDevice Interface                                                                     */
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
		/* IMouse Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual POINT3I			getMouseMovement() const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool			readMouseData();

		/*
		@describe 
		@param 
		@return 
		*/
		void			updateMouseButtonEvent(int buttonIndex,ButtonHistory& history);

	protected:
		HWND			mUsedWinHandle;
		HWND			mPreCaptureHandle;
		POINT3I			mMovement;		//mouse movement
		MBSTATE			mPrevButtonState[MSB_COUNT];
		ButtonHistory	mHistroy[MSB_COUNT];
		DWORD			mLastTimeStamp;
		DWORD			mTimeStamp;
		bool			mShareDevice;

		static SharedDeviceMap	msSharedDevice;
	};

	

}//namespace Blade


#endif // __Blade_DInput8Mouse_h__

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS