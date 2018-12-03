//==========================================================
// Author: Borodenko Oleg
// 05/08/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <ui/private/MultiPaneCtrl/DockingMarkers.h>
#include <ui/private/MultiPaneCtrl/Draw.h>
#include <ui/private/MultiPaneCtrl/Tools.h>
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4355)	// 'this' : used in base member initializer list.
#pragma warning(disable : 4640)
#pragma warning(disable : 4062)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::MarkerPane::MarkerPane(POINT const &ptPos, UINT uId, RECT const &rcArea) :
	pos(ptPos), id(uId), area(rcArea)
{
}
	// 
DockingMarkers::MarkerFrame::MarkerFrame(UINT uId, RECT const &rcArea) :
	id(uId), area(rcArea)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::LayoutPane::LayoutPane(int iWidthTotal, int iHeightTotal,
	MarkerPane const &left, MarkerPane const &top, MarkerPane const &right, MarkerPane const &bottom, MarkerPane const &tabs, 
	COLORREF clrMask)
:
	m_iWidthTotal(iWidthTotal), m_iHeightTotal(iHeightTotal), 
	m_left(left), m_top(top), m_right(right), m_bottom(bottom), m_tabs(tabs),
	m_clrMask(clrMask)
{
}
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::LayoutFrame::LayoutFrame(int iWidthTotal, int iHeightTotal,
		MarkerPane const &leftPane, MarkerPane const &topPane, MarkerPane const &rightPane, MarkerPane const &bottomPane, MarkerPane const &tabsPane, 
		MarkerFrame const &leftFrame, MarkerFrame const &topFrame, MarkerFrame const &rightFrame, MarkerFrame const &bottomFrame, 
		COLORREF clrMask)
:
	m_iWidthTotal(iWidthTotal), m_iHeightTotal(iHeightTotal), 
	m_leftPane(leftPane), m_topPane(topPane), m_rightPane(rightPane), m_bottomPane(bottomPane), m_tabsPane(tabsPane),
	m_leftFrame(leftFrame), m_topFrame(topFrame), m_rightFrame(rightFrame), m_bottomFrame(bottomFrame), 
	m_clrMask(clrMask)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Params::Params(int iTransparence, bool bAnimation, int iAnimationStep) :
	transparence(iTransparence), animation(bAnimation), animationStep(iAnimationStep)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::MarkerWnd.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(DockingMarkers::MarkerWnd, CWnd)
	ON_MESSAGE(WM_PAINT, DockingMarkers::MarkerWnd::OnPaint)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::MarkerWnd::MarkerWnd(DockingMarkers *p, DOCKMARKER marker) : 
	m_pDockingMarkers(p), m_Marker(marker)
{
	m_pDrawManager = NULL;
}
// 
DockingMarkers::MarkerWnd::~MarkerWnd()
{	DestroyWindow();	// for delete warning: 'calling DestroyWindow in CWnd::~CWnd; OnDestroy or PostNcDestroy in derived class will not be called'.
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::MarkerWnd::Create(UINT id, BOOL (__stdcall *pSetLayerWndAttr)(HWND,COLORREF,BYTE,DWORD), COLORREF clrMask)
{	CString className = Tools::RegisterWndClass(_T("MarkerWnd"),CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),NULL,NULL);
	if(CWnd::CreateEx(WS_EX_TOOLWINDOW | 0x00080000/*WS_EX_LAYERED*/,className,_T(""),WS_POPUP,CRect(0,0,0,0),NULL,0)==FALSE)
		return false;
		// 
	CBitmap bmp;
	BITMAP bmpInfo;
	if(bmp.LoadBitmap(id)==FALSE || bmp.GetBitmap(&bmpInfo)==FALSE ||
		m_Imagelist.Create(bmpInfo.bmWidth/3,bmpInfo.bmHeight,bmpInfo.bmBitsPixel,3,0)==FALSE || 
		m_Imagelist.Add(&bmp,RGB(0,0,0))==-1)
		return false;
		// 
	m_pSetLayeredWindowAttributes = pSetLayerWndAttr;
	m_clrMask = clrMask;
		// 
	::MoveWindow(m_hWnd,0,0,bmpInfo.bmWidth/3,bmpInfo.bmHeight,FALSE);	// set size.
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::MarkerWnd::Destroy()
{	if(m_Imagelist.m_hImageList!=NULL) m_Imagelist.DeleteImageList();
	if(::IsWindow(m_hWnd)!=0) ::DestroyWindow(m_hWnd);
}
/////////////////////////////////////////////////////////////////////////////
// 
LRESULT DockingMarkers::MarkerWnd::OnPaint(WPARAM /*wp*/, LPARAM /*lp*/)
{	CPaintDC dc(this);
		// 
	if(m_pDrawManager!=NULL)
	{	CRect rc;
		GetClientRect(&rc);
		m_pDrawManager->DrawMarker((DockingMarkers *)m_pDockingMarkers,&dc,&rc,m_Marker,
			&m_Imagelist,m_bHighlight,m_bDisable);
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::MarkerWnd::SetPos(POINT pt)
{	::SetWindowPos(m_hWnd, HWND_TOPMOST, pt.x,pt.y,0,0, SWP_NOSIZE | SWP_NOACTIVATE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::MarkerWnd::Disable(bool disable)
{	m_bDisable = disable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::MarkerWnd::Highlight(bool highlight)
{	m_bHighlight = highlight;
}
// 
bool DockingMarkers::MarkerWnd::IsHighlight() const
{	return m_bHighlight;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::MarkerWnd::SetTransparence(int transparence/*percents*/)
{	BYTE transp = (BYTE)(((100-transparence) * 255) / 100);
	m_pSetLayeredWindowAttributes(m_hWnd,m_clrMask,transp,0x00000001/*LWA_COLORKEY*/ | 0x00000002/*LWA_ALPHA*/);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::MarkerWnd::SetDrawManager(DockingMarkersDraw *p)
{	m_pDrawManager = p;
}
/////////////////////////////////////////////////////////////////////////////
// 
CSize DockingMarkers::MarkerWnd::GetSize() const
{	CRect rc;
	GetClientRect(&rc);
	return CSize(rc.Width(),rc.Height());
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::TimerObj.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::TimerObj::Init(Params const &params)
{	m_iMinTransparence = params.transparence;
	m_bAnimation = params.animation;
	m_iAnimationStep = params.animationStep;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::TimerObj::Destroy()
{	timer.KillTimer();
	lst_markWnds.clear();
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::TimerObj::Add(MarkerWnd *wnd)
{	try
	{	lst_markWnds.push_back(wnd);
	}
	catch(std::bad_alloc &)
	{	return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::TimerObj::Show()
{	i_lst_markWnds i, e=lst_markWnds.end();
		// 
	for(i=lst_markWnds.begin(); i!=e; ++i)
		(*i)->Highlight(false);
		// 
	if(m_bAnimation==false)
		for(i=lst_markWnds.begin(); i!=e; ++i)
			(*i)->SetTransparence(m_iMinTransparence);
	else
	{	timer.KillTimer();
			// 
		for(i=lst_markWnds.begin(); i!=e; ++i)
		{	(*i)->SetTransparence(100);
			(*i)->Invalidate();
		}
			// 
		m_iTransparence = 100;
		timer.SetTimer(this,1,40);
	}
		// 
	for(i=lst_markWnds.begin(); i!=e; ++i)
		::SetWindowPos((*i)->m_hWnd, 0, 0,0,0,0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::TimerObj::Hide()
{	if(::IsWindowVisible(lst_markWnds.front()->m_hWnd)==0) return;
		// 
	for(i_lst_markWnds i=lst_markWnds.begin(), e=lst_markWnds.end(); i!=e; ++i)
		if((*i)->IsHighlight()==true)
		{	(*i)->Highlight(false);
			if(m_bAnimation==true) (*i)->Invalidate(FALSE);
		}
		// 
	if(m_bAnimation==false)
		for(i_lst_markWnds i=lst_markWnds.begin(), e=lst_markWnds.end(); i!=e; ++i)
			::ShowWindow((*i)->m_hWnd,SW_HIDE);
	else
	{	timer.KillTimer();
		timer.SetTimer(this,2,40);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::TimerObj::OnTimer(Timer* _timer, int id, int /*time*/)
{	if(id==1)	// appearance.
	{	m_iTransparence = max(m_iMinTransparence,m_iTransparence-m_iAnimationStep);
		if(m_iTransparence==m_iMinTransparence) _timer->KillTimer();
	}
	else	// disappearance.
	{	m_iTransparence = min(100,m_iTransparence+m_iAnimationStep);
		if(m_iTransparence==100)
		{
			_timer->KillTimer();
				// 
			for(i_lst_markWnds i=lst_markWnds.begin(), e=lst_markWnds.end(); i!=e; ++i)
				::ShowWindow((*i)->m_hWnd,SW_HIDE);
		}
	}
		// 
	for(i_lst_markWnds i=lst_markWnds.begin(), e=lst_markWnds.end(); i!=e; ++i)
		if(id==1 && (*i)->IsHighlight()==true && m_iTransparence==m_iMinTransparence)	// cursor above marker and it is finish of appearance.
			(*i)->SetTransparence(0);
		else
			(*i)->SetTransparence(m_iTransparence);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::TimerObj::Highlight(MarkerWnd *wnd, bool highlight)
{	if(highlight==wnd->IsHighlight()) return;
	wnd->Highlight(highlight);
	if(m_iMinTransparence>0 && (m_bAnimation==false || m_iTransparence<=m_iMinTransparence))
		wnd->SetTransparence(highlight==false ? m_iMinTransparence : 0);
	wnd->Invalidate(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::Pane.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Pane::Pane(DockingMarkers *p) :
	left(p,DOCKMARKER_PANE_LEFT), top(p,DOCKMARKER_PANE_TOP), 
	right(p,DOCKMARKER_PANE_RIGHT), bottom(p,DOCKMARKER_PANE_BOTTOM),
	tabs(p,DOCKMARKER_PANE_TABS)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::Frame::Frame.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Frame::Frame(DockingMarkers *p) :
	left(p,DOCKMARKER_FRAME_LEFT), top(p,DOCKMARKER_FRAME_TOP), 
	right(p,DOCKMARKER_FRAME_RIGHT), bottom(p,DOCKMARKER_FRAME_BOTTOM)
{
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers::SelectWindow.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(DockingMarkers::SelectWindow, CWnd)
	ON_MESSAGE(WM_PAINT, DockingMarkers::SelectWindow::OnPaint)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::SelectWindow::SelectWindow(DockingMarkers *p) :
	m_pDockingMarkers(p)
{
	m_pDrawManager = NULL;
}
// 
DockingMarkers::SelectWindow::~SelectWindow()
{	DestroyWindow();	// for delete warning: 'calling DestroyWindow in CWnd::~CWnd; OnDestroy or PostNcDestroy in derived class will not be called'.
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::SelectWindow::Create(BOOL (__stdcall *pSetLayerWndAttr)(HWND,COLORREF,BYTE,DWORD), 
	COLORREF clrMask, int iTransparence)
{
	CString className = Tools::RegisterWndClass(_T("SelectWindow"),CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),NULL,NULL);
	if(CWnd::CreateEx(WS_EX_TOOLWINDOW | 0x00080000/*WS_EX_LAYERED*/,className,_T(""),WS_POPUP,CRect(0,0,0,0),NULL,0)==FALSE)
		return false;
		// 
	m_clrMask = clrMask;
		// 
	BYTE transp = (BYTE)(((100-iTransparence) * 255) / 100);
	pSetLayerWndAttr(m_hWnd,m_clrMask,transp,0x00000001/*LWA_COLORKEY*/ | 0x00000002/*LWA_ALPHA*/);
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SelectWindow::Destroy()
{	if(::IsWindow(m_hWnd)!=0) ::DestroyWindow(m_hWnd);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SelectWindow::SetParams(bool bTab, bool bTop, int iTabHeight)
{	m_bTab = bTab;
	m_bTop = bTop;
	m_iTabHeight = iTabHeight;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SelectWindow::Show(HWND hWndAfter, CRect const *rcScr)
{	::SetWindowPos(m_hWnd, hWndAfter, rcScr->left,rcScr->top,rcScr->Width(),rcScr->Height(), 
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS);
	::RedrawWindow(m_hWnd,NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW);
}
// 
void DockingMarkers::SelectWindow::Hide()
{	::ShowWindow(m_hWnd,SW_HIDE);
}
/////////////////////////////////////////////////////////////////////////////
// 
LRESULT DockingMarkers::SelectWindow::OnPaint(WPARAM /*wp*/, LPARAM /*lp*/)
{	CPaintDC dc(this);
	VirtualWindow virtwnd(&dc,this);
		// 
	CRect rc;
	GetClientRect(&rc);
		// 
	Draw::FillSolidRect(&virtwnd,&rc,m_clrMask);
	if(m_pDrawManager!=NULL) m_pDrawManager->DrawSelectWindow((DockingMarkers *)m_pDockingMarkers,&virtwnd,&rc,m_bTab,m_bTop,m_iTabHeight);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SelectWindow::SetDrawManager(DockingMarkersDraw *p)
{	m_pDrawManager = p;
}
// 
DockingMarkersDraw *DockingMarkers::SelectWindow::GetDrawManager() const
{	return m_pDrawManager;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkers.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::DockingMarkers() :
	m_Layout(0,0,
		MarkerPane(CPoint(0,0),0,CRect(0,0,0,0)), MarkerPane(CPoint(0,0),0,CRect(0,0,0,0)),
		MarkerPane(CPoint(0,0),0,CRect(0,0,0,0)), MarkerPane(CPoint(0,0),0,CRect(0,0,0,0)), 
		MarkerPane(CPoint(0,0),0,CRect(0,0,0,0)), 
		MarkerFrame(0,CRect(0,0,0,0)), MarkerFrame(0,CRect(0,0,0,0)),
		MarkerFrame(0,CRect(0,0,0,0)), MarkerFrame(0,CRect(0,0,0,0)), 		
		RGB(0,0,0)),
	pane1(this), pane2(this), frame(this),
	m_SelectWnd(this)
{
	m_pAbilityManager = this;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Create(LayoutPane const &layout, Params const &params, int transpSelectBar/*1...100%*/)
{	_ASSERTE(WasCreated()==false);
	_ASSERTE(params.transparence>=0 && params.transparence<=99);
	_ASSERTE(params.animation==false || (params.animationStep>=1 && params.animationStep<=100));
	_ASSERTE(transpSelectBar>=1 && transpSelectBar<=100);
		// 
	LayoutFrame layoutFrame(layout.m_iWidthTotal, layout.m_iHeightTotal, 
		layout.m_left, layout.m_top, layout.m_right, layout.m_bottom, layout.m_tabs,
		MarkerFrame(0,CRect(0,0,0,0)), MarkerFrame(0,CRect(0,0,0,0)), 
		MarkerFrame(0,CRect(0,0,0,0)), MarkerFrame(0,CRect(0,0,0,0)), 
		layout.m_clrMask);
		// 
	BOOL (__stdcall *pSetLayerWndAttr)(HWND,COLORREF,BYTE,DWORD) = Tools::GetLayerFunctionPtr();
	if(pSetLayerWndAttr==NULL) return false;
		// 
	if(CreatePanesMarkerWnd(layoutFrame,pSetLayerWndAttr,params)==false ||
		m_SelectWnd.Create(pSetLayerWndAttr,layout.m_clrMask,transpSelectBar)==false)
	{
		Destroy();
		return false;
	}
		// 
	m_LayoutType = LAYOUT_PANE;
	::CopyMemory(&m_Layout,&layoutFrame,sizeof(LayoutFrame));
	m_ShownPane = Pane::TYPE_FIRST;
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::Create(LayoutFrame const &layout, Params const &params, int transpSelectBar/*1...100%*/)
{	_ASSERTE(WasCreated()==false);
	_ASSERTE(params.transparence>=0 && params.transparence<=99);
	_ASSERTE(params.animation==false || (params.animationStep>=1 && params.animationStep<=100));
	_ASSERTE(transpSelectBar>=1 && transpSelectBar<=100);
		// 
	BOOL (__stdcall *pSetLayerWndAttr)(HWND,COLORREF,BYTE,DWORD) = Tools::GetLayerFunctionPtr();
	if(pSetLayerWndAttr==NULL) return false;
		// 
	if(CreatePanesMarkerWnd(layout,pSetLayerWndAttr,params)==false ||
		CreateFrameMarkerWnd(layout,pSetLayerWndAttr,params)==false ||
		m_SelectWnd.Create(pSetLayerWndAttr,layout.m_clrMask,transpSelectBar)==false)
	{
		Destroy();
		return false;
	}
		// 
	m_LayoutType = LAYOUT_FRAME;
	::CopyMemory(&m_Layout,&layout,sizeof(LayoutFrame));
	m_ShownPane = Pane::TYPE_FIRST;
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::CreatePanesMarkerWnd(LayoutFrame const &layout, BOOL (__stdcall *ptr)(HWND,COLORREF,BYTE,DWORD),
	Params const &params)
{
	pane1.timer.Init(params);
	pane2.timer.Init(params);
		// 
	return 
		pane1.timer.Add(&pane1.left)==true && 
		pane1.timer.Add(&pane1.top)==true && 
		pane1.timer.Add(&pane1.right)==true && 
		pane1.timer.Add(&pane1.bottom)==true && 
		pane1.timer.Add(&pane1.tabs)==true && 
			// 
		pane2.timer.Add(&pane2.left)==true && 
		pane2.timer.Add(&pane2.top)==true && 
		pane2.timer.Add(&pane2.right)==true && 
		pane2.timer.Add(&pane2.bottom)==true && 
		pane2.timer.Add(&pane2.tabs)==true && 
			// 
		pane1.left.Create(layout.m_leftPane.id,ptr,layout.m_clrMask)==true && 
		pane1.top.Create(layout.m_topPane.id,ptr,layout.m_clrMask)==true &&
		pane1.right.Create(layout.m_rightPane.id,ptr,layout.m_clrMask)==true && 
		pane1.bottom.Create(layout.m_bottomPane.id,ptr,layout.m_clrMask)==true && 
		pane1.tabs.Create(layout.m_tabsPane.id,ptr,layout.m_clrMask)==true &&
			// 
		pane2.left.Create(layout.m_leftPane.id,ptr,layout.m_clrMask)==true && 
		pane2.top.Create(layout.m_topPane.id,ptr,layout.m_clrMask)==true &&
		pane2.right.Create(layout.m_rightPane.id,ptr,layout.m_clrMask)==true && 
		pane2.bottom.Create(layout.m_bottomPane.id,ptr,layout.m_clrMask)==true && 
		pane2.tabs.Create(layout.m_tabsPane.id,ptr,layout.m_clrMask)==true;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::CreateFrameMarkerWnd(LayoutFrame const &layout, BOOL (__stdcall *ptr)(HWND,COLORREF,BYTE,DWORD),
	Params const &params)
{
	frame.timer.Init(params);
		// 
	return 
		frame.timer.Add(&frame.left)==true &&
		frame.timer.Add(&frame.top)==true &&
		frame.timer.Add(&frame.right)==true &&
		frame.timer.Add(&frame.bottom)==true &&
			// 
		frame.left.Create(layout.m_leftFrame.id,ptr,layout.m_clrMask)==true && 
		frame.top.Create(layout.m_topFrame.id,ptr,layout.m_clrMask)==true &&
		frame.right.Create(layout.m_rightFrame.id,ptr,layout.m_clrMask)==true && 
		frame.bottom.Create(layout.m_bottomFrame.id,ptr,layout.m_clrMask)==true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::Destroy()
{	pane1.timer.Destroy();
	pane2.timer.Destroy();
		// 
	pane1.left.Destroy();
	pane1.top.Destroy();
	pane1.right.Destroy();
	pane1.bottom.Destroy();
	pane1.tabs.Destroy();
	pane2.left.Destroy();
	pane2.top.Destroy();
	pane2.right.Destroy();
	pane2.bottom.Destroy();
	pane2.tabs.Destroy();
		// 
	frame.timer.Destroy();
		// 
	frame.left.Destroy();
	frame.top.Destroy();
	frame.right.Destroy();
	frame.bottom.Destroy();
		// 
	m_SelectWnd.Destroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::WasCreated() const
{	return ::IsWindow(pane1.left.m_hWnd)!=0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::ShowPaneMarkers(CRect const *rcScr)
{	_ASSERTE(WasCreated()==true);
		// 
	m_ShownPane = (m_ShownPane==Pane::TYPE_FIRST ? Pane::TYPE_SECOND : Pane::TYPE_FIRST);	// switch pane.
	Pane *p = GetWorkingPane();
		// 
		// set enable states for markers.
	p->left.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_PANE_LEFT) );
	p->top.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_PANE_TOP) );
	p->right.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_PANE_RIGHT) );
	p->bottom.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_PANE_BOTTOM) );
	p->tabs.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_PANE_TABS) );
		// 
		// calculate left-top point.
	p->basePoint.x = (rcScr->left + rcScr->right - m_Layout.m_iWidthTotal) / 2;
	p->basePoint.y = (rcScr->top + rcScr->bottom - m_Layout.m_iHeightTotal) / 2;
		// positioning and show new markers.
	p->tabs.SetPos(p->basePoint+m_Layout.m_tabsPane.pos);
	p->left.SetPos(p->basePoint+m_Layout.m_leftPane.pos);
	p->top.SetPos(p->basePoint+m_Layout.m_topPane.pos);
	p->right.SetPos(p->basePoint+m_Layout.m_rightPane.pos);
	p->bottom.SetPos(p->basePoint+m_Layout.m_bottomPane.pos);
	p->timer.Show();
		// hide old markers.
	p = GetNotWorkingPane();
	p->timer.Hide();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::ShowFrameMarkers(CRect const *rcScr)
{	_ASSERTE(WasCreated()==true);
	_ASSERTE(m_LayoutType==LAYOUT_FRAME);
		// 
		// set enable states for markers.
	frame.left.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_FRAME_LEFT) );
	frame.top.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_FRAME_TOP) );
	frame.right.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_FRAME_RIGHT) );
	frame.bottom.Disable( !m_pAbilityManager->IsMarkerEnable(this,DOCKMARKER_FRAME_BOTTOM) );
		// 
	CRect rc(rcScr);
	rc.DeflateRect(12,12);
		// 
		// calculate base points.
	frame.baseLeft = CPoint(rc.left,(rcScr->top + rcScr->bottom - frame.left.GetSize().cy) / 2);
	frame.baseTop = CPoint((rcScr->left + rcScr->right - frame.top.GetSize().cx) / 2, rc.top);
	frame.baseRight = CPoint(rc.right-frame.right.GetSize().cx,(rcScr->top + rcScr->bottom - frame.right.GetSize().cy) / 2);
	frame.baseBottom = CPoint((rcScr->left + rcScr->right - frame.bottom.GetSize().cx) / 2, rc.bottom-frame.bottom.GetSize().cy);
		// positioning and show markers.
	frame.left.SetPos(frame.baseLeft);
	frame.top.SetPos(frame.baseTop);
	frame.right.SetPos(frame.baseRight);
	frame.bottom.SetPos(frame.baseBottom);
	frame.timer.Show();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::HidePaneMarkers()
{	_ASSERTE(WasCreated()==true);
		// 
	GetWorkingPane()->timer.Hide();
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::HideFrameMarkers()
{	_ASSERTE(m_LayoutType==LAYOUT_FRAME);
	_ASSERTE(WasCreated()==true);
		// 
	frame.timer.Hide();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::ShowSelectBar(CWnd *pParentWnd, CRect const *rcScr, bool bTab, bool bTop, int iTabHeight)
{	if(WasCreated()==true)
	{	m_SelectWnd.SetParams(bTab,bTop,iTabHeight);
			// 
		HWND hWndAfter = 
			(::IsWindowVisible(frame.left.m_hWnd)!=0 ? frame.left.m_hWnd : 
			(::IsWindowVisible(GetWorkingPane()->tabs.m_hWnd)!=0 ? GetWorkingPane()->tabs.m_hWnd : HWND_TOPMOST));
		m_SelectWnd.Show(hWndAfter,rcScr);
	}
	else
		if(GetDrawManager()!=NULL) 
		{	m_bTab = bTab;
			m_bTop = bTop;
			m_iTabHeight = iTabHeight;
				// 
			CRect rc(rcScr);
			pParentWnd->ScreenToClient(&rc);
			DrawSelectFrame(pParentWnd,&rc);
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::HideSelectBar(CWnd *pParentWnd)
{	if(WasCreated()==true)
		m_SelectWnd.Hide();
	else
		if(GetDrawManager()!=NULL) 
			DrawSelectFrame(pParentWnd,NULL);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::DrawSelectFrame(CWnd *pParentWnd, CRect const *pRect)
{	static CRect rcOld(0,0,0,0);
		// 
	if(pRect!=NULL || rcOld.IsRectNull()==0) 
	{	CDC *pDC = pParentWnd->GetDCEx(NULL, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
		if(pDC!=NULL)
		{	GetDrawManager()->DrawSelectFrame(this,pDC,pRect,(rcOld.IsRectNull()==0 ? &rcOld : NULL),
				m_bTab,m_bTop,m_iTabHeight);
			pParentWnd->ReleaseDC(pDC);
		}
	}
		// 
	rcOld = (pRect!=NULL ? *pRect : CRect(0,0,0,0));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DOCKMARKER DockingMarkers::RelayMouseMove(CPoint pt, bool testPaneMarkers, bool testFrameMarkers)
{	_ASSERTE(WasCreated()==true);
		// 
	DOCKMARKER marker = HitTest(pt,testPaneMarkers,testFrameMarkers);
		// 
	if(marker!=DOCKMARKER_NONE)
		if(m_pAbilityManager->IsMarkerEnable(this,marker)==false)
			marker = DOCKMARKER_NONE;
		// 
	Pane *p = GetWorkingPane();
		// 
	switch(marker)
	{	case DOCKMARKER_PANE_LEFT: p->timer.Highlight(&p->left,true); break;
		case DOCKMARKER_PANE_TOP: p->timer.Highlight(&p->top,true); break;
		case DOCKMARKER_PANE_RIGHT: p->timer.Highlight(&p->right,true); break;
		case DOCKMARKER_PANE_BOTTOM: p->timer.Highlight(&p->bottom,true); break;
		case DOCKMARKER_PANE_TABS: p->timer.Highlight(&p->tabs,true); break;
			// 
		case DOCKMARKER_FRAME_LEFT: frame.timer.Highlight(&frame.left,true); break;
		case DOCKMARKER_FRAME_TOP: frame.timer.Highlight(&frame.top,true); break;
		case DOCKMARKER_FRAME_RIGHT: frame.timer.Highlight(&frame.right,true); break;
		case DOCKMARKER_FRAME_BOTTOM: frame.timer.Highlight(&frame.bottom,true); break;
	}
		// 
	if(marker!=DOCKMARKER_PANE_LEFT) p->timer.Highlight(&p->left,false);
	if(marker!=DOCKMARKER_PANE_TOP) p->timer.Highlight(&p->top,false);
	if(marker!=DOCKMARKER_PANE_RIGHT) p->timer.Highlight(&p->right,false);
	if(marker!=DOCKMARKER_PANE_BOTTOM) p->timer.Highlight(&p->bottom,false);
	if(marker!=DOCKMARKER_PANE_TABS) p->timer.Highlight(&p->tabs,false);
		// 
	if(m_LayoutType==LAYOUT_FRAME)
	{	if(marker!=DOCKMARKER_FRAME_LEFT) frame.timer.Highlight(&frame.left,false);
		if(marker!=DOCKMARKER_FRAME_TOP) frame.timer.Highlight(&frame.top,false);
		if(marker!=DOCKMARKER_FRAME_RIGHT) frame.timer.Highlight(&frame.right,false);
		if(marker!=DOCKMARKER_FRAME_BOTTOM) frame.timer.Highlight(&frame.bottom,false);
	}
		// 
	return marker;
}
/////////////////////////////////////////////////////////////////////////////
// 
DOCKMARKER DockingMarkers::HitTest(CPoint pt, bool testPaneMarkers, bool testFrameMarkers)
{	Pane *p = GetWorkingPane();
		// 
	if(testPaneMarkers==true)
		if((m_Layout.m_leftPane.area+p->basePoint+m_Layout.m_leftPane.pos).PtInRect(pt)!=FALSE) return DOCKMARKER_PANE_LEFT;
		else if((m_Layout.m_topPane.area+p->basePoint+m_Layout.m_topPane.pos).PtInRect(pt)!=FALSE) return DOCKMARKER_PANE_TOP;
		else if((m_Layout.m_rightPane.area+p->basePoint+m_Layout.m_rightPane.pos).PtInRect(pt)!=FALSE) return DOCKMARKER_PANE_RIGHT;
		else if((m_Layout.m_bottomPane.area+p->basePoint+m_Layout.m_bottomPane.pos).PtInRect(pt)!=FALSE) return DOCKMARKER_PANE_BOTTOM;
		else if((m_Layout.m_tabsPane.area+p->basePoint+m_Layout.m_tabsPane.pos).PtInRect(pt)!=FALSE) return DOCKMARKER_PANE_TABS;
		// 
	if(testFrameMarkers==true)
		if(m_LayoutType==LAYOUT_FRAME)
			if((m_Layout.m_leftFrame.area+frame.baseLeft).PtInRect(pt)!=FALSE) return DOCKMARKER_FRAME_LEFT;
			else if((m_Layout.m_topFrame.area+frame.baseTop).PtInRect(pt)!=FALSE) return DOCKMARKER_FRAME_TOP;
			else if((m_Layout.m_rightFrame.area+frame.baseRight).PtInRect(pt)!=FALSE) return DOCKMARKER_FRAME_RIGHT;
			else if((m_Layout.m_bottomFrame.area+frame.baseBottom).PtInRect(pt)!=FALSE) return DOCKMARKER_FRAME_BOTTOM;
		// 
	return DOCKMARKER_NONE;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool DockingMarkers::IsPaneMarker(DOCKMARKER marker)
{	return marker==DOCKMARKER_PANE_LEFT || marker==DOCKMARKER_PANE_TOP || 
		marker==DOCKMARKER_PANE_RIGHT || marker==DOCKMARKER_PANE_BOTTOM ||
		marker==DOCKMARKER_PANE_TABS;
}
// 
bool DockingMarkers::IsFrameMarker(DOCKMARKER marker)
{	return marker==DOCKMARKER_FRAME_LEFT || marker==DOCKMARKER_FRAME_TOP ||
		marker==DOCKMARKER_FRAME_RIGHT || marker==DOCKMARKER_FRAME_BOTTOM;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SetAbilityManager(DockingMarkersAbility *p)
{	m_pAbilityManager = (p!=NULL ? p : this);
}
// 
DockingMarkersAbility *DockingMarkers::GetAbilityManager() const
{	return m_pAbilityManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkers::SetDrawManager(DockingMarkersDraw *p)
{	pane1.left.SetDrawManager(p);
	pane1.top.SetDrawManager(p);
	pane1.right.SetDrawManager(p);
	pane1.bottom.SetDrawManager(p);
	pane1.tabs.SetDrawManager(p);
	pane2.left.SetDrawManager(p);
	pane2.top.SetDrawManager(p);
	pane2.right.SetDrawManager(p);
	pane2.bottom.SetDrawManager(p);
	pane2.tabs.SetDrawManager(p);
		// 
	frame.left.SetDrawManager(p);
	frame.top.SetDrawManager(p);
	frame.right.SetDrawManager(p);
	frame.bottom.SetDrawManager(p);
		// 
	m_SelectWnd.SetDrawManager(p);
}
// 
DockingMarkersDraw *DockingMarkers::GetDrawManager() const
{	return m_SelectWnd.GetDrawManager();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Pane *DockingMarkers::GetWorkingPane()
{	return (m_ShownPane==Pane::TYPE_FIRST ? &pane1 : &pane2);
}
/////////////////////////////////////////////////////////////////////////////
// 
DockingMarkers::Pane *DockingMarkers::GetNotWorkingPane()
{	return (m_ShownPane==Pane::TYPE_FIRST ? &pane2 : &pane1);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDrawBase.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawMarker(DockingMarkers * /*pObj*/, CDC *pDC, CRect const * /*pRect*/, DOCKMARKER /*marker*/, CImageList *pImageList, bool bHighlight, bool bDisable)
{	pImageList->Draw(pDC,(bDisable==false ? (bHighlight==false ? 0 : 1) : 2),CPoint(0,0),ILD_NORMAL);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawSelectWindow(DockingMarkers *pObj, CDC *pDC, CRect const *pRect, bool bTab, bool bTop, int iTabHeight)
{	CRect rc(pRect);
	int width = GetSelectBarBorderWidth(pObj);
		// 
	if(bTab==false || pRect->Width()<75 || pRect->Height()<2*iTabHeight)
	{	rc.DeflateRect(width,width);
			// 
		pDC->ExcludeClipRect(&rc);
		DrawSelectBarBorder(pObj,pDC,pRect);
		pDC->SelectClipRgn(NULL,RGN_COPY);
		DrawSelectBarInside(pObj,pDC,&rc);
	}
	else
	{	POINT pts1[8], pts2[8];
		GetOuterOutline(pObj,&rc,bTop,iTabHeight,width,pts1);
		GetInnerOutline(pObj,&rc,bTop,iTabHeight,width,pts2);
			// 
		CRgn rgn1, rgn2;
		rgn1.CreatePolygonRgn(pts1,sizeof(pts1)/sizeof(POINT),WINDING);
		rgn2.CreatePolygonRgn(pts2,sizeof(pts2)/sizeof(POINT),WINDING);
			// 
		pDC->SelectClipRgn(&rgn1);
		pDC->SelectClipRgn(&rgn2,RGN_DIFF);
		DrawSelectBarBorder(pObj,pDC,&rc);
		pDC->SelectClipRgn(&rgn2,RGN_COPY);
		DrawSelectBarInside(pObj,pDC,&rc);
		pDC->SelectClipRgn(NULL,RGN_COPY);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawSelectFrame(DockingMarkers *pObj, CDC *pDC, CRect const *pRectNew, CRect const *pRectOld, bool bTab, bool bTop, int iTabHeight)
{	int width = GetSelectFrameWidth(pObj);
	CRect rc(pRectNew!=NULL ? pRectNew : pRectOld);
		// 
	if(bTab==false || rc.Width()<75 || rc.Height()<2*iTabHeight)
	{	CRect rcClip(rc);
		rcClip.DeflateRect(width,width);
		pDC->ExcludeClipRect(&rcClip);
		DrawSelectFrame(pObj,pDC,&rc);
	}
	else
	{	POINT pts1[8], pts2[8];
		GetOuterOutline(pObj,&rc,bTop,iTabHeight,width,pts1);
		GetInnerOutline(pObj,&rc,bTop,iTabHeight,width,pts2);
			// 
		CRgn rgn1, rgn2;
		rgn1.CreatePolygonRgn(pts1,sizeof(pts1)/sizeof(POINT),WINDING);
		rgn2.CreatePolygonRgn(pts2,sizeof(pts2)/sizeof(POINT),WINDING);
			// 
		pDC->SelectClipRgn(&rgn1);
		pDC->SelectClipRgn(&rgn2,RGN_DIFF);
		DrawSelectFrame(pObj,pDC,&rc);
	}
		// 
	pDC->SelectClipRgn(NULL,RGN_COPY);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawSelectBarInside(DockingMarkers *pObj, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect,GetSelectBarInsideColor(pObj));
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawSelectBarBorder(DockingMarkers *pObj, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect,GetSelectBarBorderColor(pObj));
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::DrawSelectFrame(DockingMarkers * /*pObj*/, CDC *pDC, CRect const *pRect)
{	CBrush *pOldBrush = pDC->SelectObject(CDC::GetHalftoneBrush());
	pDC->PatBlt(pRect->left,pRect->top,pRect->Width(),pRect->Height(),PATINVERT);
	pDC->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDrawBase::GetSelectBarBorderWidth(DockingMarkers * /*pObj*/)
{	return 3;
}
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDrawBase::GetSelectFrameWidth(DockingMarkers * /*pObj*/)
{	return 4;
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDrawBase::GetSelectBarInsideColor(DockingMarkers * /*pObj*/)
{	return ::GetSysColor(COLOR_HIGHLIGHT);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDrawBase::GetSelectBarBorderColor(DockingMarkers * /*pObj*/)
{	return ::GetSysColor(COLOR_ACTIVEBORDER);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDrawBase::GetOuterOutline(DockingMarkers * /*pObj*/, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/)
{	int w = (width!=1 ? width/2 : 1);
		// 
	if(top==true)
	{	pts[0].x = rect->left; pts[0].y = rect->bottom;
		pts[1].x = rect->left; pts[1].y = rect->top+tabHeight-w;
		pts[2].x = rect->left+15; pts[2].y = rect->top+tabHeight-w;
		pts[3].x = rect->left+15; pts[3].y = rect->top;
		pts[4].x = rect->left+60; pts[4].y = rect->top;
		pts[5].x = rect->left+60; pts[5].y = rect->top+tabHeight-w;
		pts[6].x = rect->right; pts[6].y = rect->top+tabHeight-w;
		pts[7].x = rect->right; pts[7].y = rect->bottom;
	}
	else
	{	pts[0].x = rect->left; pts[0].y = rect->top;
		pts[1].x = rect->left; pts[1].y = rect->bottom-tabHeight+w;
		pts[2].x = rect->left+15; pts[2].y = rect->bottom-tabHeight+w;
		pts[3].x = rect->left+15; pts[3].y = rect->bottom;
		pts[4].x = rect->left+60; pts[4].y = rect->bottom;
		pts[5].x = rect->left+60; pts[5].y = rect->bottom-tabHeight+w;
		pts[6].x = rect->right; pts[6].y = rect->bottom-tabHeight+w;
		pts[7].x = rect->right; pts[7].y = rect->top;
	}
}
// 
void DockingMarkersDrawBase::GetInnerOutline(DockingMarkers * /*pObj*/, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/)
{	int w = width / 2;
		// 
	if(top==true)
	{	pts[0].x = rect->left+width; pts[0].y = rect->bottom-width;
		pts[1].x = rect->left+width; pts[1].y = rect->top+tabHeight+w;
		pts[2].x = rect->left+15+width; pts[2].y = rect->top+tabHeight+w;
		pts[3].x = rect->left+15+width; pts[3].y = rect->top+width;
		pts[4].x = rect->left+60-width; pts[4].y = rect->top+width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->top+tabHeight+w;
		pts[6].x = rect->right-width; pts[6].y = rect->top+tabHeight+w;
		pts[7].x = rect->right-width; pts[7].y = rect->bottom-width;
	}
	else
	{	pts[0].x = rect->left+width; pts[0].y = rect->top+width;
		pts[1].x = rect->left+width; pts[1].y = rect->bottom-tabHeight-w;
		pts[2].x = rect->left+15+width; pts[2].y = rect->bottom-tabHeight-w;
		pts[3].x = rect->left+15+width; pts[3].y = rect->bottom-width;
		pts[4].x = rect->left+60-width; pts[4].y = rect->bottom-width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->bottom-tabHeight-w;
		pts[6].x = rect->right-width; pts[6].y = rect->bottom-tabHeight-w;
		pts[7].x = rect->right-width; pts[7].y = rect->top+width;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDraw_custom1.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_custom1::GetSelectBarBorderWidth(DockingMarkers * /*pObj*/)
{	return 1;
}
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_custom1::GetSelectFrameWidth(DockingMarkers * /*pObj*/)
{	return 2;
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDraw_custom1::DrawSelectBarInside(DockingMarkers * /*pObj*/, CDC *pDC, CRect const *pRect)
{	CBrush brush;
	brush.CreateHatchBrush(HS_BDIAGONAL,RGB(110,110,110));
	pDC->SetBkColor(RGB(255,128,0));
	pDC->FillRect(pRect,&brush);
}
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDraw_custom1::DrawSelectFrame(DockingMarkers * /*pObj*/, CDC *pDC, CRect const *pRect)
{	CBrush br(HS_BDIAGONAL,::GetSysColor(COLOR_WINDOWTEXT));
		// 
	CBrush *pOldBrush = pDC->SelectObject(&br);
	pDC->PatBlt(pRect->left,pRect->top,pRect->Width(),pRect->Height(),PATINVERT);
	pDC->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDraw_custom1::GetSelectBarBorderColor(DockingMarkers * /*pObj*/)
{	return RGB(60,60,60);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDraw_VS2003.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_VS2003::GetSelectFrameWidth(DockingMarkers * /*pObj*/)
{	return 8;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDraw_VS2005.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_VS2005::GetSelectBarBorderWidth(DockingMarkers * /*pObj*/)
{	return 2;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// DockingMarkersDraw_VS2010.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int DockingMarkersDraw_VS2010::GetSelectBarBorderWidth(DockingMarkers * /*pObj*/)
{	return 5;
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF DockingMarkersDraw_VS2010::GetSelectBarInsideColor(DockingMarkers * /*pObj*/)
{	return Draw::PixelAlpha(::GetSysColor(COLOR_HIGHLIGHT),RGB(255,255,255),80);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void DockingMarkersDraw_VS2010::GetOuterOutline(DockingMarkers * /*pObj*/, CRect const *rect, bool top, int tabHeight, int /*width*/, POINT pts[8]/*out*/)
{	if(top==true)
	{	pts[0].x = rect->left; pts[0].y = rect->bottom;
		pts[1].x = rect->left; pts[1].y = rect->top;
		pts[2].x = rect->left; pts[2].y = rect->top;		// repeat.
		pts[3].x = rect->left; pts[3].y = rect->top;		// repeat.
		pts[4].x = rect->left+60; pts[4].y = rect->top;
		pts[5].x = rect->left+60; pts[5].y = rect->top+tabHeight;
		pts[6].x = rect->right; pts[6].y = rect->top+tabHeight;
		pts[7].x = rect->right; pts[7].y = rect->bottom;
	}
	else
	{	pts[0].x = rect->left; pts[0].y = rect->top;
		pts[1].x = rect->left; pts[1].y = rect->bottom;
		pts[2].x = rect->left; pts[2].y = rect->bottom;		// repeat.
		pts[3].x = rect->left; pts[3].y = rect->bottom;		// repeat.
		pts[4].x = rect->left+60; pts[4].y = rect->bottom;
		pts[5].x = rect->left+60; pts[5].y = rect->bottom-tabHeight;
		pts[6].x = rect->right; pts[6].y = rect->bottom-tabHeight;
		pts[7].x = rect->right; pts[7].y = rect->top;
	}
}
// 
void DockingMarkersDraw_VS2010::GetInnerOutline(DockingMarkers * /*pObj*/, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/)
{	if(top==true)
	{	pts[0].x = rect->left+width; pts[0].y = rect->bottom-width;
		pts[1].x = rect->left+width; pts[1].y = rect->top+width;
		pts[2].x = rect->left+width; pts[2].y = rect->top+width;		// repeat.
		pts[3].x = rect->left+width; pts[3].y = rect->top+width;		// repeat.
		pts[4].x = rect->left+60-width; pts[4].y = rect->top+width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->top+tabHeight+width;
		pts[6].x = rect->right-width; pts[6].y = rect->top+tabHeight+width;
		pts[7].x = rect->right-width; pts[7].y = rect->bottom-width;
	}
	else
	{	pts[0].x = rect->left+width; pts[0].y = rect->top+width;
		pts[1].x = rect->left+width; pts[1].y = rect->bottom-width;
		pts[2].x = rect->left+width; pts[2].y = rect->bottom-width;		// repeat.
		pts[3].x = rect->left+width; pts[3].y = rect->bottom-width;		// repeat.
		pts[4].x = rect->left+60-width; pts[4].y = rect->bottom-width;
		pts[5].x = rect->left+60-width; pts[5].y = rect->bottom-tabHeight-width;
		pts[6].x = rect->right-width; pts[6].y = rect->bottom-tabHeight-width;
		pts[7].x = rect->right-width; pts[7].y = rect->top+width;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////










