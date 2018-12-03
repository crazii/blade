/********************************************************************
	created:	2016/07/18
	filename: 	ThemeButton.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ThemeButton_h__
#define __Blade_ThemeButton_h__
#include <stdafx.h>
#include <Windowsx.h>

#if ENABLE_THEME

//////////////////////////////////////////////////////////////////////////
void Theme_PaintButton(HWND hWnd, WNDPROC /*proc*/)
{
	CRect rcUpdate;
	if(::GetUpdateRect(hWnd, &rcUpdate, FALSE) == FALSE)
		return;

	CWnd* wnd = CWnd::FromHandle(hWnd);
	DWORD style = wnd->GetStyle();
	CRect rect;
	wnd->GetClientRect(&rect);

	UINT format = (style&BS_MULTILINE) ? 0u : DT_SINGLELINE;
	int state = Button_GetState(hWnd);
	bool hot = (state&BST_HOT) != 0;
	bool pushed = (state&BST_PUSHED) != 0;
	bool disabled = (style&WS_DISABLED) != 0;

	CPaintDC dc(wnd);
	dc.IntersectClipRect(&rcUpdate);
	dc.SetBkMode(TRANSPARENT);

	if ((style&BS_OWNERDRAW) == BS_OWNERDRAW && ::GetParent(hWnd) != NULL)
	{
		::DRAWITEMSTRUCT dis;
		dis.CtlType = ODT_BUTTON;
		dis.itemID = (UINT)::GetDlgCtrlID(hWnd);
		dis.itemAction = ODA_DRAWENTIRE;
		dis.itemState = (UINT)(hot ? ODS_HOTLIGHT : (disabled ? ODS_DISABLED : ODS_DEFAULT));
		dis.hwndItem = hWnd;
		dis.hDC = dc.GetSafeHdc();
		dis.rcItem = rcUpdate;
		dis.itemData = NULL;
		::SendMessage(::GetParent(hWnd), WM_DRAWITEM, (WPARAM)dis.itemID, (LPARAM)&dis);
	}
	else
	{
		HBITMAP hBitmap = ((CButton*)wnd)->GetBitmap();
		bool bitmap = (style&BS_BITMAP) && hBitmap != NULL;
		if(bitmap || (style&BS_PUSHLIKE))
			style &= ~(BS_CHECKBOX|BS_RADIOBUTTON);

		CRect rcText(rect);
		CRect rcBox(rect);

		UINT aligment = 0;
		HBRUSH brush = (HBRUSH)::SendMessage(::GetParent(hWnd), WM_CTLCOLORBTN, (WPARAM)dc.GetSafeHdc(), (LPARAM)hWnd);
		if(((style&BS_CHECKBOX) || (style&BS_RADIOBUTTON)))
		{
			aligment = DT_LEFT;
			::FillRect(dc.GetSafeHdc(), &rect, brush);
			
			//TODO: draw check/radio
			if ((style&BS_LEFTTEXT))//check/radio on the right
			{
				rcBox.left = max(rcBox.right - rcBox.Height(), 0);
				rcText.right -= rcBox.Width();
			}
			else//check/radio on the left
			{
				rcBox.right = rcBox.left + rcBox.Height();
				rcText.left = rcBox.right;
			}
			rcBox.DeflateRect(1,1,1,1);

			const int MAX_SIZE = 13;
			int widthExt = max(rcBox.Width()-MAX_SIZE, 0);
			int heightExt = max(rcBox.Height()-MAX_SIZE, 0);
			rcBox.DeflateRect(widthExt/2, heightExt/2, (widthExt+1)/2, (heightExt+1)/2);
			dc.FillSolidRect(&rcBox, pushed ? THEMECOLOR(TC_PUSHEDBTN) : THEMECOLOR(TC_HILIGHTBTN));
		}
		else
		{
			aligment = DT_CENTER;
			dc.FillSolidRect(&rect, pushed ? THEMECOLOR(TC_PUSHEDBTN) : ((hot&&!disabled) ? THEMECOLOR(TC_HILIGHTBTN) : THEMECOLOR(TC_BTN)));
		}

		//draw frame
		CDC maskMemDC;
		maskMemDC.CreateCompatibleDC(&dc);
		CBitmap maskBitmap;
		maskBitmap.CreateCompatibleBitmap(&dc, rcBox.Width(), rcBox.Height());
		maskMemDC.SelectObject( &maskBitmap );
		CRect rcMask(0, 0, rcBox.Width(), rcBox.Height());
		UINT dfcs = (UINT)((style&BS_RADIOBUTTON) ? DFCS_BUTTONRADIO : ((style&BS_CHECKBOX) ? DFCS_BUTTONCHECK : DFCS_BUTTONPUSH) );
		if(disabled)
			dfcs |= DFCS_INACTIVE;
		else
		{
			if(hot && dfcs == DFCS_BUTTONPUSH)
				dfcs |= DFCS_HOT;
			if(pushed)
				dfcs |= DFCS_PUSHED;
			if(state&BST_CHECKED)
				dfcs |= DFCS_CHECKED;
		}
		maskMemDC.DrawFrameControl(&rcMask, DFC_BUTTON, dfcs);

		dc.BitBlt(rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(), &maskMemDC, 0, 0, SRCINVERT);
		dc.BitBlt(rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(), &maskMemDC, 0, 0, SRCAND);
		dc.BitBlt(rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(), &maskMemDC, 0, 0, SRCINVERT);

		if(bitmap)
		{
			//dc.DrawState(rect.TopLeft(), rect.Size(), hBitmap, DST_BITMAP|DSS_NORMAL);
			CDC memDC;
			memDC.CreateCompatibleDC(&dc);
			memDC.SelectObject(hBitmap);
			BITMAP bmp;
			::GetObject(hBitmap, sizeof(BITMAP), &bmp);

			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 0xFF;
			bf.AlphaFormat = AC_SRC_ALPHA;
			dc.AlphaBlend(rcBox.left+2, rcBox.top+2, rcBox.Width()-2, rcBox.Height()-2, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);
		}
		else
		{
			if(style&BS_RIGHT)
				format |= DT_RIGHT;
			else if(style&BS_LEFT)
				format |= DT_LEFT;
			else if(style&BS_CENTER)
				format |= DT_CENTER;
			else
				format |= aligment;

			if(style&BS_BOTTOM)
				format |= DT_BOTTOM;
			else if(style&BS_TOP)
				format |= DT_TOP;
			else
				format |= DT_VCENTER;

			CString text;
			wnd->GetWindowText(text);
			CFont* font = wnd->GetFont();

			COLORREF oldTextColor = dc.SetTextColor(THEMECOLOR(TC_TEXT));
			CFont* oldFont = dc.SelectObject(font);
			dc.DrawText(text, text.GetLength(), &rcText, format);
			dc.SetTextColor(oldTextColor);
			dc.SelectObject(oldFont);
		}
	}
}
#endif//#if ENABLE_THEME

#endif // __Blade_ThemeButton_h__