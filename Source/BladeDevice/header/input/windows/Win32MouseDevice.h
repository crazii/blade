/********************************************************************
	created:	2010/04/29
	filename: 	Win32MouseDevice.h
	author:		Crazii
	purpose:	gengerate input using windows messages.\n
				this is not suitable for real-time level apps,\n
				but will work well in other situation
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#ifndef __Blade_Win32MouseDevice_h__
#define __Blade_Win32MouseDevice_h__
#include <interface/public/input/IMouseDevice.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <utility/BladeContainer.h>
#include "Win32MouseBase.h"

namespace Blade
{
	class Win32MouseDevice : public Win32MouseBase, public IWindowMessagePump::IMessageListener, public Allocatable
	{
	public:
		Win32MouseDevice();
		virtual ~Win32MouseDevice();

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

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		void					checkDragging(bool testMove = false);

	protected:
		/** @brief  */
		static LRESULT CALLBACK		MouseProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam );

		/** @brief  */
		inline void setCapture()
		{
			if (::GetCapture() != mWinHandle)
				mPreCaptureWindow = ::SetCapture(mWinHandle);
		}

		/** @brief  */
		inline void releaseCapture()
		{
			if (mPreCaptureWindow != NULL && mPreCaptureWindow != mWinHandle)
				::SetCapture(mPreCaptureWindow);
			else
				::ReleaseCapture();
			mPreCaptureWindow = NULL;
		}

		typedef Set<IListener*>		ListenerList;

		WNDPROC		mPrevWndProc;
		HWND		mPreCaptureWindow;

		MBSTATE			mPrevButtonState[MSB_COUNT];
		POINT2I			mPrevMousePos;
		POINT3I			mMouseMovement;
		bool			mIsDragging;

		typedef		Map<HWND,Win32MouseDevice*> DeviceMap;
		static		DeviceMap	msDeviceMap;
	};
	
}//namespace Blade


#endif //__Blade_Win32MouseDevice_h__


#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS