/********************************************************************
	created:	2009/02/15
	filename: 	Win32WindowDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4312)
#pragma warning(disable:4244)
#endif

#include <interface/public/window/IWindowEventHandler.h>
#include <window/windows/Win32WindowDevice.h>

static HINSTANCE hWin32DllInstance = NULL;
BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD /*fdwReason*/, LPVOID /*lpvReserved*/)
{
	hWin32DllInstance = hinstDLL;
	return TRUE;
}

namespace Blade
{
	const TString Win32WindowDevice::WIN32_WINDOW_DEVICE_TYPE = BTString("Win32Native");
	bool Win32WindowDevice::msWinClassRegistered = false;
	Win32WindowDevice::WindowMap Win32WindowDevice::msWindowMap;

	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	Win32WindowDevice::Win32WindowDevice()
		:mDefWindowProc(NULL)
		,mWin32Handle(NULL)
		,mWidth(0)
		,mHeight(0)
		,mDefaultWidth(0)
		,mDefaultHeight(0)
		,mDepth(16)
		,mFullScreen(false)
		,mVisible(false)
		,mExternalAttached(false)
	{
		mCaption = BTString("BladeWindow");
	}

	//////////////////////////////////////////////////////////////////////////
	Win32WindowDevice::~Win32WindowDevice()
	{
		this->close();
	}

	/************************************************************************/
	/*IDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::open()
	{
		if( mWin32Handle != NULL )
			return true;

		return this->registerWindowClass() && this->createWindow();
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::isOpen() const
	{
		return ::IsWindow(mWin32Handle) == TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::close()
	{
		//if( mExternalAttached )
		//{
		//	if( mWin32Handle != NULL )
		//	{
		//		msWindowMap.erase(mWin32Handle);
		//		::SetWindowLongPtr(mWin32Handle,GWLP_WNDPROC,(LONG_PTR)mDefWindowProc);
		//	}

		//	mWin32Handle = NULL;
		//	return;
		//}

		//if(mWin32Handle)
		//{
		//	this->destroyWindow();
		//}
		this->closeWindow();
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::reset()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::update()
	{
		if( mExternalAttached )
			return ::IsWindow(mWin32Handle) == TRUE;

		MSG msg;
		
		bool result = false;
		if( ::IsWindow(mWin32Handle) )
		{
			result = true;
			while(::PeekMessage(&msg, mWin32Handle, 0, 0, PM_REMOVE) != FALSE )
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);

				//result = result?(msg.message != WM_DESTROY):result;
			}
		}
		return result;
	}

	/************************************************************************/
	/*IWindowDevice specs                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const  TString&	Win32WindowDevice::getUniqueName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::setSize(size_t width,size_t height)
	{
		mDefaultWidth	= width;
		mDefaultHeight	= height;

		mWidth	= width;
		mHeight = height;

		if( mWin32Handle )
		{
			RECT rect = {0,0,(int)width,(int)height};

			LONG dwStyle = ::GetWindowLong(mWin32Handle,GWL_STYLE);
			::AdjustWindowRect(&rect, (DWORD)dwStyle, FALSE);

			int x = 0,y = 0;
			HWND hWndInsertAfter;
			if( mFullScreen )
				hWndInsertAfter = HWND_TOPMOST;
			else
			{
				hWndInsertAfter = HWND_NOTOPMOST;
				this->calculatePosition(x,y);
			}
			
			::SetWindowPos(mWin32Handle,hWndInsertAfter,x,y,rect.right-rect.left,rect.bottom-rect.top,
				/*SWP_NOMOVE | SWP_NOZORDER | */SWP_FRAMECHANGED | SWP_NOSENDCHANGING |SWP_NOOWNERZORDER );


			//::SetWindowPos(mWin32Handle,NULL,x,y,width,height,SWP_NOMOVE | SWP_NOZORDER  );
		}


	}

	//////////////////////////////////////////////////////////////////////////
	size_t Win32WindowDevice::getWidth() const
	{
		return mWidth;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t Win32WindowDevice::getHeight() const
	{
		return mHeight;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::setCaption(const TString& name)
	{
		//set the name as first caption
		if( mName == TString::EMPTY )
			mName = name;

		if( mWin32Handle )
			::SetWindowText(mWin32Handle, name.c_str() );
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& Win32WindowDevice::getCaption() const
	{
		return mCaption;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::setVisibility(bool bShow)
	{
		if( mWin32Handle != NULL && mVisible != bShow )
		{
			::ShowWindow(mWin32Handle,bShow?SW_SHOW:SW_HIDE);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::isVisible() const
	{
		if (!this->isOpen())
			return false;

		if (::IsWindowVisible(mWin32Handle) && !::IsIconic(mWin32Handle))
		{
			//TODO: incompatible with Qt - Qt created window always get a NULL clip box.
			//it's not working on Windows 6.0+ anyway.
#if 0
			HDC hDC = ::GetDC(mWin32Handle);
			RECT visibleRect;
			int region = ::GetClipBox(hDC, &visibleRect);
			::ReleaseDC(mWin32Handle, hDC);

			//it's not working on Windows 6.0+ since DWM with glass effect has different implementations from XP
			if (region == NULLREGION || visibleRect.bottom - visibleRect.top <= 0 || visibleRect.right - visibleRect.left <= 0)
				return false;
#endif
			return mVisible;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::setFullScreen(bool bFull)
	{
		if( mFullScreen != bFull )
		{
			bool backup = mFullScreen;
			if( mWin32Handle == NULL )
				return;

			//test if allow full screen switch
			mFullScreen = bFull;
			FnOnWinResizing ret = for_each( FnOnWinResizing(this,mDefaultWidth,mDefaultHeight) );
			mFullScreen = backup;
			if( !ret.mAllowResize )
				return;

			::ShowWindow(mWin32Handle,SW_HIDE);
			//DWORD dwStyle = ::GetWindowLongPtr(mWin32Handle,GWL_STYLE);

			if( bFull )
				::SetWindowLongPtr(mWin32Handle,GWL_STYLE,FULL_SCRREN_STYLE);
			else
				::SetWindowLongPtr(mWin32Handle, GWL_STYLE, mStyle);

			::ShowWindow(mWin32Handle,SW_SHOW);
			
			mFullScreen = bFull;
			this->setSize(mDefaultWidth,mDefaultHeight);

			this->for_each( FnOnWinResized(this) );

			::UpdateWindow(mWin32Handle);
			::SetForegroundWindow(mWin32Handle);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::getFullScreen() const
	{
		return mFullScreen;
	}

	//////////////////////////////////////////////////////////////////////////
	int Win32WindowDevice::getColorDepth() const
	{
		return mDepth;
	}

	//////////////////////////////////////////////////////////////////////////
	uintptr_t  Win32WindowDevice::getNativeHandle() const
	{
		return (uintptr_t)(void*)mWin32Handle;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::setParent(uintptr_t  internalImpl)
	{
		HWND hwnd = (HWND)(void*)internalImpl;
		if( ::IsWindow(mWin32Handle) && ::IsWindow(hwnd) )
			::SetParent(mWin32Handle, hwnd);
	}

	//////////////////////////////////////////////////////////////////////////
	uintptr_t Win32WindowDevice::getParent() const
	{
		HWND hParent = ::GetParent(mWin32Handle);
		return (uintptr_t)(void*)hParent;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::closeWindow()
	{
		if( ::IsWindow(mWin32Handle) )
		{
			::SendMessage(mWin32Handle, WM_CLOSE, 0, 0);
			bool closed = !::IsWindow(mWin32Handle);
			if( closed )
				mWin32Handle = NULL;
			return closed;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::addEventHandler(IWindowEventHandler* handler)
	{
		return mHandlers.insert(handler).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::removeEventHandler(IWindowEventHandler* handler)
	{
		return mHandlers.erase(handler) == 1;
	}

	/************************************************************************/
	/* IWindowDevice interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::attachWindow(uintptr_t internalImpl)
	{
		HWND hwnd = static_cast<HWND>( (void*)internalImpl );
		if(!::IsWindow(hwnd))
		{
			assert(false);
			return;
		}

		if( mWin32Handle )
			this->close();
		

		mWin32Handle = hwnd;

		if( mWin32Handle == NULL || ::IsWindow(mWin32Handle) != TRUE )
		{
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("window handle invalid.") );
		}

		mName = TStringHelper::fromUInt( uintptr_t(mWin32Handle) );

		mExternalAttached = true;

		int len = ::GetWindowTextLength(mWin32Handle);

		if( len != 0 )
		{
			TempBuffer buffer;
			buffer.reserve( (len + 1)*sizeof(tchar) );
			::GetWindowText(mWin32Handle,(tchar*)buffer.getData(),len+1 );
			mCaption = (tchar*)buffer.getData();
		}

		WINDOWINFO info;
		std::memset( &info,0,sizeof(info) );

		::GetWindowInfo(mWin32Handle,&info);

		mVisible = (::IsWindowVisible(mWin32Handle) == TRUE);
		mWidth = size_t(info.rcClient.right - info.rcClient.left);
		mHeight = size_t(info.rcClient.bottom - info.rcClient.top);

		//just set a default value
		mFullScreen = false;
		mDepth = 16;

		mDefWindowProc = (WNDPROC)::GetWindowLongPtr(mWin32Handle,GWLP_WNDPROC);
		mStyle = ::GetWindowLongPtr(mWin32Handle,GWL_STYLE);
		msWindowMap[mWin32Handle] = this;
		::SetWindowLongPtr(mWin32Handle,GWLP_WNDPROC,(LONG_PTR)Win32WindowDevice::BladeWindowProc);
	}


	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::setColorDepth(int depth)
	{
		assert( depth == 16 || depth == 32);
		mDepth = depth;
	}

	//////////////////////////////////////////////////////////////////////////
	void Win32WindowDevice::destroyWindow()
	{
		if( mWin32Handle != NULL )
		{
			if( !mExternalAttached )
			{
				//message handler will fire the event
				::DestroyWindow(mWin32Handle);
				msWindowMap.erase(mWin32Handle);
			}
			else
			{
				//fire event manually, and detach window (don't care if window actually destroyed)
				this->for_each( FnOnWinDestroy(this) );

				msWindowMap.erase(mWin32Handle);
				::SetWindowLongPtr(mWin32Handle,GWLP_WNDPROC,(LONG_PTR)mDefWindowProc);
			}
			mWin32Handle = NULL;
		}
	}

	/************************************************************************/
	/*internal use                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::registerWindowClass()
	{
		if( !msWinClassRegistered )
		{
			WNDCLASS wc;

			wc.style			= 0/*CS_DBLCLKS*/;
			wc.lpfnWndProc		= ::DefWindowProc;
			wc.cbClsExtra		= 0;
			wc.cbWndExtra		= 0;
			wc.hInstance		= hWin32DllInstance;
			wc.hIcon			= NULL;
			wc.hCursor			= ::LoadCursor( NULL, IDC_ARROW);
			wc.hbrBackground	= (HBRUSH)::GetStockObject(BLACK_BRUSH);
			wc.lpszMenuName		= NULL;
			wc.lpszClassName	= TEXT("BLADE_WindClass");

			return msWinClassRegistered = (RegisterClass(&wc) != 0);

		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool Win32WindowDevice::createWindow()
	{
		int x = 0,y = 0;

		this->calculatePosition(x,y);

		mStyle = DEFAULT_STYLE;

		DWORD style;

		if( this->mFullScreen )
			style = FULL_SCRREN_STYLE;
		else
			style = DEFAULT_STYLE;

		RECT rect = {0,0,(int)mWidth,(int)mHeight};
		::AdjustWindowRect(&rect,style,FALSE);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		mWin32Handle = ::CreateWindow(TEXT("BLADE_WindClass"), mCaption.c_str(), style,
			x, y,width, height, NULL, NULL, hWin32DllInstance, NULL );

		if( mWin32Handle == NULL )
		{
			BLADE_EXCEPT(EXC_API_ERROR,BTString("cannot create window.") );
		}

		msWindowMap[mWin32Handle] = this;
		//mDefWindowProc = ::DefWindowProc;
		//::SetWindowLongPtr(mWin32Handle,GWLP_WNDPROC,(LONG_PTR)Win32WindowDevice::BladeWindowProc);
		mDefWindowProc = (::WNDPROC)::SetWindowLongPtr(mWin32Handle,GWLP_WNDPROC,(LONG_PTR)Win32WindowDevice::BladeWindowProc);

		//dispatch window created event
		this->for_each( FnOnWinCreate(this) );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Win32WindowDevice::calculatePosition(int& x,int& y)
	{
		int parent_w;
		int parent_h;
		HWND parent = ::GetParent(mWin32Handle);
		if( parent != NULL )
		{
			RECT r;
			::GetWindowRect(parent, &r);
			parent_w = (r.right - r.left);
			parent_h = (r.bottom - r.top);
		}
		else
		{
			parent_w = GetSystemMetrics(SM_CXSCREEN);
			parent_h = GetSystemMetrics(SM_CYSCREEN);
		}

		x = (parent_w - (int)mWidth) /2;
		y = (parent_h - (int)mHeight)/2;
	}

	//////////////////////////////////////////////////////////////////////////
	LRESULT CALLBACK Win32WindowDevice::BladeWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam )
	{
		//TStringStream s;
		//s << TEXT("MESSAGE:") << (void*)uMsg << TEXT("\tWPARAM") << (void*)wParam << TEXT("\tLPRAM") << (void*)lParam << std::endl;
		//OutputDebugString( s.str().c_str() );
		//return DefWindowProc(hwnd,uMsg,wParam,lParam);

		Win32WindowDevice* window;

		{
			WindowMap::const_iterator i = msWindowMap.find( hwnd );
			if( i == msWindowMap.end())
				BLADE_EXCEPT(EXC_INTERNAL_ERROR,BTString("window not registered in message procedure.") );
			window =  i->second;
		}

		WNDPROC WinProc = window->mDefWindowProc;

		switch (uMsg)
		{
		case WM_DESTROY:
			{
				window->for_each( FnOnWinDestroy(window) );
				window->mWin32Handle = NULL;	//in case explicit destroyWindow called again, an event fired twice.
				if(window->mExternalAttached)
					return WinProc(hwnd,uMsg,wParam,lParam);
				break;
			}
		case WM_NCDESTROY:
			{
				if( window->mExternalAttached )
				{
					LRESULT ret = WinProc(hwnd,uMsg,wParam,lParam);
					return ret;
				}
				break;
			}
		case WM_SHOWWINDOW:
			//show/hide notification
			if( wParam == TRUE )
			{
				window->mVisible = true;
				window->for_each( FnOnWinShow(window) );
			}
			else
			{
				window->mVisible = false;
				window->for_each( FnOnWinHide(window) );
			}
			return WinProc(hwnd,uMsg,wParam,lParam);
			//size/close query
		case WM_WINDOWPOSCHANGING:
			{
				bool allowResize = true;
				WINDOWPOS* winpos = reinterpret_cast<WINDOWPOS*>(lParam);
				if( (winpos->flags&SWP_NOSIZE) && !(winpos->flags&SWP_FRAMECHANGED) )
					return WinProc(hwnd,uMsg,wParam,lParam);

				RECT rcClient, rcWindow;
				::GetClientRect(hwnd, &rcClient);
				::GetWindowRect(hwnd, &rcWindow);
				LONG wdiff = rcWindow.right-rcWindow.left - (rcClient.right - rcClient.left);
				LONG hdiff = rcWindow.bottom-rcWindow.top- (rcClient.bottom - rcClient.top);

				size_t width = (size_t)(winpos->cx - wdiff);
				size_t height = (size_t)(winpos->cy - hdiff);
				if( width != window->mWidth || height != window->mHeight )
					allowResize = window->for_each( FnOnWinResizing(window, width, height) ).mAllowResize;
				else
					allowResize = true;

				if( !allowResize )
					winpos->flags |= SWP_NOSIZE;
				return WinProc(hwnd,uMsg,wParam,lParam);
			}
		case WM_CLOSE:
			{
				bool toClose = window->for_each( FnOnWinClose(window) ).getResult();
				if( toClose )
				{
					if( window->mExternalAttached )
						return WinProc(hwnd,uMsg,wParam,lParam);
					else
					{
						window->destroyWindow();
						return 0;
					}
				}
				else
					return 0;
			}
		case WM_SIZE:
			//notification for the new size
			{
				//size_t width = LOWORD(lParam);
				//size_t height = HIWORD(lParam);

				RECT rect;
				::GetClientRect(hwnd,&rect);
				size_t width = (size_t)(rect.right - rect.left);
				size_t height = (size_t)(rect.bottom - rect.top);

				//full screen mode switch using manual method
				if( window->getFullScreen() )
					return WinProc(hwnd,uMsg,wParam,lParam);

				bool notifyhandler = false;
				if(window->mWidth != width )
				{
					notifyhandler = true;
					window->mWidth = width;
				}

				if( window->mHeight != height )
				{
					window->mHeight = height;
					notifyhandler = true;
				}

				if( notifyhandler )
					window->for_each( FnOnWinResized(window) );

				return WinProc(hwnd,uMsg,wParam,lParam);
			}
		case WM_ACTIVATE :
			{
				WORD ActiveCode = LOWORD(wParam);
				if( ActiveCode == WA_ACTIVE || ActiveCode == WA_CLICKACTIVE )
					window->for_each( FnOnWinActivate(window) );
				else if(ActiveCode == WA_INACTIVE )
					window->for_each( FnOnWinDeactivate(window) );
			}
			return WinProc(hwnd,uMsg,wParam,lParam);
		case WM_SETTEXT:
			{
				const tchar* text = (const tchar*)lParam;
				window->mCaption = text;
				window->for_each( FnOnWinCaptionChange(window) );
			}
		}
		return WinProc(hwnd,uMsg,wParam,lParam);
	}

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
