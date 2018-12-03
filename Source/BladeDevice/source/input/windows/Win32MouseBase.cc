/********************************************************************
	created:	2011/05/14
	filename: 	Win32MouseBase.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>

#include <interface/public/window/IWindow.h>
#include <input/windows/Win32MouseBase.h>

namespace Blade
{
	POINT2I	Win32MouseBase::msMouseHidePos;
	bool	Win32MouseBase::msCursorVisible = true;

	//////////////////////////////////////////////////////////////////////////
	Win32MouseBase::Win32MouseBase()
		:mWinHandle(NULL)
		,mWinWidth(0)
		,mWinHeight(0)
		,mMousePos(0,0)
	{

		for( int i = MSB_BEGIN; i < MSB_COUNT; ++i )
		{
			mButtonState[i] = MBS_UP;
		}

	}

	//////////////////////////////////////////////////////////////////////////
	Win32MouseBase::~Win32MouseBase()
	{

	}

	/************************************************************************/
	/* IMouseDevice Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			Win32MouseBase::attach(IWindow* window)
	{
		mWinWidth = window->getWidth();
		mWinHeight = window->getHeight();
		mWinHandle = (HWND)(void*)window->getNativeHandle();
	}


	//////////////////////////////////////////////////////////////////////////
	void			Win32MouseBase::notifyWindowSize(size_t width,size_t height)
	{
		mWinWidth = width;
		mWinHeight = height;
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Win32MouseBase::addListener(IListener* listener)
	{
		if(listener == NULL )
			return false;
		else
			return mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Win32MouseBase::removeListener(IListener* listener)
	{
		return mListeners.erase(listener) == 1;
	}

	/************************************************************************/
	/* IMouse Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	MBSTATE			Win32MouseBase::getButtonState(MSBUTTON button) const
	{
		assert( button >= MSB_BEGIN && button < MSB_COUNT );
		return mButtonState[button];
	}

	//////////////////////////////////////////////////////////////////////////
	POINT2I			Win32MouseBase::getGlobalMousePosition() const
	{
		POINT p;
		::GetCursorPos(&p);
		return POINT2I(p.x,p.y);
	}

	//////////////////////////////////////////////////////////////////////////
	POINT2I			Win32MouseBase::getMousePosition() const
	{
		POINT p;
		::GetCursorPos(&p);

		if( ::IsWindow(mWinHandle) )
			::ScreenToClient(mWinHandle,&p);

		mMousePos.x = (int16)p.x;
		mMousePos.y = (int16)p.y;
		return mMousePos;
	}

	//////////////////////////////////////////////////////////////////////////
	POINT2			Win32MouseBase::getNormalizedMousePosition() const
	{
		POINT2I pos = this->getMousePosition();

		return POINT2(
			(scalar)pos.x/(scalar)mWinWidth,
			(scalar)pos.y/(scalar)mWinHeight
			);
	}

	//////////////////////////////////////////////////////////////////////////
	void			Win32MouseBase::setGlobalMousePosition(POINT2I pos)
	{
		::SetCursorPos(pos.x,pos.y);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Win32MouseBase::hide()
	{
		bool result = this->hideCursor();

		if(result)
			msMouseHidePos = this->getGlobalMousePosition();

		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			Win32MouseBase::show()
	{

		bool result = this->showCursor();

		if(result)
			this->setGlobalMousePosition(msMouseHidePos);

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	POINT3			Win32MouseBase::getNormalizedMovement() const
	{
		POINT3I movement = this->getMouseMovement();

		return POINT3(
			(scalar)movement.x/(scalar)mWinWidth,
			(scalar)movement.y/(scalar)mWinHeight,
			(scalar)movement.z
			);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool			Win32MouseBase::showCursor()
	{
		if(msCursorVisible)
			return false;
		else
		{
			while( ::ShowCursor(TRUE) < 0 );
			msCursorVisible = true;
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Win32MouseBase::hideCursor()
	{
		if(!msCursorVisible)
			return false;
		else
		{
			while( ::ShowCursor(FALSE) >= 0 );
			msCursorVisible = false;
			return true;
		}
	}

	

}//namespace Blade



#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS