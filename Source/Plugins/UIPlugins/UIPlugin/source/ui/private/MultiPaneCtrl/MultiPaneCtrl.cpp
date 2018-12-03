//==========================================================
// Author: Borodenko Oleg
// 17/12/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <ui/private/MultiPaneCtrl/MultiPaneCtrl.h>
#include <ui/private/MultiPaneCtrl/Draw.h>
/////////////////////////////////////////////////////////////////////////////
using namespace Blade;
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4290)	// C++ exception specification ignored...
#pragma warning(disable : 4062)	
#pragma warning(disable : 4640)	
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define CSTRING_BAD_ALLOC(exp)	\
	try										\
	{	exp;									\
	}											\
	catch(CMemoryException *p)			\
	{	p->Delete();						\
		throw std::bad_alloc();			\
	}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// MultiPaneCtrl.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(MultiPaneCtrl,CWnd)
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(MultiPaneCtrl, CWnd)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::MultiPaneCtrl()
{	m_pTabStyle = NULL;
		// 
	m_pDrawManager = NULL;
	m_pRecalcManager = this;
	m_pAbilityManager = this;
	m_pNotifyManager = NULL;
		// 
	m_DockMarkMngr.SetAbilityManager(this);
	m_szMinSize.cx = m_szMinSize.cy = 10;
	m_bShowBorder = false;
	m_bDragTabEnable = false;
	m_bDropOptimiz = true;
	m_szMinSizeForDrop.cx = m_szMinSizeForDrop.cy = 70;
		// 
	m_SplitterDragMode = SPLITTER_DRAGGING_STATIC;
	m_hSplitDragPane = NULL;
		// 
	m_pImageListRef = m_pImageListDisabledRef = NULL;
	m_pSysImageListRef = NULL;
	m_hCurTab = m_hCurSplitterHorz = m_hCurSplitterVert = m_hCurDragEnable = m_hCurDragDisable = NULL;
	m_hpCurTabRef = m_hpCurSplitterHorzRef = m_hpCurSplitterVertRef = 
		m_hpCurDragEnableRef = m_hpCurDragDisableRef = NULL;
	m_pFontRef = m_pFontSelectRef = NULL;
		// 
	m_bActive = false;
	m_hLastActiveWnd = NULL;
		// 
	m_hMarkedPane = NULL;
	m_SelectedMarker = DOCKMARKER_NONE;
	m_bDragFinishing = false;
}
// 
MultiPaneCtrl::~MultiPaneCtrl()
{	DestroyWindow();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL MultiPaneCtrl::Create(LPCTSTR /*lpszClassName*/, LPCTSTR /*lpszWindowName*/, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* /*pContext*/)
{	return Create(pParentWnd,dwStyle,rect,nID)==true ? 1 : 0;
}
// 
bool MultiPaneCtrl::Create(CWnd *parent, DWORD style, RECT const &rect, UINT id)
{	CFont *font = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	if(GetFont()==NULL) SetFont(font);
	if(GetFontSelect()==NULL) SetFontSelect(font);
		// 
	CString className = Tools::RegisterWndClass(_T("MultiPaneCtrl"),CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),NULL,NULL);
	if(CWnd::Create(className,_T(""),style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,rect,parent,id)==FALSE)
	{
		CFont::DeleteTempMap();
		return false;
	}
		// 
	if(CreatePane(m_Tree.GetRoot(),true)==false) return false;
		// 
	if(IsWatchActivityCtrl()==true)
		m_ActivityHook.Add(this,m_hWnd);
		// 
	CFont::DeleteTempMap();
	return true;
}
// 
void MultiPaneCtrl::OnDestroy()
{	DeleteAll();
		// 
	Pane *rootPane = m_Tree.GetData(NULL);
	if(rootPane->tab!=NULL)
	{	delete rootPane->tab;	// delete TabCtrl of root pane.
		rootPane->tab = NULL;	//  !!! not delete (necessary for the DestroyPane(...)).
	}
		// 
	if(m_hCurTab!=NULL) ::DestroyCursor(m_hCurTab);
	if(m_hCurSplitterHorz!=NULL) ::DestroyCursor(m_hCurSplitterHorz);
	if(m_hCurSplitterVert!=NULL) ::DestroyCursor(m_hCurSplitterVert);
	if(m_hCurDragEnable!=NULL) ::DestroyCursor(m_hCurDragEnable);
	if(m_hCurDragDisable!=NULL) ::DestroyCursor(m_hCurDragDisable);
		// 
	if(IsWatchActivityCtrl()==true)
		if(m_ActivityHook.IsExist(this)==true)
			m_ActivityHook.Delete(this);
		// 
	CWnd::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnActive(bool active, HWND wnd)
{	m_bActive = active;
		// 
	if(active==false)		// kill focus.
		m_hLastActiveWnd = wnd;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnSetFocus(CWnd *pOldWnd)
{	CWnd::OnSetFocus(pOldWnd);
		// 
	if(m_hLastActiveWnd!=NULL && ::IsWindow(m_hLastActiveWnd)!=0 && 
		Tools::IsDescendantWindow(m_hWnd,m_hLastActiveWnd)==true)
		::SetFocus(m_hLastActiveWnd);
	else
	{	HPANE pane = GetFirstPaneWithTabs();
		if(pane!=NULL) GetTabCtrl(pane)->SetFocus();
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::CreatePane(HPANE pane, bool createTab)
{	Pane *p = m_Tree.GetData(pane);
		// 
	p->tab = NULL;
		// 
	if(createTab==true)
	{	try
		{	p->tab = BLADE_NEW TabCtrl;
			PreCreateTabCtrl(p->tab);
			if(p->tab->Create(this,WS_CHILD,CRect(0,0,0,0),100)==false) throw std::bad_alloc();
		}
		catch(std::bad_alloc &)
		{	if(p->tab!=NULL) BLADE_DELETE p->tab;
			return false;
		}
	}
	p->horz = false;
	p->data = 0;
	p->owner = this;
	p->pane = pane;
	p->real.factor = 1.0;
	p->activeSplitter = true;
		// 
	if(m_pNotifyManager!=NULL)
		m_pNotifyManager->OnPanePostCreate(this,pane);
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DestroyPane(HPANE pane)
{	if(m_pNotifyManager!=NULL)
		m_pNotifyManager->OnPanePreDestroy(this,pane);
		// 
	if(m_Tree.IsExist(pane)==true)
	{	HPANE parent = m_Tree.GetParent(pane);
			// 
		if(parent!=NULL && m_Tree.GetCount(parent)==1 && m_Tree.GetCount(pane)==0)
			m_Tree.GetData(parent)->tab = m_Tree.GetData(pane)->tab;
		else
		{	Pane *p = m_Tree.GetData(pane);
				// 
			if(p->tab!=NULL)
			{	for(HWND h; (h=::GetWindow(p->tab->m_hWnd,GW_CHILD))!=NULL; )	// for all child windows of TabCtrl.
				{	::ShowWindow(h,SW_HIDE);
					::SetParent(h,m_hWnd);
				}
					// 
				BLADE_DELETE p->tab;
				p->tab = NULL;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::Pane::Pane()
{	tab = NULL;
	owner = NULL;
}
// 
MultiPaneCtrl::Pane::~Pane()
{	if(owner!=NULL) owner->DestroyPane(pane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnSize(UINT nType, int cx, int cy)
{	CWnd::OnSize(nType, cx, cy);
		// 
	Recalc(true);
	::RedrawWindow(m_hWnd,NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW);	// update window.
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Update()
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	Recalc(true);
	Invalidate(FALSE);
	UpdateAllTabs();
}
// 
void MultiPaneCtrl::Update(HPANE pane)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
		// 
	if(GetCount(pane)>0)
		for(HPANE h=GetFirstChild(pane); h!=NULL; h=GetNextChild(h))
			Update(h);
	else
		GetTabCtrl(pane)->Update();
		// 
	CRect rc = GetRect(pane);
	InvalidateRect(&rc,FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::UpdateAllTabs()
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)
			GetTabCtrl(h)->Update();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Recalc(bool redraw)
{	CRect rc;
	GetClientRect(&rc);
		// 
	if(m_bShowBorder==true)
	{	int width = GetBorderWidth();
		rc.DeflateRect(width,width);
	}
		// 
	DeferWindow dw;
	Recalc(&dw,NULL,rc,redraw);
	m_Tree.GetData(NULL)->real.rcSplitter.SetRectEmpty();
}
// 
void MultiPaneCtrl::Recalc(DeferWindow *dw, HPANE pane, CRect rc, bool redraw)
{	Pane *p = m_Tree.GetData(pane);
		// 
	if(p->owner!=NULL)
	{	int count = m_Tree.GetCount(pane);
		p->real.rc = rc;
			// 
		if(count==0)	// pane has tabs.
			dw->MoveShowWindow(p->tab->m_hWnd,&rc,redraw);
		else		// pane has child panes.
		{	CRect rect = rc;
			CSize splitterSize = GetSplitterSize();
			HPANE h, hLast=m_Tree.GetLastChild(pane);
			int correctedTail=0, totalOver=0;
			int cx,cy;
				// 
			if(p->horz==true)		// horizontal line.
			{	int iMinSizeCX = m_szMinSize.cx;
					// 
				int totalWidth = max(0, rc.Width()-(count-1)*splitterSize.cx);
				if(totalWidth < count*iMinSizeCX)
					iMinSizeCX = max(0, totalWidth / count);
					// 
				int pos = rc.left;
					// 
				for(h=m_Tree.GetFirstChild(pane); h!=NULL; h=m_Tree.GetNextChild(h))
				{	p = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cx = (int)((double)totalWidth * p->real.factor + 0.5);
					else	// last column.
						cx = rc.right-pos;
						// 
					if(cx<=iMinSizeCX)
						correctedTail += iMinSizeCX-cx;
					else
						totalOver += cx-iMinSizeCX;
					pos += (cx + splitterSize.cx);
				}
					// 
				pos = rc.left;
					// 
				for(h=m_Tree.GetFirstChild(pane); h!=NULL; h=m_Tree.GetNextChild(h))
				{	p = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cx = (int)((double)totalWidth * p->real.factor + 0.5);
					else	// last column.
						cx = rc.right-pos;
						// 
					if(h!=hLast)	// except last column.
						if(cx>iMinSizeCX)
							cx -= (int)((float)correctedTail * ((float)(cx-iMinSizeCX)/(float)totalOver) + 0.5f);
					cx = max(cx,iMinSizeCX);
						// 
					rect.left = pos;
					rect.right = pos + cx;
					Recalc(dw,h,rect,redraw);
					pos += (cx + splitterSize.cx);
						// 
					if(h!=hLast)
						p->real.rcSplitter.SetRect(rect.right,rect.top,rect.right+splitterSize.cx,rect.bottom);
					else	// last pane in line.
						p->real.rcSplitter.SetRectEmpty();
				}
			}
			else		// vertical line.
			{	int iMinSizeCY = m_szMinSize.cy;
					// 
				int totalHeight = max(0, rc.Height()-(count-1)*splitterSize.cy);
				if(totalHeight < count*iMinSizeCY)
					iMinSizeCY = max(0, totalHeight / count);
					// 
				int pos = rc.top;
					// 
				for(h=m_Tree.GetFirstChild(pane); h!=NULL; h=m_Tree.GetNextChild(h))
				{	p = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cy = (int)((double)totalHeight * p->real.factor + 0.5);
					else	// last column.
						cy = rc.bottom-pos;
						// 
					if(cy<=iMinSizeCY)
						correctedTail += iMinSizeCY-cy;
					else
						totalOver += cy-iMinSizeCY;
					pos += (cy + splitterSize.cy);
				}
					// 
				pos = rc.top;
					// 
				for(h=m_Tree.GetFirstChild(pane); h!=NULL; h=m_Tree.GetNextChild(h))
				{	p = m_Tree.GetData(h);
						// 
					if(h!=hLast)
						cy = (int)((double)totalHeight * p->real.factor + 0.5);
					else	// last column.
						cy = rc.bottom-pos;
						// 
					if(h!=hLast)	// except last column.
						if(cy>iMinSizeCY)
							cy -= (int)((float)correctedTail * ((float)(cy-iMinSizeCY)/(float)totalOver) + 0.5f);
					cy = max(cy,iMinSizeCY);
						// 
					rect.top = pos;
					rect.bottom = pos + cy;
					Recalc(dw,h,rect,redraw);
					pos += (cy + splitterSize.cy);
						// 
					if(h!=hLast)
						p->real.rcSplitter.SetRect(rect.left,rect.bottom,rect.right,rect.bottom+splitterSize.cy);
					else	// last pane in line.
						p->real.rcSplitter.SetRectEmpty();
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnPaint()
{	CPaintDC dc(this);
		// 
	if(m_pDrawManager==NULL) return;
		// 
	VirtualWindow virtwnd(&dc,this);
		// 
	m_pDrawManager->DrawBegin(this,&virtwnd);
		// 
	for(HPANE h=m_Tree.GetFirst(); h!=NULL; h=m_Tree.GetNext(h))
	{	Pane *p = m_Tree.GetData(h);
			// 
		if(p->real.rcSplitter.IsRectEmpty()==0)
		{	bool horz = !m_Tree.GetData( m_Tree.GetParent(h) )->horz;	// orientation of splitter.
			m_pDrawManager->DrawSplitter(this,&virtwnd,horz,&p->real.rcSplitter);
		}
	}
		// 
	if(m_bShowBorder==true)
		if(GetBorderWidth()>0)
		{	CRect rc;
			GetClientRect(&rc);
			m_pDrawManager->DrawBorder(this,&virtwnd,&rc);
		}
		// 
	m_pDrawManager->DrawEnd(this,&virtwnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::ConvertToLine(HPANE pane, bool horz) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==false);
		// 
	HPANE h = m_Tree.Add(pane);
		// 
	if(CreatePane(h,false)==false)
	{	m_Tree.Delete(h);
		throw std::bad_alloc();
	}
		// 
	Pane *parent = m_Tree.GetData(pane);
	m_Tree.GetData(h)->tab = parent->tab;
	parent->tab = NULL;
	parent->horz = horz;
		// 
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::Add(HPANE parent) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
	_ASSERTE(IsLine(parent)==true);
		// 
	HPANE h = m_Tree.Add(parent);
		// 
	if(CreatePane(h,true)==false)
	{	m_Tree.Delete(h);
		throw std::bad_alloc();
	}
		// 
	Pane *prev = m_Tree.GetData( m_Tree.GetPrevChild(h) );
	Pane *pane = m_Tree.GetData(h);
	pane->real.factor = prev->real.factor / 2.0;
	prev->real.factor -= pane->real.factor;
		// 
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::Insert(HPANE before, SPACE space) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(before)==true);
	_ASSERTE(GetParent(before)!=NULL);
		// 
	HPANE h = m_Tree.Insert(before);
		// 
	if(CreatePane(h,true)==false)
	{	m_Tree.Delete(h);
		throw std::bad_alloc();
	}
		// 
	Pane *p = m_Tree.GetData(h);
	Pane *pNext = m_Tree.GetData(before);
		// 
	if(m_Tree.GetPrevChild(h)==NULL)	// inserted as first pane.
	{	p->real.factor = pNext->real.factor/2.0;
		pNext->real.factor -= p->real.factor;
	}
	else
	{	Pane *pPrev = m_Tree.GetData( m_Tree.GetPrevChild(h) );
			// 
		switch(space)
		{	case SPACE_BETWEEN:
			{	double totalfactor = pPrev->real.factor + pNext->real.factor;
				pPrev->real.factor *= 2.0/3.0;
				pNext->real.factor *= 2.0/3.0;
				p->real.factor = totalfactor - pPrev->real.factor - pNext->real.factor;
			}	break;
				// 
			case SPACE_PREVIOUS:
				p->real.factor = pPrev->real.factor/2.0;
				pPrev->real.factor -= p->real.factor;
				break;
				// 
			case SPACE_NEXT:
				p->real.factor = pNext->real.factor/2.0;
				pNext->real.factor -= p->real.factor;
				break;
		}
	}
		// 
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Remove(HPANE before, HPANE src)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(before!=src);
	_ASSERTE(IsExist(before)==true && IsExist(src)==true);
	_ASSERTE(GetParent(before)==GetParent(src));
		// 
	m_Tree.Remove(before,src);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::Delete(HPANE pane, SPACE space)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
		// 
	HPANE prev = m_Tree.GetPrevChild(pane);
	HPANE next = m_Tree.GetNextChild(pane);
		// 
	Pane *pPrev = (prev!=NULL ? m_Tree.GetData(prev) : NULL);
	Pane *p = m_Tree.GetData(pane);
	Pane *pNext = (next!=NULL ? m_Tree.GetData(next) : NULL);
		// 
	if(pPrev==NULL)	// first pane.
	{	if(pNext!=NULL) pNext->real.factor += p->real.factor;
	}
	else if(pNext==NULL)	// last pane.
	{	if(pPrev!=NULL) pPrev->real.factor += p->real.factor;
	}
	else
		switch(space)
		{	case SPACE_BETWEEN:
			{	double totalfactor = pPrev->real.factor + p->real.factor + pNext->real.factor;
				pNext->real.factor = (pNext->real.factor * totalfactor) / (pPrev->real.factor + pNext->real.factor);
				pPrev->real.factor = totalfactor - pNext->real.factor;
			}	break;
				// 
			case SPACE_PREVIOUS:
				pPrev->real.factor += p->real.factor;
				break;
				// 
			case SPACE_NEXT:
				pNext->real.factor += p->real.factor;
				break;
		}
		// 
	m_Tree.Delete(pane);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteAll()
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	m_Tree.DeleteAll();
		// 
	Pane *rootPane = m_Tree.GetData(NULL);
	rootPane->tab->DeleteAll();	// delete all tabs in the root pane.
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::InsertIntermediate(HPANE parent, bool horz) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
	_ASSERTE(IsLine(parent)==true);
		// 
	HPANE h = m_Tree.InsertIntermediate(parent);
		// 
	if(CreatePane(h,false)==false)
	{	m_Tree.DeleteIntermediate(h);
		throw std::bad_alloc();
	}
		// 
	m_Tree.GetData(h)->horz = horz;
	return h;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteIntermediate(HPANE pane)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
	_ASSERTE(GetCount(GetParent(pane))==1);
		// 
	SetOrientation(GetParent(pane),IsHorizontal(pane));	// copy orientation to parent.
	m_Tree.DeleteIntermediate(pane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetDockMarkers(DockingMarkers::LayoutPane const &layout, 
	DockingMarkers::Params const &params, int transpSelectBar/*1...100%*/)
{
	if(m_DockMarkMngr.WasCreated()==true) m_DockMarkMngr.Destroy();
	return m_DockMarkMngr.Create(layout,params,transpSelectBar);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DisableDockMarkers()
{	m_DockMarkMngr.Destroy();
}
// 
bool MultiPaneCtrl::IsDockMarkersEnable() const
{	return m_DockMarkMngr.WasCreated();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetImageLists(CImageList *pImageList, CImageList *pImageListDisabled)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
	_ASSERTE(pImageListDisabled==NULL || pImageListDisabled->m_hImageList!=NULL);
		// 
	if(m_ImageList.m_hImageList!=NULL) m_ImageList.DeleteImageList();
	m_pImageListRef = NULL;
		// 
	if(m_ImageListDisabled.m_hImageList!=NULL) m_ImageListDisabled.DeleteImageList();
	m_pImageListDisabledRef = NULL;
		// 
	bool res = true;
		// 
	if(pImageList!=NULL)
	{	if(m_ImageList.Create(pImageList)!=0) 
			m_pImageListRef = &m_ImageList;
		else
			res = false;
	}
		// 
	if(pImageListDisabled!=NULL)
	{	if(m_ImageListDisabled.Create(pImageListDisabled)!=0) 
			m_pImageListDisabledRef = &m_ImageListDisabled;
		else
			res = false;
	}
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetImageListsRef(m_pImageListRef,m_pImageListDisabledRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetImageListsRef(CImageList *pImageList, CImageList *pImageListDisabled)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
	_ASSERTE(pImageListDisabled==NULL || pImageListDisabled->m_hImageList!=NULL);
		// 
	if(m_ImageList.m_hImageList!=NULL) m_ImageList.DeleteImageList();
	m_pImageListRef = pImageList;
		// 
	if(m_ImageListDisabled.m_hImageList!=NULL) m_ImageListDisabled.DeleteImageList();
	m_pImageListDisabledRef = pImageListDisabled;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetImageListsRef(m_pImageListRef,m_pImageListDisabledRef);
			}
}
// 
void MultiPaneCtrl::GetImageLists(CImageList **pImageList/*out*/, CImageList **pImageListDisabled/*out*/)
{	if(pImageList!=NULL) *pImageList = m_pImageListRef;
	if(pImageListDisabled!=NULL) *pImageListDisabled = m_pImageListDisabledRef;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetSystemImageList(CImageList *pImageList)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
		// 
	if(m_SysImageList.m_hImageList!=NULL) m_SysImageList.DeleteImageList();
	m_pSysImageListRef = NULL;
		// 
	bool res = true;
		// 
	if(pImageList!=NULL)
		if(m_SysImageList.Create(pImageList)!=0) 
			m_pSysImageListRef = &m_SysImageList;
		else
			res = false;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetSystemImageListRef(m_pSysImageListRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetSystemImageListRef(CImageList *pImageList)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
		// 
	if(m_SysImageList.m_hImageList!=NULL) m_SysImageList.DeleteImageList();
	m_pSysImageListRef = pImageList;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetSystemImageListRef(m_pSysImageListRef);
			}
}
// 
CImageList *MultiPaneCtrl::GetSystemImageList() const
{	return m_pSysImageListRef;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetCursors(UINT tab, UINT splitterHorz, UINT splitterVert, UINT dragEnable, UINT dragDisable)
{	return SetCursors(AfxGetResourceHandle(),tab,splitterHorz,splitterVert,dragEnable,dragDisable);
}
// 
bool MultiPaneCtrl::SetCursors(HMODULE module, UINT tab, UINT splitterHorz, UINT splitterVert, UINT dragEnable, UINT dragDisable)
{	if(m_hCurTab!=NULL)
	{	::DestroyCursor(m_hCurTab);
		m_hCurTab = NULL;
	}
	m_hpCurTabRef = NULL;
		// 
	if(m_hCurSplitterHorz!=NULL)
	{	::DestroyCursor(m_hCurSplitterHorz);
		m_hCurSplitterHorz = NULL;
	}
	m_hpCurSplitterHorzRef = NULL;
		// 
	if(m_hCurSplitterVert!=NULL)
	{	::DestroyCursor(m_hCurSplitterVert);
		m_hCurSplitterVert = NULL;
	}
	m_hpCurSplitterVertRef = NULL;
		// 
	if(m_hCurDragEnable!=NULL)
	{	::DestroyCursor(m_hCurDragEnable);
		m_hCurDragEnable = NULL;
	}
	m_hpCurDragEnableRef = NULL;
		// 
	if(m_hCurDragDisable!=NULL)
	{	::DestroyCursor(m_hCurDragDisable);
		m_hCurDragDisable = NULL;
	}
	m_hpCurDragDisableRef = NULL;
		// 
	bool res = true;
		// 
	if(module!=NULL)
	{	if(tab!=0)
			if((m_hCurTab = ::LoadCursor(module,MAKEINTRESOURCE(tab)))!=NULL)
				m_hpCurTabRef = &m_hCurTab;
			else
				res = false;
			// 
		if(splitterHorz!=0)
			if((m_hCurSplitterHorz = ::LoadCursor(module,MAKEINTRESOURCE(splitterHorz)))!=NULL)
				m_hpCurSplitterHorzRef = &m_hCurSplitterHorz;
			else
				res = false;
			// 
		if(splitterVert!=0)
			if((m_hCurSplitterVert = ::LoadCursor(module,MAKEINTRESOURCE(splitterVert)))!=NULL)
				m_hpCurSplitterVertRef = &m_hCurSplitterVert;
			else
				res = false;
			// 
		if(dragEnable!=0)
			if((m_hCurDragEnable = ::LoadCursor(module,MAKEINTRESOURCE(dragEnable)))!=NULL)
				m_hpCurDragEnableRef = &m_hCurDragEnable;
			else
				res = false;
			// 
		if(dragDisable!=0)
			if((m_hCurDragDisable = ::LoadCursor(module,MAKEINTRESOURCE(dragDisable)))!=NULL)
				m_hpCurDragDisableRef = &m_hCurDragDisable;
			else
				res = false;
	}
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *t = GetTabCtrl(h);
				t->SetCursorRef(m_hpCurTabRef);
			}
		// 
	return res;
}
// 
bool MultiPaneCtrl::SetCursors(HCURSOR tab, HCURSOR splitterHorz, HCURSOR splitterVert, HCURSOR dragEnable, HCURSOR dragDisable)
{	if(m_hCurTab!=NULL)
	{	::DestroyCursor(m_hCurTab);
		m_hCurTab = NULL;
	}
	m_hpCurTabRef = NULL;
		// 
	if(m_hCurSplitterHorz!=NULL)
	{	::DestroyCursor(m_hCurSplitterHorz);
		m_hCurSplitterHorz = NULL;
	}
	m_hpCurSplitterHorzRef = NULL;
		// 
	if(m_hCurSplitterVert!=NULL)
	{	::DestroyCursor(m_hCurSplitterVert);
		m_hCurSplitterVert = NULL;
	}
	m_hpCurSplitterVertRef = NULL;
		// 
	if(m_hCurDragEnable!=NULL)
	{	::DestroyCursor(m_hCurDragEnable);
		m_hCurDragEnable = NULL;
	}
	m_hpCurDragEnableRef = NULL;
		// 
	if(m_hCurDragDisable!=NULL)
	{	::DestroyCursor(m_hCurDragDisable);
		m_hCurDragDisable = NULL;
	}
	m_hpCurDragDisableRef = NULL;
		// 
	bool res = true;
		// 
	if(tab!=NULL)
		if((m_hCurTab = (HCURSOR)CopyImage(tab,IMAGE_CURSOR,0,0,0))!=NULL)
			m_hpCurTabRef = &m_hCurTab;
		else
			res = false;
		// 
	if(splitterHorz!=NULL)
		if((m_hCurSplitterHorz = (HCURSOR)CopyImage(splitterHorz,IMAGE_CURSOR,0,0,0))!=NULL)
			m_hpCurSplitterHorzRef = &m_hCurSplitterHorz;
		else
			res = false;
		// 
	if(splitterVert!=NULL)
		if((m_hCurSplitterVert = (HCURSOR)CopyImage(splitterVert,IMAGE_CURSOR,0,0,0))!=NULL)
			m_hpCurSplitterVertRef = &m_hCurSplitterVert;
		else
			res = false;
		// 
	if(dragEnable!=NULL)
		if((m_hCurDragEnable = (HCURSOR)CopyImage(dragEnable,IMAGE_CURSOR,0,0,0))!=NULL)
			m_hpCurDragEnableRef = &m_hCurDragEnable;
		else
			res = false;
		// 
	if(dragDisable!=NULL)
		if((m_hCurDragDisable = (HCURSOR)CopyImage(dragDisable,IMAGE_CURSOR,0,0,0))!=NULL)
			m_hpCurDragDisableRef = &m_hCurDragDisable;
		else
			res = false;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *t = GetTabCtrl(h);
				t->SetCursorRef(m_hpCurTabRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetCursorsRef(HCURSOR *tab, HCURSOR *splitterHorz, HCURSOR *splitterVert, HCURSOR *dragEnable, HCURSOR *dragDisable)
{	if(m_hCurTab!=NULL)
	{	::DestroyCursor(m_hCurTab);
		m_hCurTab = NULL;
	}
	m_hpCurTabRef = tab;
		// 
	if(m_hCurSplitterHorz!=NULL)
	{	::DestroyCursor(m_hCurSplitterHorz);
		m_hCurSplitterHorz = NULL;
	}
	m_hpCurSplitterHorzRef = splitterHorz;
		// 
	if(m_hCurSplitterVert!=NULL)
	{	::DestroyCursor(m_hCurSplitterVert);
		m_hCurSplitterVert = NULL;
	}
	m_hpCurSplitterVertRef = splitterVert;
		// 
	if(m_hCurDragEnable!=NULL)
	{	::DestroyCursor(m_hCurDragEnable);
		m_hCurDragEnable = NULL;
	}
	m_hpCurDragEnableRef = dragEnable;
		// 
	if(m_hCurDragDisable!=NULL)
	{	::DestroyCursor(m_hCurDragDisable);
		m_hCurDragDisable = NULL;
	}
	m_hpCurDragDisableRef = dragDisable;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *t = GetTabCtrl(h);
				t->SetCursorRef(m_hpCurTabRef);
			}
}
// 
void MultiPaneCtrl::GetCursors(HCURSOR *tab/*out*/, HCURSOR *splitterHorz/*out*/, HCURSOR *splitterVert/*out*/, HCURSOR *dragEnable/*out*/, HCURSOR *dragDisable/*out*/) const
{	if(tab!=NULL) *tab = (m_hpCurTabRef!=NULL ? *m_hpCurTabRef : NULL);
	if(splitterHorz!=NULL) *splitterHorz = (m_hpCurSplitterHorzRef!=NULL ? *m_hpCurSplitterHorzRef : NULL);
	if(splitterVert!=NULL) *splitterVert = (m_hpCurSplitterVertRef!=NULL ? *m_hpCurSplitterVertRef : NULL);
	if(dragEnable!=NULL) *dragEnable = (m_hpCurDragEnableRef!=NULL ? *m_hpCurDragEnableRef : NULL);
	if(dragDisable!=NULL) *dragDisable = (m_hpCurDragDisableRef!=NULL ? *m_hpCurDragDisableRef : NULL);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetFont(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	LOGFONT logfont;
	pFont->GetLogFont(&logfont);
	return SetFont(&logfont);
}
// 
bool MultiPaneCtrl::SetFont(LOGFONT const *pLf)
{	_ASSERTE(pLf!=NULL);
		// 
	if(m_Font.m_hObject!=NULL) m_Font.DeleteObject();
	m_pFontRef = NULL;
		// 
	bool res = (m_Font.CreateFontIndirect(pLf)!=0);
	if(res==true) m_pFontRef = &m_Font;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetFontRef(m_pFontRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetFontRef(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	if(m_Font.m_hObject!=NULL) m_Font.DeleteObject();
	m_pFontRef = pFont;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetFontRef(m_pFontRef);
			}
}
// 
CFont *MultiPaneCtrl::GetFont()
{	return m_pFontRef;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SetFontSelect(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	LOGFONT logfont;
	pFont->GetLogFont(&logfont);
	return SetFontSelect(&logfont);
}
// 
bool MultiPaneCtrl::SetFontSelect(LOGFONT const *pLf)
{	_ASSERTE(pLf!=NULL);
		// 
	if(m_FontSelect.m_hObject!=NULL) m_FontSelect.DeleteObject();
	m_pFontSelectRef = NULL;
		// 
	bool res = (m_FontSelect.CreateFontIndirect(pLf)!=0);
	if(res==true) m_pFontSelectRef = &m_FontSelect;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetFontSelectRef(m_pFontSelectRef);
			}
		// 
	return res;
}
// 
void MultiPaneCtrl::SetFontSelectRef(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	if(m_FontSelect.m_hObject!=NULL) m_FontSelect.DeleteObject();
	m_pFontSelectRef = pFont;
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->SetFontSelectRef(m_pFontSelectRef);
			}
}
// 
CFont *MultiPaneCtrl::GetFontSelect()
{	return m_pFontSelectRef;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetData(HPANE pane, __int64 data)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
		// 
	m_Tree.GetData(pane)->data = data;
}
// 
__int64 MultiPaneCtrl::GetData(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
		// 
	return m_Tree.GetData(pane)->data;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetRoot() const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	return m_Tree.GetRoot();
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetParent(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
		// 
	return m_Tree.GetParent(pane);
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetPane(HPANE parent, int idx) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
	_ASSERTE(idx>=0 && idx<GetCount(parent));
		// 
	return m_Tree.GetNode(parent,idx);
}
/////////////////////////////////////////////////////////////////////////////
// 
int MultiPaneCtrl::GetIndex(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(pane!=GetRoot());
		// 
	return m_Tree.GetIndex(pane);
}
/////////////////////////////////////////////////////////////////////////////
// 
int MultiPaneCtrl::GetCount(HPANE parent) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
		// 
	return m_Tree.GetCount(parent);
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsExist(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane!=NULL);
		// 
	return m_Tree.IsExist(pane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetFirstChild(HPANE parent) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
		// 
	return m_Tree.GetFirstChild(parent);
}
// 
HPANE MultiPaneCtrl::GetPrevChild(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(pane!=GetRoot());
		// 
	return m_Tree.GetPrevChild(pane);
}
// 
HPANE MultiPaneCtrl::GetNextChild(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(pane!=GetRoot());
		// 
	return m_Tree.GetNextChild(pane);
}
// 
HPANE MultiPaneCtrl::GetLastChild(HPANE parent) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
		// 
	return m_Tree.GetLastChild(parent);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetFirst() const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	return m_Tree.GetFirst();
}
// 
HPANE MultiPaneCtrl::GetPrev(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
		// 
	return m_Tree.GetPrev(pane);
}
// 
HPANE MultiPaneCtrl::GetNext(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
		// 
	return m_Tree.GetNext(pane);
}
// 
HPANE MultiPaneCtrl::GetLast() const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	return m_Tree.GetLast();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::HitTest(CPoint ptScr) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	CRect rc;
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)	// has tabs.
		{	TabCtrl *tabCtrl = GetTabCtrl(h);
			tabCtrl->GetWindowRect(&rc);
			if(rc.PtInRect(ptScr)!=0)
				return h;
		}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsLine(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
		// 
	return GetCount(pane)>0;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsHorizontal(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsLine(pane)==true);
		// 
	return m_Tree.GetData(pane)->horz;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetOrientation(HPANE pane, bool horz)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsLine(pane)==true);
		// 
	m_Tree.GetData(pane)->horz = horz;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowBorder(bool show)
{	m_bShowBorder = show;
}
// 
bool MultiPaneCtrl::IsShowBorder() const
{	return m_bShowBorder;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DragTabEnable(bool enable)
{	m_bDragTabEnable = enable;
}
bool MultiPaneCtrl::IsDragTabEnable() const
{	return m_bDragTabEnable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DropOptimization(bool optimiz)
{	m_bDropOptimiz = optimiz;
}
// 
bool MultiPaneCtrl::IsDropOptimization() const
{	return m_bDropOptimiz;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetMinSizeForDrop(CSize const &size)
{	_ASSERTE(size.cx>=0 && size.cy>=0);
		// 
	m_szMinSizeForDrop = size;
}
// 
CSize MultiPaneCtrl::GetMinSizeForDrop() const
{	return m_szMinSizeForDrop;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetPortions(HPANE parent, int *percent/*in*/)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
	_ASSERTE(IsLine(parent)==true);
	_ASSERTE(percent!=NULL);
		// 
	int count = GetCount(parent);
		// 
	__try
	{	int sum = 0;
		for(int c=0; c<count; ++c)
			sum += percent[c];
			// 
		_ASSERTE(sum==100);
		if(sum!=100) return;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{	_ASSERTE(!"count items in 'percent' != GetCount(parent)");
		return;
	}
		// 
	int i = 0;
	for(HPANE h=GetFirstChild(parent); h!=NULL; h=GetNextChild(h), ++i)
	{	double factor = (double)percent[i]/100.0;
		m_Tree.GetData(h)->real.factor = factor;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
CRect MultiPaneCtrl::GetRect(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
		// 
	return m_Tree.GetData(pane)->real.rc;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetPortion(HPANE pane, double portion, SPACE space) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
	_ASSERTE(portion>0.0 && portion<1.0);
		// 
	HPANE prev = m_Tree.GetPrevChild(pane);
	HPANE next = m_Tree.GetNextChild(pane);
		// 
	Pane *pPrev = (prev!=NULL ? m_Tree.GetData(prev) : NULL);
	Pane *p = m_Tree.GetData(pane);
	Pane *pNext = (next!=NULL ? m_Tree.GetData(next) : NULL);
		// 
	if(pPrev==NULL)	// first pane.
	{	if(pNext!=NULL)
		{	double totalfactor = pNext->real.factor + p->real.factor;
				// 
			if(portion < totalfactor)
			{	pNext->real.factor = totalfactor - portion;
				p->real.factor = portion;
			}
		}
	}
	else if(pNext==NULL)	// last pane.
	{	if(pPrev!=NULL)
		{	double totalfactor = pPrev->real.factor + p->real.factor;
				// 
			if(portion < totalfactor)
			{	pPrev->real.factor = totalfactor - portion;
				p->real.factor = portion;
			}
		}
	}
	else	// intermediate pane.
		switch(space)
		{	case SPACE_BETWEEN:
			{	double totalfactor = pPrev->real.factor + p->real.factor + pNext->real.factor;
					// 
				if(portion < totalfactor)
				{	pPrev->real.factor = pPrev->real.factor * (totalfactor - portion) / (totalfactor - p->real.factor);
					pNext->real.factor = totalfactor - pPrev->real.factor - portion;
					p->real.factor = portion;
				}
			}	break;
				// 
			case SPACE_PREVIOUS:
			{	double totalfactor = pPrev->real.factor + p->real.factor;
					// 
				if(portion < totalfactor)
				{	pPrev->real.factor = totalfactor - portion;
					p->real.factor = portion;
				}
			}	break;
				// 
			case SPACE_NEXT:
			{	double totalfactor = pNext->real.factor + p->real.factor;
					// 
				if(portion < totalfactor)
				{	pNext->real.factor = totalfactor - portion;
					p->real.factor = portion;
				}
			}	break;
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
double MultiPaneCtrl::GetPortion(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
		// 
	return m_Tree.GetData(pane)->real.factor;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::CopyPortions(HPANE dst, MultiPaneCtrl const *paneCtrlSrc, HPANE src)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(dst==NULL || IsExist(dst)==true);
	_ASSERTE(IsLine(dst)==true);
	_ASSERTE(paneCtrlSrc!=NULL && paneCtrlSrc->IsExist(src)==true);
	_ASSERTE(paneCtrlSrc->IsLine(src)==true);
	_ASSERTE(GetCount(dst)==paneCtrlSrc->GetCount(src));
		// 
	for(int i=0, c=GetCount(dst); i<c; ++i)
	{	HPANE d = GetPane(dst,i);
		HPANE s = paneCtrlSrc->GetPane(src,i);
			// 
		m_Tree.GetData(d)->real.factor = paneCtrlSrc->GetPortion(s);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetEqualPaneSize(HPANE parent)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(parent==NULL || IsExist(parent)==true);
	_ASSERTE(IsLine(parent)==true);
		// 
	double f = 1.0 / (double)GetCount(parent);
		// 
	for(HPANE h=GetFirstChild(parent); h!=NULL; h=GetNextChild(h))
		m_Tree.GetData(h)->real.factor = f;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetEqualPaneSize()
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(IsLine(h)==true)
			SetEqualPaneSize(h);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetMinSize(CSize const &size)
{	_ASSERTE(size.cx>=0 && size.cy>=0);
		// 
	m_szMinSize.cx = size.cx;
	m_szMinSize.cy = size.cy;
}
// 
CSize MultiPaneCtrl::GetMinSize() const
{	return m_szMinSize;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::WatchActivityCtrl(bool watch)
{	if(IsWatchActivityCtrl()!=watch)
	{	m_TabState.WatchActivityCtrl(watch);
		m_bActive = false;
			// 
		if(GetSafeHwnd()!=NULL)
		{	if(watch==true)	// on.
				m_ActivityHook.Add(this,m_hWnd);
			else	// off.
			{	if(m_ActivityHook.IsExist(this)==true)
					m_ActivityHook.Delete(this);
			}
				// 
			for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
				if(GetCount(h)==0)
					GetTabCtrl(h)->WatchActivityCtrl(watch);
		}
	}
}
// 
bool MultiPaneCtrl::IsWatchActivityCtrl() const
{	return m_TabState.IsWatchActivityCtrl();
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsActive() const
{	return m_bActive;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetSplitterDraggingMode(SPLITTER_DRAGGING mode)
{	m_SplitterDragMode = mode;
}
// 
SPLITTER_DRAGGING MultiPaneCtrl::GetSplitterDraggingMode() const
{	return m_SplitterDragMode;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsSplitterDragging(bool *horz/*out*/) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	if(m_hSplitDragPane!=NULL)
	{	if(horz!=NULL) *horz = IsSplitterHorizontal(m_hSplitDragPane);
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::CancelDragging()
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	StopSplitterDragging(true);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ActiveSplitter(HPANE pane, bool active)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
	_ASSERTE( GetLastChild(GetParent(pane))!=pane );	// it isn't last pane in line.
		// 
	m_Tree.GetData(pane)->activeSplitter = active;
}
// 
bool MultiPaneCtrl::IsActiveSplitter(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
	_ASSERTE( GetLastChild(GetParent(pane))!=pane );	// it isn't last pane in line.
		// 
	return m_Tree.GetData(pane)->activeSplitter;
}
/////////////////////////////////////////////////////////////////////////////
// 
CRect MultiPaneCtrl::GetSplitterRect(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
	_ASSERTE( GetLastChild(GetParent(pane))!=pane );	// it isn't last pane in line.
		// 
	return m_Tree.GetData(pane)->real.rcSplitter;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HTAB MultiPaneCtrl::AddTab(HPANE pane, HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==false && GetTabCtrl(pane)!=NULL);
	_ASSERTE(wnd!=NULL && ::IsWindow(wnd)!=0);
		// 
	HWND hParentOld = ::GetParent(wnd);
		// 
	TabCtrl *tab = GetTabCtrl(pane);
	::SetParent(wnd,tab->m_hWnd);
		// 
	try
	{	return tab->Add(wnd,text,image);
	}
	catch(std::bad_alloc &)
	{	::SetParent(wnd,hParentOld);
		throw;
	}
}
// 
HTAB MultiPaneCtrl::AddTab(HPANE pane, Tab const *tab) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(tab!=NULL);
		// 
	HTAB hTab = AddTab(pane,tab->GetHWND(),tab->GetText(),tab->GetImage());
	if( hTab != NULL )
	{
		TabCtrl* tabCtrl = this->GetTabCtrl(pane);
		tabCtrl->SetTabData(hTab, tab->GetData() );
		tabCtrl->SetTabTooltipText(hTab, tab->GetToolTipText() );
		tabCtrl->Disable(hTab, tab->IsDisable() );
	}
	return hTab;
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB MultiPaneCtrl::InsertTab(HPANE pane, HTAB before, HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==false);
	_ASSERTE(GetTabCtrl(pane)->IsExist(before)==true);
	_ASSERTE(wnd!=NULL && ::IsWindow(wnd)!=0);
		// 
	HWND hParentOld = ::GetParent(wnd);
		// 
	TabCtrl *tab = GetTabCtrl(pane);
	::SetParent(wnd,tab->m_hWnd);
		// 
	try
	{	return tab->Insert(before,wnd,text,image);
	}
	catch(std::bad_alloc &)
	{	::SetParent(wnd,hParentOld);
		throw;
	}
}
// 
HTAB MultiPaneCtrl::InsertTab(HPANE pane, HTAB before, Tab const *tab) throw(std::bad_alloc)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(tab!=NULL);
		// 
	return InsertTab(pane,before,tab->GetHWND(),tab->GetText(),tab->GetImage());
}
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrl *MultiPaneCtrl::GetTabCtrl(HPANE pane) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==false);
		// 
	return m_Tree.GetData(pane)->tab;
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetPane(TabCtrl const *ctrl) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(ctrl!=NULL);
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)
			if(GetTabCtrl(h)==ctrl) return h;
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetPane(HTAB tab) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(tab!=NULL);
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)
		{	TabCtrl *tabCtrl = GetTabCtrl(h);
			if(tabCtrl->IsExist(tab)==true) return h;
		}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB MultiPaneCtrl::GetTabWithWindow(HWND hWnd, HPANE *pane/*out*/) const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)
		{	TabCtrl *tabCtrl = GetTabCtrl(h);
			HTAB tab = tabCtrl->GetTabWithWindow(hWnd);
			if(tab!=NULL)
			{	if(pane!=NULL) *pane = h;
				return tab;
			}
		}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsSinglePaneWithTabs() const
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	int count = 0;
		// 
	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)
			if(++count==2) return false;
		// 
	return true;
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::GetFirstPaneWithTabs() const
{	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
		if(GetCount(h)==0)
			return h;
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ActivateTab(HPANE pane)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==false && GetTabCtrl(pane)!=NULL);
		// 
	TabCtrl *tabCtrl = GetTabCtrl(pane);
	HTAB tab = tabCtrl->GetSel();
		// 
	if(tab!=NULL)
	{	HWND wnd = tabCtrl->GetTabWnd(tab);
		if(wnd!=NULL && ::IsWindow(wnd)!=0)
			if(::GetFocus()!=wnd) ::SetFocus(wnd);
			// 
		tabCtrl->EnsureVisible(tab);
		tabCtrl->Update();
	}
}
// 
void MultiPaneCtrl::ActivateTab(HPANE pane, HTAB tab)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(pane==NULL || IsExist(pane)==true);
	_ASSERTE(IsLine(pane)==false && GetTabCtrl(pane)!=NULL);
	_ASSERTE(GetTabCtrl(pane)->IsExist(tab)==true);
		// 
	TabCtrl *tabCtrl = GetTabCtrl(pane);
		// 
	if(tab!=NULL)
	{	HWND wnd = tabCtrl->GetTabWnd(tab);
		if(wnd!=NULL && ::IsWindow(wnd)!=0)
			if(::GetFocus()!=wnd) ::SetFocus(wnd);
			// 
		tabCtrl->SetSel(tab);
		tabCtrl->EnsureVisible(tab);
		tabCtrl->Update();
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabLayout(TAB_LAYOUT layout)
{	m_TabState.SetLayout(layout);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetLayout(layout);
}
// 
TAB_LAYOUT MultiPaneCtrl::GetTabLayout() const
{	return m_TabState.GetLayout();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabBehavior(TAB_BEHAVIOR behavior)
{	m_TabState.SetBehavior(behavior);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetBehavior(behavior);
}
// 
TAB_BEHAVIOR MultiPaneCtrl::GetTabBehavior() const
{	return m_TabState.GetBehavior();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabScrollingStep(int step)
{	m_TabState.SetScrollingStep(step);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetScrollingStep(step);
}
// 
int MultiPaneCtrl::GetTabScrollingStep() const
{	return m_TabState.GetScrollingStep();
}
/////////////////////////////////////////////////////////////////////////////
//
void MultiPaneCtrl::ShowTabBorder(bool show)
{	m_TabState.ShowBorder(show);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->ShowBorder(show);
}
// 
bool MultiPaneCtrl::IsShowTabBorder() const
{	return m_TabState.IsShowBorder();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::EqualTabsSize(bool equal)
{	m_TabState.EqualTabsSize(equal);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->EqualTabsSize(equal);
}
// 
bool MultiPaneCtrl::IsEqualTabsSize() const
{	return m_TabState.IsEqualTabsSize();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::RemoveTabEnable(bool remove)
{	m_TabState.RemoveTabEnable(remove);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->RemoveTabEnable(remove);
}
// 
bool MultiPaneCtrl::IsRemoveTabEnable() const
{	return m_TabState.IsRemoveTabEnable();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::HideSingleTab(bool hide)
{	m_TabState.HideSingleTab(hide);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->HideSingleTab(hide);
}
// 
bool MultiPaneCtrl::IsHideSingleTab() const
{	return m_TabState.IsHideSingleTab();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowTabCloseButton(bool show)
{	m_TabState.ShowCloseButton(show);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->ShowCloseButton(show);
}
// 
bool MultiPaneCtrl::IsShowTabCloseButton() const
{	return m_TabState.IsShowCloseButton();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowTabMenuButton(bool show)
{	m_TabState.ShowMenuButton(show);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->ShowMenuButton(show);
}
// 
bool MultiPaneCtrl::IsShowTabMenuButton() const
{	return m_TabState.IsShowMenuButton();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::ShowTabScrollButtons(bool show)
{	m_TabState.ShowScrollButtons(show);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->ShowScrollButtons(show);
}
// 
bool MultiPaneCtrl::IsShowTabScrollButtons() const
{	return m_TabState.IsShowScrollButtons();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::TabToolTipEnable(bool enable)
{	m_TabState.ToolTipEnable(enable);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->ToolTipEnable(enable);
}
// 
bool MultiPaneCtrl::IsTabToolTipEnable() const
{	return m_TabState.IsToolTipEnable();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabCloseButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	m_TabState.SetCloseButToolTipText(text);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetCloseButToolTipText(text);
}
// 
CString MultiPaneCtrl::GetTabCloseButToolTipText() const
{	return m_TabState.GetCloseButToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabMenuButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	m_TabState.SetMenuButToolTipText(text);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetMenuButToolTipText(text);
}
// 
CString MultiPaneCtrl::GetTabMenuButToolTipText() const
{	return m_TabState.GetMenuButToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabLeftScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	m_TabState.SetLeftScrollButToolTipText(text);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetLeftScrollButToolTipText(text);
}
// 
CString MultiPaneCtrl::GetTabLeftScrollButToolTipText() const
{	return m_TabState.GetLeftScrollButToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetTabRightScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	m_TabState.SetRightScrollButToolTipText(text);
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
				GetTabCtrl(h)->SetRightScrollButToolTipText(text);
}
// 
CString MultiPaneCtrl::GetTabRightScrollButToolTipText() const
{	return m_TabState.GetRightScrollButToolTipText();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetDrawManager(MultiPaneCtrlDraw *p)
{	m_pDrawManager = p;
}
// 
MultiPaneCtrlDraw *MultiPaneCtrl::GetDrawManager() const
{	return m_pDrawManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetRecalcManager(IMultiPaneCtrlRecalc *p)
{	m_pRecalcManager = (p!=NULL ? p : this);
}
// 
IMultiPaneCtrlRecalc *MultiPaneCtrl::GetRecalcManager() const
{	return m_pRecalcManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetAbilityManager(MultiPaneCtrlUserAbility *p)
{	m_pAbilityManager = (p!=NULL ? p : this);
}
// 
MultiPaneCtrlUserAbility *MultiPaneCtrl::GetAbilityManager() const
{	return m_pAbilityManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetNotifyManager(MultiPaneCtrlNotify *p)
{	m_pNotifyManager = p;
}
// 
MultiPaneCtrlNotify *MultiPaneCtrl::GetNotifyManager() const
{	return m_pNotifyManager;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::PreCreateTabCtrl(TabCtrl *tab)
{	if(m_pTabStyle!=NULL) tab->InstallStyle(m_pTabStyle);
	tab->SetAbilityManager(this);
	tab->SetNotifyManager(this);
		// 
		// 
		// set state.
	tab->SetLayout( m_TabState.GetLayout() );
	tab->SetBehavior( m_TabState.GetBehavior() );
		// 
	tab->SetImageListsRef(m_pImageListRef,m_pImageListDisabledRef);
	tab->SetSystemImageListRef(m_pSysImageListRef);
		// 
	tab->SetCursorRef(m_hpCurTabRef);
		// 
	tab->SetFontRef( GetFont() );
	tab->SetFontSelectRef( GetFontSelect() );
		// 
	tab->SetScrollingStep( m_TabState.GetScrollingStep() );
		// 
	tab->ShowBorder( m_TabState.IsShowBorder() );
	tab->EqualTabsSize( m_TabState.IsEqualTabsSize() );
	tab->RemoveTabEnable( m_TabState.IsRemoveTabEnable() );
	tab->HideSingleTab( m_TabState.IsHideSingleTab() );
	tab->ShowCloseButton( m_TabState.IsShowCloseButton() );
	tab->ShowMenuButton( m_TabState.IsShowMenuButton() );
	tab->ShowScrollButtons( m_TabState.IsShowScrollButtons() );
	tab->WatchActivityCtrl( m_TabState.IsWatchActivityCtrl() );
		// 
	tab->ToolTipEnable( m_TabState.IsToolTipEnable() );
	tab->SetCloseButToolTipText( m_TabState.GetCloseButToolTipText() );
	tab->SetMenuButToolTipText( m_TabState.GetMenuButToolTipText() );
	tab->SetLeftScrollButToolTipText( m_TabState.GetLeftScrollButToolTipText() );
	tab->SetRightScrollButToolTipText( m_TabState.GetRightScrollButToolTipText() );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::InstallStyle(IMultiPaneCtrlStyle *style)
{	_ASSERTE(style!=NULL);
		// 
	m_pTabStyle = style->GetTabStyle();
		// 
	SetDrawManager( style->GetDrawManager() );
	SetRecalcManager( style->GetRecalcManager() );
		// 
	if(GetSafeHwnd()!=NULL)
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tab = GetTabCtrl(h);
				tab->InstallStyle(m_pTabStyle);
			}
		// 
	m_DockMarkMngr.SetDrawManager(style->GetDockMarkersDraw());
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
int MultiPaneCtrl::GetBorderWidth()
{	return m_pRecalcManager->GetBorderWidth(this,this);
}
// 
CSize MultiPaneCtrl::GetSplitterSize()
{	return m_pRecalcManager->GetSplitterSize(this,this);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL MultiPaneCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{	*pResult = CWnd::GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{	if(m_hSplitDragPane==NULL)
	{	m_hSplitDragPane = HitTestSplitter(&point);
		if(IsActiveSplitter(m_hSplitDragPane)==false) m_hSplitDragPane = NULL;
			// 
		if(m_hSplitDragPane!=NULL)
		{	StartSplitterDragging(point);
				// 
			if(m_SplitterDragMode==SPLITTER_DRAGGING_STATIC)	// primary drawing of drag rect.
			{	CRect rc = GetSplitterRect(m_hSplitDragPane);
				bool horz = IsSplitterHorizontal(m_hSplitDragPane);
				DrawSplitterDragRect(&rc,horz);
			}
		}
	}
		// 
	CWnd::OnLButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{	HPANE pane = HitTestSplitter(&point);
		// 
	if(pane!=NULL && IsActiveSplitter(pane)==true)
		if(CanSplitterDrag(pane)==true)
		{	bool horz = IsSplitterHorizontal(pane);
			HPANE next = GetNextChild(pane);
				// 
			(horz==true ? HalveHeightPanes(pane,next) : HalveWidthPanes(pane,next));
			UpdateAdjacentPanes(pane,next);	// update two panes around splitter.
		}
		// 
	CWnd::OnLButtonDblClk(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{	StopSplitterDragging(true);
		// 
	CWnd::OnMButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{	StopSplitterDragging(true);
		// 
	CWnd::OnRButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnMouseMove(UINT nFlags, CPoint point)
{	if(m_hSplitDragPane!=NULL)
	{	bool canDrag = CanSplitterDrag(m_hSplitDragPane);
		bool horz = IsSplitterHorizontal(m_hSplitDragPane);	// orientation of splitter.
		CRect rcSplitter;
			// 
		if(canDrag==true)
		{	CPoint ptShift = point - m_ptSplitDragStart;
				// 
			if(horz==true)
				SetDraggingHeight(m_hSplitDragPane,ptShift.y,&rcSplitter);
			else
				SetDraggingWidth(m_hSplitDragPane,ptShift.x,&rcSplitter);
		}
		else
			rcSplitter = GetSplitterRect(m_hSplitDragPane);
			// 
		if(m_SplitterDragMode==SPLITTER_DRAGGING_STATIC)
			DrawSplitterDragRect(&rcSplitter,horz);
	}
		// 
	CWnd::OnMouseMove(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{	StopSplitterDragging(false);
		// 
	CWnd::OnLButtonUp(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnCaptureChanged(CWnd *pWnd)
{	if(pWnd!=this) StopSplitterDragging(true);
		// 
	CWnd::OnCaptureChanged(pWnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::StartSplitterDragging(CPoint point)
{	m_ptSplitDragStart = point;
	SaveAllPanesState();	// save state for all panes.
	SetCapture();
		// 
	m_KeyboardHook.Add(this);
}
// 
void MultiPaneCtrl::StopSplitterDragging(bool reset)
{	if(m_hSplitDragPane==NULL) return;
	HPANE DragPanel = m_hSplitDragPane;
	m_hSplitDragPane = NULL;
		// 
	if(m_KeyboardHook.IsExist(this)==true)
		m_KeyboardHook.Delete(this);
		// 
	if(m_SplitterDragMode==SPLITTER_DRAGGING_STATIC)
	{	bool horz = IsSplitterHorizontal(DragPanel);
		DrawSplitterDragRect(NULL,horz);	// erase dragging rect.
			// 
		if(reset==false)
		{	RestoreAllPanesState();		// set new state from storage.
			UpdateAdjacentPanes(DragPanel,GetNextChild(DragPanel));	// update two panes around splitter.
		}
		else	// reset.
			Update();
	}
	else if(m_SplitterDragMode==SPLITTER_DRAGGING_DYNAMIC && reset==true)
	{	RestoreAllPanesState();		// restore state for all panes.
		Update();
	}

		// 
	if(GetCapture()==this) ::ReleaseCapture();

	if( !reset && m_pNotifyManager != NULL)
		m_pNotifyManager->OnSplitterFinishDrag(this);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SaveAllPanesState()
{	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))	// save state for all panes.
	{	Pane *p = m_Tree.GetData(h);
		p->store = p->real;
	}
}
// 
void MultiPaneCtrl::RestoreAllPanesState()
{	for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))	// restore state for all panes.
	{	Pane *p = m_Tree.GetData(h);
		p->real = p->store;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnKeyDown(UINT keyCode, UINT /*msgFlag*/)
{	if(keyCode==VK_ESCAPE)
		if(IsSplitterDragging(NULL)==true)
			CancelDragging();
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DrawSplitterDragRect(CRect const *pRect, bool horz)
{	static CRect rcOld(0,0,0,0);
		// 
	if(m_pDrawManager!=NULL) 
	{	CDC *pDC = GetDCEx(NULL, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
		if(pDC!=NULL)
		{	m_pDrawManager->DrawSplitterDragRect(this,pDC,horz,pRect,(rcOld.IsRectNull()==0 ? &rcOld : NULL));
			ReleaseDC(pDC);
		}
	}
	rcOld = (pRect!=NULL ? *pRect : CRect(0,0,0,0));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL MultiPaneCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{	CPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);
		// 
	HPANE pane = HitTestSplitter(&pt);
	if(pane!=NULL && IsActiveSplitter(pane)==true)
	{	bool horz = IsSplitterHorizontal(pane);
			// 
		if(horz==true)
		{	if(m_hpCurSplitterHorzRef!=NULL && *m_hpCurSplitterHorzRef!=NULL)
				SetCursor(*m_hpCurSplitterHorzRef);
			else
				SetCursor( LoadCursor(NULL,IDC_SIZENS) );
		}
		else
			if(m_hpCurSplitterVertRef!=NULL && *m_hpCurSplitterVertRef!=NULL)
				SetCursor(*m_hpCurSplitterVertRef);
			else
				SetCursor( LoadCursor(NULL,IDC_SIZEWE) );
			// 
		return TRUE;
	}
		// 
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
/////////////////////////////////////////////////////////////////////////////
// 
HPANE MultiPaneCtrl::HitTestSplitter(CPoint const *pt)
{	for(HPANE h=m_Tree.GetFirst(); h!=NULL; h=m_Tree.GetNext(h))
	{	Pane *p = m_Tree.GetData(h);
			// 
		if(p->real.rcSplitter.PtInRect(*pt)!=0)
			return h;
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetDraggingWidth(HPANE pane, int offset, CRect *rcdrag/*out*/)
{	HPANE next = GetNextChild(pane);
		// 
	Pane *p1 = m_Tree.GetData(pane);
	Pane *p2 = m_Tree.GetData(next);
		// 
		// 1.
	Pane::State *state1 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p1->real : &p1->store);
	Pane::State *state2 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p2->real : &p2->store);
		// 
	int totalcx = state1->rc.Width() + state2->rc.Width();
	double totalfactor = state1->factor + state2->factor;
		// 
	int cx1 = state1->rc.Width() + offset;
	if(cx1<m_szMinSize.cx) cx1 = m_szMinSize.cx;
		// 
	int cx2 = totalcx - cx1;
	if(cx2<m_szMinSize.cx)
	{	cx2 = m_szMinSize.cx;
		cx1 = totalcx - cx2;
	}
		// 
		// 2.
	state1 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p1->store : &p1->real);
	state2 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p2->store : &p2->real);
		// 
	state1->factor = totalfactor * ((double)cx1/(double)totalcx);
	state2->factor = totalfactor - state1->factor;
	if(state2->factor<0.0) state2->factor = 0.0;
		// 
	state1->rc.right = state1->rc.left + cx1;
	state2->rc.left = state2->rc.right - cx2;
	state1->rcSplitter.SetRect(state1->rc.right,state1->rc.top,state1->rc.right+GetSplitterSize().cx,state1->rc.bottom);
		// 
	if(m_SplitterDragMode==SPLITTER_DRAGGING_STATIC)
		*rcdrag = state1->rcSplitter;
	else
		UpdateAdjacentPanes(pane,next);	// update two panes around splitter.
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SetDraggingHeight(HPANE pane, int offset, CRect *rcdrag/*out*/)
{	HPANE next = GetNextChild(pane);
		// 
	Pane *p1 = m_Tree.GetData(pane);
	Pane *p2 = m_Tree.GetData(next);
		// 
		// 1.
	Pane::State *state1 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p1->real : &p1->store);
	Pane::State *state2 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p2->real : &p2->store);
		// 
	int totalcy = state1->rc.Height() + state2->rc.Height();
	double totalfactor = state1->factor + state2->factor;
		// 
	int cy1 = state1->rc.Height() + offset;
	if(cy1<m_szMinSize.cy) cy1 = m_szMinSize.cy;
		// 
	int cy2 = totalcy - cy1;
	if(cy2<m_szMinSize.cy)
	{	cy2 = m_szMinSize.cy;
		cy1 = totalcy - cy2;
	}
		// 
		// 2.
	state1 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p1->store : &p1->real);
	state2 = (m_SplitterDragMode==SPLITTER_DRAGGING_STATIC ? &p2->store : &p2->real);
		// 
	state1->factor = totalfactor * ((double)cy1/(double)totalcy);
	state2->factor = totalfactor - state1->factor;
	if(state2->factor<0.0) state2->factor = 0.0;
		// 
	state1->rc.bottom = state1->rc.top + cy1;
	state2->rc.top = state2->rc.bottom - cy2;
	state1->rcSplitter.SetRect(state1->rc.left,state1->rc.bottom,state1->rc.right,state1->rc.bottom+GetSplitterSize().cy);
		// 
	if(m_SplitterDragMode==SPLITTER_DRAGGING_STATIC)
		*rcdrag = state1->rcSplitter;
	else
		UpdateAdjacentPanes(pane,next);	// update two panes around splitter.
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::UpdateAdjacentPanes(HPANE pane1, HPANE pane2)
{	CRect *rc1 = &m_Tree.GetData(pane1)->real.rc;
	CRect *rc2 = &m_Tree.GetData(pane2)->real.rc;
		// 
	DeferWindow dw;
	Recalc(&dw,pane1,*rc1,false);
	Recalc(&dw,pane2,*rc2,false);
	dw.Process();
		// 
	CRect rc(rc1->TopLeft(),rc2->BottomRight());
	::RedrawWindow(m_hWnd,&rc,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::HalveWidthPanes(HPANE prev, HPANE next)
{	Pane *p1 = m_Tree.GetData(prev);
	Pane *p2 = m_Tree.GetData(next);
		// 
	int totalcx = p1->real.rc.Width() + p2->real.rc.Width();
	double totalfactor = p1->real.factor + p2->real.factor;
		// 
	int cx1 = totalcx / 2;
	if(cx1<m_szMinSize.cx) cx1 = m_szMinSize.cx;
		// 
	int cx2 = totalcx - cx1;
	if(cx2<m_szMinSize.cx)
	{	cx2 = m_szMinSize.cx;
		cx1 = totalcx - cx2;
	}
		// 
	p1->real.factor = totalfactor * ((double)cx1/(double)totalcx);
	p2->real.factor = totalfactor - p1->real.factor;
	if(p2->real.factor<0.0) p2->real.factor = 0.0;
		// 
	p1->real.rc.right = p1->real.rc.left + cx1;
	p2->real.rc.left = p2->real.rc.right - cx2;
	p1->real.rcSplitter.SetRect(p1->real.rc.right,p1->real.rc.top,p1->real.rc.right+GetSplitterSize().cx,p1->real.rc.bottom);
}
// 
void MultiPaneCtrl::HalveHeightPanes(HPANE prev, HPANE next)
{	Pane *p1 = m_Tree.GetData(prev);
	Pane *p2 = m_Tree.GetData(next);
		// 
	int totalcy = p1->real.rc.Height() + p2->real.rc.Height();
	double totalfactor = p1->real.factor + p2->real.factor;
		// 
	int cy1 = totalcy / 2;
	if(cy1<m_szMinSize.cy) cy1 = m_szMinSize.cy;
		// 
	int cy2 = totalcy - cy1;
	if(cy2<m_szMinSize.cy)
	{	cy2 = m_szMinSize.cy;
		cy1 = totalcy - cy2;
	}
		// 
	p1->real.factor = totalfactor * ((double)cy1/(double)totalcy);
	p2->real.factor = totalfactor - p1->real.factor;
	if(p2->real.factor<0.0) p2->real.factor = 0.0;
		// 
	p1->real.rc.bottom = p1->real.rc.top + cy1;
	p2->real.rc.top = p2->real.rc.bottom - cy2;
	p1->real.rcSplitter.SetRect(p1->real.rc.left,p1->real.rc.bottom,p1->real.rc.right,p1->real.rc.bottom+GetSplitterSize().cy);
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsSplitterHorizontal(HPANE pane) const
{	_ASSERTE(IsExist(pane)==true);
	_ASSERTE(GetParent(pane)!=NULL);
		// 
	return !IsHorizontal( GetParent(pane) );
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::CanSplitterDrag(HPANE pane)
{	bool horz = IsSplitterHorizontal(pane);	// orientation of splitter.
		// 
	HPANE parent = GetParent(pane);
	CRect rcParent = GetRect(parent);
	int count = GetCount(parent);
		// 
	if(horz==true)
	{	int height = count*m_szMinSize.cy + (count-1)*GetSplitterSize().cy;
		return (rcParent.Height() > height);
	}
	else
	{	int width = count*m_szMinSize.cx + (count-1)*GetSplitterSize().cx;
		return (rcParent.Width() > width);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::CanShowCloseButton(TabCtrl *pCtrl)
{	return m_pAbilityManager->CanShowCloseButton(this,pCtrl);
}
// 
bool MultiPaneCtrl::CanShowMenuButton(TabCtrl *pCtrl)
{	return m_pAbilityManager->CanShowMenuButton(this,pCtrl);
}
// 
bool MultiPaneCtrl::CanShowScrollButtons(TabCtrl *pCtrl)
{	return m_pAbilityManager->CanShowScrollButtons(this,pCtrl);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnTabPreCreate(TabCtrl *pCtrl, HWND hWnd, TCHAR const *text, int image)
{
#ifdef _DEBUG
	if(m_bDragFinishing==false) 
		_ASSERTE(GetTabWithWindow(hWnd,NULL)==NULL);	// WINDOW FOR EACH TAB SHOULD HAS UNIQUE ID.
#endif
		// 
	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabPreCreate(this,pCtrl,hWnd,text,image);
}
// 
void MultiPaneCtrl::OnTabPostCreate(TabCtrl *pCtrl, HTAB hTab)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabPostCreate(this,pCtrl,hTab);
}
// 
void MultiPaneCtrl::OnTabPreDestroy(TabCtrl *pCtrl, HTAB hTab)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabPreDestroy(this,pCtrl,hTab);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnCloseButtonClicked(TabCtrl *pCtrl, CRect const *pRect, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabCloseButtonClicked(this,pCtrl,pRect,ptScr);
}
// 
void MultiPaneCtrl::OnMenuButtonClicked(TabCtrl *pCtrl, CRect const *pRect, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabMenuButtonClicked(this,pCtrl,pRect,ptScr);
}
// 
void MultiPaneCtrl::OnTabSelected(TabCtrl *pCtrl, HTAB hTab)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabSelected(this,pCtrl,hTab);
}
// 
void MultiPaneCtrl::OnLButtonDown(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabLButtonDown(this,pCtrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::OnLButtonDblClk(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabLButtonDblClk(this,pCtrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::OnRButtonDown(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabRButtonDown(this,pCtrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::OnRButtonUp(TabCtrl * pCtrl, HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabRButtonUp(this,pCtrl,hTab,ptScr);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::OnStartDrag(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr)
{	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabStartDrag(this,pCtrl,hTab,ptScr);
}
// 
void MultiPaneCtrl::OnDrag(TabCtrl *pTabSrc, HTAB hTabSrc, CPoint ptScr, bool outside)
{	if(IsDragTabEnable()==true)
	{		// 
			// show cursor.
		if(m_hpCurDragEnableRef!=NULL || m_hpCurDragDisableRef!=NULL)
		{	CRect rc;
			GetWindowRect(&rc);
			bool inside = (rc.PtInRect(ptScr)!=0);
			if(m_hpCurDragEnableRef!=NULL && *m_hpCurDragEnableRef!=NULL && inside==true) ::SetCursor(*m_hpCurDragEnableRef);
			if(m_hpCurDragDisableRef!=NULL && *m_hpCurDragDisableRef!=NULL && inside==false) ::SetCursor(*m_hpCurDragDisableRef);
		}
			// 
			// show markers.
		HPANE paneDst = GetDragDstPane(hTabSrc,ptScr);
			// 
		if(paneDst!=NULL)	// need to show markers.
		{	TabCtrl *pTabDst = GetTabCtrl(paneDst);
				// 
			CRect rcTabDst;
			pTabDst->GetClientRect(&rcTabDst);
			pTabDst->ClientToScreen(&rcTabDst);
				// 
			m_hDraggingTab = hTabSrc;
				// 
			if(paneDst!=m_hMarkedPane)
			{	m_hMarkedPane = paneDst;
				if(m_DockMarkMngr.WasCreated()==true) m_DockMarkMngr.ShowPaneMarkers(&rcTabDst);
				m_SelectedMarker = DOCKMARKER_NONE;
				m_DockMarkMngr.HideSelectBar(this);
			}
				// 
			DOCKMARKER selectedMarker = RelayMouseMove(pTabDst,ptScr);	// get marker under cursor.
				// 
				// show select bar.
			if(selectedMarker!=m_SelectedMarker)
			{	if(selectedMarker==DOCKMARKER_NONE || m_DockMarkMngr.WasCreated()==false)
					m_DockMarkMngr.HideSelectBar(this);	// hide old markers or SelectFrame.
					// 
				switch(selectedMarker)
				{	case DOCKMARKER_PANE_LEFT: 
						rcTabDst.right = rcTabDst.CenterPoint().x;
						m_DockMarkMngr.ShowSelectBar(this,&rcTabDst,false,false,0);
						break;
					case DOCKMARKER_PANE_TOP:
						rcTabDst.bottom = rcTabDst.CenterPoint().y;
						m_DockMarkMngr.ShowSelectBar(this,&rcTabDst,false,false,0);
						break;
					case DOCKMARKER_PANE_RIGHT:
						rcTabDst.left = rcTabDst.CenterPoint().x;
						m_DockMarkMngr.ShowSelectBar(this,&rcTabDst,false,false,0);
						break;
					case DOCKMARKER_PANE_BOTTOM:
						rcTabDst.top = rcTabDst.CenterPoint().y;
						m_DockMarkMngr.ShowSelectBar(this,&rcTabDst,false,false,0);
						break;
					case DOCKMARKER_PANE_TABS:
						m_DockMarkMngr.ShowSelectBar(this,&rcTabDst,true,(pTabDst->GetLayout()==TAB_LAYOUT_TOP),pTabDst->CalcCtrlAreaHeight());
						break;
				}
			}
				// 
			m_SelectedMarker = selectedMarker;
		}
		else	// need to hide markers.
			if(m_hMarkedPane!=NULL)	// there are still visible markers.
			{	if(m_DockMarkMngr.WasCreated()==true) m_DockMarkMngr.HidePaneMarkers();
				m_DockMarkMngr.HideSelectBar(this);
				m_hMarkedPane = NULL;
				m_SelectedMarker = DOCKMARKER_NONE;
			}
	}
		// 
	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabDrag(this,pTabSrc,hTabSrc,ptScr,outside);
}
// 
HPANE MultiPaneCtrl::GetDragDstPane(HTAB hTabSrc, CPoint ptScr)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(hTabSrc==NULL || GetPane(hTabSrc)!=NULL);
		// 
	HPANE hPaneDst = HitTest(ptScr);
	if(hPaneDst==NULL) return NULL;
		// 
	TabCtrl *pTabDst = GetTabCtrl(hPaneDst);
		// 
	if(hTabSrc!=NULL)
		if(GetPane(hTabSrc)==hPaneDst)
		{	if(pTabDst->GetCount()==1) return NULL;	// drag to itself.
				// 
			CRect rc(pTabDst->GetWindowsArea());
			pTabDst->ClientToScreen(&rc);
			if(rc.PtInRect(ptScr)==FALSE) return NULL;
		}
		// 
	CRect rcTabDst;
	pTabDst->GetClientRect(&rcTabDst);
	if(rcTabDst.Width()<m_szMinSizeForDrop.cx || rcTabDst.Height()<m_szMinSizeForDrop.cy)
		return NULL;
		// 
	return hPaneDst;
}
// 
DOCKMARKER MultiPaneCtrl::RelayMouseMove(TabCtrl *pTabDst, CPoint ptScr)
{	DOCKMARKER marker = DOCKMARKER_NONE;
		// 
	if(m_DockMarkMngr.WasCreated()==true)	// use markers.
		marker = m_DockMarkMngr.RelayMouseMove(ptScr,true,false);
	else	// markers wasn't created - use select frame.
	{	CRect rc(pTabDst->GetWindowsArea());
		pTabDst->ClientToScreen(&rc);
			// 
		if(rc.PtInRect(ptScr)!=0)	// inside of child window area of destination TabCtrl.
		{	CRect rcTab(rc);
			rcTab.DeflateRect(rcTab.Width()/3,rcTab.Height()/3);
				// 
			if(rcTab.PtInRect(ptScr)!=0 && IsMarkerEnable(NULL,DOCKMARKER_PANE_TABS)==true)
				marker = DOCKMARKER_PANE_TABS;
			else
			{	CPoint center = rc.CenterPoint();
				if(Tools::PtInTriangle(ptScr,CPoint(rc.left,rc.bottom),CPoint(rc.left,rc.top),center)==true &&
					IsMarkerEnable(NULL,DOCKMARKER_PANE_LEFT)==true) marker = DOCKMARKER_PANE_LEFT;
				else if(Tools::PtInTriangle(ptScr,CPoint(rc.left,rc.top),CPoint(rc.right,rc.top),center)==true &&
					IsMarkerEnable(NULL,DOCKMARKER_PANE_TOP)==true) marker = DOCKMARKER_PANE_TOP;
				else if(Tools::PtInTriangle(ptScr,CPoint(rc.right,rc.top),CPoint(rc.right,rc.bottom),center)==true &&
					IsMarkerEnable(NULL,DOCKMARKER_PANE_RIGHT)==true) marker = DOCKMARKER_PANE_RIGHT;
				else if(Tools::PtInTriangle(ptScr,CPoint(rc.right,rc.bottom),CPoint(rc.left,rc.bottom),center)==true &&
					IsMarkerEnable(NULL,DOCKMARKER_PANE_BOTTOM)==true) marker = DOCKMARKER_PANE_BOTTOM;
			}
		}
	}
		// 
	if(marker==DOCKMARKER_NONE)	// check for location inside of control area of destination TabCtrl.
	{	CRect rc(pTabDst->GetCtrlArea());
		pTabDst->ClientToScreen(&rc);
			// 
		if(rc.PtInRect(ptScr)!=0 && IsMarkerEnable(NULL,DOCKMARKER_PANE_TABS)==true)
			marker = DOCKMARKER_PANE_TABS;
	}
		// 
	return marker;
}
// 
void MultiPaneCtrl::OnFinishDrag(TabCtrl *pTabSrc, HTAB hTabSrc, bool cancel)
{
	TabCtrl* pTabDst = NULL;
	HTAB hTabDst = NULL;

	if(IsDragTabEnable()==true)
	{	m_bDragFinishing = true;
			// 
			// hide markers and select bar.
		if(m_DockMarkMngr.WasCreated()==true) m_DockMarkMngr.HidePaneMarkers();
		m_DockMarkMngr.HideSelectBar(this);
			// 
			// insert new and delete old tab.
		if(cancel==false && m_SelectedMarker!=DOCKMARKER_NONE)
		{	if(m_SelectedMarker==DOCKMARKER_PANE_TABS)
			{	HPANE hPaneDst = m_hMarkedPane;
				pTabDst = GetTabCtrl(hPaneDst);
					// 
				hTabDst = NULL;
				try
				{	if(pTabDst->GetCount()>0)	// insert tab in 0 position.
					{	hTabDst = InsertTab(hPaneDst,pTabDst->GetTab(0),pTabSrc->GetTabWnd(hTabSrc),_T(""),-1);
						pTabDst->EnsureVisible(hTabDst);
					}
					else	// add first tab.
						hTabDst = AddTab(hPaneDst,pTabSrc->GetTabWnd(hTabSrc),_T(""),-1);
					pTabDst->CopyTab(hTabDst,pTabSrc,hTabSrc);	// copy: text, image, data, tooltip text.
					pTabDst->SetSel(hTabDst);
						// 
					HPANE hPaneSrc = GetPane(pTabSrc);
					DeleteSrcTab(hPaneSrc,pTabSrc,hTabSrc,m_hMarkedPane,m_SelectedMarker);
						// 
					pTabDst->SetFocus();
				}
				catch(std::bad_alloc &)
				{	if(hTabDst!=NULL)
					{	::SetParent(pTabDst->GetTabWnd(hTabDst),pTabSrc->m_hWnd);	// restore parent window.
						pTabDst->Delete(hTabDst);
					}
				}
			}
			else
			{	HPANE hParent = GetParent(m_hMarkedPane);
				HWND hWnd = pTabSrc->GetTabWnd(hTabSrc);
					// 
				HPANE hPaneDst = NULL;
				hTabDst = NULL;
				pTabDst = NULL;
					// 
				bool bNeedHorizOrient = (m_SelectedMarker==DOCKMARKER_PANE_LEFT || m_SelectedMarker==DOCKMARKER_PANE_RIGHT);
				bool bInsertPrev = (m_SelectedMarker==DOCKMARKER_PANE_LEFT || m_SelectedMarker==DOCKMARKER_PANE_TOP);
					// 
				try
				{	if(hParent!=NULL && IsHorizontal(hParent)==bNeedHorizOrient)
					{	if(bInsertPrev==true)
							hPaneDst = Insert(m_hMarkedPane,SPACE_NEXT);
						else
						{	HPANE hNext = GetNextChild(m_hMarkedPane);
							hPaneDst = (hNext!=NULL ? Insert(hNext,SPACE_PREVIOUS) : Add(hParent));
						}
					}
					else
					{	ConvertToLine(m_hMarkedPane,bNeedHorizOrient);
						if(bInsertPrev==true)
							hPaneDst = Insert( GetPane(m_hMarkedPane,0), SPACE_BETWEEN);
						else
							hPaneDst = Add(m_hMarkedPane);
					}
					pTabDst = GetTabCtrl(hPaneDst);
						// 
					hTabDst = AddTab(hPaneDst,hWnd,_T(""),-1);	// add first tab.
					pTabDst->CopyTab(hTabDst,pTabSrc,hTabSrc);	// copy: text, image, data, tooltip text.
						// 
					HPANE hPaneSrc = GetPane(pTabSrc);
					DeleteSrcTab(hPaneSrc,pTabSrc,hTabSrc,m_hMarkedPane,m_SelectedMarker);
						// 
					pTabDst->SetFocus();
				}
				catch(std::bad_alloc &)
				{	if(hTabDst!=NULL)
					{	::SetParent(hWnd,pTabSrc->m_hWnd);
						pTabDst->Delete(hTabDst);
					}
					if(hPaneDst!=NULL) Delete(hPaneDst,SPACE_BETWEEN);
				}
			}
			Update();
		}
			// 
		m_hMarkedPane = NULL;
		m_SelectedMarker = DOCKMARKER_NONE;
			// 
		m_bDragFinishing = false;
	}
		// 
	if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabFinishDrag(this,pTabDst,hTabDst,cancel);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteSrcTab(HPANE paneSrc, TabCtrl *pTabSrc, HTAB hTabSrc, HPANE paneMarked, DOCKMARKER markerSelected)
{	pTabSrc->Delete(hTabSrc);
		// 
	if(pTabSrc->GetCount()==0)	// deleted last tab.
	{	SPACE space = SPACE_BETWEEN;
			// 
		HPANE hPrev = GetPrevChild(paneMarked);
		HPANE hNext = GetNextChild(paneMarked);
			// 
		if(GetParent(paneSrc)==GetParent(paneMarked))
			if(hPrev==paneSrc ||
				(markerSelected==DOCKMARKER_PANE_LEFT && hPrev!=NULL && GetPrevChild(hPrev)==paneSrc))
				space = SPACE_PREVIOUS;
			else if(hNext==paneSrc || 
				(markerSelected==DOCKMARKER_PANE_RIGHT && hNext!=NULL && GetNextChild(hNext)==paneSrc))
				space = SPACE_NEXT;
			// 
			// delete pane down with optimization.
		DeleteOptimizDown(paneSrc,IsDropOptimization(),space);
	}
	else	// there are still some tabs.
		pTabSrc->SetFocus();	// for stable work selection between TabCtrls.
}
// 
void MultiPaneCtrl::DeleteOptimizDown(HPANE paneSrc, bool optimize, SPACE space)
{	HPANE parentPane = NULL;
		// 
		// delete pane down (in the direction of root).
	for(HPANE h=paneSrc; h!=NULL && GetCount(h)==0; h=parentPane)
	{	parentPane = GetParent(h);
		if(parentPane!=NULL) Delete(h,space);
	}
		// 
	if(optimize==true)	// use optimization of layout.
		for(; parentPane!=NULL; )
		{	int count = GetCount(parentPane);
				// 
			if(count==0)	// has tabs.
			{	HPANE parent = GetParent(parentPane);
					// 
/*	old version.
				if(parent!=NULL && GetCount(parent)==1)
					Delete(parentPane,SPACE_BETWEEN);
*/
				if(parent!=NULL)
					if(GetCount(parent)==1 || GetTabCtrl(parentPane)->GetCount()==0)
						Delete(parentPane,SPACE_BETWEEN);
					else
						break;
				parentPane = parent;
			}
			else if(count==1)	// has one child pane.
				parentPane = GetPane(parentPane,0);
			else	// has more than one child pane.
			{	HPANE parent = GetParent(parentPane);
					// 
				if(parent!=NULL && GetCount(parent)==1)
				{	DeleteIntermediate(parentPane);
					parentPane = parent;
				}
				else
					break;
			}
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::DeleteOptimizDown(HPANE hPane)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(IsExist(hPane)==true);
	_ASSERTE(GetParent(hPane)!=NULL);
	_ASSERTE(GetCount(hPane)==0);	// source pane mustn't has child panes.
	_ASSERTE(GetTabCtrl(hPane)->GetCount()==0);	// source tab control mustn't has child tabs.
		// 
	DeleteOptimizDown(hPane,true,SPACE_BETWEEN);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::IsMarkerEnable(DockingMarkers * /*pObj*/, DOCKMARKER marker)
{	if(marker==DOCKMARKER_PANE_TABS && 
		GetTabCtrl(m_hMarkedPane)->IsExist(m_hDraggingTab)==true)
		return false;
		// 
	return m_pAbilityManager->CanDrop(this,m_hDraggingTab,MarkerToSide(marker),m_hMarkedPane);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
DOCKSIDE MultiPaneCtrl::MarkerToSide(DOCKMARKER marker) const
{	switch(marker)
	{	case DOCKMARKER_PANE_LEFT: return DOCKSIDE_LEFT;
		case DOCKMARKER_PANE_TOP: return DOCKSIDE_TOP;
		case DOCKMARKER_PANE_RIGHT: return DOCKSIDE_RIGHT;
		case DOCKMARKER_PANE_BOTTOM: return DOCKSIDE_BOTTOM;
		case DOCKMARKER_PANE_TABS: return DOCKSIDE_TABS;
	}
	_ASSERTE(!"Type of 'marker' is not correct");
	return DOCKSIDE_NONE;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
int MultiPaneCtrl::GetBorderWidth(MultiPaneCtrl * /*pCtrl*/, IMultiPaneCtrlRecalc * /*base*/)
{	return 1;
}
// 
CSize MultiPaneCtrl::GetSplitterSize(MultiPaneCtrl * /*pCtrl*/, IMultiPaneCtrlRecalc * /*base*/)
{	return CSize(4,4);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct TabData : public MultiPaneCtrl::Tab
{	HWND wnd;
	CString text;
	int image;
	CString tooltip;
	bool disable;
	__int64 data;
};
// 
HWND MultiPaneCtrl::Tab::GetHWND() const
{	return ((TabData *)this)->wnd;
}
// 
CString MultiPaneCtrl::Tab::GetText() const
{	return ((TabData *)this)->text;
}
// 
int MultiPaneCtrl::Tab::GetImage() const
{	return ((TabData *)this)->image;
}
// 
CString MultiPaneCtrl::Tab::GetToolTipText() const
{	return ((TabData *)this)->tooltip;
}
// 
bool MultiPaneCtrl::Tab::IsDisable() const
{	return ((TabData *)this)->disable;
}
// 
__int64 MultiPaneCtrl::Tab::GetData() const
{	return ((TabData *)this)->data;
}
/////////////////////////////////////////////////////////////////////////////
// 
struct TabsContainer : public Blade::Allocatable
{	
	Vector<TabData *> vtr_tabs;
	Map<int/*id*/,TabData> map_id_to_data;
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabsHelper : public MultiPaneCtrl::Tabs
{	TabsHelper(MultiPaneCtrl::Tabs const *tabs)
	{	Owner = tabs->Owner;
	}
	~TabsHelper()
	{	Owner = NULL;	// that don't delete vector in the ~Tabs.
	}
		// 
	void DeleteAll()
	{	if(Owner!=NULL)
		{	TabsContainer *p = (TabsContainer *)Owner;
			p->vtr_tabs.clear();
			p->map_id_to_data.clear();
		}
	}
		// 
	TabData *GetTabWithCtrlID(int id) const
	{	_ASSERTE(id>0);
			// 
		if(Owner!=NULL)
		{	TabsContainer *p = (TabsContainer *)Owner;
			Map<int,TabData>::const_iterator ic = p->map_id_to_data.find(id);
			if(ic==p->map_id_to_data.end()) return NULL;
			return (TabData *)&ic->second;
		}
		return NULL;
	}
};
/////////////////////////////////////////////////////////////////////////////
// 
MultiPaneCtrl::Tabs::Tabs()
{	Owner = NULL;
}
// 
MultiPaneCtrl::Tabs::~Tabs()
{	if(Owner!=NULL)
	{	BLADE_DELETE (TabsContainer *)Owner;
		Owner = NULL;
	}
}
// 
int MultiPaneCtrl::Tabs::Add(HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc)
{	return Add(wnd,text,image,_T(""),false,0);
}
// 
int MultiPaneCtrl::Tabs::Add(HWND wnd, TCHAR const *text, int image, TCHAR const *tooltip, bool disable, __int64 data) throw(std::bad_alloc)
{	_ASSERTE(wnd!=NULL && ::IsWindow(wnd)!=0);
	_ASSERTE(::GetDlgCtrlID(wnd)!=0);
	_ASSERTE(text!=NULL && tooltip!=NULL);
	_ASSERTE(image>=-1);
		// 
	if(Owner==NULL)
		Owner = (PSID)BLADE_NEW TabsContainer;
	TabsContainer *p = (TabsContainer *)Owner;
		// 
	int id = ::GetDlgCtrlID(wnd);
	_ASSERTE(TabsHelper(this).GetTabWithCtrlID(id)==NULL);	// window with the same id must not be added twice.
		// 
	typedef Map<int,TabData>::iterator i_map;
	std::pair<i_map,bool> pr = p->map_id_to_data.insert( std::pair<int,TabData>(id,TabData()) );
	TabData *ptd = &pr.first->second;
	p->vtr_tabs.push_back(ptd);
		// 
	ptd->wnd = wnd;
	CSTRING_BAD_ALLOC( ptd->text=text );
	ptd->image = image;
	CSTRING_BAD_ALLOC( ptd->tooltip=tooltip );
	ptd->disable = disable;
	ptd->data = data;
		// 
	return (int)p->vtr_tabs.size()-1;
}
// 
MultiPaneCtrl::Tab const *MultiPaneCtrl::Tabs::operator[](int idx)
{	if(Owner==NULL) return NULL;
		// 
	TabsContainer *p = (TabsContainer *)Owner;
	_ASSERTE(idx>=0 && idx<(int)p->vtr_tabs.size());
		// 
	return p->vtr_tabs[idx];
}
// 
int MultiPaneCtrl::Tabs::GetCount() const
{	if(Owner==NULL) return 0;
		// 
	TabsContainer *p = (TabsContainer *)Owner;
	return (int)p->vtr_tabs.size();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SaveTabs(Tabs *tabs/*out*/) const
{	try
	{	TabsHelper(tabs).DeleteAll();
			// 
		for(HPANE h=GetFirst(); h!=NULL; h=GetNext(h))
			if(GetCount(h)==0)
			{	TabCtrl *tabCtrl = GetTabCtrl(h);
					// 
				for(int i=0, c=tabCtrl->GetCount(); i<c; ++i)
				{	HTAB tab = tabCtrl->GetTab(i);
						// 
					tabs->Add(tabCtrl->GetTabWnd(tab),tabCtrl->GetTabText(tab),tabCtrl->GetTabImage(tab),
						tabCtrl->GetTabTooltipText(tab),tabCtrl->IsDisable(tab),tabCtrl->GetTabData(tab));
				}
			}
	}
	catch(std::bad_alloc &)
	{	TabsHelper(tabs).DeleteAll();
		return false;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const
{	_ASSERTE(app!=NULL && section!=NULL && entry!=NULL);
		// 
	if(GetSafeHwnd()!=NULL)
	{	CMemFile file;
		CArchive ar(&file,CArchive::store);
		if(SaveState(&ar)==false) return false;
		ar.Flush();
		ar.Close();
			// 
		UINT uDataSize = (UINT)file.GetLength();
		BYTE *pData = file.Detach();
		bool res = app->WriteProfileBinary(section,entry,pData,uDataSize)!=0;
		free(pData);
		return res;
	}
	else
		return false;
}
// 
bool MultiPaneCtrl::LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack)
{	_ASSERTE(GetSafeHwnd()!=NULL);
	_ASSERTE(app!=NULL && section!=NULL && entry!=NULL);
		//
	bool res = false;
	BYTE *pData = NULL;
	UINT uDataSize;
		// 
	try
	{	if(app->GetProfileBinary(section,entry,&pData,&uDataSize)!=0)
		{	CMemFile file(pData,uDataSize);
			CArchive ar(&file,CArchive::load);
			res = LoadState(&ar,tabs,ignoreLack);
		}
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	if(pData!=NULL) delete [] pData;
		// 
	return res;
}
// 
bool MultiPaneCtrl::LoadStateAndUpdate(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack)
{	SetRedraw(FALSE);
		// 
	bool res = LoadState(app,section,entry,tabs,ignoreLack);
		// 
	Update();
	SetRedraw(TRUE);
	RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		// 
	return res;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::SaveState(CArchive *ar) const
{	_ASSERTE(ar!=NULL);
		// 
	if(GetSafeHwnd()!=NULL)
	{	try
		{	SaveStateInner(ar);
			return true;
		}
		catch(CMemoryException* pEx)
		{	pEx->Delete();
		}
		catch(CArchiveException* pEx)
		{	pEx->Delete();
		}
		catch(...)
		{
		}
	}
	return false;
}
// 
bool MultiPaneCtrl::LoadState(CArchive *ar, Tabs const *tabs, bool ignoreLack)
{	_ASSERTE(GetSafeHwnd()!=NULL);
		// 
	try
	{	if(LoadStateInner(ar,tabs,ignoreLack)==false)
		{	DeleteAll();
			return false;
		}
		return true;
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	catch(CArchiveException* pEx)
	{	pEx->Delete();
	}
	catch(...)
	{
	}
	return false;
}
// 
bool MultiPaneCtrl::LoadStateAndUpdate(CArchive *ar, Tabs const *tabs, bool ignoreLack)
{	SetRedraw(FALSE);
		// 
	bool res = LoadState(ar,tabs,ignoreLack);
		// 
	Update();
	SetRedraw(TRUE);
	RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		// 
	return res;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrl::SaveStateInner(CArchive *ar) const
{	SaveStateInner(ar,NULL);
}
// 
void MultiPaneCtrl::SaveStateInner(CArchive *ar, HPANE pane) const
{	int count = GetCount(pane);
	*ar << count;
		// 
	if(count>0)
	{	char horz = (char)(IsHorizontal(pane)==true ? 1 : 0);
		*ar << horz;
			// 
		int i;
		for(i=0; i<count; ++i)
		{	HPANE h = GetPane(pane,i);
			SaveStateInner(ar,h);
		}
		for(i=0; i<count; ++i)	// save factors.
		{	HPANE h = GetPane(pane,i);
				// 
			double factor = m_Tree.GetData(h)->real.factor;
			*ar << factor;
		}
	}
	else
	{	TabCtrl *tab = GetTabCtrl(pane);
			// 
		count = tab->GetCount();
		*ar << count;
			// 
		if(count>0)
		{	if(count>1)
			{	HTAB hSelTab = tab->GetSel();
				*ar << (hSelTab!=NULL ? tab->GetIndex(hSelTab) : -1);
			}
				// 
			for(int i=0, c=count; i<c; ++i)
			{	HWND hWnd = tab->GetTabWnd( tab->GetTab(i) );
					// 
				_ASSERTE(::IsWindow(hWnd)!=0);
					// 
				*ar << ::GetDlgCtrlID(hWnd);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
bool MultiPaneCtrl::LoadStateInner(CArchive *ar, Tabs const *tabs, bool ignoreLack) throw(std::bad_alloc)
{	DeleteAll();
		// 
	List<HPANE> emptyPanes;	// empty panes to delete.
		// 
	try
	{	if(LoadStateInner(ar,GetRoot(),tabs,ignoreLack,&emptyPanes)==false) return false;
			// 
		for(List<HPANE>::iterator i=emptyPanes.begin(), e=emptyPanes.end(); i!=e; ++i)
			if(IsExist(*i)==true && GetParent(*i)!=NULL)
				DeleteOptimizDown(*i);
			// 
		return true;
	}
	catch(std::bad_alloc &)
	{	DeleteAll();
		throw;
	}
}
// 
bool MultiPaneCtrl::LoadStateInner(CArchive *ar, HPANE parent, Tabs const *tabs, bool ignoreLack, List<HPANE> *panes) throw(std::bad_alloc)
{	int count;
	*ar >> count;
		// 
	if(count>0)
	{	char horz;
		*ar >> horz;
			// 
		int i;
		for(i=0; i<count; ++i)
		{	HPANE h = (i==0 ? ConvertToLine(parent,horz!=0) : Add(parent));
			if(LoadStateInner(ar,h,tabs,ignoreLack,panes)==false) return false;
		}
		for(i=0; i<count; ++i)
		{	HPANE h = GetPane(parent,i);
				// 
			double factor;
			*ar >> factor;
			m_Tree.GetData(h)->real.factor = factor;
		}
	}
	else
	{	*ar >> count;
			// 
		if(count>0)
		{	TabCtrl *tabCtrl = GetTabCtrl(parent);
				// 
			int selIdx = -1;
			if(count>1)
				*ar >> selIdx;
				// 
			TabsHelper th(tabs);
				// 
			for(int i=0, c=count; i<c; ++i)
			{	int id;
				*ar >> id;
					// 
				TabData *tabData = th.GetTabWithCtrlID(id);
					// 
				if(tabData==NULL)
				{	if(ignoreLack==true)
					{	if(selIdx!=-1)
							if(selIdx==i) selIdx = -1;
							else if(i<selIdx) --selIdx;
						continue;
					}
						// 
					TRACE("Not found tab with control id=%d (MultiPaneCtrl::LoadStateInner)",id);
					return false;
				}
					// 
				HTAB h = AddTab(parent,tabData->wnd,tabData->text,tabData->image);
				tabCtrl->SetTabTooltipText(h,tabData->tooltip);
				tabCtrl->Disable(h,tabData->disable);
				tabCtrl->SetTabData(h,tabData->data);
			}
				// 
			int finalCount = tabCtrl->GetCount();
				// 
			if(selIdx>=0 && selIdx<finalCount)
			{	HTAB h = tabCtrl->GetTab(selIdx);
				tabCtrl->SetSel(h);
			}
				// 
			if(finalCount==0 && count>0)
				panes->push_back(parent);
		}
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// MultiPaneCtrlRecalcStub.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int MultiPaneCtrlRecalcStub::GetBorderWidth(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base)
{	return base->GetBorderWidth(pCtrl,NULL);
}
// 
CSize MultiPaneCtrlRecalcStub::GetSplitterSize(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base)
{	return base->GetSplitterSize(pCtrl,NULL);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// MultiPaneCtrlDraw.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlDrawBase::DrawSplitter(MultiPaneCtrl *pCtrl, CDC *pDC, bool /*horz*/, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect,GetSplitterColor(pCtrl));
}
/////////////////////////////////////////////////////////////////////////////
//
void MultiPaneCtrlDrawBase::DrawSplitterDragRect(MultiPaneCtrl *pCtrl, CDC *pDC, bool /*horz*/, CRect const *pRectNew, CRect const *pRectOld)
{	if(pRectOld==NULL)		// first draw.
		DrawDragRect(pCtrl,pDC,pRectNew);
	else if(pRectNew==NULL)	// last draw.
		DrawDragRect(pCtrl,pDC,pRectOld);
	else
	{	RgnEx rgn, rgnNew(pRectNew), rgnOld(pRectOld);
			// draw new.
		rgn.CombineRgn(&rgnNew,&rgnOld,RGN_DIFF);
		pDC->SelectClipRgn(&rgn);
		DrawDragRect(pCtrl,pDC,pRectNew);
			// erase old.
		rgn.CombineRgn(&rgnOld,&rgnNew,RGN_DIFF);
		pDC->SelectClipRgn(&rgn);
		DrawDragRect(pCtrl,pDC,pRectOld);
			// 
		pDC->SelectClipRgn(NULL,RGN_COPY);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlDrawBase::DrawBorder(MultiPaneCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	Draw::DrawRect(pDC,pRect,GetBorderColor(pCtrl));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF MultiPaneCtrlDrawBase::GetSplitterColor(MultiPaneCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNFACE);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF MultiPaneCtrlDrawBase::GetBorderColor(MultiPaneCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNSHADOW);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlDrawBase::DrawDragRect(MultiPaneCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect)
{	CBrush *pOldBrush = pDC->SelectObject(CDC::GetHalftoneBrush());
	pDC->PatBlt(pRect->left,pRect->top,pRect->Width(),pRect->Height(),PATINVERT);
	pDC->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// MultiPaneCtrlStyle_VS2010_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
CSize MultiPaneCtrlStyle_VS2010_base::GetSplitterSize(MultiPaneCtrl * /*pCtrl*/, IMultiPaneCtrlRecalc * /*base*/)
{	return CSize(6,6);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF MultiPaneCtrlStyle_VS2010_base::GetSplitterColor(MultiPaneCtrl * /*pCtrl*/)
{	return RGB(46,64,94);
}
/////////////////////////////////////////////////////////////////////////////
// 
void MultiPaneCtrlStyle_VS2010_base::DrawDragRect(MultiPaneCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect)
{	CBrush br(~::GetSysColor(COLOR_BTNSHADOW) & 0x00ffffff);
		// 
	CBrush *pOldBrush = pDC->SelectObject(&br);
	pDC->PatBlt(pRect->left,pRect->top,pRect->Width(),pRect->Height(),PATINVERT);
	pDC->SelectObject(pOldBrush);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////












