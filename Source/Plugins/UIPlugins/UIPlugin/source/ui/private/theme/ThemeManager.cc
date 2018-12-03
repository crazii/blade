/********************************************************************
	created:	2016/07/17
	filename: 	ThemeManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <stdafx.h>
#include "ThemeCommon.h"
#include "ThemeButton.h"
#include "ThemeScrollBar.h"
#include "ThemeHeader.h"


using namespace Blade;
#if ENABLE_THEME
Theme ThemeManager::DEFAULT_THEME =
{
	RGB(0x75,0x75,0x75),//TC_FRAME
	RGB(0xc9,0xc9,0xc9),//TC_ACTIVEFRAME
	RGB(0x53,0x53,0x53),//TC_WINBK
	RGB(0x25,0x25,0x25),//TC_EMPTYBK
	RGB(0x44,0x44,0x44),//TC_CTLBK
	RGB(0x00,0x7a,0xcc),//TC_HILIGHTBK
	RGB(0xf1,0xf1,0xf1),//TC_TEXT
	RGB(0x99,0x99,0x99),//TC_GRAYTEXT
	RGB(0x2d,0x2d,0x2d),//TC_LINE
	RGB(0x33,0x33,0x33),//TC_MENULINE
	RGB(0x2b,0x2b,0x2b),//TC_MENUBK
	RGB(0x3e,0x3e,0x40),//TC_HILIGHTMENUBK
	RGB(0x65,0x65,0x65),//TC_GRAYMENUTEXT
	RGB(0x3a,0x3a,0x3a),//TC_SPLITTER
	RGB(0x25,0x25,0x25),//TC_TEXTBKDARK
	RGB(0x25,0x25,0x25),//TC_TEXTBKDARK
	RGB(0x53,0x53,0x53),//TC_BTN,
	RGB(0x66,0x66,0x66),//TC_HILIGHTBTN,
	RGB(0x45,0x45,0x45),//TC_PUSHEDBTN,

	TEXT("dark"),
};
#else

Theme ThemeManager::DEFAULT_THEME =
{
	::GetSysColor(COLOR_WINDOWFRAME),//TC_FRAME
	::GetSysColor(COLOR_ACTIVEBORDER),//TC_ACTIVEFRAME
	::GetSysColor(COLOR_BTNFACE),//TC_WINBK
	::GetSysColor(COLOR_BTNSHADOW),//TC_EMPTYBK
	::GetSysColor(COLOR_BTNFACE),//TC_CTLBK
	::GetSysColor(COLOR_MENUHILIGHT),//TC_HILIGHTBK
	::GetSysColor(COLOR_WINDOWTEXT),//TC_TEXT
	::GetSysColor(COLOR_GRAYTEXT),//TC_GRAYTEXT
	::GetSysColor(COLOR_BTNSHADOW),//TC_LINE
	::GetSysColor(COLOR_BTNSHADOW),//TC_MENULINE
	::GetSysColor(COLOR_MENU),//TC_MENUBK
	::GetSysColor(COLOR_MENUHILIGHT),//TC_HILIGHTMENUBK
	::GetSysColor(COLOR_GRAYTEXT),//TC_GRAYMENUTEXT
	::GetSysColor(COLOR_WINDOWFRAME),//TC_SPLITTER
	::GetSysColor(COLOR_BACKGROUND),//TC_TEXTBK
	::GetSysColor(COLOR_BTNTEXT),//TC_TEXTBKDARK
	::GetSysColor(COLOR_BTNFACE),//TC_BTN,
	::GetSysColor(COLOR_BTNHIGHLIGHT),//TC_HILIGHTBTN,
	::GetSysColor(COLOR_BTNFACE),//TC_PUSHEDBTN,

	TEXT("default"),
};
#endif//ENABLE_THEME

//////////////////////////////////////////////////////////////////////////
ThemeManager::ThemeManager()
{
	mHook = NULL;
	mInvalidBrush.CreateSolidBrush(RGB(0,0,0));
	this->setTheme(DEFAULT_THEME);

#if ENABLE_THEME
	WinThemeProc buttonTheme = 
	{
		Theme_PaintButton, 0, 0
	};

	WinThemeProc dlgTheme = 
	{
		Theme_PaintWindowCommon, 0
	};	//CtlColor only

	//only works for scroll bar control...
	WinThemeProc scrollbarTheme =
	{
		/*Theme_PaintScrollBar,*/ 0,
	};

	WinThemeProc listViewTheme =
	{
		0, Theme_PaintBorder, 0,
	};

	WinThemeProc editTheme =
	{
		0, Theme_PaintBorder, 0,
	};

	WinThemeProc headerTheme =
	{
		Theme_PaintHeader, Theme_PaintBorder, 0,
	};

	mThemeMap[BTString(WC_BUTTONA)] = buttonTheme;
	mThemeMap[BTString("#32770")] = dlgTheme;
	mThemeMap[BTString(WC_SCROLLBARA)] = scrollbarTheme;
	mThemeMap[BTString(WC_LISTVIEWA)] = listViewTheme;
	mThemeMap[BTString(WC_LISTBOXA)] = listViewTheme;
	mThemeMap[BTString(WC_EDITA)] = editTheme;
	mThemeMap[BTString(WC_HEADERA)] = headerTheme;	//header ctrl
	//mThemeMap[BTString("msctls_trackbar32")] = trackbarTheme;
#endif//ENABLE_THEME
}

//////////////////////////////////////////////////////////////////////////
ThemeManager::~ThemeManager()
{
#if ENABLE_THEME
	if(mHook != NULL)
	{
		::UnhookWindowsHookEx(mHook);
		mHook = NULL;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void	ThemeManager::initialize()
{
#if ENABLE_THEME
	if( mHook == NULL )
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		mHook = ::SetWindowsHookEx(WH_CBT, &ThemeManager::ThemeCBTProc, ::AfxGetInstanceHandle(), ::GetCurrentThreadId());
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void	ThemeManager::setTheme(Theme theme)
{
	mTheme = theme;
	for (int i = TC_BEGIN; i < TC_COUNT; ++i)
	{
		mBrushes[i].DeleteObject();
		mBrushes[i].CreateSolidBrush(mTheme.mColors[i]);
	}
}

//////////////////////////////////////////////////////////////////////////
void ThemeManager::addWindow(HWND hWnd)
{
	bool subClass = false;
	bool dialog = false;

	TCHAR szClassName[256];
	int nClassName = ::GetClassName(hWnd, szClassName, 256);
	if ( nClassName > 0)
	{
		TString tmp(szClassName, (size_t)nClassName, TString::CONST_HINT);
		subClass = mThemeMap.find(tmp) != mThemeMap.end();
		dialog = tmp == BTString("#32770");
	}

	if(subClass )
	{
		WNDPROC oldWndProc = NULL;
		if(dialog)
			oldWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, DWLP_DLGPROC, (LONG_PTR)&ThemeManager::ThemeProc);
		else
			oldWndProc = (WNDPROC)::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&ThemeManager::ThemeProc);

		if(dialog && (oldWndProc == NULL || oldWndProc == &ThemeManager::ThemeProc))
			return;

		assert(oldWndProc != NULL);
		ThemeData data = { oldWndProc, dialog};
		// subclass the window with standard WndProc
		mMap[hWnd] = data;
	}
}

//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ThemeManager::ThemeCBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	ThemeManager& mgr = ThemeManager::getSingleton();

	if (nCode != HCBT_CREATEWND && nCode != HCBT_ACTIVATE)
		return ::CallNextHookEx(mgr.mHook, nCode,wParam, lParam);

	HWND hWnd = (HWND)wParam;

	mgr.addWindow(hWnd);

	return ::CallNextHookEx(mgr.mHook, nCode,wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ThemeManager::ThemeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ThemeManager& mgr = ThemeManager::getSingleton();

	const WinThemeProc* theme = NULL;

	TCHAR szClassName[256];
	int nClassName = ::GetClassName(hWnd, szClassName, 256);
	if ( nClassName > 0)
	{
		TString tmp(szClassName, (size_t)nClassName, TString::CONST_HINT);
		ThemeMap::iterator i = mgr.mThemeMap.find(tmp);
		if(i != mgr.mThemeMap.end() )
			theme = &i->second;
	}

	WNDPROC defProc = &::DefWindowProc;
	WindProcMap::const_iterator i = mgr.mMap.find(hWnd);
	if(i != mgr.mMap.end() )
		defProc = i->second.proc;

	if(theme == NULL ||
		(message != WM_PAINT && message != WM_NCPAINT && message != WM_ERASEBKGND && message != WM_NCCALCSIZE &&
		 !(message >= WM_CTLCOLORMSGBOX && message <= WM_CTLCOLORSTATIC) ) 
		)
	{
		if(message == WM_DESTROY )
		{
			if(i->second.dialog )
			{
				if(::GetWindowLongPtr(hWnd, DWLP_DLGPROC) == (LONG_PTR)&ThemeManager::ThemeProc)
					::SetWindowLongPtr(hWnd, DWLP_DLGPROC, (LONG_PTR)defProc);
			}
			else
			{
				if(::GetWindowLongPtr(hWnd, GWLP_WNDPROC) == (LONG_PTR)&ThemeManager::ThemeProc)
					::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)defProc);
			}
			mgr.mMap.erase(i);
		}
		return ::CallWindowProc(defProc, hWnd, message, wParam, lParam);
	}

	switch(message)
	{
	case WM_ERASEBKGND:
		{
			HDC hDC = (HDC)wParam;
			if(theme->EraseBkgndProc != NULL)
				return theme->EraseBkgndProc(hWnd, hDC, defProc);
		}
		break;
	case WM_PAINT:
		if(theme->PaintProc != NULL)
		{
			theme->PaintProc(hWnd, defProc);
			return 0;
		}
		break;
	case WM_NCCALCSIZE:
		if(theme->NcCalcSizeProc != NULL)
			return theme->NcCalcSizeProc(hWnd, (BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam, defProc);
		break;
	case WM_NCPAINT:
		if(theme->NcPaintProc != NULL)
		{
			theme->NcPaintProc(hWnd, defProc);
			return 0;
		}
		break;
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		return (LRESULT)Theme_OnCtlColor((HDC)wParam, (HWND)lParam, message - WM_CTLCOLOREDIT);
	}
	return ::CallWindowProc(defProc, hWnd, message, wParam, lParam);
}