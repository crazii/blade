/********************************************************************
	created:	2011/05/14
	filename: 	Win32MouseBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#ifndef __Blade_Win32MouseBase_h__
#define __Blade_Win32MouseBase_h__
#include <BladeWin32API.h>
#include <interface/public/input/IMouseDevice.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class Win32MouseBase : public IMouseDevice
	{
	public:
		Win32MouseBase();
		virtual ~Win32MouseBase();

		/************************************************************************/
		/* IMouseDevice Interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void			attach(IWindow* window);

		/*
		@describe
		@param
		@return
		*/
		virtual void			notifyWindowSize(size_t width,size_t height);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addListener(IListener* listener);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeListener(IListener* listener);

		/************************************************************************/
		/* IMouse Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual MBSTATE			getButtonState(MSBUTTON button) const;

		/*
		@describe get mouse position, in screen coordinates (0~screen_width,0~screen_height)
		@param 
		@return 
		*/
		virtual POINT2I			getGlobalMousePosition() const;

		/*
		@describe get mouse position, in window coordinates (0~window_width,0~window_height)
		@param 
		@return 
		*/
		virtual POINT2I			getMousePosition() const;

		/*
		@describe get normalized mouse position (0~1,0~1), in window coordinates
		@param 
		@return 
		*/
		virtual POINT2			getNormalizedMousePosition() const;

		/*
		@describe set mouse position in screen coordinates
		@param 
		@return 
		*/
		virtual void			setGlobalMousePosition(POINT2I pos);

		/*
		@describe return false if already hide
		@param 
		@return 
		*/
		virtual bool			hide();

		/*
		@describe return false if already show
		@param 
		@return 
		*/
		virtual bool			show();

		/*
		@describe 
		@param 
		@return 
		@note: z movement is not normalized
		*/
		virtual POINT3			getNormalizedMovement() const;

	protected:

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		static bool			showCursor();

		/** @brief  */
		static bool			hideCursor();

		typedef Set<IListener*>		ListenerList;

		HWND		mWinHandle;
		size_t		mWinWidth;
		size_t		mWinHeight;

		mutable POINT2I		mMousePos;
		MBSTATE		mButtonState[MSB_COUNT];

		ListenerList mListeners;

		static POINT2I	msMouseHidePos;
		static bool	msCursorVisible;
	};
	

}//namespace Blade



#endif // __Blade_Win32MouseBase_h__

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS