/********************************************************************
	created:	2016/07/19
	filename: 	ThemeCommon.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ThemeCommon_h__
#define __Blade_ThemeCommon_h__
#include <stdafx.h>
#include <Windowsx.h>
#if ENABLE_THEME

//////////////////////////////////////////////////////////////////////////
void Theme_PaintBorder(HWND hWnd, WNDPROC /*defProc*/)
{
	CWnd* wnd = CWnd::FromHandle(hWnd);
	DWORD style = GetWindowStyle(hWnd);
	if(style&WS_BORDER)
	{
		CRect rect;
		wnd->GetWindowRect(&rect);
		rect.OffsetRect(-rect.left, -rect.top);

		CWindowDC dc(wnd);
		dc.FrameRect(&rect, &THEMEBRUSH(TC_FRAME));
	}
}

//////////////////////////////////////////////////////////////////////////
void Theme_PaintWindowCommon(HWND hWnd, WNDPROC /*defProc*/)
{
	CWnd* wnd = CWnd::FromHandle(hWnd);
	CPaintDC dc(wnd);
	CRect rect;
	wnd->GetClientRect(&rect);

	dc.FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
}

//////////////////////////////////////////////////////////////////////////
void Theme_PaintControlCommon(HWND hWnd, WNDPROC /*defProc*/)
{
	CWnd* wnd = CWnd::FromHandle(hWnd);
	CPaintDC dc(wnd);
	CRect rect;
	wnd->GetClientRect(&rect);

	dc.FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));
}
#endif//ENABLE_THEME

//////////////////////////////////////////////////////////////////////////
HBRUSH  Theme_OnCtlColor(HDC hDC, HWND /*hWnd*/, UINT nCtlColor)
{
	::SetBkMode(hDC, TRANSPARENT);
	::SetTextColor(hDC, THEMECOLOR(TC_TEXT));

	if(nCtlColor == CTLCOLOR_EDIT)
	{
		return THEMEBRUSH(TC_TEXTBK);
	}
	return THEMEBRUSH(TC_WINBK);
}

#endif // __Blade_ThemeCommon_h__