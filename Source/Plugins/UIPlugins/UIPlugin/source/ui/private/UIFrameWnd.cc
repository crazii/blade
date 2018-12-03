/********************************************************************
	created:	2015/08/12
	filename: 	UIFrameWnd.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <afxpriv.h>
#include <ui/MenuManager.h>
#include <ui/private/UIToolBar.h>
#include <ui/private/UIFrameWnd.h>
#include <ConfigDialog/OptionPopup.h>

using namespace Blade;

class UIDockBar : public CDockBar, public Allocatable
{
	DECLARE_MESSAGE_MAP();
public:
	using Allocatable::operator new;
	using Allocatable::operator delete;
public:
#if ENABLE_THEME
	/** @brief  */
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC)
	{
		CRect rect;
		this->GetClientRect(&rect);
		pDC->FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
		return TRUE;
	}
	/** @brief  */
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor( THEMECOLOR(TC_TEXT) );
		return THEMEBRUSH(TC_CTLBK);
	}
	afx_msg void	OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* /*lpncsp*/)	{}
	afx_msg void	OnNcPaint()	{}
	afx_msg void	OnPaint()
	{
		CPaintDC dc(this);
		CRect rect;
		this->GetClientRect(&rect);
		dc.FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
	}
#endif
};

BEGIN_MESSAGE_MAP(UIDockBar, CControlBar)
#if ENABLE_THEME
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
#endif
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CUIFrameWnd, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_MENUCHAR()
	ON_MESSAGE(WM_MENUCOMMAND, &CUIFrameWnd::OnMenuCommand)
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, &CUIFrameWnd::OnToolbarDropDown)
#if ENABLE_THEME
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
#endif
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
int		CUIFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = __super::OnCreate(lpCreateStruct);
	this->EnableDocking(CBRS_ALIGN_TOP);
	return ret;
}


//////////////////////////////////////////////////////////////////////////
LRESULT	CUIFrameWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	//we use a owner draw menu, so the menu mnemonics should be handled manually
	//see http://msdn.microsoft.com/en-us/library/windows/desktop/ms647558(v=vs.85).aspx#_win32_Owner_Drawn_Menus_and_the_WM_MENUCHAR_Message
	return MenuManager::getSingleton().OnMenuChar(nChar, nFlags, pMenu);
}

//////////////////////////////////////////////////////////////////////////
LRESULT	CUIFrameWnd::OnMenuCommand(WPARAM nPos, LPARAM hMenu)
{
	return MenuManager::getSingleton().handleMenuCommand(nPos, hMenu);
}

//////////////////////////////////////////////////////////////////////////
void	CUIFrameWnd::OnToolbarDropDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTOOLBAR pHeader = reinterpret_cast<LPNMTOOLBAR>(pNMHDR);
	CUIToolBar* bar = static_cast<CUIToolBar*>(CWnd::FromHandle(pHeader->hdr.hwndFrom));
	bar->ClientToScreen(&(pHeader->rcButton));
	this->ScreenToClient(&(pHeader->rcButton));
	bool handled = MenuManager::getSingleton().handlePopupMenuCommand(pHeader->rcButton.left, pHeader->rcButton.bottom, (UINT_PTR)pHeader->iItem, this);
	if (!handled)
	{
		IConfig* config = MenuManager::getSingleton().getConfig((UINT_PTR)pHeader->iItem);
		if (config != NULL && config->getSubConfigCount() > 0)
			COptionPopup::showOptionPopup(pHeader->rcButton.left, pHeader->rcButton.bottom, this, config);
	}
	*pResult = TBDDRET_DEFAULT;
	CWnd::DeleteTempMap();
}

#if ENABLE_THEME
//////////////////////////////////////////////////////////////////////////
BOOL CUIFrameWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	this->GetClientRect(&rect);
	pDC->FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CUIFrameWnd::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	this->GetClientRect(&rect);
	dc.FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
}

//////////////////////////////////////////////////////////////////////////
void CUIFrameWnd::OnNcPaint()
{
	CWindowDC dc(this);
	CRect rect;
	this->GetWindowRect(&rect);
	rect = CRect(0,0,rect.right-rect.left,rect.bottom-rect.top);
	dc.FrameRect(&rect, &THEMEBRUSH(TC_FRAME));
}
#endif

//////////////////////////////////////////////////////////////////////////
BOOL CUIFrameWnd::OnCmdMsg(UINT nID,int nCode,void* pExtra,AFX_CMDHANDLERINFO* pHandlerInfo)
{
	CCmdUI* cmdui = (CCmdUI*)pExtra;

	//note: this command handling & updating routine is per view port
	//set the view port tool bars' parent owner to this, and they'll be updated & executed here
	if( nCode == CN_COMMAND  )
	{
		if( nID >= MenuManager::getSingleton().getBaseCommandID() && nID < MenuManager::getSingleton().getMaxCommandID() )
		{
			if (pHandlerInfo == NULL)
				return TRUE;
			else
			{
				return MenuManager::getSingleton().getCommand(nID) != NULL || MenuManager::getSingleton().getConfig(nID) != NULL
					|| MenuManager::getSingleton().isPopupMenu(nID);
			}
		}
	}

	if( nCode == CN_UPDATE_COMMAND_UI )
	{
		if( pHandlerInfo == NULL )
			return MenuManager::getSingleton().updateCommand(nID, cmdui, this->getIndex()) ? TRUE : FALSE;
	}
	return __super::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

//////////////////////////////////////////////////////////////////////////
BOOL	CUIFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = (HWND)lParam;

	//command from menu. handled at OnMenuCommand, skip here
	if (hWnd == NULL)
		return FALSE;

	CWnd* cwnd = CWnd::FromHandle(hWnd);
	CUIToolBar* tb = DYNAMIC_DOWNCAST(CUIToolBar, cwnd);
	if (tb == NULL)
		return FALSE;

	return tb->handleToolCommand((UINT_PTR)LOWORD(wParam)) ? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////////
void CUIFrameWnd::EnableDocking(DWORD dwDockStyle)
{
	m_pFloatingFrameClass = RUNTIME_CLASS(CMiniDockFrameWnd);

	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI)) == 0); BLADE_UNREFERENCED(dwDockStyle);
	DWORD style = WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE;

	UIDockBar* pDock = BLADE_NEW UIDockBar();
	pDock->Create(this,  style | CBRS_ALIGN_TOP, AFX_IDW_DOCKBAR_TOP);

	pDock = BLADE_NEW UIDockBar();
	pDock->Create(this,  style | CBRS_ALIGN_BOTTOM, AFX_IDW_DOCKBAR_BOTTOM);

	pDock = BLADE_NEW UIDockBar();
	pDock->Create(this,  style | CBRS_ALIGN_LEFT, AFX_IDW_DOCKBAR_LEFT);

	pDock = BLADE_NEW UIDockBar();
	pDock->Create(this,  style | CBRS_ALIGN_RIGHT, AFX_IDW_DOCKBAR_RIGHT);
}