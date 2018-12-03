/********************************************************************
	created:	2011/05/09
	filename: 	UIBaseTabCtrl.cc
	author:		.dan.g.@codeproject.com
	purpose:	implementation file
*********************************************************************/
#include "stdafx.h"
#include <ui/private/UIBaseTabCtrl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CUIBaseTabCtrl, CTabCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CUIBaseTabCtrl::CUIBaseTabCtrl(int nType)
{
	m_crBack = (COLORREF)-1; // use default color
	m_nDrawType = BTC_NONE;

	mTabHeight = 0;
	mTabWidth = 0;

	EnableDraw(nType);
}

/////////////////////////////////////////////////////////////////////////////
CUIBaseTabCtrl::~CUIBaseTabCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
void CUIBaseTabCtrl::SetBkgndColor(COLORREF color)
{
	// set new color
	m_crBack = color;

	// redraw
	if (GetSafeHwnd())
		Invalidate();
}

//////////////////////////////////////////////////////////////////////////
int CUIBaseTabCtrl::GetMaxTabHeight() const
{
	if(mTabHeight != 0)
		return mTabHeight;
	else
	{
		int nItemCount = this->GetItemCount();
		if( nItemCount != 0)
		{
			CRect totalrect;
			totalrect.SetRectEmpty();
			for( int i = 0; i < nItemCount; ++i)
			{
				CRect rect;
				this->GetItemRect(0,&rect);
				totalrect.UnionRect(rect,totalrect);
			}
			mTabHeight = totalrect.Height();
			mTabWidth = totalrect.Width();
		}
		return mTabHeight;
	}
}

//////////////////////////////////////////////////////////////////////////
int	CUIBaseTabCtrl::GetMaxTabWidth() const
{
	if(mTabWidth != 0)
		return mTabWidth;
	else
	{
		int nItemCount = this->GetItemCount();
		if( nItemCount != 0)
		{
			CRect totalrect;
			totalrect.SetRectEmpty();
			for( int i = 0; i < nItemCount; ++i)
			{
				CRect rect;
				this->GetItemRect(0,&rect);
				totalrect.UnionRect(rect,totalrect);
			}
			mTabHeight = totalrect.Height();
			mTabWidth = totalrect.Width();
		}
		return mTabWidth;
	}
}

BOOL CUIBaseTabCtrl::EnableDraw(int nType)
{
	ASSERT (nType >= BTC_NONE && nType <= BTC_ALL);

	if (nType < BTC_NONE || nType > BTC_ALL)
		return FALSE;

	m_nDrawType = nType;

	if (GetSafeHwnd())
	{
		if (m_nDrawType != BTC_NONE) 
			ModifyStyle(0, TCS_OWNERDRAWFIXED);
		else
			ModifyStyle(TCS_OWNERDRAWFIXED, 0);

		Invalidate();
	}

	return TRUE;
}

void CUIBaseTabCtrl::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	ASSERT (0);
}

void CUIBaseTabCtrl::DrawItemBorder(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	ASSERT (0);
}

void CUIBaseTabCtrl::DrawMainBorder(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	ASSERT (0);
}

COLORREF CUIBaseTabCtrl::GetTabColor(BOOL /*bSelected*/)
{
	return ::GetSysColor(COLOR_3DFACE);
}

COLORREF CUIBaseTabCtrl::GetTabTextColor(BOOL /*bSelected*/)
{
	return ::GetSysColor(COLOR_WINDOWTEXT);
}

void CUIBaseTabCtrl::PreSubclassWindow() 
{
	CTabCtrl::PreSubclassWindow();

	if (m_nDrawType != BTC_NONE) 
		ModifyStyle(0, TCS_OWNERDRAWFIXED);
}

BOOL CUIBaseTabCtrl::OnEraseBkgnd(CDC* pDC) 
{
	CRect rClient, rTab, rTotalTab, rBkgnd, rEdge;
	COLORREF crBack;
	int nTab, nTabHeight = 0,nTabWidth = 0;

	//CTabCtrl::OnEraseBkgnd(pDC);

	// calc total tab width
	GetClientRect(rClient);
	nTab = GetItemCount();
	rTotalTab.SetRectEmpty();

	while (nTab--)
	{
		GetItemRect(nTab, rTab);
		rTotalTab.UnionRect(rTab, rTotalTab);
	}

	nTabHeight = rTotalTab.Height();
	nTabWidth = rTotalTab.Width();
	mTabWidth = nTabWidth;
	mTabHeight = nTabHeight;


	// add a bit
	rTotalTab.InflateRect(2, 3);
	rEdge = rTotalTab;

	// then if background color is set, paint the visible background
	// area of the tabs in the bkgnd color
	// note: the mfc code for drawing the tabs makes all sorts of assumptions
	// about the background color of the tab control being the same as the page
	// color - in some places the background color shows thru' the pages!!
	// so we must only paint the background color where we need to, which is that
	// portion of the tab area not excluded by the tabs themselves
	crBack = (m_crBack == -1) ? ::GetSysColor(COLOR_3DFACE) : m_crBack;
	
	DWORD dwStyle = (DWORD)::GetWindowLong(m_hWnd, GWL_STYLE);

	if( (dwStyle&TCS_VERTICAL) )
	{
		if( (dwStyle&TCS_RIGHT) )
		{
			// full width of tab ctrl below top of tabs
			rBkgnd = rClient;
			rBkgnd.left = rTotalTab.right - 3;
			pDC->SetBkColor(crBack);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd, TEXT(""), NULL);

			// width of tab ctrl visible bkgnd including bottom pixel of tabs to left of tabs
			rBkgnd = rClient;
			rBkgnd.bottom = 2;
			rBkgnd.left = rBkgnd.right - (nTabWidth + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);

			// to right of tabs
			rBkgnd = rClient;
			rBkgnd.top += nTabHeight - 2;
			rBkgnd.left = rBkgnd.right - (nTabHeight + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);
		}
		else
		{
			// full width of tab ctrl above top of tabs
			rBkgnd = rClient;
			rBkgnd.right = rTotalTab.left + 3;
			pDC->SetBkColor(crBack);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd, TEXT(""), NULL);

			// width of tab ctrl visible bkgnd including bottom pixel of tabs to left of tabs
			rBkgnd = rClient;
			rBkgnd.bottom = 2;
			rBkgnd.right = rBkgnd.left + (nTabWidth + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);

			// to right of tabs
			rBkgnd = rClient;
			rBkgnd.top += nTabHeight - 2;
			rBkgnd.right = rBkgnd.left + (nTabHeight + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);
		}
	}
	else 
	{
		if( dwStyle & TCS_BOTTOM )
		{
			// full width of tab ctrl below top of tabs
			rBkgnd = rClient;
			rBkgnd.top = rTotalTab.bottom - 3;
			pDC->SetBkColor(crBack);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd, TEXT(""), NULL);

			// width of tab ctrl visible bkgnd including bottom pixel of tabs to left of tabs
			rBkgnd = rClient;
			rBkgnd.right = 2;
			rBkgnd.top = rBkgnd.bottom - (nTabHeight + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);

			// to right of tabs
			rBkgnd = rClient;
			rBkgnd.left += nTabWidth - 2;
			rBkgnd.top = rBkgnd.bottom - (nTabHeight + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);
		}
		else
		{
			// full width of tab ctrl above top of tabs
			rBkgnd = rClient;
			rBkgnd.bottom = rTotalTab.top + 3;
			pDC->SetBkColor(crBack);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd, TEXT(""), NULL);

			// width of tab ctrl visible bkgnd including bottom pixel of tabs to left of tabs
			rBkgnd = rClient;
			rBkgnd.right = 2;
			rBkgnd.bottom = rBkgnd.top + (nTabHeight + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);

			// to right of tabs
			rBkgnd = rClient;
			rBkgnd.left += nTabWidth - 2;
			rBkgnd.bottom = rBkgnd.top + (nTabHeight + 2);
			pDC->ExtTextOut(rBkgnd.left, rBkgnd.top, ETO_CLIPPED | ETO_OPAQUE, rBkgnd,  TEXT(""), NULL);
		}
	}


	return TRUE;
}

void CUIBaseTabCtrl::OnPaint() 
{
	if (m_nDrawType == BTC_NONE)
	{
		Default();
	}
	else if (m_nDrawType == BTC_TABS)
	{
		ASSERT (GetStyle() & TCS_OWNERDRAWFIXED);
		Default();
	}
	else // all
	{
		CPaintDC dc(this); // device context for painting

		// prepare dc
		dc.SelectObject(GetFont());

		DRAWITEMSTRUCT dis;
		dis.CtlType = ODT_TAB;
		dis.CtlID = (UINT)GetDlgCtrlID();
		dis.hwndItem = GetSafeHwnd();
		dis.hDC = dc.GetSafeHdc();
		dis.itemAction = ODA_DRAWENTIRE;

		// draw the rest of the border
		CRect rClient, rPage;
		GetClientRect(&dis.rcItem);
		rPage = dis.rcItem;
		AdjustRect(FALSE, rPage);

		DWORD dwStyle = (DWORD)::GetWindowLong(m_hWnd,GWL_STYLE);
		if( dwStyle & TCS_VERTICAL )
		{
			if( dwStyle&TCS_RIGHT )
			{
				dis.rcItem.right -= mTabWidth + 2;
				DrawMainBorder(&dis);
			}
			else
			{
				dis.rcItem.left = rPage.left- 2;
				DrawMainBorder(&dis);
			}
		}
		else
		{
			if( dwStyle&TCS_BOTTOM )
			{
				dis.rcItem.bottom -= mTabHeight + 2;
				DrawMainBorder(&dis);
			}
			else
			{
				dis.rcItem.top = rPage.top - 2;
				DrawMainBorder(&dis);
			}
		}

		// paint the tabs first and then the borders
		int nTab = GetItemCount();
		int nSel = GetCurSel();

		if (!nTab) // no pages added
			return;

		while (nTab--)
		{
			if (nTab != nSel)
			{
				dis.itemID = (UINT)nTab;
				dis.itemState = 0;
				VERIFY(GetItemRect(nTab, &dis.rcItem));
				dis.rcItem.bottom -= 2;
				DrawItem(&dis);
				DrawItemBorder(&dis);
			}
		}

		// now selected tab
		dis.itemID = (UINT)nSel;
		dis.itemState = ODS_SELECTED;

		VERIFY(GetItemRect(nSel, &dis.rcItem));

		dis.rcItem.bottom += 2;
		dis.rcItem.top -= 2;
		DrawItem(&dis);
		DrawItemBorder(&dis);
	}
}

