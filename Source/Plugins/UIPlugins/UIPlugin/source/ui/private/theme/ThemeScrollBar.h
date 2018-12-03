/********************************************************************
	created:	2016/7/20
	filename: 	ThemeScrollBar.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ThemeScrollBar_h__
#define __Blade_ThemeScrollBar_h__
#include <stdafx.h>
#if ENABLE_THEME

//////////////////////////////////////////////////////////////////////////
static void DrawScrollbarButton(CDC* pDC, CRect rButton, bool horiz, bool back, bool disabled)
{
	if (horiz)
	{
		if(back)
			rButton.right = rButton.left + rButton.Height();
		else
			rButton.left = rButton.right - rButton.Height();
	}
	else
	{
		if(back)
			rButton.bottom = rButton.top + rButton.Width();
		else
			rButton.top = rButton.bottom - rButton.Width();
	}

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CRect rcMem(rButton);
	rcMem.OffsetRect(-rcMem.TopLeft());

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rButton.Width(), rButton.Height());
	CBitmap* pOldBM = memDC.SelectObject(&bmp);

	UINT state = (UINT)(horiz ? (back ? DFCS_SCROLLLEFT : DFCS_SCROLLRIGHT) : (back ? DFCS_SCROLLUP : DFCS_SCROLLDOWN));
	if(disabled)
		state |= DFCS_INACTIVE;
	memDC.DrawFrameControl(&rcMem, DFC_SCROLL, state);
	memDC.SelectObject(pOldBM);

	pDC->BitBlt(rButton.left, rButton.top, rButton.Width(), rButton.Height(), &memDC, 0, 0, SRCINVERT);
	pDC->BitBlt(rButton.left, rButton.top, rButton.Width(), rButton.Height(), &memDC, 0, 0, SRCAND);
	pDC->BitBlt(rButton.left, rButton.top, rButton.Width(), rButton.Height(), &memDC, 0, 0, SRCINVERT);
}


void Theme_PaintScrollBar(HWND hWnd, WNDPROC proc)
{
	CRect rcUpdate;
	if (::GetUpdateRect(hWnd, &rcUpdate, FALSE) == FALSE)
		return;

	CWnd* wnd = CWnd::FromHandle(hWnd);
	DWORD style = wnd->GetStyle();
	if( !(style&(SBS_HORZ|SBS_VERT)) )	//sizebox
	{
		::CallWindowProc(proc, hWnd, WM_PAINT, 0, 0);
		return;
	}

	CPaintDC dc(wnd);
	CRect rect;
	wnd->GetClientRect(&rect);
	dc.FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));

	bool horiz = (style&SBS_HORZ) != 0;
	assert(horiz || (style&SBS_VERT));
	bool disabled = (style&WS_DISABLED) != 0;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	wnd->GetScrollInfo(SB_CTL, &si);

	//BOOL bMouseDown = (!m_bDragging && pCursor) ? (GetAsyncKeyState(MK_LBUTTON) & 0x8000) : FALSE;

	// Draw the scrollbar arrows
	DrawScrollbarButton(&dc, rect, horiz, true, disabled);
	DrawScrollbarButton(&dc, rect, horiz, false, disabled);

	float rate = (float)si.nPos / (float)(si.nMax - si.nMin);

	int length = horiz ? rect.Width() - rect.Height() : rect.Height() - rect.Width();
	int pos = int((float)length * rate + 0.5f);
	int thumbSize = horiz ? rect.Height() : rect.Width();
	CRect rcThumb(0, 0, thumbSize, thumbSize);
	rcThumb.OffsetRect(horiz ? CSize(pos, 0) : CSize(0, pos));

	dc.FillSolidRect(&rcThumb, THEMECOLOR(TC_BTN));
}
#endif//#if ENABLE_THEME

#endif//__Blade_ThemeScrollBar_h__