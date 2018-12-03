/********************************************************************
	created:	2009/02/15
	filename: 	Win32WindowDevice.h
	author:		Crazii
	
	purpose:	windows device in win32 platform
*********************************************************************/
#ifndef __Blade_Win32Windowdevice_h__
#define __Blade_Win32Windowdevice_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <interface/public/window/IWindowdevice.h>
#include <interface/public/window/Helpers.h>
#include <utility/Variant.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class Win32WindowDevice : public IWindowDevice, public Allocatable
	{
	public:
		static const TString WIN32_WINDOW_DEVICE_TYPE;
	public:
		//ctor & dector
		Win32WindowDevice();
		virtual ~Win32WindowDevice();

		/************************************************************************/
		/* IDevice specs                                                                     */
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
		/*IWindow specs                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWindowType() const
		{
			return WIN32_WINDOW_DEVICE_TYPE;
		}
		/** @brief  */
		virtual const  TString&	getUniqueName() const;

		/** @brief set the window size */
		virtual void setSize(size_t width,size_t height);

		/** @brief get the window size */
		virtual size_t getWidth() const;

		/** @brief get the window size */
		virtual size_t getHeight() const;

		/** @brief set the window caption */
		virtual void setCaption(const TString& caption);

		/** @brief get the window caption */
		virtual const TString& getCaption() const;

		/** @brief set the window visibility */
		virtual void setVisibility(bool visibile);

		/** @brief get the window visibility */
		virtual bool isVisible() const;

		/** @brief set the window's full screen mode */
		virtual void setFullScreen(bool fullscreen);

		/** @brief get whether the window is in full screen mode */
		virtual bool getFullScreen() const;

		/** @brief get the window's specific color depth */
		virtual int  getColorDepth() const;

		/** @brief get the native platform/API related window pointer(i.e. HWND for win32) */
		virtual uintptr_t	getNativeHandle() const;

		/** @brief set the parent of the window by internal implementation
		note: internalImpl may NOT be a native handle */
		virtual void setParent(uintptr_t internalImpl);

		/** @brief get the internal implementation of parent window */
		virtual uintptr_t getParent() const;

		/** @brief try to close the window */
		virtual bool closeWindow();

		/** @brief  */
		virtual bool addEventHandler(IWindowEventHandler* handler);

		/** @brief  */
		virtual bool removeEventHandler(IWindowEventHandler* handler);

		/************************************************************************/
		/* IWindowDevice interface                                                                    */
		/************************************************************************/
		/** @brief specify a external window to the device */
		/* @remark in this situation,when calling destroyWindow, it doesn't destroy the internal window and just detach it */
		virtual void attachWindow(uintptr_t internalImpl);

		/** @brief set the window's specific color depth */
		virtual void setColorDepth(int depth);

		/** @brief  */
		virtual void destroyWindow();

		/************************************************************************/
		/*internal use                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		bool		registerWindowClass();

		/*
		@describe 
		@param 
		@return 
		*/
		bool		createWindow();

		/*
		@describe 
		@param 
		@return 
		*/
		void		calculatePosition(int& x,int& y);

	protected:

		typedef Set<IWindowEventHandler*>	WinHandlerList;
		typedef TempVector<IWindowEventHandler*> HandlersBuffer;

		static const DWORD DEFAULT_STYLE		= WS_OVERLAPPEDWINDOW;
		static const DWORD FULL_SCRREN_STYLE	= WS_POPUP;

		/*
		@describe override for_each
		@param 
		@return 
		*/
		template< typename operation>
		operation		for_each(operation op)
		{
			//note: because a handler may remove itself on notifying iteration 
			//so the iterator may become invalid

			//copy to a tmp buffer: use local buffer instead of this to avoid re-enter
			//if a message called in a message, HandlersBuffer of this will be cleared!
			HandlersBuffer hanlders;
			hanlders.reserve(mHandlers.size());
			hanlders.insert(hanlders.end(), mHandlers.begin(),mHandlers.end());

			for(size_t i = 0; i < hanlders.size(); ++i)
			{
				IWindowEventHandler* handler = hanlders[i];
				//check if handler is deleted already
				if( mHandlers.find(handler) != mHandlers.end() )
					op(handler);
			}
			return op;
		}

		WNDPROC			mDefWindowProc;

		WinHandlerList	mHandlers;

		TString			mName;
		TString			mCaption;

		HWND			mWin32Handle;
		LONG			mStyle;
		size_t			mWidth;
		size_t			mHeight;
		size_t			mDefaultWidth;
		size_t			mDefaultHeight;
		int				mDepth;

		bool			mFullScreen;
		bool			mVisible;
		bool			mExternalAttached;
	
		typedef Map<HWND,Win32WindowDevice*>	WindowMap;
		static LRESULT CALLBACK BladeWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam );
		friend LRESULT CALLBACK BladeWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam );
		
		static bool			msWinClassRegistered;
		static WindowMap	msWindowMap;
	};//class Win32WindowDevice

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif // __Blade_Win32Windowdevice_h__