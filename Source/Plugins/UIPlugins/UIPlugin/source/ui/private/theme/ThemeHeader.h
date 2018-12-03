/********************************************************************
	created:	2016/7/22
	filename: 	ThemeHeader.h
	author:		Crazii
	purpose:	themed header control
*********************************************************************/
#ifndef __Blade_ThemeHeader_h__
#define __Blade_ThemeHeader_h__
#include <stdafx.h>
#if ENABLE_THEME

//////////////////////////////////////////////////////////////////////////
void Theme_PaintHeader(HWND hWnd, WNDPROC /*proc*/)
{
	CRect rcUpdate;
	if (::GetUpdateRect(hWnd, &rcUpdate, FALSE) == FALSE)
		return;

	CHeaderCtrl* wnd = static_cast<CHeaderCtrl*>( CWnd::FromHandle(hWnd) );
	//DWORD style = wnd->GetStyle();
	CRect rect;
	wnd->GetClientRect(&rect);

	CPaintDC dc(wnd);

	dc.FillSolidRect(&rect, THEMECOLOR(TC_BTN));
	int count = wnd->GetItemCount();
	for (int i = 0; i < count; ++i)
	{
		const size_t TEXT_LEN = 256;
		TCHAR strBuffer[TEXT_LEN] = { 0 };
		HDITEM item;
		item.mask = HDI_FORMAT | /*HDI_STATE |*/ HDI_TEXT;
		item.pszText = strBuffer;
		item.cchTextMax = TEXT_LEN;
		wnd->GetItem(i, &item);

		CRect rcItem;
		wnd->GetItemRect(i, &rcItem);

		dc.FrameRect(&rcItem, &THEMEBRUSH(TC_FRAME));

		if ((item.fmt & HDF_STRING))
		{
			rcItem.DeflateRect(1, 1, 1, 1);
			UINT format = DT_VCENTER;
			if ((item.fmt&HDF_LEFT))
				format |= DT_LEFT;
			else if ((item.fmt&HDF_RIGHT))
				format |= DT_RIGHT;
			else
				format |= DT_CENTER;

			CFont* oldFont = dc.SelectObject(wnd->GetFont());
			COLORREF oldColor = dc.SetTextColor(THEMECOLOR(TC_TEXT));
			dc.SetBkMode(TRANSPARENT);
			dc.DrawText(item.pszText, (int)::_tcslen(item.pszText), &rcItem, format);
			dc.SetTextColor(oldColor);
			dc.SelectObject(oldFont);
		}
		//TODO: image
		//TODO: this is a global painting function, we need handle all drawings
	}
}
#endif//#if ENABLE_THEME

#endif//__Blade_ThemeHeader_h__