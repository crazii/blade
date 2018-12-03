//_ **********************************************************
//_ 
//_ Name: EnTabCtrl.cc
//_ Purpose: 
//_ Created: 15 September 1998 
//_ Author: D.R.Godson
//_ Modified By: 
//_ 
//_ Copyright (c) 1998 Brilliant Digital Entertainment Inc. 
//_ 
//_ **********************************************************

// EnTabCtrl.cc : implementation file
//
#include "stdafx.h"
#include <ui/private/UITabCtrl.h>
#include <interface/public/ui/UITypes.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Blade;

/////////////////////////////////////////////////////////////////////////////
// CUITabCtrl

//DWORD CUITabCtrl::m_dwCustomLook = 0;

enum { PADDING = 3, EDGE = 20};

//////////////////////////////////////////////////////////////////////////////
// helpers
COLORREF Darker(COLORREF crBase, float fFactor)
{
	ASSERT (fFactor < 1.0f && fFactor > 0.0f);

	fFactor = min(fFactor, 1.0f);
	fFactor = max(fFactor, 0.0f);

	BYTE bRed, bBlue, bGreen;
	BYTE bRedShadow, bBlueShadow, bGreenShadow;

	bRed = GetRValue(crBase);
	bBlue = GetBValue(crBase);
	bGreen = GetGValue(crBase);

	bRedShadow = (BYTE)(bRed * fFactor);
	bBlueShadow = (BYTE)(bBlue * fFactor);
	bGreenShadow = (BYTE)(bGreen * fFactor);

	return RGB(bRedShadow, bGreenShadow, bBlueShadow);
}

COLORREF Lighter(COLORREF crBase, float fFactor) 
{
	ASSERT (fFactor > 1.0f);

	fFactor = max(fFactor, 1.0f);

	BYTE bRed, bBlue, bGreen;
	BYTE bRedHilite, bBlueHilite, bGreenHilite;

	bRed = GetRValue(crBase);
	bBlue = GetBValue(crBase);
	bGreen = GetGValue(crBase);

	bRedHilite = (BYTE)min((int)(bRed * fFactor), 255);
	bBlueHilite = (BYTE)min((int)(bBlue * fFactor), 255);
	bGreenHilite = (BYTE)min((int)(bGreen * fFactor), 255);

	return RGB(bRedHilite, bGreenHilite, bBlueHilite);
}

CSize FormatText(CString& sText, CDC* pDC, int nWidth)
{
	CRect rect(0, 0, nWidth, 20);
	UINT uFlags = DT_CALCRECT | DT_SINGLELINE | DT_MODIFYSTRING | DT_END_ELLIPSIS;
	
	::DrawText(pDC->GetSafeHdc(), sText.GetBuffer(sText.GetLength() + 4), -1, rect, uFlags);
	sText.ReleaseBuffer();

	return pDC->GetTextExtent(sText);
}

////////////////////////////////////////////////////////////////////////////////////////

CUITabCtrl::CUITabCtrl()
:m_dwCustomLook(0)
{
	m_crBack = THEMECOLOR(TC_WINBK);
}

CUITabCtrl::~CUITabCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CUITabCtrl message handlers

void CUITabCtrl::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
	TC_ITEM     tci;
	CDC* pDC = CDC::FromHandle(lpdis->hDC);
	HIMAGELIST hilTabs = (HIMAGELIST)TabCtrl_GetImageList(GetSafeHwnd());

	BOOL bSelected = (lpdis->itemID == (UINT)GetCurSel());
	BOOL bColor = BOOL(m_dwCustomLook & ETC_COLOR);

	CRect rItem(lpdis->rcItem);

	DWORD dwStyle = (DWORD)::GetWindowLong(m_hWnd,GWL_STYLE);

	if( dwStyle & TCS_VERTICAL )
	{
	}
	else
	{
		if(dwStyle&TCS_BOTTOM)
		{
			if (bSelected)
			{
				rItem.top -= 1;
				this->DrawItemRect(pDC,rItem,bColor,bSelected);
				rItem.top += 4;
			}
			else
			{
				rItem.bottom += 2;
				this->DrawItemRect(pDC,rItem,bColor,bSelected);
			}
		}
		else
		{
			if (bSelected)
				rItem.bottom -= 2;
			else
				rItem.bottom += 2;

			this->DrawItemRect(pDC,rItem,bColor,bSelected);
		}
	}

	// text & icon
	rItem.left += PADDING;
	rItem.top += PADDING;

	pDC->SetBkMode(TRANSPARENT);

	CString sTemp;
	tci.mask        = TCIF_TEXT | TCIF_IMAGE;
	tci.pszText     = sTemp.GetBuffer(100);
	tci.cchTextMax  = 99;
	this->GetItem((int)lpdis->itemID, &tci);
	sTemp.ReleaseBuffer();

	// icon
	if (tci.iImage != INVALID_ICON_INDEX && hilTabs != NULL )
	{
		ImageList_Draw(hilTabs, tci.iImage, *pDC, rItem.left, rItem.top, ILD_NORMAL);
		rItem.left += 16 + PADDING;
	}

	// text
	if( sTemp.GetLength() > 0)
	{
		rItem.right -= PADDING;
		FormatText(sTemp, pDC, rItem.Width());

		pDC->SetTextColor(GetTabTextColor(bSelected));

		CFont* oldFont = pDC->SelectObject( this->GetFont() );
		pDC->DrawText(sTemp, rItem, DT_NOPREFIX | DT_LEFT | DT_VCENTER);
		pDC->SelectObject(oldFont);
	}
}

void CUITabCtrl::DrawItemBorder(LPDRAWITEMSTRUCT lpdis)
{
	ASSERT (m_dwCustomLook & ETC_FLAT);

	BOOL bSelected = (lpdis->itemID == (UINT)GetCurSel());
	//BOOL bBackTabs = (m_dwCustomLook & ETC_BACKTABS);

	CRect rItem(lpdis->rcItem);
	CDC* pDC = CDC::FromHandle(lpdis->hDC); 

	COLORREF crTab = GetTabColor(bSelected);
	COLORREF crHighlight = Lighter(crTab, 1.2f);
	COLORREF crShadow = Darker(crTab, 0.75f);

	//modified by Crazii
	{

		//if (bSelected || bBackTabs)
		//{
		//	rItem.bottom += bSelected ? -1 : 1;

		//	// edges
		//	pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.left + 1, rItem.bottom), crHighlight);
		//	pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.right, rItem.top + 1), crHighlight);
		//	pDC->FillSolidRect(CRect(rItem.right - 1, rItem.top, rItem.right, rItem.bottom), crShadow);
		//}
		//else // draw simple dividers
		//{
		//	pDC->FillSolidRect(CRect(rItem.left - 1, rItem.top, rItem.left, rItem.bottom), crShadow);
		//	pDC->FillSolidRect(CRect(rItem.right - 1, rItem.top, rItem.right, rItem.bottom), crShadow);
		//}

		DWORD dwStyle = (DWORD)::GetWindowLong(m_hWnd,GWL_STYLE);

		if( dwStyle & TCS_VERTICAL )
		{
			if( dwStyle & TCS_RIGHT )
			{
				rItem.right += bSelected ? -1 : 1;
				rItem.left += bSelected ? 1 : -1;
			}

			// edges
			pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.right, rItem.top+1), crHighlight);

			if( dwStyle & TCS_RIGHT )
				pDC->FillSolidRect(CRect(rItem.right, rItem.top, rItem.right+1, rItem.bottom), crHighlight);
			else
				pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.left+1, rItem.bottom), crHighlight);

			pDC->FillSolidRect(CRect(rItem.left, rItem.bottom-1, rItem.right, rItem.bottom), crShadow);
		}
		else
		{
			if( dwStyle & TCS_BOTTOM )
			{
				rItem.bottom += bSelected ? -1 : 1;
				rItem.top += bSelected ? 1 : -1;
			}
			else
			{
				rItem.right += bSelected ? -1 : 1;
				rItem.left += bSelected ? 1 : -1;
				rItem.bottom += bSelected ? -2 : 2;
			}
				
			// edges
			pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.left + 1, rItem.bottom), crHighlight);

			if( dwStyle & TCS_BOTTOM )
				pDC->FillSolidRect(CRect(rItem.left, rItem.bottom, rItem.right, rItem.bottom + 1), crShadow);
			else
				pDC->FillSolidRect(CRect(rItem.left, rItem.top, rItem.right, rItem.top + 1), crHighlight);

			pDC->FillSolidRect(CRect(rItem.right - 1, rItem.top, rItem.right, rItem.bottom), crShadow);
		}

	}
}

//////////////////////////////////////////////////////////////////////////
void	CUITabCtrl::DrawItemRect(CDC* pDC,const CRect& rect,BOOL bUseColor,BOOL bSelected)
{
	// tab
	// blend from back color to COLOR_3DFACE if 16 bit mode or better
	COLORREF crFrom = GetTabColor(bSelected);

	if (m_dwCustomLook & ETC_GRADIENT && pDC->GetDeviceCaps(BITSPIXEL) >= 16)
	{
		COLORREF crTo = bSelected ? ::GetSysColor(COLOR_3DFACE) : Darker(!bUseColor || m_crBack == -1 ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 0.7f);

		int nROrg = GetRValue(crFrom);
		int nGOrg = GetGValue(crFrom);
		int nBOrg = GetBValue(crFrom);
		int nRDiff = GetRValue(crTo) - nROrg;
		int nGDiff = GetGValue(crTo) - nGOrg;
		int nBDiff = GetBValue(crTo) - nBOrg;

		int nHeight = rect.Height();

		for (int nLine = 0; nLine < nHeight; nLine += 2)
		{
			int nRed = nROrg + (nLine * nRDiff) / nHeight;
			int nGreen = nGOrg + (nLine * nGDiff) / nHeight;
			int nBlue = nBOrg + (nLine * nBDiff) / nHeight;

			pDC->FillSolidRect(CRect(rect.left, rect.top + nLine, rect.right, rect.top + nLine + 2), 
				RGB(nRed, nGreen, nBlue));
		}
	}
	else // simple solid fill
		pDC->FillSolidRect(rect, crFrom);
}

void CUITabCtrl::DrawMainBorder(LPDRAWITEMSTRUCT lpdis)
{
	CRect rBorder(lpdis->rcItem);
	CDC* pDC = CDC::FromHandle(lpdis->hDC); 

	pDC->FillSolidRect(rBorder, m_crBack);

	BOOL bHiliteSel = BOOL(m_dwCustomLook & ETC_SELECTION);
	if(bHiliteSel)
	{
		rBorder.DeflateRect(1,1,1,1);
		pDC->FillSolidRect(&rBorder, this->GetTabColor(TRUE) );
	}
}

COLORREF CUITabCtrl::GetTabColor(BOOL bSelected)
{
	BOOL bColor = BOOL(m_dwCustomLook & ETC_COLOR);
	BOOL bHiliteSel = BOOL(m_dwCustomLook & ETC_SELECTION);
	BOOL bBackTabs = BOOL(m_dwCustomLook & ETC_BACKTABS);
	BOOL bFlat = BOOL(m_dwCustomLook & ETC_FLAT);

	if (bSelected && bHiliteSel)
	{
		if (bColor)
			return Lighter((m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 1.4f);
		else
			return Lighter(::GetSysColor(COLOR_3DFACE), 1.4f);
	}
	else if (!bSelected)
	{
		if (bBackTabs || !bFlat)
		{
			if (bColor)
				return Darker((m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack, 0.9f);
			else
				return Darker(::GetSysColor(COLOR_3DFACE), 0.9f);
		}
		else
			return (m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack;
	}

	// else
	return (m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack;
}

COLORREF CUITabCtrl::GetTabTextColor(BOOL bSelected)
{
	//BOOL bColor = BOOL(m_dwCustomLook & ETC_COLOR);
	//BOOL bFlat = BOOL(m_dwCustomLook & ETC_FLAT);

	if (bSelected)
		return THEMECOLOR(TC_TEXT);
	else
		return Darker(THEMECOLOR(TC_TEXT), 0.8f);
}

void CUITabCtrl::EnableCustomLook(BOOL bEnable, DWORD dwStyle)
{
	if (!bEnable)
		dwStyle = 0;

	m_dwCustomLook = dwStyle;
}

//////////////////////////////////////////////////////////////////////////
BOOL CUITabCtrl::Create(DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID)
{
	BOOL result = CUIBaseTabCtrl::Create(dwStyle,rect,pParentWnd,nID);
	if(result)
	{
		//LOGFONT lf; 
		//::GetObject(::GetStockObject(DEFAULT_GUI_FONT),sizeof(lf),&lf);
		//m_Font.CreateFontIndirect(&lf);

		CFont *font = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
		this->SetFont(font);
	}
	return result;
}

void CUITabCtrl::PreSubclassWindow() 
{
	CUIBaseTabCtrl::PreSubclassWindow();

	if (m_dwCustomLook)
		ModifyStyle(0, TCS_OWNERDRAWFIXED);
}
