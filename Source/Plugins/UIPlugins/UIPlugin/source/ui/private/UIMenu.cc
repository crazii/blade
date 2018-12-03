/********************************************************************
	created:	2011/05/08
	filename: 	UIMenu.cc
	author:		Crazii
	purpose:	MFC CMenu derived class
*********************************************************************/
#include "stdafx.h"
#include <ui/private/UIMenu.h>
#include <ui/IconManager.h>

/************************************************************************/
/*                                                                      */
/************************************************************************/

using namespace Blade;

typedef Blade::MENU_DATA const* LPMENUDATA;

#define ICON_SIZE				(16)
#define ICONSIZE_ENUM			(IS_16)
#define ICON_RECT_WIDTH			(28)
#define ICON_RECT_HEIGHT		(22)
#define ICON_CHECKBORDER_SIZE	(2)

#define TEXT_BORDER		(8)
#define TEXT_OFFSET		(ICON_RECT_WIDTH+TEXT_BORDER)
#define BITMAP_SIZE		(16)	//bitmap used for draw check/radio/arrow mask

#define ICON_CHECKED_COLOR COLORREF(0xe8e6e1)
#define ICON_CHECKED_HICOLOR COLORREF(0xe2b598)

/************************************************************************/
/*                                                                      */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
void CUIMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	IMenu* menu = (IMenu*)lpMeasureItemStruct->itemData;

	//separator
	if(menu == NULL || menu->isSeparator())
	{
		lpMeasureItemStruct->itemHeight = 3;
		return;
	}
	LPMENUDATA	lpItem = menu->getMenuData();

	CDC* pDC = ::AfxGetMainWnd()->GetDC();
	const TString& strText= lpItem->mText;
	CSize  size;
	size = pDC->GetTextExtent(strText.c_str(), (int)strText.size() );

	if(menu->getParent() != NULL)	//common menu
	{
		lpMeasureItemStruct->itemWidth = (UINT)size.cx + TEXT_OFFSET + BITMAP_SIZE;
		if ( (lpItem->mFlags & UIMF_SUBITEM) )
			lpMeasureItemStruct->itemWidth += size.cy;

		lpMeasureItemStruct->itemHeight = max((UINT)size.cy, ICON_RECT_HEIGHT);
	}
	else	//root menus in menu bar
	{
		lpMeasureItemStruct->itemWidth = (UINT)size.cx;
		lpMeasureItemStruct->itemHeight = (UINT)size.cy;
	}
	::AfxGetMainWnd()->ReleaseDC(pDC);
}

//////////////////////////////////////////////////////////////////////////
void CUIMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	VERIFY(pDC);

	pDC->SetBkMode(TRANSPARENT);
	CRect rcItem = lpDIS->rcItem;
	UINT uState = lpDIS->itemState;
	//use standard bool to make compiler happy
	bool grayed = (uState & (ODS_GRAYED|ODS_DISABLED)) != 0;
	bool highLight = (uState & (ODS_SELECTED|ODS_HOTLIGHT)) != 0;

	IMenu* menu = (IMenu*)lpDIS->itemData;
	bool topLevel = menu != NULL && menu->getParent() == NULL;

	COLORREF foregroundColor = THEMECOLOR(TC_TEXT);
	if( grayed )
		foregroundColor = topLevel ? THEMECOLOR(TC_GRAYTEXT) : THEMECOLOR(TC_GRAYMENUTEXT);

	COLORREF backColor = topLevel ? THEMECOLOR(TC_WINBK) : THEMECOLOR(TC_MENUBK);

	//selected (high-light)
	if( highLight && !grayed )
		backColor = THEMECOLOR(TC_HILIGHTMENUBK);

	COLORREF oldTextColor = pDC->SetTextColor( foregroundColor );

	if(!topLevel)
	{
		pDC->FillSolidRect(&rcItem, backColor);

		CRect IconRect = rcItem;
		IconRect.right = ICON_RECT_WIDTH;
		pDC->FillSolidRect(&IconRect, THEMECOLOR(TC_MENULINE));

		//draw vertical line on the right of the icon bar
		{
			CPen pen(PS_SOLID, 1, THEMECOLOR(TC_MENULINE));
			CPen* oldPen = pDC->SelectObject(&pen);
			POINT ptSepStart;
			POINT ptSepEnd;
			ptSepStart.x = ICON_RECT_WIDTH;
			ptSepStart.y = rcItem.top;
			ptSepEnd.x = ptSepStart.x;
			ptSepEnd.y = rcItem.bottom;
			pDC->MoveTo( ptSepStart );
			pDC->LineTo(ptSepEnd);
			pDC->SelectObject(oldPen);
		}
		rcItem.DeflateRect(TEXT_OFFSET, 0, 0, 0);
	}
	else
		pDC->FillSolidRect(&rcItem, backColor);

	//draw separator
	if(menu == NULL || menu->isSeparator())
	{
		CPen pen(PS_SOLID, 1, THEMECOLOR(TC_MENULINE));
		CPen* oldPen = pDC->SelectObject(&pen);

		POINT ptSepStart;
		POINT ptSepEnd;

		ptSepStart.x = ICON_RECT_WIDTH + TEXT_BORDER/2;
		ptSepStart.y = rcItem.CenterPoint().y;

		ptSepEnd.x = rcItem.right;
		ptSepEnd.y = ptSepStart.y;
		pDC->MoveTo( ptSepStart );
		pDC->LineTo(ptSepEnd);

		pDC->SelectObject(oldPen);
		pDC->SetTextColor(oldTextColor);
		CDC::DeleteTempMap();
		return;
	}

	//draw checked border around icon
	if( (uState&ODS_CHECKED) && !grayed )
	{
		int x = lpDIS->rcItem.left + (ICON_RECT_WIDTH-ICON_SIZE)/2;
		int y = lpDIS->rcItem.top + (ICON_RECT_HEIGHT-ICON_SIZE)/2;
		CRect rect;
		rect.left = x - ICON_CHECKBORDER_SIZE;
		rect.top = y - ICON_CHECKBORDER_SIZE;
		rect.right = x + ICON_SIZE+ICON_CHECKBORDER_SIZE;
		rect.bottom = y + ICON_SIZE+ICON_CHECKBORDER_SIZE;

		pDC->FrameRect(&rect, &THEMEBRUSH(TC_TEXT));
		rect.DeflateRect(1,1,1,1);
	}

	CRect maskRect(0, 0, BITMAP_SIZE, BITMAP_SIZE);

	CDC maskMemDC;
	CBitmap maskBitmap;
	maskMemDC.CreateCompatibleDC(pDC);
	maskBitmap.CreateCompatibleBitmap(pDC, BITMAP_SIZE, BITMAP_SIZE);
	maskMemDC.SelectObject( &maskBitmap );

	CDC fillMemDC;
	fillMemDC.CreateCompatibleDC(pDC);
	CBitmap fillBitmap;
	fillBitmap.CreateCompatibleBitmap(pDC, BITMAP_SIZE, BITMAP_SIZE);
	fillMemDC.SelectObject(&fillBitmap);
	{
		CBrush brush;
		brush.CreateSolidBrush(foregroundColor);
		fillMemDC.FillRect(maskRect, &brush);
	}

	LPMENUDATA	lpItem = menu->getMenuData();
	//draw icons
	if(topLevel)
	{
	}
	else if( lpItem->mIcon != INVALID_ICON_INDEX )
	{
		CImageList* list = IconManager::getSingleton().getIconList(ICONSIZE_ENUM, grayed);
		int x = lpDIS->rcItem.left + (ICON_RECT_WIDTH-ICON_SIZE)/2;
		int y = lpDIS->rcItem.top + (ICON_RECT_HEIGHT-ICON_SIZE)/2;
		::ImageList_Draw(*list, lpItem->mIcon, *pDC, x, y, ILD_NORMAL);
	}
	else	//no icon, default check or radio
	{
		int x = lpDIS->rcItem.left + (ICON_RECT_WIDTH-ICON_SIZE)/2;
		int y = lpDIS->rcItem.top + (ICON_RECT_HEIGHT-ICON_SIZE)/2;

		if( (uState & ODS_CHECKED) )
		{
			CRect rect;
			rect.left = x - ICON_CHECKBORDER_SIZE;
			rect.top = y - ICON_CHECKBORDER_SIZE;
			rect.right = x + ICON_SIZE+ICON_CHECKBORDER_SIZE;
			rect.bottom = y + ICON_SIZE+ICON_CHECKBORDER_SIZE;

			UINT nState; 
			if( (lpItem->mFlags&UIMF_GROUP) )
				//radio
				nState = DFCS_MENUBULLET;
			else
				//checked
				nState = DFCS_MENUCHECK;

			maskMemDC.DrawFrameControl(maskRect, DFC_MENU, nState);
			pDC->BitBlt(rect.left + (rect.Width()-maskRect.Width())/2, rect.top + (rect.Height()-maskRect.Height())/2,
				maskRect.Width(), maskRect.Height(), &fillMemDC, 0, 0, SRCINVERT);
			pDC->BitBlt(rect.left + (rect.Width()-maskRect.Width())/2, rect.top + (rect.Height()-maskRect.Height())/2,
				maskRect.Width(), maskRect.Height(), &maskMemDC, 0, 0, SRCAND);
			pDC->BitBlt(rect.left + (rect.Width()-maskRect.Width())/2, rect.top + (rect.Height()-maskRect.Height())/2,
				maskRect.Width(), maskRect.Height(), &fillMemDC, 0, 0, SRCINVERT);
		}
	}

	//draw text
	UINT fmt = DT_VCENTER | DT_SINGLELINE;
	if(!topLevel)
		rcItem.DeflateRect(0, 0, TEXT_BORDER , 0);

	const TString& sText  = lpItem->mText;

	TStringTokenizer tokenizer;
	tokenizer.tokenize( sText, TEXT("\t") );
	if( tokenizer.size() == 2 )
	{
		pDC->DrawText(tokenizer[0].c_str(), &rcItem, DT_LEFT | fmt);
		pDC->DrawText(tokenizer[1].c_str(), &rcItem, DT_RIGHT | fmt);
	}
	else
		pDC->DrawText(sText.c_str(), &rcItem, topLevel ? (fmt | DT_CENTER) : (DT_LEFT | fmt));

	pDC->SetTextColor(oldTextColor);

	//draw the sub menu arrow
	if ( !(lpItem->mFlags&UIMF_SUBITEM) && !(lpItem->mFlags&UIMF_EMBEDDED) && !topLevel)
	{
		CRect rect(maskRect);
		CRect itemRect(lpDIS->rcItem);
		rect.OffsetRect(itemRect.left + itemRect.Width()-BITMAP_SIZE,itemRect.top + (itemRect.Height()-BITMAP_SIZE)/2);

		maskMemDC.DrawFrameControl(maskRect, DFC_MENU, DFCS_MENUARROW);
		pDC->BitBlt(rect.left, rect.top, maskRect.Width(), maskRect.Height(), &fillMemDC, 0, 0, SRCINVERT);
		pDC->BitBlt(rect.left, rect.top, maskRect.Width(), maskRect.Height(), &maskMemDC, 0, 0, SRCAND);
		pDC->BitBlt(rect.left, rect.top, maskRect.Width(), maskRect.Height(), &fillMemDC, 0, 0, SRCINVERT);
	}
	//prevent system's default arrow drawing
	::ExcludeClipRect(pDC->m_hDC, lpDIS->rcItem.left, lpDIS->rcItem.top, lpDIS->rcItem.right, lpDIS->rcItem.bottom);        
	CDC::DeleteTempMap();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
#if BLADE_USE_MFC_FEATURE_PACK
IMPLEMENT_DYNAMIC(CUIPopMenu, CMFCPopupMenu);

//////////////////////////////////////////////////////////////////////////
class CUIMenuButton : public CMFCToolBarMenuButton, public Blade::Allocatable
{
	DECLARE_DYNCREATE(CUIMenuButton);
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	/** @brief  */
	virtual CMFCPopupMenu* CreatePopupMenu() { return BLADE_NEW CUIPopMenu; }
	/** @brief  */
	//virtual BOOL OnClickMenuItem() { return TRUE; }
	/** @brief  */
	virtual void OnAfterCreatePopupMenu() 
	{
		CUIPopMenu* popMenu = DYNAMIC_DOWNCAST(CUIPopMenu, m_pPopupMenu);
		assert(popMenu != NULL);
		HMENU hMenu = m_pPopupMenu->GetHMenu();
		assert(::IsMenu(hMenu));

		HMENU realMenu = NULL;
		int index = -1;
		{
			CUIMenuBar* menu = DYNAMIC_DOWNCAST(CUIMenuBar, this->GetParentWnd());
			if(menu != NULL)
			{
				realMenu = menu->GetHMenu();
				int count = menu->GetCount();
				for(int i = 0; i < count; ++i)
				{
					if(menu->GetMenuItem(i) == this )
					{
						index = i;
						break;
					}
				}
			}
		}

		{
			CUIPopupMenuBar* popMenuBar = DYNAMIC_DOWNCAST(CUIPopupMenuBar, this->GetParentWnd());
			CUIPopMenu* popMenu = popMenuBar != NULL ? DYNAMIC_DOWNCAST(CUIPopMenu, popMenuBar->GetParent()) : NULL;
			if(popMenu != NULL)
			{
				realMenu = popMenu->mMenu;
				int count = popMenu->GetMenuItemCount();
				for(int i = 0; i < count; ++i)
				{
					if(popMenu->GetMenuItem(i) == this)
					{
						index = i;
						break;
					}
				}
			}
		}

		assert(realMenu != NULL && index != -1);
		popMenu->mMenu = ::GetSubMenu(realMenu, index);

		//update cmd ui when menu is ready
		CUIPopupMenuBar* popMenuBar = DYNAMIC_DOWNCAST(CUIPopupMenuBar, popMenu->GetMenuBar());
		if( popMenuBar != NULL)
			popMenuBar->OnUpdateCmdUI(popMenuBar->getCmdTarget(), FALSE);
	}
};
BLADE_IMPLEMENT_DYNCREATE(CUIMenuButton, CMFCToolBarMenuButton);

//////////////////////////////////////////////////////////////////////////
BOOL CUIPopupMenuBar::OnSendCommand(const CMFCToolBarButton* pButton)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	HMENU hMenu = NULL;
	UINT uiID = pButton->m_nID;
	int pos = -1;
	CUIPopMenu* pParentMenu = DYNAMIC_DOWNCAST(CUIPopMenu, GetParent());
	CWnd* pWnd = this->GetCommandTarget();
	//CWnd* pWnd = ::AfxGetMainWnd();

	if (uiID != 0 && pParentMenu != NULL && uiID != -1)
	{
		hMenu = pParentMenu->mMenu;
		int count = ::GetMenuItemCount(hMenu);
		for (int i = 0; i < count; ++i)
		{
			if ((int)::GetMenuItemID(hMenu, i) == uiID)
			{
				pos = i;
				break;
			}
		}
		assert(pos != -1);
	}

	SetInCommand();
	if(hMenu != NULL && pos != -1 && uiID != 0 && uiID != -1 && pWnd != NULL)
		pWnd->PostMessage(WM_MENUCOMMAND, (WPARAM)pos, (LPARAM)hMenu);
	SetInCommand(FALSE);
	return __super::OnSendCommand(pButton);
	//return TRUE;
}

//////////////////////////////////////////////////////////////////////////
int CUIPopupMenuBar::InsertButton(const CMFCToolBarButton& button, INT_PTR iInsertAt/* = -1*/)
{
	CRuntimeClass* pClass = RUNTIME_CLASS(CUIMenuButton);
	ENSURE(pClass != NULL);

	CMFCToolBarButton* pButton = (CMFCToolBarButton*) pClass->CreateObject();
	ENSURE(pButton != NULL);

	ASSERT_VALID(pButton);

	pButton->CopyFrom(button);

	INT_PTR iIndex = this->CMFCPopupMenuBar::InsertButton(pButton, (int) iInsertAt);
	if (iIndex < 0)
	{
		delete pButton;
	}
	return(int) iIndex;
}

////////////////////////////////////////////////////////////////////////////
//void CUIPopupMenuBar::LateUpdateCmdUI(CUIPopMenu* pParentMenu)
//{
//	CFrameWnd* pTarget = (CFrameWnd*)this->GetCommandTarget();
//	CCmdUI state;
//	state.m_pMenu = pParentMenu != NULL ? CMenu::FromHandle(pParentMenu->mMenu) : NULL;
//	state.m_pOther = this;
//
//	if(state.m_pMenu == NULL)
//		return;
//
//	state.m_nIndexMax = (UINT)state.m_pMenu->GetMenuItemCount();
//	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
//	{
//		state.m_pSubMenu = state.m_pMenu->GetSubMenu((int)state.m_nIndex);
//		state.m_nID = state.m_pMenu->GetMenuItemID((int)state.m_nIndex);
//		state.DoUpdate(pTarget, FALSE);
//	}
//}


//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CUIMenuBar,CMFCMenuBar);

CUIMenuBar::CUIMenuBar()
{
	this->SetMenuButtonRTC(RUNTIME_CLASS(CUIMenuButton));
}
#endif//BLADE_USE_MFC_FEATURE_PACK