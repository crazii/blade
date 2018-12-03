/********************************************************************
	created:	2013/04/23
	filename: 	CUIViewport.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ConfigDialog/ConfigDialog.h>
#include <ConfigDialog/OptionPopup.h>
#include <ui/Menu.h>
#include <ui/MenuManager.h>
#include <ui/private/UIToolBar.h>
#include <ui/private/UIViewport.h>

using namespace Blade;

class ViewportContent : public CFrameWnd, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	ViewportContent();
	~ViewportContent();
	virtual BOOL Create(CWnd* parent);

	DECLARE_MESSAGE_MAP()
	/** @brief  */
	afx_msg void OnPaint();
	/** @brief  */
	afx_msg void OnNcPaint();
	/** @brief  */
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	/** @brief  */
	afx_msg	void OnMouseMove(UINT nFlags, CPoint point);
	/** @brief  */
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	/** @brief  */
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);
	/** @brief  */
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	/** @brief  */
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
};

BEGIN_MESSAGE_MAP(ViewportContent, CFrameWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
END_MESSAGE_MAP()

ViewportContent::ViewportContent()
{

}

ViewportContent::~ViewportContent()
{

}

BOOL ViewportContent::Create(CWnd* parent)
{
	CRect rect;
	parent->GetClientRect(&rect);
	return this->__super::Create(NULL, TEXT(""), WS_CHILD | WS_VISIBLE, CFrameWnd::rectDefault, parent);
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	PAINTSTRUCT ps;
	BOOL ret = GetUpdateRect(&rect);
	if( ret )
		BeginPaint(&ps);

	CUIViewport* parent = static_cast<CUIViewport*>( this->GetParent() );
	parent->draw();

	if(ret)
		EndPaint(&ps);
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnNcPaint()
{
	CWindowDC dc(this);
}

//////////////////////////////////////////////////////////////////////////
BOOL ViewportContent::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnMouseMove(UINT nFlags, CPoint point)
{
	__super::OnMouseMove(nFlags,point);

	if (!this->IsWindowVisible())
		return;

	//dirty hack: do not gain focus if current window is a config dialog
	if( DYNAMIC_DOWNCAST(CConfigDialog, CWnd::GetActiveWindow() ) == NULL )
		this->SetFocus();

	CUIViewport* parent = static_cast<CUIViewport*>( this->GetParent() );
	if( !parent->getViewport()->isRealtimeUpdate() )
		parent->draw();
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	__super::OnKeyDown(nChar,nRepCnt,nFlags);
	this->SetFocus();
	CUIViewport* parent = static_cast<CUIViewport*>( this->GetParent() );
	if( !parent->getViewport()->isRealtimeUpdate() )
		parent->draw();
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnLButtonDown(UINT nFlags, CPoint point)
{
	__super::OnLButtonDown(nFlags, point);
	CUIViewport* parent = static_cast<CUIViewport*>( this->GetParent() );
	parent->activate();
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnRButtonDown(UINT nFlags, CPoint point)
{
	__super::OnRButtonDown(nFlags, point);
	CUIViewport* parent = static_cast<CUIViewport*>( this->GetParent() );
	parent->activate();
}

//////////////////////////////////////////////////////////////////////////
void ViewportContent::OnMButtonDown(UINT nFlags, CPoint point)
{
	__super::OnMButtonDown(nFlags, point);
	CUIViewport* parent = static_cast<CUIViewport*>( this->GetParent() );
	parent->activate();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
BEGIN_MESSAGE_MAP(CUIViewport, CUIFrameWnd)
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CUIViewport::CUIViewport()
{
	mViewport = NULL;

	//note: it will be auto-delete on PostNcDestroy
	mContent = BLADE_NEW ViewportContent();
}

//////////////////////////////////////////////////////////////////////////
CUIViewport::~CUIViewport()
{

}

/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
BOOL CUIViewport::Create(CWnd* parent)
{
	if( parent == NULL )
	{
		assert(false);
		return FALSE;
	}

	BOOL ret =  this->__super::Create(NULL, TEXT(""), WS_CHILD | WS_VISIBLE, __super::rectDefault, parent);
	if( ret )
	{
		this->EnableDocking(CBRS_ALIGN_TOP);
		BOOL success = mContent->Create(this);
		assert(success);
		BLADE_UNREFERENCED(success);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
BOOL CUIViewport::OnCmdMsg(UINT nID,int nCode,void* pExtra,AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if( mViewport == NULL || !this->IsWindowVisible() )
		return __super::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	return __super::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

//////////////////////////////////////////////////////////////////////////
BOOL CUIViewport::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_LBUTTONUP || pMsg->message == WM_RBUTTONDOWN || pMsg->message == WM_MBUTTONDOWN)
		this->activate();
	return __super::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
int	 CUIViewport::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = __super::OnCreate(lpCreateStruct);
	this->EnableDocking(CBRS_ALIGN_TOP);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CUIViewport::OnNcPaint()
{
	CWindowDC dc(this);
	if( mViewport == NULL || !mViewport->isActive() )
	{
		__super::OnNcPaint();
		return;
	}

	CRect rect;
	this->GetWindowRect(&rect);
	rect = CRect(0,0,rect.right-rect.left,rect.bottom-rect.top);
	dc.FrameRect(&rect, &THEMEBRUSH(TC_ACTIVEFRAME));
}

//////////////////////////////////////////////////////////////////////////
void CUIViewport::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	if( lpwndpos != NULL )
		__super::OnWindowPosChanged(lpwndpos);

	if (mContent->GetSafeHwnd() != NULL)
	{
		CRect rect;
		this->GetClientRect(&rect);
		CRect toolbarRect;
		this->GetControlBar(AFX_IDW_DOCKBAR_TOP)->GetWindowRect(&toolbarRect);
		rect.top += toolbarRect.Height();
		mContent->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
	}
}

//////////////////////////////////////////////////////////////////////////
void CUIViewport::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if( bShow )
	{
		//re-dock tool bars
		CRect rect;
		CControlBar* dockBar = this->GetControlBar(AFX_IDW_DOCKBAR_TOP);
		CToolBar* toolbar = static_cast<CToolBar*>(dockBar->GetWindow(GW_CHILD));
		if( toolbar == NULL )
			return;

		dockBar->GetWindowRect(&rect);
		rect.right = rect.left;
		while( toolbar != NULL )
		{
			CSize size;
			toolbar->GetToolBarCtrl().GetMaxSize(&size);
			rect.left = rect.right+1;
			rect.right += size.cx;

			this->DockControlBar(toolbar, AFX_IDW_DOCKBAR_TOP, rect);
			toolbar = static_cast<CToolBar*>( toolbar->GetWindow(GW_HWNDNEXT) );
		}
		this->RecalcLayout();
	}
	__super::OnShowWindow(bShow, nStatus);
}

//////////////////////////////////////////////////////////////////////////
void	CUIViewport::setViewport(IViewport* viewport)
{
	mViewport = viewport;
}

//////////////////////////////////////////////////////////////////////////
void	CUIViewport::updateLayout()
{
	CRect rect;
	this->calculcateRect(rect);
	this->MoveWindow(&rect);
	this->RecalcLayout();
	if(mViewport->isActive())
		mViewport->activate();
}

//////////////////////////////////////////////////////////////////////////
void	CUIViewport::activate()
{
	mViewport->activate();
	CWnd* parent = this->GetParent();
	for (CWnd* sibling = parent->GetNextWindow(GW_CHILD | GW_HWNDFIRST); sibling != NULL; sibling = sibling->GetNextWindow(GW_HWNDNEXT))
		sibling->SendMessage(WM_NCPAINT);
}

//////////////////////////////////////////////////////////////////////////
void	CUIViewport::draw()
{
	if( mViewport != NULL )
		mViewport->drawViewport();
}

//////////////////////////////////////////////////////////////////////////
HWND	CUIViewport::getContentHWND() const
{
	 return mContent->GetSafeHwnd();
}

//////////////////////////////////////////////////////////////////////////
bool	CUIViewport::setMaximize(bool max)
{
	mViewport->setMaximized(max);
	this->updateLayout();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void	CUIViewport::calculcateRect(CRect& rect)
{
	CWnd* parent = this->GetParent();
	parent->GetClientRect(&rect);

	if( mViewport->isMaximized() )
	{
		if( mViewport != NULL && mViewport->getView() != NULL )
			mViewport->getView()->setSizePosition(0.0f,0.0f,1.0f,1.0f);
		return;
	}
	else
	{
		if( mViewport != NULL && mViewport->getView() != NULL )
		{
			const VIEWPORT_DESC& desc = mViewport->getDesc();
			mViewport->getView()->setSizePosition(desc.mLeft,desc.mTop,desc.mRight,desc.mBottom);
		}
	}

	const VIEWPORT_DESC& desc = mViewport->getDesc();
	int left = (int)( rect.Width()*desc.mLeft );
	int right = (int)( rect.Width()*desc.mRight);
	int top = (int)( rect.Height()*desc.mTop);
	int bottom = (int)( rect.Height()*desc.mBottom);
	rect.SetRect(left, top, right, bottom);
	//rect.DeflateRect(2,2,2,2);
}

//////////////////////////////////////////////////////////////////////////
void CUIViewport::RecalcLayout(BOOL bNotify/* = TRUE*/)
{
	__super::RecalcLayout(bNotify);
	this->OnWindowPosChanged(NULL);
}