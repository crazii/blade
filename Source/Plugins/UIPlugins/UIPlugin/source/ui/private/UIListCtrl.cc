/********************************************************************
	created:	2011/05/25
	filename: 	UIListCtrl.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ui/private/UIListCtrl.h>
#include <ui/IconManager.h>

IMPLEMENT_DYNAMIC(CUIListCtrl,CListCtrl);

BEGIN_MESSAGE_MAP(CUIListCtrl,CListCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CUIListCtrl::CUIListCtrl()
{

}

//////////////////////////////////////////////////////////////////////////
CUIListCtrl::~CUIListCtrl()
{

}

/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
BOOL CUIListCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED | WS_BORDER;
	return CListCtrl::PreCreateWindow(cs);
}

//////////////////////////////////////////////////////////////////////////
void CUIListCtrl::PreSubclassWindow()
{
	__super::PreSubclassWindow();
}

//////////////////////////////////////////////////////////////////////////
void CUIListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	pDC->SetBkMode(TRANSPARENT);

	CRect rcItem(lpDrawItemStruct->rcItem);
	int nItem = (int)lpDrawItemStruct->itemID;
	CImageList* pImageList;

	// Save dc state
	int nSavedDC = pDC->SaveDC();

	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;		// get all state flags
	GetItem(&lvi);

	// Should the item be highlighted
	BOOL bHighlight = ((lvi.state & LVIS_DROPHILITED)
		|| ( (lvi.state & LVIS_SELECTED)
		&& ((GetFocus() == this)
		|| (GetStyle() & LVS_SHOWSELALWAYS) ) ) );

	// Get rectangles for drawing
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel, LVIR_LABEL);
	GetItemRect(nItem, rcIcon, LVIR_ICON);
	CRect rcCol( rcBounds );

	CString sLabel = GetItemText( nItem, 0 );

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;

	CRect rcHighlight;
	CRect rcWnd;
	rcHighlight = rcLabel;

	// Draw the background color
	pDC->FillSolidRect(rcHighlight, THEMECOLOR(bHighlight ? TC_HILIGHTBK : TC_CTLBK));

	// Set clip region
	rcCol.right = rcCol.left + GetColumnWidth(0);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcCol);
	pDC->SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// Draw normal and overlay icon
	bool cut = (lvi.state & LVIS_CUT) != 0;

	if( lvi.iImage != -1 )
	{
		pImageList = Blade::IconManager::getSingleton().getIconList(Blade::IS_24, cut);
		if (pImageList != NULL)
			pImageList->Draw(pDC, lvi.iImage, CPoint(rcCol.left, rcCol.top), ILD_NORMAL);
	}

	// Draw item label - Column 0
	rcLabel.left += offset/2;
	rcLabel.right -= offset;

	COLORREF textColor = THEMECOLOR(cut ? TC_GRAYTEXT : TC_TEXT);
	pDC->SetTextColor( textColor );

	pDC->DrawText(sLabel,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP
		| DT_VCENTER | DT_END_ELLIPSIS);

	// Draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right :
		rcBounds.right;
	rgn.CreateRectRgnIndirect(&rcBounds);
	pDC->SelectClipRgn(&rgn);

	for(int nColumn = 1; this->GetColumn(nColumn, &lvc); nColumn++)
	{
		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		sLabel = GetItemText(nItem, nColumn);
		if (sLabel.GetLength() == 0)
			continue;

		// Get the text justification
		UINT nJustify = DT_LEFT;
		switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
		case LVCFMT_RIGHT:
			nJustify = DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nJustify = DT_CENTER;
			break;
		default:
			break;
		}

		rcLabel = rcCol;
		rcLabel.left += offset;
		rcLabel.right -= offset;

		pDC->DrawText(sLabel, -1, rcLabel, nJustify | DT_SINGLELINE |
			DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
	}

	// Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
		pDC->DrawFocusRect(rcHighlight);

	// Restore dc
	pDC->RestoreDC( nSavedDC );
}

//////////////////////////////////////////////////////////////////////////
BOOL CUIListCtrl::OnEraseBkgnd(CDC* pDC)
{
#if ENABLE_THEME
	CRect rect;
	this->GetClientRect(&rect);
	pDC->FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));
	return TRUE;
#else
	return CListCtrl::OnEraseBkgnd(pDC);
#endif
}


//////////////////////////////////////////////////////////////////////////
HBRUSH  CUIListCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
#if ENABLE_THEME
	BLADE_UNREFERENCED(pWnd);
	BLADE_UNREFERENCED(nCtlColor);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(THEMECOLOR(TC_TEXT));
	return THEMEBRUSH(TC_CTLBK);
#else
	return CListCtrl::OnCtlColor(pDC, pWnd, nCtlColor);
#endif
}

//////////////////////////////////////////////////////////////////////////
int CUIListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = __super::OnCreate(lpCreateStruct);
#if ENABLE_THEME
	this->SetBkColor(THEMECOLOR(TC_CTLBK));
#endif
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CUIListCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	__super::OnNcCalcSize(bCalcValidRects, lpncsp);
	CRect rc(lpncsp->rgrc[0]);
	rc.DeflateRect(1,1,1,1);
	lpncsp->rgrc[0] = rc;
}

//////////////////////////////////////////////////////////////////////////
void CUIListCtrl::OnNcPaint()
{
	__super::OnNcPaint();
#if ENABLE_THEME
	CWindowDC dc(this);
	CRect rect;
	this->GetWindowRect(&rect);
	rect.OffsetRect(-rect.left, -rect.top);
	dc.FrameRect(&rect, &THEMEBRUSH(TC_FRAME));
#endif
}
