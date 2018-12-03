/********************************************************************
	created:	2013/11/14
	filename: 	OptionPopup.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "resource.h"
#include <ConfigDialog/OptionPopup.h>

using namespace Blade;

static const int DEFAULT_WIDTH = 300;
static const int DEFAULT_HEIGHT = 400;

/************************************************************************/
/*                                                                      */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
bool	COptionPopup::showOptionPopup(int x, int y, CWnd* parent, Blade::IConfig* config)
{
	if( parent == NULL || config == NULL )
		return false;

	COptionPopup* optPopup = BLADE_NEW COptionPopup(parent, config);
	optPopup->popup(x, y);
	return true;
}
//////////////////////////////////////////////////////////////////////////
COptionPopup::COptionPopup(CWnd* parent, IConfig* config)
	:mParent(parent)
{
	CRect rect(0,0, DEFAULT_WIDTH, DEFAULT_HEIGHT);

	//WS_EX_TOOLWINDOW: no icon in task bar
	if( this->CreateEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST, TEXT("STATIC"), TEXT(""), WS_POPUP, rect, NULL, 0) )
	{
		CRect rcWin;
		this->GetWindowRect(&rcWin);
		CRect rcClient;
		this->GetClientRect(&rcClient);
		int NCHeight = rcWin.Height() - rcClient.Height();

		mList.Create(this, rect, IDC_OPTION_LIST);
		mList.setDefaultExpand(true);
		mList.setSubConfigs( config );
		this->SetWindowPos(NULL, 0, 0, rect.Width(), (int)mList.getIdealHeight()+NCHeight, SWP_NOMOVE|SWP_NOZORDER);
	}
}

//////////////////////////////////////////////////////////////////////////
COptionPopup::~COptionPopup()
{
	if( mParent != NULL )
	{
		mParent->EnableWindow(mParentEnable);
		mParent->UnlockWindowUpdate();
	}
}

//////////////////////////////////////////////////////////////////////////
void	COptionPopup::popup(int x, int y)
{
	CPoint pt(x,y);
	if( mParent != NULL )
	{
		mParentEnable = mParent->IsWindowEnabled();
		mParent->EnableWindow(FALSE);
		AfxGetApp()->GetMainWnd()->LockWindowUpdate();	//keep the bool bar dropdown button pressed
		mParent->ClientToScreen(&pt);
	}

	this->ShowWindow(SW_SHOW);
	this->SetWindowPos(&CWnd::wndTopMost, pt.x, pt.y, 0, 0, SWP_NOSIZE);
}

//////////////////////////////////////////////////////////////////////////
BOOL COptionPopup::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE )
	{
		BLADE_DELETE this;
		return TRUE;
	}
	else
		return CWnd::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(COptionPopup, CWnd)
	ON_WM_ACTIVATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_PAINT()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void COptionPopup::OnActivate(UINT nState, CWnd* /*pWndOther*/, BOOL /*bMinimized*/)
{
	if( nState == WA_INACTIVE )
		BLADE_DELETE this;
}

//////////////////////////////////////////////////////////////////////////
void COptionPopup::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanged(lpwndpos);
	CRect rect;
	this->GetClientRect(&rect);
	mList.SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), lpwndpos->flags);
}

//////////////////////////////////////////////////////////////////////////
void COptionPopup::OnPaint()
{
#if ENABLE_THEME
	CPaintDC dc(this);
	CRect rect;
	this->GetClientRect(&rect);
	dc.FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));
#else
	return CWnd::OnPaint();
#endif
}