//==========================================================
// Author: Borodenko Oleg
// 05/08/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <ui/private/MultiPaneCtrl/TabCtrl.h>
#include <ui/private/MultiPaneCtrl/Draw.h>
/////////////////////////////////////////////////////////////////////////////
using namespace Blade;
/////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4290)	// C++ exception specification ignored...
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
// TabCtrl.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define HNDLtoPTR(h)		((Tab *)h)
/////////////////////////////////////////////////////////////////////////////
#define TIMER_ID_LEFT_SCROLL_CLICK		1
#define TIMER_ID_LEFT_SCROLLING			2
#define TIMER_ID_RIGHT_SCROLL_CLICK		3
#define TIMER_ID_RIGHT_SCROLLING			4
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BLADE_IMPLEMENT_DYNCREATE(TabCtrl,CWnd)
/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(TabCtrl, CWnd)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_CAPTURECHANGED()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrl::TabCtrl() :
	m_hButClose((HTAB)1), m_hButMenu((HTAB)2),
	m_hButLeftScroll((HTAB)3), m_hButRightScroll((HTAB)4)
{
	m_hCurTab = NULL;
	m_hHoverArea = NULL;
	m_hPushedArea = NULL;
	m_iTabsOffset = 0;
	m_bPartialView = m_bScrollLeftAllow = m_bScrollRightAllow = false;
	m_bTabDrag = false;
	m_bWatchActivityCtrl = false;
	m_bActive = false;
		// 
	m_pToolTip = NULL;
		// 
	m_pDrawManager = NULL;
	m_pRecalcManager = &m_Service;
	m_pBehaviorManager = &m_Service;
	m_pToolTipManager = &m_Service;
	m_pAbilityManager = this;
	m_pNotifyManager = NULL;
	m_Layout = TAB_LAYOUT_TOP;
	m_Behavior = TAB_BEHAVIOR_SCALE;
	m_bShowBorder = true;
	m_bEqualTabsSize = false;
	m_bRemoveTabEnable = false;
	m_bHideSingleTab = false;
	m_bShowCloseButton = true;
	m_bShowMenuButton = true;
	m_bShowScrollButtons = true;
	m_bToolTipEnable = true;
	m_iScrollingStep = 15;
		// 
	m_pImageListRef = m_pImageListDisabledRef = NULL;
	m_pSysImageListRef = NULL;
	m_szSysImage.cx = m_szSysImage.cy = 0;
	m_szImage = m_szImageDisabled = m_szSysImage;
	m_pFontRef = m_pFontSelectRef = NULL;
	m_hCursor = NULL;
	m_phCursorRef = NULL;
		// 
	m_pLifeStatus = NULL;
}
// 
TabCtrl::~TabCtrl()
{	DestroyWindow();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL TabCtrl::Create(LPCTSTR /*lpszClassName*/, LPCTSTR /*lpszWindowName*/, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* /*pContext*/)
{	return Create(pParentWnd,dwStyle,rect,nID)==true ? 1 : 0;
}
// 
bool TabCtrl::Create(CWnd *parent, DWORD dwStyle, RECT const &rect, UINT nID)
{	CFont *font = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	if(GetFont()==NULL) SetFont(font);
	if(GetFontSelect()==NULL) SetFontSelect(font);
		// 
	CString sClass = Tools::RegisterWndClass(_T("TabCtrl"),CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),NULL,NULL);
	if(CWnd::Create(sClass,_T(""),dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,rect,parent,nID)==FALSE)
	{
		CFont::DeleteTempMap();
		return false;
	}
		// 
	m_pToolTip = m_pToolTipManager->CreateToolTip(this,&m_Service);
		// 
	if(m_pToolTip!=NULL && m_pToolTip->m_hWnd!=NULL)
	{	CRect rc(0,0,0,0);
		if(m_pToolTip->AddTool(this,LPSTR_TEXTCALLBACK,&rc,1)==FALSE) return false;
			// 
		try
		{	if(m_sToolTipButClose.IsEmpty()!=0) m_sToolTipButClose = _T("Close");
			if(m_sToolTipButMenu.IsEmpty()!=0) m_sToolTipButMenu = _T("Menu");
			if(m_sToolTipButLeftScroll.IsEmpty()!=0) m_sToolTipButLeftScroll = _T("Scroll Left");
			if(m_sToolTipButRightScroll.IsEmpty()!=0) m_sToolTipButRightScroll = _T("Scroll Right");
		}
		catch(CMemoryException *p)
		{	p->Delete();
			CFont::DeleteTempMap();
			return false;
		}
	}
		// 
	if(m_bWatchActivityCtrl==true)
		m_ActivityHook.Add(this,m_hWnd);
		// 
	CFont::DeleteTempMap();
	return true;
}
// 
void TabCtrl::OnDestroy()
{	DeleteAll();
		// 
	if(m_hCursor!=NULL) ::DestroyCursor(m_hCursor);
		// 
	if(m_bWatchActivityCtrl==true)
		if(m_ActivityHook.IsExist(this)==true)
			m_ActivityHook.Delete(this);
		// 
	if(m_pLifeStatus!=NULL) *m_pLifeStatus = false;
		// 
	CWnd::OnDestroy();
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnActive(bool active, HWND /*wnd*/)	// ActivityHook::INotify.
{	if(active!=m_bActive)
	{	m_bActive = active;
		Invalidate(FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::WatchActivityCtrl(bool watch)
{	if(m_bWatchActivityCtrl!=watch)
	{	m_bWatchActivityCtrl = watch;
		m_bActive = false;
			// 
		if(GetSafeHwnd()!=NULL)
			if(m_bWatchActivityCtrl==true)	// on.
				m_ActivityHook.Add(this,m_hWnd);
			else	// off.
			{	if(m_ActivityHook.IsExist(this)==true)
					m_ActivityHook.Delete(this);
			}
	}
}
// 
bool TabCtrl::IsWatchActivityCtrl() const
{	return m_bWatchActivityCtrl;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::IsActive() const
{	return m_bActive;
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::Add(HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc)
{	return Insert(m_vtrtabs.end(),wnd,text,image);
}
// 
HTAB TabCtrl::Insert(HTAB hBefore, HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc)
{	_ASSERTE(IsExist(hBefore)==true);
		// 
	m_i_vtrtabs before = m_vtrtabs.begin() + GetIndex(hBefore);
	return Insert(before,wnd,text,image);
}
// 
HTAB TabCtrl::Insert(m_i_vtrtabs before, HWND hWnd, TCHAR const *text, int image) throw(std::bad_alloc)
{	_ASSERTE(hWnd!=NULL && ::IsWindow(hWnd)!=0 && ::GetParent(hWnd)==m_hWnd);
	_ASSERTE(text!=NULL);
	_ASSERTE(image>=-1);
	_ASSERTE(::GetDlgCtrlID(hWnd)!=0);	// ID==0 - this is error.
	_ASSERTE(GetTabWithWindow(hWnd)==NULL);	// window with this ID has inserted.
		// 
	if(m_pNotifyManager!=NULL)
		m_pNotifyManager->OnTabPreCreate(this,hWnd,text,image);
		// 
	Tab *tab = NULL;
		// 
	try
	{	tab = BLADE_NEW Tab;
		tab->hWnd = hWnd;
		tab->image = image;
		tab->text = text;
		tab->disable = false;
		tab->data = 0;
			// 
		m_vtrtabs.insert(before,tab);
	}
	catch(CMemoryException *p)
	{	p->Delete();
		BLADE_DELETE tab;
		throw std::bad_alloc();
	}
		// 
	if(m_hCurTab==NULL)
		m_hCurTab = GetFirstEnableTab();
	else
		Tools::ShowWindow(hWnd,false);
		// 
	if(m_pNotifyManager!=NULL)
		m_pNotifyManager->OnTabPostCreate(this,tab);
		// 
	return tab;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::RemoveBefore(HTAB hBefore, HTAB hSrc) throw(std::bad_alloc)
{	_ASSERTE(IsExist(hBefore)==true && IsExist(hSrc)==true);
		// 
	if(hBefore==hSrc) return;
		// 
	int src = GetIndex(hSrc);
	int before = GetIndex(hBefore);
	if(src+1==before) return;
	m_vtrtabs.insert(m_vtrtabs.begin()+before, HNDLtoPTR(hSrc));
	if(before<src) ++src;
	m_vtrtabs.erase( m_vtrtabs.begin()+src );
}
// 
void TabCtrl::RemoveAfter(HTAB hAfter, HTAB hSrc) throw(std::bad_alloc)
{	_ASSERTE(IsExist(hAfter)==true && IsExist(hSrc)==true);
		// 
	if(hAfter==hSrc) return;
		// 
	int src = GetIndex(hSrc);
	int after = GetIndex(hAfter);
	if(src-1==after) return;
	m_vtrtabs.insert(m_vtrtabs.begin()+1+after, HNDLtoPTR(hSrc));
	if(after<src) ++src;
	m_vtrtabs.erase( m_vtrtabs.begin()+src );
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::Delete(HTAB hTab)
{	_ASSERTE(IsExist(hTab)==true);
		// 
	if(hTab==m_hPushedArea)
	{	StopScrolling();
		StopDragging(true);
	}
	if(hTab==m_hHoverArea)
		m_hHoverArea = NULL;
		// 
	if(m_hCurTab==hTab)
	{	m_hCurTab = GetNextEnableTab(hTab);
		if(m_hCurTab==NULL) m_hCurTab = GetPrevEnableTab(hTab);
	}
		// 
	HWND hWnd = GetTabWnd(hTab);
	if(hWnd!=NULL && ::IsWindow(hWnd)!=0 &&
		::GetParent(hWnd)==m_hWnd)		// if only this is our child.
		Tools::ShowWindow(hWnd,false);
		// 
	if(m_pNotifyManager!=NULL)
		m_pNotifyManager->OnTabPreDestroy(this,hTab);
		// 
	BLADE_DELETE HNDLtoPTR(hTab);
	m_vtrtabs.erase( m_vtrtabs.begin()+GetIndex(hTab) );
		// 
	if(m_vtrtabs.empty()==true)
	{	m_iTabsOffset = 0;
		m_bPartialView = m_bScrollLeftAllow = m_bScrollRightAllow = false;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::DeleteAll()
{	for(m_i_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
	{	if(m_pNotifyManager!=NULL)
			m_pNotifyManager->OnTabPreDestroy(this,*i);
			// 
		HWND hWnd = GetTabWnd(*i);
		if(hWnd!=NULL && ::IsWindow(hWnd)!=0 && 
			::GetParent(hWnd)==m_hWnd)		// if only this is our child.
			Tools::ShowWindow(hWnd,false);
			// 
		BLADE_DELETE *i;
	}
	m_vtrtabs.clear();
	StopScrolling();
	StopDragging(true);
	m_hHoverArea = m_hPushedArea = m_hCurTab = NULL;
	m_iTabsOffset = 0;
	m_bPartialView = m_bScrollLeftAllow = m_bScrollRightAllow = false;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetDrawManager(TabCtrlDraw *p)
{	m_pDrawManager = p;
}
// 
TabCtrlDraw *TabCtrl::GetDrawManager() const
{	return m_pDrawManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetRecalcManager(ITabCtrlRecalc *p)
{	m_pRecalcManager = (p!=NULL ? p : &m_Service);
}
// 
ITabCtrlRecalc *TabCtrl::GetRecalcManager() const
{	return m_pRecalcManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetBehaviorManager(ITabCtrlBehavior *p)
{	m_pBehaviorManager = (p!=NULL ? p : &m_Service);
}
// 
ITabCtrlBehavior *TabCtrl::GetBehaviorManager() const
{	return m_pBehaviorManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetToolTipManager(ITabCtrlToolTip *p)
{	m_pToolTipManager = (p!=NULL ? p : &m_Service);
}
// 
ITabCtrlToolTip *TabCtrl::GetToolTipManager() const
{	return m_pToolTipManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetAbilityManager(TabCtrlUserAbility *p)
{	m_pAbilityManager = (p!=NULL ? p : this);
}
// 
TabCtrlUserAbility *TabCtrl::GetAbilityManager() const
{	return m_pAbilityManager;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetNotifyManager(TabCtrlNotify *p)
{	m_pNotifyManager = p;
}
// 
TabCtrlNotify *TabCtrl::GetNotifyManager() const
{	return m_pNotifyManager;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetLayout(TAB_LAYOUT layout)
{	m_Layout = layout;
}
// 
TAB_LAYOUT TabCtrl::GetLayout() const
{	return m_Layout;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetBehavior(TAB_BEHAVIOR behavior)
{	m_Behavior = behavior;
}
// 
TAB_BEHAVIOR TabCtrl::GetBehavior() const
{	return m_Behavior;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::SetImageLists(CImageList *pImageList, CImageList *pImageListDisabled)
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
	{	if(m_ImageList.Create(pImageList)==0 ||
			::ImageList_GetIconSize(m_ImageList.m_hImageList,(int *)&m_szImage.cx,(int *)&m_szImage.cy)==0)
		{
			m_szImage = CSize(0,0);
			res = false;
		}
		m_pImageListRef = &m_ImageList;
	}
	else
		m_szImage = CSize(0,0);
		// 
	if(pImageListDisabled!=NULL)
	{	if(m_ImageListDisabled.Create(pImageListDisabled)==0 ||
			::ImageList_GetIconSize(m_ImageListDisabled.m_hImageList,(int *)&m_szImageDisabled.cx,(int *)&m_szImageDisabled.cy)==0)
		{
			m_szImageDisabled = CSize(0,0);
			res = false;
		}
		m_pImageListDisabledRef = &m_ImageListDisabled;
	}
	else
		m_szImageDisabled = CSize(0,0);
		// 
	return res;
}
// 
void TabCtrl::SetImageListsRef(CImageList *pImageList, CImageList *pImageListDisabled)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
	_ASSERTE(pImageListDisabled==NULL || pImageListDisabled->m_hImageList!=NULL);
		// 
	if(m_ImageList.m_hImageList!=NULL) m_ImageList.DeleteImageList();
	m_pImageListRef = pImageList;
		// 
	if(m_pImageListRef==NULL ||
		::ImageList_GetIconSize(m_pImageListRef->m_hImageList,(int *)&m_szImage.cx,(int *)&m_szImage.cy)==0)
		m_szImage = CSize(0,0);
		// 
	if(m_ImageListDisabled.m_hImageList!=NULL) m_ImageListDisabled.DeleteImageList();
	m_pImageListDisabledRef = pImageListDisabled;
		// 
	if(m_pImageListDisabledRef==NULL || 
		::ImageList_GetIconSize(m_pImageListDisabledRef->m_hImageList,(int *)&m_szImageDisabled.cx,(int *)&m_szImageDisabled.cy)==0)
		m_szImageDisabled = CSize(0,0);
}
// 
void TabCtrl::GetImageLists(CImageList **pImageList/*out*/, CImageList **pImageListDisabled/*out*/)
{	if(pImageList!=NULL) *pImageList = m_pImageListRef;
	if(pImageListDisabled!=NULL) *pImageListDisabled = m_pImageListDisabledRef;
}
// 
void TabCtrl::GetImageSizes(CSize *pszImage/*out*/, CSize *pszImageDisabled/*out*/) const
{	if(pszImage!=NULL)
	{	pszImage->cx = m_szImage.cx;
		pszImage->cy = m_szImage.cy;
	}
	if(pszImageDisabled!=NULL)
	{	pszImageDisabled->cx = m_szImageDisabled.cx;
		pszImageDisabled->cy = m_szImageDisabled.cy;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::SetSystemImageList(CImageList *pImageList)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
		// 
	if(m_SysImageList.m_hImageList!=NULL) m_SysImageList.DeleteImageList();
	m_pSysImageListRef = NULL;
		// 
	if(pImageList!=NULL)
	{	if(m_SysImageList.Create(pImageList)==0 ||
			::ImageList_GetIconSize(m_SysImageList.m_hImageList,(int *)&m_szSysImage.cx,(int *)&m_szSysImage.cy)==0)
		{
			m_szSysImage = CSize(0,0);
			return false;
		}
		m_pSysImageListRef = &m_SysImageList;
	}
	else
		m_szSysImage = CSize(0,0);
	return true;
}
// 
void TabCtrl::SetSystemImageListRef(CImageList *pImageList)
{	_ASSERTE(pImageList==NULL || pImageList->m_hImageList!=NULL);
		// 
	if(m_SysImageList.m_hImageList!=NULL) m_SysImageList.DeleteImageList();
	m_pSysImageListRef = pImageList;
		// 
	if(m_pSysImageListRef==NULL ||
		::ImageList_GetIconSize(m_pSysImageListRef->m_hImageList,(int *)&m_szSysImage.cx,(int *)&m_szSysImage.cy)==0)
		m_szSysImage = CSize(0,0);
}
// 
CImageList *TabCtrl::GetSystemImageList() const
{	return m_pSysImageListRef;
}
// 
CSize TabCtrl::GetSystemImageSize() const
{	return m_szSysImage;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::SetCursor(UINT uId)
{	return SetCursor(AfxGetResourceHandle(),uId);
}
// 
bool TabCtrl::SetCursor(HMODULE hModule, UINT uId)
{	if(m_hCursor!=NULL)
	{	::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
	m_phCursorRef = NULL;
		// 
	if(hModule!=NULL && uId!=0)
	{	m_hCursor = ::LoadCursor(hModule,MAKEINTRESOURCE(uId));
		if(m_hCursor==NULL) return false;
		m_phCursorRef = &m_hCursor;
	}
	return true;
}
// 
bool TabCtrl::SetCursor(HCURSOR hCursor)
{	if(m_hCursor!=NULL)
	{	::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
	m_phCursorRef = NULL;
		// 
	if(hCursor!=NULL)
	{	m_hCursor = (HCURSOR)CopyImage(hCursor,IMAGE_CURSOR,0,0,0);
		if(m_hCursor==NULL) return false;
		m_phCursorRef = &m_hCursor;
	}
	return true;
}
// 
void TabCtrl::SetCursorRef(HCURSOR *phCursor)
{	if(m_hCursor!=NULL)
	{	::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
	m_phCursorRef = phCursor;
}
// 
HCURSOR TabCtrl::GetCursor() const
{	return (m_phCursorRef!=NULL ? *m_phCursorRef : NULL);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::ShowBorder(bool show)
{	m_bShowBorder = show;
}
// 
bool TabCtrl::IsShowBorder() const
{	return m_bShowBorder;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetTabText(HTAB hTab, TCHAR const *text) throw(std::bad_alloc)
{	_ASSERTE(IsExist(hTab)==true);
	_ASSERTE(text!=NULL);
		// 
	CSTRING_BAD_ALLOC( HNDLtoPTR(hTab)->text = text );
}
// 
CString TabCtrl::GetTabText(HTAB hTab) const
{	_ASSERTE(IsExist(hTab)==true);
		// 
	return HNDLtoPTR(hTab)->text;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetTabImage(HTAB hTab, int image)
{	_ASSERTE(IsExist(hTab)==true);
	_ASSERTE(image>=-1);
	HNDLtoPTR(hTab)->image = image;
}
// 
int TabCtrl::GetTabImage(HTAB hTab) const
{	_ASSERTE(IsExist(hTab)==true);
	return HNDLtoPTR(hTab)->image;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetTabWnd(HTAB hTab, HWND hWnd)
{	_ASSERTE(IsExist(hTab)==true);
	_ASSERTE(hWnd!=NULL && ::IsWindow(hWnd)!=0 && ::GetParent(hWnd)==m_hWnd);
	_ASSERTE(::GetDlgCtrlID(hWnd)!=0);
	_ASSERTE(GetTabWithWindow(hWnd,hTab)==NULL);	// window with this ID has inserted.
		// 
	HNDLtoPTR(hTab)->hWnd = hWnd;
}
// 
HWND TabCtrl::GetTabWnd(HTAB hTab) const
{	_ASSERTE(IsExist(hTab)==true);
	return HNDLtoPTR(hTab)->hWnd;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetTabData(HTAB hTab, __int64 data)
{	_ASSERTE(IsExist(hTab)==true);
	HNDLtoPTR(hTab)->data = data;
}
// 
__int64 TabCtrl::GetTabData(HTAB hTab) const
{	_ASSERTE(IsExist(hTab)==true);
	return HNDLtoPTR(hTab)->data;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// copy: text, image, data, tooltip text and disable/enable state.
// 
void TabCtrl::CopyTab(HTAB hDst, TabCtrl const *pTabCtrlSrc, HTAB hSrc) throw(std::bad_alloc)
{	_ASSERTE(IsExist(hDst)==true);
	_ASSERTE(pTabCtrlSrc!=NULL && pTabCtrlSrc->IsExist(hSrc)==true);
		// 
	CString text, tooltip_text;
	CSTRING_BAD_ALLOC( text=pTabCtrlSrc->GetTabText(hSrc) );
	CSTRING_BAD_ALLOC( tooltip_text=pTabCtrlSrc->GetTabTooltipText(hSrc) );
		// 
	SetTabText(hDst,text);
	SetTabTooltipText(hDst,tooltip_text);
		// 
	SetTabImage(hDst,pTabCtrlSrc->GetTabImage(hSrc));
	SetTabData(hDst,pTabCtrlSrc->GetTabData(hSrc));
	Disable(hDst,pTabCtrlSrc->IsDisable(hSrc));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetTabTooltipText(HTAB hTab, TCHAR const *text) throw(std::bad_alloc)
{	_ASSERTE(IsExist(hTab)==true);
	_ASSERTE(text!=NULL);
		// 
	CSTRING_BAD_ALLOC( HNDLtoPTR(hTab)->tooltipText = text );
}
// 
CString TabCtrl::GetTabTooltipText(HTAB hTab) const
{	_ASSERTE(IsExist(hTab)==true);
		// 
	return HNDLtoPTR(hTab)->tooltipText;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetCloseButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	CSTRING_BAD_ALLOC( m_sToolTipButClose=text );
}
// 
CString TabCtrl::GetCloseButToolTipText() const
{	return m_sToolTipButClose;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetMenuButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	CSTRING_BAD_ALLOC( m_sToolTipButMenu=text );
}
// 
CString TabCtrl::GetMenuButToolTipText() const
{	return m_sToolTipButMenu;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetLeftScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	CSTRING_BAD_ALLOC( m_sToolTipButLeftScroll=text );
}
// 
CString TabCtrl::GetLeftScrollButToolTipText() const
{	return m_sToolTipButLeftScroll;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetRightScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc)
{	CSTRING_BAD_ALLOC( m_sToolTipButRightScroll=text );
}
// 
CString TabCtrl::GetRightScrollButToolTipText() const
{	return m_sToolTipButRightScroll;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::ToolTipEnable(bool enable)
{	m_bToolTipEnable = enable;
}
// 
bool TabCtrl::IsToolTipEnable() const
{	return m_bToolTipEnable;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::Update(bool redraw)
{	Recalc(redraw);
	if(redraw==true) Invalidate(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::InstallStyle(ITabCtrlStyle *style)
{	_ASSERTE(style!=NULL);
		// 
	SetDrawManager( style->GetDrawManager() );
	SetRecalcManager( style->GetRecalcManager() );
	SetBehaviorManager( style->GetBehaviorManager() );
	SetToolTipManager( style->GetToolTipManager() );
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnSize(UINT nType, int cx, int cy)
{	CWnd::OnSize(nType, cx, cy);
		// 
	Recalc(true);
	Invalidate(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// Recalculate control.
// 
void TabCtrl::Recalc(bool redraw)
{	GetClientRect(&m_rcWindows);
		// 
	if(m_bShowBorder==true)
	{	int width = GetBorderWidth();
		m_rcWindows.DeflateRect(width,width);
	}
		// 
	int count = GetCount();
	bool bHideSingleTab = (count==1 && IsHideSingleTab()==true);
		// 
	if(count>0 && bHideSingleTab==false)
	{	m_rcCtrlArea = m_rcWindows;
			// 
		if(m_Layout==TAB_LAYOUT_TOP)
		{	m_rcWindows.top = m_rcCtrlArea.bottom = m_rcCtrlArea.top + CalcCtrlAreaHeight();
			m_rcWindows.bottom = max(m_rcWindows.bottom,m_rcWindows.top);
		}
		else
		{	m_rcWindows.bottom = m_rcCtrlArea.top = m_rcCtrlArea.bottom - CalcCtrlAreaHeight();
			m_rcWindows.top = min(m_rcWindows.top,m_rcWindows.bottom);
		}
			// 
			// 
		if(m_hCurTab!=NULL && IsExist(m_hCurTab)==false) m_hCurTab=NULL;
			// 
		HTAB hOldCurTab = m_hCurTab;
			// 
		if(m_hCurTab==NULL)
			m_hCurTab = GetFirstEnableTab();
		else
			if(HNDLtoPTR(m_hCurTab)->disable==true)
			{	HTAB hCurTab = GetNextEnableTab(m_hCurTab);
				if(hCurTab==NULL) hCurTab = GetPrevEnableTab(m_hCurTab);
				m_hCurTab = hCurTab;
			}
			// 
		if(m_hCurTab!=NULL)
		{	CRect rcWindows(m_rcWindows);
			rcWindows.DeflateRect( GetWndsAreaPadding() );
			Tools::MoveChangedWindow(HNDLtoPTR(m_hCurTab)->hWnd,&rcWindows,redraw);
			if(::IsWindowVisible(HNDLtoPTR(m_hCurTab)->hWnd)==0)
				Tools::ShowWindow(HNDLtoPTR(m_hCurTab)->hWnd,true);
		}
		if(hOldCurTab!=m_hCurTab)
			if(hOldCurTab!=NULL) Tools::ShowWindow(HNDLtoPTR(hOldCurTab)->hWnd,false);
			// 
			// 
		CRect rcCtrlAreaPadding = GetCtrlAreaPadding();
		m_rcTabs = m_rcCtrlArea;
		m_rcTabs.DeflateRect(&rcCtrlAreaPadding);
			// 
		CalcTabsWidth();
			// 
			// 
		bool bShowCloseButton = (m_bShowCloseButton==true && 
			m_pSysImageListRef!=NULL && m_pSysImageListRef->m_hImageList!=NULL &&
			m_pAbilityManager->CanShowCloseButton(this)==true);
		bool bShowMenuButton = (m_bShowMenuButton==true && 
			m_pSysImageListRef!=NULL && m_pSysImageListRef->m_hImageList!=NULL &&
			m_pAbilityManager->CanShowMenuButton(this)==true);
		bool bShowScrollButtons = (m_Behavior==TAB_BEHAVIOR_SCROLL && m_bShowScrollButtons==true && 
			m_pSysImageListRef!=NULL && m_pSysImageListRef->m_hImageList!=NULL &&
			m_pAbilityManager->CanShowScrollButtons(this)==true);
			// 
		int iSysImagePosY = (m_rcTabs.top + m_rcTabs.bottom - m_szSysImage.cy) / 2;
			// 
		if(bShowCloseButton==true)
		{	CRect rcCloseHorzMargin = GetCloseButtonHorzMargin();
				// 
			m_rcCloseButton = m_rcTabs;
			m_rcCloseButton.right -= rcCloseHorzMargin.right;
			m_rcCloseButton.left = m_rcCloseButton.right - m_szSysImage.cx;
			m_rcTabs.right = m_rcCloseButton.left - rcCloseHorzMargin.left;
				// 
			m_rcCloseButton.top = iSysImagePosY;
			m_rcCloseButton.bottom = m_rcCloseButton.top + m_szSysImage.cy;
		}
		else
			m_rcCloseButton.SetRectEmpty();
			// 
		if(bShowMenuButton==true)
		{	CRect rcMenuHorzMargin = GetMenuButtonHorzMargin();
				// 
			m_rcMenuButton = m_rcTabs;
			m_rcMenuButton.right -= rcMenuHorzMargin.right;
			m_rcMenuButton.left = m_rcMenuButton.right - m_szSysImage.cx;
			m_rcTabs.right = m_rcMenuButton.left - rcMenuHorzMargin.left;
				// 
			m_rcMenuButton.top = iSysImagePosY;
			m_rcMenuButton.bottom = m_rcMenuButton.top + m_szSysImage.cy;
		}
		else
			m_rcMenuButton.SetRectEmpty();
			// 
		if(bShowScrollButtons==true)
		{	CRect rcLeftScrollMargin = GetLeftScrollButtonHorzMargin();
			CRect rcRightScrollMargin = GetRightScrollButtonHorzMargin();
				// 
			m_rcScrollRight = m_rcTabs;
			m_rcScrollRight.right -= rcRightScrollMargin.right;
			m_rcScrollRight.left = m_rcScrollRight.right - m_szSysImage.cx;
			m_rcScrollLeft.right = m_rcScrollRight.left - rcRightScrollMargin.left - rcLeftScrollMargin.right;
			m_rcScrollLeft.left = m_rcScrollLeft.right - m_szSysImage.cx;
			m_rcTabs.right = m_rcScrollLeft.left - rcLeftScrollMargin.left;
				// 
			m_rcScrollLeft.top = m_rcScrollRight.top = iSysImagePosY;
			m_rcScrollLeft.bottom = m_rcScrollRight.bottom = m_rcScrollLeft.top + m_szSysImage.cy;
		}
		else
		{	m_rcScrollLeft.SetRectEmpty();
			m_rcScrollRight.SetRectEmpty();
		}
			// 
			// 
		int iVisibleTabsWidth = max(0,m_rcTabs.Width()-1);
		int iFullTabsWidth = GetFullTabsWidth();
			// 
		m_iMaxTabsOffset = max(0, iFullTabsWidth-iVisibleTabsWidth);
			// 
		if(m_iTabsOffset<0) m_iTabsOffset = 0;
		if(m_iTabsOffset>m_iMaxTabsOffset) m_iTabsOffset = m_iMaxTabsOffset;
			// 
		m_bPartialView = (m_iMaxTabsOffset>0);
		m_bScrollLeftAllow = (m_bPartialView==true && m_iTabsOffset>0);
		m_bScrollRightAllow = (m_bPartialView==true && m_iTabsOffset<m_iMaxTabsOffset);
			// 
		if(m_Behavior==TAB_BEHAVIOR_SCALE)
			RecalcScale(iVisibleTabsWidth,iFullTabsWidth);
		else
			RecalcScroll();
	}
	else
	{	if(bHideSingleTab==true)
		{	Tab *tab = m_vtrtabs.front();
			tab->rc.SetRectEmpty();
			tab->width = 0;
				// 
			if(m_hCurTab!=NULL && HNDLtoPTR(m_hCurTab)!=tab)
				if(::IsWindow(HNDLtoPTR(m_hCurTab)->hWnd)!=0)
					::ShowWindow(HNDLtoPTR(m_hCurTab)->hWnd,SW_HIDE);
				// 
			m_hCurTab = (tab->disable==false ? tab : NULL);
				// 
			if(m_hCurTab!=NULL)
			{	CRect rcWindows(m_rcWindows);
				rcWindows.DeflateRect( GetWndsAreaPadding() );
				Tools::MoveChangedWindow(HNDLtoPTR(m_hCurTab)->hWnd,&rcWindows,redraw);
				if(::IsWindowVisible(HNDLtoPTR(m_hCurTab)->hWnd)==0)
					Tools::ShowWindow(HNDLtoPTR(m_hCurTab)->hWnd,true);
			}
			else
				if(::IsWindow(tab->hWnd)!=0)
					::ShowWindow(tab->hWnd,SW_HIDE);
		}
			// 
		m_rcCtrlArea.SetRectEmpty();
		m_rcTabs.SetRectEmpty();
		m_rcCloseButton.SetRectEmpty();
		m_rcMenuButton.SetRectEmpty();
		m_rcScrollLeft.SetRectEmpty();
		m_rcScrollRight.SetRectEmpty();
	}
		// 
		// 
	if(m_pToolTip!=NULL && m_pToolTip->m_hWnd!=NULL)
		m_pToolTip->SetToolRect(this,1,&m_rcCtrlArea);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::CalcTabsWidth()
{	int iMinTabWidth = GetTabMinWidth();
		// 
	if(m_bEqualTabsSize==false)
		for(m_i_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
			(*i)->width = max(iMinTabWidth,CalcTabWidth(*i));
	else
	{	int maxWidth = 0;
		m_i_vtrtabs i, e=m_vtrtabs.end();
			// 
		for(i=m_vtrtabs.begin(); i!=e; ++i)
			maxWidth = max(maxWidth,CalcTabWidth(*i));
		maxWidth = max(maxWidth,iMinTabWidth);
			// 
		for(i=m_vtrtabs.begin(); i!=e; ++i)
			(*i)->width = maxWidth;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::GetFullTabsWidth()
{	int width = 0;
		// 
	for(m_i_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		width += (*i)->width;
		// 
	CRect rcTabHorzMargin = GetTabHorzMargin();
	return width + (int)m_vtrtabs.size()*(rcTabHorzMargin.left+rcTabHorzMargin.right);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::RecalcScale(int iVisibleTabsWidth, int iFullTabsWidth)
{	CRect rcTabHorzMargin = GetTabHorzMargin();
	bool bPartialView = (iFullTabsWidth > iVisibleTabsWidth);
		// 
	int pos = m_rcTabs.left;
	m_i_vtrtabs i, e=m_vtrtabs.end();
		// 
	if(bPartialView==false)
		for(i=m_vtrtabs.begin(); i!=e; ++i)
		{	(*i)->rc = m_rcTabs;
			(*i)->rc.left = pos;
			(*i)->rc.right = pos += (rcTabHorzMargin.left + (*i)->width + rcTabHorzMargin.right);
			(*i)->rc.DeflateRect(rcTabHorzMargin.left,0,rcTabHorzMargin.right,0);
		}
	else
	{	int iMinTabWidth = GetTabMinWidth();
		int totalTabsIndent = (int)m_vtrtabs.size() * (rcTabHorzMargin.left + rcTabHorzMargin.right);
		int iEqualWidth = max(1, (iVisibleTabsWidth-totalTabsIndent) / (int)m_vtrtabs.size());
			// 
		if(m_bEqualTabsSize==true)
			for(i=m_vtrtabs.begin(); i!=e; ++i)
			{	(*i)->rc = m_rcTabs;
				(*i)->rc.left = pos;
				(*i)->rc.right = pos += (rcTabHorzMargin.left + max(iMinTabWidth,iEqualWidth) + rcTabHorzMargin.right);
				(*i)->rc.DeflateRect(rcTabHorzMargin.left,0,rcTabHorzMargin.right,0);
			}
		else
		{	int iTotalCorrectWidth = 0;
			for(i=m_vtrtabs.begin(); i!=e; ++i)
				if((*i)->width>iEqualWidth) iTotalCorrectWidth += (*i)->width-iEqualWidth;
				// 
			int iTail = iFullTabsWidth - iVisibleTabsWidth;
			int width;
				// 
			for(i=m_vtrtabs.begin(); i!=e; ++i)
			{	if(i!=e-1)
				{	if((*i)->width<=iEqualWidth)
						width = (*i)->width;
					else
						width = max(iMinTabWidth, (*i)->width - (int)((double)iTail * ((double)((*i)->width-iEqualWidth) / (double)iTotalCorrectWidth) + 0.5) );
				}
				else
					width = max(iMinTabWidth, (m_rcTabs.right-1) - pos - (rcTabHorzMargin.left + rcTabHorzMargin.right));
					// 
				(*i)->rc = m_rcTabs;
				(*i)->rc.left = pos;
				(*i)->rc.right = pos += (rcTabHorzMargin.left + width + rcTabHorzMargin.right);
				(*i)->rc.DeflateRect(rcTabHorzMargin.left,0,rcTabHorzMargin.right,0);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::RecalcScroll()
{	CRect rcTabHorzMargin = GetTabHorzMargin();
		// 
	int pos = m_rcTabs.left - m_iTabsOffset;
		// 
	for(m_i_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
	{	(*i)->rc = m_rcTabs;
		(*i)->rc.left = pos;
		(*i)->rc.right = pos += (rcTabHorzMargin.left + (*i)->width + rcTabHorzMargin.right);
		(*i)->rc.DeflateRect(rcTabHorzMargin.left,0,rcTabHorzMargin.right,0);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::CalcCtrlAreaHeight()
{	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject( GetFont() );
	int iTextHeight = dc.GetTextExtent(_T("H"),1).cy;
	dc.SelectObject( GetFontSelect() );
	iTextHeight = max(iTextHeight, dc.GetTextExtent(_T("H"),1).cy);
	dc.SelectObject(pOldFont);
		// 
	CRect rcCtrlAreaPadding = GetCtrlAreaPadding();
	CRect rcTabPadding = GetTabPadding();
		// 
	return rcCtrlAreaPadding.top + 
		max( rcTabPadding.top + max(max(m_szImage.cy,m_szImageDisabled.cy),iTextHeight) + rcTabPadding.bottom, m_szSysImage.cy) +
		rcCtrlAreaPadding.bottom;
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::CalcTabWidth(HTAB hTab)
{	_ASSERT(IsExist(hTab)==true);
		// 
	int imageWidth = 0;
		// 
	if(HNDLtoPTR(hTab)->image!=-1 &&
		((HNDLtoPTR(hTab)->disable==false && m_pImageListRef!=NULL && m_pImageListRef->m_hImageList!=NULL) || 
		(HNDLtoPTR(hTab)->disable==true && m_pImageListDisabledRef!=NULL && m_pImageListDisabledRef->m_hImageList!=NULL)))
			imageWidth = (HNDLtoPTR(hTab)->disable==false ? m_szImage.cx : m_szImageDisabled.cx) + GetTabImageTextGap();
		// 
	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject( GetFont() );
	int textWidth = dc.GetTextExtent(HNDLtoPTR(hTab)->text).cx;
	dc.SelectObject( GetFontSelect() );
	textWidth = max(textWidth, dc.GetTextExtent(HNDLtoPTR(hTab)->text).cx);
	dc.SelectObject(pOldFont);
		// 
	CRect rcTabPadding = GetTabPadding();
	return rcTabPadding.left + imageWidth + textWidth + GetTabExtraWidth(hTab) + rcTabPadding.right;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL TabCtrl::PreTranslateMessage(MSG *pMsg)
{	if(m_bToolTipEnable==true)
		if(m_pToolTip!=NULL && m_pToolTip->m_hWnd!=NULL)
			m_pToolTip->RelayEvent(pMsg);
	return CWnd::PreTranslateMessage(pMsg);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL TabCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{	NMTTDISPINFO *info = (NMTTDISPINFO *)lParam;
		// 
	if(info->hdr.code==TTN_NEEDTEXT)
		TtnNeedText(info);
	else
		*pResult = GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::TtnNeedText(NMTTDISPINFO *pTT)
{	m_sToolTip = _T("");
		// 
	if(m_hHoverArea!=NULL)
		if(m_hHoverArea==m_hButClose) m_sToolTip = m_sToolTipButClose;
		else if(m_hHoverArea==m_hButMenu) m_sToolTip = m_sToolTipButMenu;
		else if(m_hHoverArea==m_hButLeftScroll) m_sToolTip = m_sToolTipButLeftScroll;
		else if(m_hHoverArea==m_hButRightScroll) m_sToolTip = m_sToolTipButRightScroll;
		else
		{	Tab *tab = HNDLtoPTR(m_hHoverArea);
				// 
			CRect rc(tab->rc);
			rc.left = max(rc.left,m_rcTabs.left);
			rc.right = min(rc.right,m_rcTabs.right);
				// 
			if(rc.IsRectEmpty()==FALSE)
				if(tab->tooltipText.IsEmpty()==FALSE)
					m_sToolTip = tab->tooltipText;
				else 
					if(rc.Width() < tab->width)	// partial view of tab.
						m_sToolTip = tab->text;
		}
	pTT->lpszText = (TCHAR *)(TCHAR const *)m_sToolTip;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnPaint()
{	CPaintDC dc(this);
		// 
	if(m_pDrawManager==NULL) return;
		// 
	VirtualWindow virtwnd(&dc,this);
		// 
	m_pDrawManager->DrawBegin(this,&virtwnd);
		// 
	if(m_vtrtabs.empty()==false)
	{	m_pDrawManager->DrawCtrlAreaBack(this,&virtwnd,&m_rcCtrlArea);
			// 
		CFont *pOldFont = (CFont *)virtwnd.SelectObject( GetFont() );
			// 
		CRgn rgn;
		rgn.CreateRectRgn(m_rcTabs.left,m_rcTabs.top,max(m_rcTabs.left,m_rcTabs.right),m_rcTabs.bottom);
		virtwnd.SelectClipRgn(&rgn,RGN_COPY);
			// 
		if(m_pDrawManager->IsStraightOrder(this)==true)	// left to right.
		{	for(m_i_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
				if(*i!=m_hCurTab && IsTabVisible(*i,NULL)==true)
					m_pDrawManager->DrawTab(this,&virtwnd,*i,&rgn);
		}
		else	// right to left.
			for(m_ri_vtrtabs i=m_vtrtabs.rbegin(), e=m_vtrtabs.rend(); i!=e; ++i)
				if(*i!=m_hCurTab && IsTabVisible(*i,NULL)==true)
					m_pDrawManager->DrawTab(this,&virtwnd,*i,&rgn);
			// 
		if(m_hCurTab!=NULL)
			if(m_bWatchActivityCtrl==false || m_bActive==true)
			{	CFont *oldFont = (CFont *)virtwnd.SelectObject( GetFontSelect() );
				m_pDrawManager->DrawTab(this,&virtwnd,m_hCurTab,&rgn);
				virtwnd.SelectObject(oldFont);
			}
			else
				m_pDrawManager->DrawTab(this,&virtwnd,m_hCurTab,&rgn);
			// 
		virtwnd.SelectClipRgn(NULL,RGN_COPY);
		virtwnd.SelectObject(pOldFont);
			// 
		if(m_rcScrollLeft.IsRectEmpty()==0)
		{	bool bHover = m_hHoverArea==m_hButLeftScroll && (m_hPushedArea==NULL || m_hPushedArea==m_hButLeftScroll);
			m_pDrawManager->DrawButtonScrollLeft(this,&virtwnd,&m_rcScrollLeft,bHover,m_hPushedArea==m_hButLeftScroll,m_bScrollLeftAllow);
			bHover = m_hHoverArea==m_hButRightScroll && (m_hPushedArea==NULL || m_hPushedArea==m_hButRightScroll);
			m_pDrawManager->DrawButtonScrollRight(this,&virtwnd,&m_rcScrollRight,bHover,m_hPushedArea==m_hButRightScroll,m_bScrollRightAllow);
		}
		if(m_rcMenuButton.IsRectEmpty()==0)
		{	bool bHover = m_hHoverArea==m_hButMenu && (m_hPushedArea==NULL || m_hPushedArea==m_hButMenu);
			m_pDrawManager->DrawButtonMenu(this,&virtwnd,&m_rcMenuButton,bHover,m_hPushedArea==m_hButMenu,m_bPartialView);
		}
		if(m_rcCloseButton.IsRectEmpty()==0)
		{	bool bHover = m_hHoverArea==m_hButClose && (m_hPushedArea==NULL || m_hPushedArea==m_hButClose);
			m_pDrawManager->DrawButtonClose(this,&virtwnd,&m_rcCloseButton,bHover,m_hPushedArea==m_hButClose);
		}
	}
		// 
	CRect rcWndsAreaPadding = GetWndsAreaPadding();
	if(m_vtrtabs.empty()==true || m_hCurTab==NULL || rcWndsAreaPadding.IsRectNull()==0)
		m_pDrawManager->DrawWndsAreaBack(this,&virtwnd,&m_rcWindows);
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
// 
void TabCtrl::EqualTabsSize(bool equal)
{	m_bEqualTabsSize = equal;
}
// 
bool TabCtrl::IsEqualTabsSize() const
{	return m_bEqualTabsSize;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::RemoveTabEnable(bool enable)
{	m_bRemoveTabEnable = enable;
}
// 
bool TabCtrl::IsRemoveTabEnable() const
{	return m_bRemoveTabEnable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::HideSingleTab(bool hide)
{	m_bHideSingleTab = hide;
}
// 
bool TabCtrl::IsHideSingleTab() const
{	return m_bHideSingleTab;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::ShowScrollButtons(bool show)
{	m_bShowScrollButtons = show;
}
// 
bool TabCtrl::IsShowScrollButtons() const
{	return m_bShowScrollButtons;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::ShowMenuButton(bool show)
{	m_bShowMenuButton = show;
}
// 
bool TabCtrl::IsShowMenuButton() const
{	return m_bShowMenuButton;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::ShowCloseButton(bool show)
{	m_bShowCloseButton = show;
}
// 
bool TabCtrl::IsShowCloseButton() const
{	return m_bShowCloseButton;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::SetFont(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	LOGFONT logfont;
	pFont->GetLogFont(&logfont);
	return SetFont(&logfont);
}
// 
bool TabCtrl::SetFont(LOGFONT const *pLf)
{	_ASSERTE(pLf!=NULL);
		// 
	if(m_Font.m_hObject!=NULL) m_Font.DeleteObject();
	m_pFontRef = NULL;
		// 
	if(m_Font.CreateFontIndirect(pLf)==0) return false;
	m_pFontRef = &m_Font;
	return true;
}
// 
void TabCtrl::SetFontRef(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	if(m_Font.m_hObject!=NULL) m_Font.DeleteObject();
	m_pFontRef = pFont;
}
// 
CFont *TabCtrl::GetFont()
{	return m_pFontRef;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::SetFontSelect(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	LOGFONT logfont;
	pFont->GetLogFont(&logfont);
	return SetFontSelect(&logfont);
}
// 
bool TabCtrl::SetFontSelect(LOGFONT const *pLf)
{	_ASSERTE(pLf!=NULL);
		// 
	if(m_FontSelect.m_hObject!=NULL) m_FontSelect.DeleteObject();
	m_pFontSelectRef = NULL;
		// 
	if(m_FontSelect.CreateFontIndirect(pLf)==0) return false;
	m_pFontSelectRef = &m_FontSelect;
	return true;
}
// 
void TabCtrl::SetFontSelectRef(CFont *pFont)
{	_ASSERTE(pFont!=NULL && pFont->m_hObject!=NULL);
		// 
	if(m_FontSelect.m_hObject!=NULL) m_FontSelect.DeleteObject();
	m_pFontSelectRef = pFont;
}
// 
CFont *TabCtrl::GetFontSelect()
{	return m_pFontSelectRef;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::GetCount() const
{	return (int)m_vtrtabs.size();
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::GetTab(int idx) const
{	_ASSERTE(idx>=0 && idx<GetCount());
	return m_vtrtabs[idx];
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::GetFirstEnableTab() const
{	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		if((*i)->disable==false) return *i;
	return NULL;
}
// 
HTAB TabCtrl::GetPrevEnableTab(HTAB hTab) const
{	for(m_ci_vtrtabs i=m_vtrtabs.begin()+GetIndex(hTab); i!=m_vtrtabs.begin(); )
		if((*--i)->disable==false) return *i;
	return NULL;
}
// 
HTAB TabCtrl::GetNextEnableTab(HTAB hTab) const
{	for(m_ci_vtrtabs i=m_vtrtabs.begin()+GetIndex(hTab)+1, e=m_vtrtabs.end(); i!=e; ++i)
		if((*i)->disable==false) return *i;
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetSel(HTAB hTab)
{	_ASSERTE(IsExist(hTab)==true);
		// 
	if(m_hCurTab==hTab || HNDLtoPTR(hTab)->disable==true) return;
		// 
	CRect rcWindows(m_rcWindows);
	rcWindows.DeflateRect( GetWndsAreaPadding() );
	Tools::MoveChangedWindow(HNDLtoPTR(hTab)->hWnd,&rcWindows,false);
	if(m_hCurTab!=NULL) Tools::ShowWindow(HNDLtoPTR(m_hCurTab)->hWnd,false);
	// 
	m_hCurTab = hTab;
	Tools::ShowWindow(HNDLtoPTR(m_hCurTab)->hWnd,true);
}
// 
HTAB TabCtrl::GetSel() const
{	return m_hCurTab;
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::GetHover() const
{	return (m_hHoverArea!=NULL && IsSystemButton(m_hHoverArea)==false ? m_hHoverArea : NULL);
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::GetPushed() const
{	return (m_hPushedArea!=NULL && IsSystemButton(m_hPushedArea)==false ? m_hPushedArea : NULL);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::Disable(HTAB hTab, bool disable) const
{	_ASSERTE(IsExist(hTab)==true);
	HNDLtoPTR(hTab)->disable = disable;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::IsDisable(HTAB hTab) const
{	_ASSERTE(IsExist(hTab)==true);
	return HNDLtoPTR(hTab)->disable;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::EnsureVisible(HTAB hTab)
{	if(m_Behavior==TAB_BEHAVIOR_SCROLL)
	{	Tab *i = HNDLtoPTR(hTab);
			// 
		if(i->rc.left<m_rcTabs.left)
			m_iTabsOffset -= m_rcTabs.left-i->rc.left;
		else if(i->rc.right>m_rcTabs.right)
			m_iTabsOffset += i->rc.right-m_rcTabs.right;
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::ScrollToBegin()
{	if(m_Behavior==TAB_BEHAVIOR_SCROLL)
		m_iTabsOffset = 0;
}
// 
void TabCtrl::ScrollToEnd()
{	if(m_Behavior==TAB_BEHAVIOR_SCROLL)
		m_iTabsOffset = m_iMaxTabsOffset;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetScrollingStep(int step)
{	_ASSERTE(step>=1);
	m_iScrollingStep = step;
}
// 
int TabCtrl::GetScrollingStep() const
{	return m_iScrollingStep;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::HitTest(CPoint point) const
{	return m_pBehaviorManager->HitTest((TabCtrl *)this,(ITabCtrlBehavior *)&m_Service,point);
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::GetIndex(HTAB hTab) const
{	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		if(*i==HNDLtoPTR(hTab)) return int(i-m_vtrtabs.begin());
	return -1;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::IsExist(HTAB hTab) const
{	return GetIndex(hTab)!=-1;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::GetTabRect(HTAB hTab, RECT *rc/*out*/) const
{	_ASSERTE(IsExist(hTab)==true);
	*rc = HNDLtoPTR(hTab)->rc;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::IsTabVisible(HTAB hTab, bool *partially/*out*/) const
{	_ASSERTE(IsExist(hTab)==true);
		// 
	CRect const &rc = HNDLtoPTR(hTab)->rc;
		// 
	if(rc.right<=m_rcTabs.left || rc.left>=m_rcTabs.right)
	{	if(partially!=NULL) *partially = false;
		return false;
	}
	else
	{	if(partially!=NULL)
			*partially = (rc.left<m_rcTabs.left || rc.right>m_rcTabs.right);
		return true;
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//// 
//HTAB TabCtrl::GetTabWithWindowID(int id) const
//{	return GetTabWithWindowID(id,NULL);
//}
//// 
//HTAB TabCtrl::GetTabWithWindowID(int id, HTAB exceptTab) const
//{	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
//		if(*i!=exceptTab && ::GetDlgCtrlID((*i)->hWnd)==id) return *i;
//	return NULL;
//}
//////////////////////////////////////////////////////////////////////////
HTAB TabCtrl::GetTabWithWindow(HWND hWnd) const
{	return GetTabWithWindow(hWnd, NULL);
}

HTAB TabCtrl::GetTabWithWindow(HWND hWnd, HTAB exceptTab) const
{	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		if(*i!=exceptTab && (*i)->hWnd==hWnd) return *i;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::GetTabAndIndex(int id, HTAB *tab/*out*/, int *idx/*out*/) const
{	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		if(::GetDlgCtrlID((*i)->hWnd)==id)
		{	if(tab!=NULL) *tab = *i;
			if(idx!=NULL) *idx = int(i-m_vtrtabs.begin());
			return true;
		}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::CmpTabs(HTAB hTab1, HTAB hTab2) const
{	_ASSERTE(IsExist(hTab1)==true && IsExist(hTab2)==true);
		// 
	if(hTab1==hTab2) return 0;
		// 
	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		if(*i==hTab1) return -1;
		else if(*i==hTab2) return 1;
	return -1;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::Service::GetBorderWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 1;
}
// 
CRect TabCtrl::Service::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(3,1,3,2/*indent*/) : CRect(3,2/*indent*/,3,1));
}
// 
CRect TabCtrl::Service::GetWndsAreaPadding(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,0,0,0);
}
// 
CRect TabCtrl::Service::GetTabHorzMargin(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,0,0,0);
}
// 
CRect TabCtrl::Service::GetTabPadding(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(5,2+1/*border*/,5,2+1/*border*/);
}
// 
int TabCtrl::Service::GetTabImageTextGap(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 3;
}
// 
int TabCtrl::Service::GetTabExtraWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/, HTAB /*hTab*/)
{	return 0;
}
// 
int TabCtrl::Service::GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	CRect rcTabPadding = pCtrl->GetTabPadding();
	CSize szImage, szImageDisabled;
	pCtrl->GetImageSizes(&szImage,&szImageDisabled);
	return rcTabPadding.left + max(szImage.cx,szImageDisabled.cx) + rcTabPadding.right;
}
// 
CRect TabCtrl::Service::GetCloseButtonHorzMargin(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(2,0,2,0);
}
// 
CRect TabCtrl::Service::GetMenuButtonHorzMargin(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(8,0,0,0);
}
// 
CRect TabCtrl::Service::GetLeftScrollButtonHorzMargin(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(8,0,0,0);
}
// 
CRect TabCtrl::Service::GetRightScrollButtonHorzMargin(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,0,0,0);
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrl::Service::HitTest(TabCtrl *pCtrl, ITabCtrlBehavior * /*base*/, CPoint point)	// get tab in the given point.
{	if(CRect(pCtrl->GetTabsArea()).PtInRect(point)!=0)
		for(int i=0, c=pCtrl->GetCount(); i<c; ++i)
		{	HTAB hTab = pCtrl->GetTab(i);
			CRect rc;
			pCtrl->GetTabRect(hTab,&rc);
			if(rc.PtInRect(point)!=0) return hTab;
		}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::Service::SetCursor(TabCtrl *pCtrl, ITabCtrlBehavior * /*base*/)
{	HCURSOR hCursor = pCtrl->GetCursor();
		// 
	if(hCursor!=NULL)
	{	CPoint point;
		::GetCursorPos(&point);
		pCtrl->ScreenToClient(&point);
			// 
		if(pCtrl->HitTest(point)!=NULL)
		{	::SetCursor(hCursor);
			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
CToolTipCtrl *TabCtrl::Service::CreateToolTip(TabCtrl *pCtrl, ITabCtrlToolTip * /*base*/)
{	if(m_ToolTipCtrl.Create(pCtrl,TTS_ALWAYSTIP)==0) return NULL;
		// 
	DWORD dwClassStyle = ::GetClassLong(m_ToolTipCtrl.m_hWnd,GCL_STYLE);
	dwClassStyle |= 0x00020000/*CS_DROPSHADOW*/;		// enables the drop shadow effect.
	::SetClassLong(m_ToolTipCtrl.m_hWnd,GCL_STYLE,dwClassStyle);
		// 
	return &m_ToolTipCtrl;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrl::GetBorderWidth()
{	return m_pRecalcManager->GetBorderWidth(this,&m_Service);
}
// 
CRect TabCtrl::GetCtrlAreaPadding()
{	return m_pRecalcManager->GetCtrlAreaPadding(this,&m_Service);
}
// 
CRect TabCtrl::GetWndsAreaPadding()
{	return m_pRecalcManager->GetWndsAreaPadding(this,&m_Service);
}
// 
CRect TabCtrl::GetTabHorzMargin()
{	return m_pRecalcManager->GetTabHorzMargin(this,&m_Service);
}
// 
CRect TabCtrl::GetTabPadding()
{	return m_pRecalcManager->GetTabPadding(this,&m_Service);
}
// 
int TabCtrl::GetTabImageTextGap()
{	return m_pRecalcManager->GetTabImageTextGap(this,&m_Service);
}
// 
int TabCtrl::GetTabExtraWidth(HTAB hTab)
{	return m_pRecalcManager->GetTabExtraWidth(this,&m_Service,hTab);
}
// 
int TabCtrl::GetTabMinWidth()
{	return m_pRecalcManager->GetTabMinWidth(this,&m_Service);
}
// 
CRect TabCtrl::GetCloseButtonHorzMargin()
{	return m_pRecalcManager->GetCloseButtonHorzMargin(this,&m_Service);
}
// 
CRect TabCtrl::GetMenuButtonHorzMargin()
{	return m_pRecalcManager->GetMenuButtonHorzMargin(this,&m_Service);
}
// 
CRect TabCtrl::GetLeftScrollButtonHorzMargin()
{	return m_pRecalcManager->GetLeftScrollButtonHorzMargin(this,&m_Service);
}
// 
CRect TabCtrl::GetRightScrollButtonHorzMargin()
{	return m_pRecalcManager->GetRightScrollButtonHorzMargin(this,&m_Service);
}
/////////////////////////////////////////////////////////////////////////////
// 
CToolTipCtrl *TabCtrl::GetToolTip() const
{	return m_pToolTip;
}
/////////////////////////////////////////////////////////////////////////////
// 
RECT TabCtrl::GetCtrlArea() const
{	return m_rcCtrlArea;
}
// 
RECT TabCtrl::GetTabsArea() const
{	return m_rcTabs;
}
// 
RECT TabCtrl::GetWindowsArea() const
{	return m_rcWindows;
}
/////////////////////////////////////////////////////////////////////////////
// 
RECT TabCtrl::GetCloseButtonRect() const
{	return m_rcCloseButton;
}
// 
RECT TabCtrl::GetMenuButtonRect() const
{	return m_rcMenuButton;
}
// 
RECT TabCtrl::GetScrollLeftRect() const
{	return m_rcScrollLeft;
}
// 
RECT TabCtrl::GetScrollRightRect() const
{	return m_rcScrollRight;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{	CWnd::OnMouseMove(nFlags, point);
		// 
	AssignHoverArea(point);
		// 
	if(IsRemoveTabEnable()==true &&
		m_hPushedArea!=NULL && IsSystemButton(m_hPushedArea)==false)
		if(m_bTabDrag==false)
		{	if(CRect(m_ptStartTabDrag-CSize(3,3),CSize(6,6)).PtInRect(point)==0)
				StartDragging(point);
		}
		else
		{	if(m_rcCtrlArea.PtInRect(point)==0)
			{	if(m_pNotifyManager!=NULL)
				{	CPoint pt(point);
					ClientToScreen(&pt);
					m_pNotifyManager->OnDrag(this,m_hPushedArea,pt,true);
				}
			}
			else
			{	if(m_hHoverArea!=NULL && IsSystemButton(m_hHoverArea)==false && m_hPushedArea!=m_hHoverArea)
				{	if(GetIndex(m_hHoverArea) < GetIndex(m_hPushedArea))
					{	CRect rc(HNDLtoPTR(m_hHoverArea)->rc);
						rc.right = rc.left + HNDLtoPTR(m_hPushedArea)->rc.Width();
							// 
						if(rc.PtInRect(point)!=0)
						{	RemoveBefore(m_hHoverArea,m_hPushedArea);
							Recalc(true);
							AssignHoverArea(point);
							Invalidate(FALSE);
						}
					}
					else
					{	CRect rc(HNDLtoPTR(m_hHoverArea)->rc);
						rc.left = rc.right - HNDLtoPTR(m_hPushedArea)->rc.Width();
							// 
						if(rc.PtInRect(point)!=0)
						{	RemoveAfter(m_hHoverArea,m_hPushedArea);
							Recalc(true);
							AssignHoverArea(point);
							Invalidate(FALSE);
						}
					}
				}
					// 
				if(m_pNotifyManager!=NULL)
				{	CPoint pt(point);
					ClientToScreen(&pt);
					m_pNotifyManager->OnDrag(this,m_hPushedArea,pt,false);
				}
			}
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::AssignHoverArea(CPoint point)
{	HTAB hHoverAreaOld = m_hHoverArea;
	m_hHoverArea = NULL;
		// 
	if(m_hCurTab!=NULL)
		if(m_rcCloseButton.PtInRect(point)!=0)
			m_hHoverArea = m_hButClose;
		else if(m_rcMenuButton.PtInRect(point)!=0)
			m_hHoverArea = m_hButMenu;
		else if(m_rcScrollLeft.PtInRect(point)!=0)
			m_hHoverArea = m_hButLeftScroll;
		else if(m_rcScrollRight.PtInRect(point)!=0)
			m_hHoverArea = m_hButRightScroll;
		// 
	if(m_hHoverArea==NULL)
		m_hHoverArea = HitTest(point);
		// 
	if(hHoverAreaOld!=m_hHoverArea)
	{	::RedrawWindow(m_hWnd,NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);	// !!! no use Invalidate(...) - can be artifactes while tracking tab.
			// 
		if(m_pToolTip!=NULL && m_pToolTip->m_hWnd!=NULL)
			m_pToolTip->Pop();
	}
		// 
	TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT),TME_LEAVE,m_hWnd,0 };
	::TrackMouseEvent(&tme);
}
/////////////////////////////////////////////////////////////////////////////
// 
LRESULT TabCtrl::OnMouseLeave(WPARAM wp, LPARAM lp)
{	if(m_hHoverArea!=NULL)
	{	m_hHoverArea = NULL;
		Invalidate(FALSE);
	}
		// 
	return CWnd::DefWindowProc(WM_MOUSELEAVE,wp,lp);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{	LButtonDown(point);
	SetFocusInChildWnd();
		// 
	CWnd::OnLButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{	LButtonDown(point);
		// 
	CWnd::OnLButtonDblClk(nFlags, point);
		// 
	if(m_hHoverArea!=NULL && IsSystemButton(m_hHoverArea)==false)
		if(m_pNotifyManager!=NULL)
		{	CPoint pt(point);
			ClientToScreen(&pt);
			m_pNotifyManager->OnLButtonDblClk(this,m_hHoverArea,pt);
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::LButtonDown(CPoint point)
{	if(m_hHoverArea!=NULL)
	{	if(IsSystemButton(m_hHoverArea)==false)	// this is tab.
		{	bool bSelect = false;
				// 
			if(IsDisable(m_hHoverArea)==false)
			{	if(IsRemoveTabEnable()==true)
				{	m_ptStartTabDrag = point;
					m_hPushedArea = m_hHoverArea;
					SetCapture();
				}
					// 
				bSelect = (GetSel()!=m_hHoverArea);
			}
				// 
			if(m_pNotifyManager!=NULL)
			{	CPoint pt(point);
				ClientToScreen(&pt);
				m_pNotifyManager->OnLButtonDown(this,m_hHoverArea,pt);
			}
				// 
			if(bSelect==true)
			{	SetSel(m_hHoverArea);
				Recalc(true);
				if(m_pNotifyManager!=NULL) m_pNotifyManager->OnTabSelected(this,m_hHoverArea);
			}
		}
		else
		{	m_hPushedArea = m_hHoverArea;
				// 
			if(m_hPushedArea==m_hButLeftScroll)
			{	StepLeft();
				SetTimer(TIMER_ID_LEFT_SCROLL_CLICK,300,NULL);
				Recalc(true);
			}
			else if(m_hPushedArea==m_hButRightScroll)
			{	StepRight();
				SetTimer(TIMER_ID_RIGHT_SCROLL_CLICK,300,NULL);
				Recalc(true);
			}
				// 
			SetCapture();
		}
	}
		// 
	Invalidate(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{	CWnd::OnLButtonUp(nFlags, point);
		// 
	if(m_hPushedArea!=NULL)
	{	bool close = (m_hPushedArea==m_hButClose && m_rcCloseButton.PtInRect(point)!=0);
		bool menu = (m_hPushedArea==m_hButMenu && m_rcMenuButton.PtInRect(point)!=0);
			// 
		StopScrolling();
		Invalidate(FALSE);
		StopDragging(false);
			// 
		if(m_pNotifyManager!=NULL)
		{	bool alive = true;
				// 
			if(close==true)
			{	CPoint pt(point);
				ClientToScreen(&pt);
					// 
				m_pLifeStatus = &alive;
				m_hPushedArea = m_hButClose;
				m_pNotifyManager->OnCloseButtonClicked(this,&m_rcCloseButton,pt);
					// 
				if(alive==true)
				{	m_hPushedArea = NULL;
					Invalidate(FALSE);
					m_pLifeStatus = NULL;
				}
			}
			else if(menu==true)
			{	CPoint pt(point);
				ClientToScreen(&pt);
					// 
				m_pLifeStatus = &alive;
				m_hPushedArea = m_hButMenu;
				m_pNotifyManager->OnMenuButtonClicked(this,&m_rcMenuButton,pt);
					// 
				if(alive==true)
				{	m_hPushedArea = NULL;
					Invalidate(FALSE);
					m_pLifeStatus = NULL;
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{	SetFocusInChildWnd();
		// 
	if(m_hHoverArea==NULL || IsSystemButton(m_hHoverArea)==false)
		if(m_pNotifyManager!=NULL)
		{	CPoint pt(point);
			ClientToScreen(&pt);
			m_pNotifyManager->OnRButtonDown(this,m_hHoverArea,pt);
		}
		// 
	CWnd::OnRButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{	CWnd::OnRButtonUp(nFlags, point);
		// 
	if(m_hHoverArea==NULL || IsSystemButton(m_hHoverArea)==false)
		if(m_pNotifyManager!=NULL)
		{	CPoint pt(point);
			ClientToScreen(&pt);
			m_pNotifyManager->OnRButtonUp(this,m_hHoverArea,pt);
		}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnMButtonDown(UINT nFlags, CPoint point)
{	SetFocusInChildWnd();
		// 
	CWnd::OnMButtonDown(nFlags, point);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnCaptureChanged(CWnd *pWnd)
{	if(pWnd!=this)
	{	StopScrolling();
		StopDragging(true);
		Invalidate(FALSE);
	}
		// 
	CWnd::OnCaptureChanged(pWnd);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnKeyDown(UINT keyCode, UINT /*msgFlag*/)
{	if(keyCode==VK_ESCAPE)
		if(IsDragging()==true)
			CancelDragging();
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::StopScrolling()
{	if(m_hPushedArea!=NULL && IsSystemButton(m_hPushedArea)==true)
	{	if(m_hPushedArea==m_hButLeftScroll)
		{	KillTimer(TIMER_ID_LEFT_SCROLL_CLICK);
			KillTimer(TIMER_ID_LEFT_SCROLLING);
		}
		else if(m_hPushedArea==m_hButRightScroll)
		{	KillTimer(TIMER_ID_RIGHT_SCROLL_CLICK);
			KillTimer(TIMER_ID_RIGHT_SCROLLING);
		}
			// 
		m_hPushedArea = NULL;
		if(::GetCapture()==m_hWnd) ::ReleaseCapture();
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::StartDragging(CPoint point)
{	m_bTabDrag = true;
		// 
	m_KeyboardHook.Add(this);
		// 
	m_vtrtabs_dragstore.clear();
	m_vtrtabs_dragstore = m_vtrtabs;
		// 
	if(m_pNotifyManager!=NULL)
	{	CPoint pt(point);
		ClientToScreen(&pt);
		m_pNotifyManager->OnStartDrag(this,m_hPushedArea,pt);
	}
}
// 
void TabCtrl::StopDragging(bool cancel)
{	if(m_hPushedArea!=NULL && IsSystemButton(m_hPushedArea)==false)
	{	HTAB hPushedArea = m_hPushedArea;
			// 
		m_hPushedArea = NULL;
		if(::GetCapture()==m_hWnd) ::ReleaseCapture();
			// 
		if(m_bTabDrag==true)
		{	m_bTabDrag = false;
				// 
			if(m_KeyboardHook.IsExist(this)==true)
				m_KeyboardHook.Delete(this);
				// 
			if(cancel==true && m_vtrtabs_dragstore.empty()==false)
			{	m_vtrtabs.swap(m_vtrtabs_dragstore);
				Recalc(true);
			}
				// 
			if(m_pNotifyManager!=NULL)
				m_pNotifyManager->OnFinishDrag(this,hPushedArea,cancel);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::IsDragging() const
{	return m_bTabDrag;
}
// 
void TabCtrl::CancelDragging()
{	StopDragging(true);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnTimer(UINT_PTR nIDEvent)
{	switch(nIDEvent)
	{	case TIMER_ID_LEFT_SCROLL_CLICK:
			KillTimer(TIMER_ID_LEFT_SCROLL_CLICK);
			if(m_hPushedArea==m_hButLeftScroll)
				SetTimer(TIMER_ID_LEFT_SCROLLING,20,NULL);
			break;
		case TIMER_ID_LEFT_SCROLLING:
			if(m_hHoverArea==m_hButLeftScroll)
			{	StepLeft();
				Update();
			}
			break;
			// 
		case TIMER_ID_RIGHT_SCROLL_CLICK:
			KillTimer(TIMER_ID_RIGHT_SCROLL_CLICK);
			if(m_hPushedArea==m_hButRightScroll)
				SetTimer(TIMER_ID_RIGHT_SCROLLING,20,NULL);
			break;
		case TIMER_ID_RIGHT_SCROLLING:
			if(m_hHoverArea==m_hButRightScroll)
			{	StepRight();
				Update();
			}
			break;
	}
		// 
	CWnd::OnTimer(nIDEvent);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::StepLeft()
{	m_iTabsOffset -= m_iScrollingStep;
}
// 
void TabCtrl::StepRight()
{	m_iTabsOffset += m_iScrollingStep;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::OnSetFocus(CWnd* pOldWnd)
{	CWnd::OnSetFocus(pOldWnd);
		// 
	SetFocusInChildWnd();
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrl::SetFocusInChildWnd()
{	HTAB hTab = GetSel();
	if(hTab!=NULL)
	{	HWND hWnd = GetTabWnd(hTab);
		if(hWnd!=NULL && ::IsWindow(hWnd)!=0 && ::GetFocus()!=hWnd)
			::SetFocus(hWnd);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
BOOL TabCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{	if(m_pBehaviorManager->SetCursor((TabCtrl *)this,(ITabCtrlBehavior *)&m_Service)==true)
		return TRUE;
		// 
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::IsSystemButton(HTAB hTab) const
{	return hTab==m_hButClose || hTab==m_hButMenu || hTab==m_hButLeftScroll || hTab==m_hButRightScroll;
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const
{	_ASSERTE(app!=NULL && section!=NULL && entry!=NULL);
		// 
	CMemFile file;
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
// 
bool TabCtrl::LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry)
{	_ASSERTE(app!=NULL && section!=NULL && entry!=NULL);
		//
	bool res = false;
	BYTE *pData = NULL;
	UINT uDataSize;
		// 
	try
	{	if(app->GetProfileBinary(section,entry,&pData,&uDataSize)!=0)
		{	CMemFile file(pData,uDataSize);
			CArchive ar(&file,CArchive::load);
			res = LoadState(&ar);
		}
	}
	catch(CMemoryException* pEx)
	{	pEx->Delete();
	}
	if(pData!=NULL) delete [] pData;
		// 
	return res;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::LoadState(CArchive *ar)
{	try
	{	LoadStateInner(ar);
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
bool TabCtrl::SaveState(CArchive *ar) const
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
	return false;
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrl::LoadStateInner(CArchive *ar)
{	int count;
	*ar >> count;	// count tabs.
		// 
	_ASSERTE(count==GetCount());	// changed count of tabs.
		// 
	int select = -1;
	if(count>1)
		*ar >> select;	// index of selected tab.
		// 
	int id, target;
	HTAB hTab;
		// 
	for(int i=0; i<count; ++i)
	{	*ar >> id;
			// 
		bool found = GetTabAndIndex(id,&hTab,&target);
			// 
		if(found==false)
		{	TRACE("Not found tab with control id=%d (TabCtrl::LoadStateInner)",id);
			return false;
		}
			// 
		if(i!=count-1)
		{	m_vtrtabs.insert(m_vtrtabs.begin()+i,HNDLtoPTR(hTab));
			m_vtrtabs.erase(m_vtrtabs.begin()+(++target));
		}
	}
		// 
	if(select!=-1) SetSel(GetTab(select));
		// 
	return true;
}
// 
void TabCtrl::SaveStateInner(CArchive *ar) const
{	int count = GetCount();
	*ar << count;
		// 
	if(count>1)
	{	HTAB hSelTab = GetSel();
		*ar << (hSelTab!=NULL ? GetIndex(hSelTab) : -1);
	}
		// 
	for(m_ci_vtrtabs i=m_vtrtabs.begin(), e=m_vtrtabs.end(); i!=e; ++i)
		*ar << ::GetDlgCtrlID((*i)->hWnd);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
TabCtrlDraw *TabCtrlStyle_base::GetDrawManager()
{	return this;
}
// 
ITabCtrlRecalc *TabCtrlStyle_base::GetRecalcManager()
{	return NULL;	// use default manager.
}
// 
ITabCtrlBehavior *TabCtrlStyle_base::GetBehaviorManager()
{	return NULL;	// use default manager.
}
// 
ITabCtrlToolTip *TabCtrlStyle_base::GetToolTipManager()
{	return NULL;	// use default manager.
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawBorder(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	COLORREF clr = GetBorderColor(pCtrl);
	Draw::DrawRect(pDC,pRect,clr);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	CRect rc(*pRect);
		// 
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
	{		// draw background for tabs area.
		rc.bottom -= pCtrl->GetCtrlAreaPadding().bottom;
		Draw::FillSolidRect(pDC,&rc,GetCtrlAreaBackColor(pCtrl));
			// draw separator.
		Draw::DrawLine(pDC,pRect->left,rc.bottom-1,pRect->right,rc.bottom-1,GetTabBorderColor(pCtrl));
			// draw background for windows area.
		rc.top = rc.bottom;
		rc.bottom = pRect->bottom;
		if(rc.IsRectEmpty()==0)
			Draw::FillSolidRect(pDC,&rc,GetChildWndBackColor(pCtrl));
	}
	else
	{		// draw background for windows area.
		rc.bottom = rc.top + pCtrl->GetCtrlAreaPadding().top;
		if(rc.IsRectEmpty()==0)
			Draw::FillSolidRect(pDC,&rc,GetChildWndBackColor(pCtrl));
			// draw separator.
		Draw::DrawLine(pDC,pRect->left,rc.bottom,pRect->right,rc.bottom,GetTabBorderColor(pCtrl));
			// draw background for tabs area.
		rc.top = rc.bottom+1;
		rc.bottom = pRect->bottom;
		Draw::FillSolidRect(pDC,&rc,GetCtrlAreaBackColor(pCtrl));
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	if(pCtrl->GetCount()==0 || pCtrl->GetSel()==NULL)
		Draw::FillSolidRect(pDC,pRect, GetEmptyWndsAreaBackColor(pCtrl) );
	else
		Draw::FillSolidRect(pDC,pRect, GetTabSelectedBackColor(pCtrl) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawTab(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRgn *pRgn)
{	CRect rc;
	pCtrl->GetTabRect(hTab,&rc);
		// 
		// draw background.
	DrawTabBack(pCtrl,pDC,hTab,&rc,pRgn);
		// draw image and text;
	DrawTabContext(pCtrl,pDC,hTab,&rc,pRgn);
}
/////////////////////////////////////////////////////////////////////////////
// Draw image and text.
// 
void TabCtrlStyle_base::DrawTabContext(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn)
{	CRect rcTabPadding = pCtrl->GetTabPadding();
		// 
	CRect rc(pRect);
	rc.DeflateRect(&rcTabPadding);
		// 
	bool disable = pCtrl->IsDisable(hTab);
		// 
	CString text = pCtrl->GetTabText(hTab);
		// 
	int textWidth = pDC->GetTextExtent(text).cx;
		// 
	CImageList *pImageList;
	(disable==false ? pCtrl->GetImageLists(&pImageList,NULL) : pCtrl->GetImageLists(NULL,&pImageList));
		// 
		// draw image.
	if(pCtrl->GetTabImage(hTab)>-1 && pImageList!=NULL && pImageList->m_hImageList!=NULL)
	{	CSize szImage;
		(disable==false ? pCtrl->GetImageSizes(&szImage,NULL) : pCtrl->GetImageSizes(NULL,&szImage));
			// 
		int iContentWidth = szImage.cx + pCtrl->GetTabImageTextGap() + textWidth;
		rc.left += (iContentWidth<rc.Width() ? (rc.Width()-iContentWidth)/2 : 0);
			// 
		DrawTabImage(pCtrl,pDC,hTab,&rc,pRgn);
			// 
		rc.left += szImage.cx + pCtrl->GetTabImageTextGap();
	}
	else
		rc.left += (textWidth<rc.Width() ? (rc.Width()-textWidth)/2 : 0);
		// 
		// draw text.
	if(rc.IsRectEmpty()==FALSE) DrawTabText(pCtrl,pDC,hTab,&rc,pRgn);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	COLORREF clrBorder = GetTabBorderColor(pCtrl);
	COLORREF clrBack = GetTabSelectedBackColor(pCtrl);
		// 
	if(pCtrl->GetSel()!=hTab)
	{	if(pCtrl->GetHover()==hTab && pCtrl->GetPushed()==NULL && pCtrl->IsDisable(hTab)==false)	// highlight tab.
		{	CRect rc(pRect);
				// 
			if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
			{	rc.DeflateRect(2,1,1,1);
				Draw::DrawHalfRoundFrame(pDC,&rc,Draw::SIDE_TOP,1,clrBorder,clrBack);
				rc.DeflateRect(1,rc.Height()/2,1,0);
				Draw::DrawGradient(pDC,&rc,false,clrBack,clrBorder);
			}
			else
			{	rc.DeflateRect(2,1,1,1);
				Draw::DrawHalfRoundFrame(pDC,&rc,Draw::SIDE_BOTTOM,1,clrBorder,clrBack);
			}
		}
			// 
		CRect rcTabPadding = pCtrl->GetTabPadding();
		int topMargin = rcTabPadding.top + 1;
		int bottomMargin = rcTabPadding.bottom + 1;
		(pCtrl->GetLayout()==TAB_LAYOUT_TOP ? bottomMargin+=1/*separator*/ : topMargin+=1/*separator*/);
			// 
		if(hTab==pCtrl->GetTab(0))	// this is first tab.
			Draw::DrawLine(pDC,pRect->left,pRect->top+topMargin,pRect->left,pRect->bottom-bottomMargin,clrBorder);
		Draw::DrawLine(pDC,pRect->right,pRect->top+topMargin,pRect->right,pRect->bottom-bottomMargin,clrBorder);
	}
	else
		Draw::DrawHalfRoundFrame(pDC,pRect,
			(pCtrl->GetLayout()==TAB_LAYOUT_TOP ? Draw::SIDE_TOP : Draw::SIDE_BOTTOM),2,clrBorder,clrBack);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawTabImage(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	CImageList *pImageList;
	CSize szImage;
		// 
	if(pCtrl->IsDisable(hTab)==false)
	{	pCtrl->GetImageLists(&pImageList,NULL);
		pCtrl->GetImageSizes(&szImage,NULL);
	}
	else
	{	pCtrl->GetImageLists(NULL,&pImageList);
		pCtrl->GetImageSizes(NULL,&szImage);
	}
	CPoint pt(pRect->left,(pRect->top+pRect->bottom-szImage.cy+1)/2);
	pImageList->Draw(pDC,pCtrl->GetTabImage(hTab),pt,ILD_NORMAL);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawTabText(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	CString text = pCtrl->GetTabText(hTab);
	pDC->SetTextColor( GetTabTextColor(pCtrl,hTab) );
	pDC->DrawText(text,(CRect *)pRect,DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,Draw::PixelAlpha(::GetSysColor(COLOR_HIGHLIGHT),::GetSysColor(COLOR_WINDOW),30));	// it is approximate color (VisualStudio uses some another way).
		Draw::DrawRect(pDC,pRect, ::GetSysColor(COLOR_HIGHLIGHT) );
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,Draw::PixelAlpha(::GetSysColor(COLOR_HIGHLIGHT),::GetSysColor(COLOR_WINDOW),22));	// it is approximate color (VisualStudio uses some another way).
		Draw::DrawRect(pDC,pRect, ::GetSysColor(COLOR_HIGHLIGHT) );
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,pRect,pImageList,TabCtrl::IMAGE_CLOSE, 
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,pRect,pImageList,
		(bPartialView==false ? TabCtrl::IMAGE_MENU : TabCtrl::IMAGE_MENU_PARTIAL),
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,pRect,pImageList,
		(bPartialView==false ? TabCtrl::IMAGE_ARROW_LEFT_DISABLE : TabCtrl::IMAGE_ARROW_LEFT),
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_base::DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,pRect,pImageList,
		(bPartialView==false ? TabCtrl::IMAGE_ARROW_RIGHT_DISABLE : TabCtrl::IMAGE_ARROW_RIGHT),
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_base::GetBorderColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNSHADOW);
}
// 
COLORREF TabCtrlStyle_base::GetTabBorderColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNSHADOW);
}
// 
COLORREF TabCtrlStyle_base::GetCtrlAreaBackColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNFACE);
}
// 
COLORREF TabCtrlStyle_base::GetWndsAreaBackColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNFACE);
}
// 
COLORREF TabCtrlStyle_base::GetTabSelectedBackColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_WINDOW);
}
// 
COLORREF TabCtrlStyle_base::GetTabTextColor(TabCtrl *pCtrl, HTAB hTab)
{	return (pCtrl->IsDisable(hTab)==false ? ::GetSysColor(COLOR_BTNTEXT) : ::GetSysColor(COLOR_GRAYTEXT));
}
// 
COLORREF TabCtrlStyle_base::GetButtonMarkerColor(TabCtrl *pCtrl, bool /*bHover*/, bool /*bPushed*/)
{	return (pCtrl->GetSel()!=NULL ? ::GetSysColor(COLOR_BTNTEXT) : ::GetSysColor(COLOR_GRAYTEXT));
}
// 
COLORREF TabCtrlStyle_base::GetChildWndBackColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_WINDOW);
}
// 
COLORREF TabCtrlStyle_base::GetEmptyWndsAreaBackColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_APPWORKSPACE);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2003_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
ITabCtrlRecalc *TabCtrlStyle_VS2003_base::GetRecalcManager()
{	return this;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_base::DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CRect rc(pRect);
	if(bHover==true && bPushed==true) rc.OffsetRect(1,1);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,&rc,pImageList,TabCtrl::IMAGE_CLOSE, 
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
// 
void TabCtrlStyle_VS2003_base::DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CRect rc(pRect);
	if(bHover==true && bPushed==true) rc.OffsetRect(1,1);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,&rc,pImageList,
		(bPartialView==false ? TabCtrl::IMAGE_MENU : TabCtrl::IMAGE_MENU_PARTIAL),
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
// 
void TabCtrlStyle_VS2003_base::DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CRect rc(pRect);
	if(bHover==true && bPushed==true) rc.OffsetRect(1,1);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,&rc,pImageList,
		(bPartialView==false ? TabCtrl::IMAGE_ARROW_LEFT_DISABLE : TabCtrl::IMAGE_ARROW_LEFT),
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
// 
void TabCtrlStyle_VS2003_base::DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
		// 
	CRect rc(pRect);
	if(bHover==true && bPushed==true) rc.OffsetRect(1,1);
		// 
	CImageList *pImageList = pCtrl->GetSystemImageList();
	Draw::DrawMarker(pDC,&rc,pImageList,
		(bPartialView==false ? TabCtrl::IMAGE_ARROW_RIGHT_DISABLE : TabCtrl::IMAGE_ARROW_RIGHT),
		GetButtonMarkerColor(pCtrl,bHover,bPushed) );
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_base::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover==true && bPushed==false)
		pDC->DrawEdge((RECT *)pRect,BDR_RAISEDOUTER,BF_RECT);
	else if(bHover==true && bPushed==true)
		pDC->DrawEdge((RECT *)pRect,BDR_SUNKENINNER,BF_RECT);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2003_base::GetCtrlAreaBackColor(TabCtrl * /*pCtrl*/)
{	return Draw::PixelAlpha(::GetSysColor(COLOR_BTNFACE),::GetSysColor(COLOR_BTNHIGHLIGHT),45);	// it is approximate color (VS2003 uses some another way).
}
// 
COLORREF TabCtrlStyle_VS2003_base::GetTabTextColor(TabCtrl *pCtrl, HTAB hTab)
{	if(pCtrl->GetSel()==hTab)
		if(pCtrl->IsWatchActivityCtrl()==false || pCtrl->IsActive()==true)
			return ::GetSysColor(COLOR_BTNTEXT);
		else
			return Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(0,0,0),55);
		// 
	return (pCtrl->IsDisable(hTab)==false ? 
		Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(0,0,0),55) :		// it is approximate color (VS2003 uses some another way).
		TabCtrlStyle_base::GetTabTextColor(pCtrl,hTab));
}
// 
COLORREF TabCtrlStyle_VS2003_base::GetButtonMarkerColor(TabCtrl *pCtrl, bool bHover, bool bPushed)
{	return (pCtrl->GetSel()!=NULL ?
		Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(0,0,0),55) :
		TabCtrlStyle_base::GetButtonMarkerColor(pCtrl,bHover,bPushed));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2003_client.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
CRect TabCtrlStyle_VS2003_client::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(5,2,3,0) : CRect(5,0,3,2));
}
// 
CRect TabCtrlStyle_VS2003_client::GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	bool bHideSingleTab = (pCtrl->GetCount()==1 && pCtrl->IsHideSingleTab()==true);
	if(bHideSingleTab==false)
		return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(2,3,2,2) : CRect(2,2,2,3));
	else
		return CRect(2,2,2,2);
}
// 
CRect TabCtrlStyle_VS2003_client::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? 
		CRect(6,1/*border*/+2,6,2+1/*line*/) : CRect(6,1/*line*/+2,6,2+1/*border*/));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_client::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect,GetCtrlAreaBackColor(pCtrl));
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
		Draw::DrawLine(pDC,pRect->left,pRect->bottom-1,pRect->right,pRect->bottom-1,::GetSysColor(COLOR_BTNHIGHLIGHT));
	else
		Draw::DrawLine(pDC,pRect->left,pRect->top,pRect->right,pRect->top,::GetSysColor(COLOR_BTNHIGHLIGHT));
	Draw::DrawLine(pDC,pRect->left,pRect->top,pRect->left,pRect->bottom,GetWndsAreaBackColor(pCtrl));
	Draw::DrawLine(pDC,pRect->right-1,pRect->top,pRect->right-1,pRect->bottom,GetWndsAreaBackColor(pCtrl));
}
// 
void TabCtrlStyle_VS2003_client::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
	bool select = (pCtrl->GetSel()==hTab);
		// 
	CRect rc(pRect);
		// 
	if(select==true)
	{	CPen penBorder(PS_SOLID,1, GetTabBorderColor(pCtrl) );
		CPen *pOldPen = pDC->SelectObject(&penBorder);
			// 
		if(top==true)
		{	Draw::FillSolidRect(pDC,&rc,GetWndsAreaBackColor(pCtrl));	// fill background.
				// draw left-top border.
			pDC->MoveTo(rc.left,rc.bottom-1);
			pDC->LineTo(rc.left,rc.top);
			pDC->LineTo(rc.right,rc.top);
				// draw right border.
			Draw::DrawLine(pDC,rc.right-1,rc.top+1,rc.right-1,rc.bottom,::GetSysColor(COLOR_BTNTEXT));
		}
		else
		{	Draw::FillSolidRect(pDC,&rc,GetWndsAreaBackColor(pCtrl));	// fill background.
				// draw left-bottom border.
			pDC->MoveTo(rc.left,rc.top);
			pDC->LineTo(rc.left,rc.bottom-1);
			pDC->LineTo(rc.right,rc.bottom-1);
				// draw right border.
			Draw::DrawLine(pDC,rc.right-1,rc.top,rc.right-1,rc.bottom-1,::GetSysColor(COLOR_BTNTEXT));
		}
		pDC->SelectObject(pOldPen);
	}
	else
	{	CRect rcTabPadding = pCtrl->GetTabPadding();
		rc.DeflateRect(0,rcTabPadding.top-1,0,rcTabPadding.bottom-1);
		Draw::DrawLine(pDC,rc.right,rc.top,rc.right,rc.bottom, ::GetSysColor(COLOR_BTNSHADOW) );	// draw right separator.
	}
}
// 
void TabCtrlStyle_VS2003_client::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	if(pCtrl->GetCount()==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	CRect rc(pRect);
		Draw::FillSolidRect(pDC,&rc,GetWndsAreaBackColor(pCtrl));
			// 
		rc.DeflateRect( GetWndsAreaPadding(pCtrl,NULL) );
		rc.InflateRect(1,1);
			// 
		if(rc.IsRectEmpty()==0)
		{	COLORREF color = GetBorderColor(pCtrl);
			Draw::DrawRect(pDC,&rc,color);	// rect around child window.
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2003_client::GetTabBorderColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNHIGHLIGHT);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2003_client_custom1.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_client_custom1::DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	TabCtrlStyle_base::DrawButtonClose(pCtrl,pDC,pRect,bHover,bPushed);
}
// 
void TabCtrlStyle_VS2003_client_custom1::DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	TabCtrlStyle_base::DrawButtonMenu(pCtrl,pDC,pRect,bHover,bPushed,bPartialView);
}
// 
void TabCtrlStyle_VS2003_client_custom1::DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	TabCtrlStyle_base::DrawButtonScrollLeft(pCtrl,pDC,pRect,bHover,bPushed,bPartialView);
}
// 
void TabCtrlStyle_VS2003_client_custom1::DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	TabCtrlStyle_base::DrawButtonScrollRight(pCtrl,pDC,pRect,bHover,bPushed,bPartialView);
}
// 
void TabCtrlStyle_VS2003_client_custom1::DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	TabCtrlStyle_base::DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2003_bars.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlStyle_VS2003_bars::GetBorderWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 0;
}
// 
CRect TabCtrlStyle_VS2003_bars::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(5,2,3,2) : CRect(5,2,3,2));
}
// 
CRect TabCtrlStyle_VS2003_bars::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? 
		CRect(6,1/*border*/+1,6,1+1/*line*/) : CRect(6,1/*line*/+1,6,1+1/*border*/));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_bars::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	CRect rc(pRect);
	COLORREF clrWndsAreaBack = GetWndsAreaBackColor(pCtrl);
		// 
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
	{	Draw::DrawLine(pDC,rc.left+1,rc.top,rc.right-1,rc.top,clrWndsAreaBack);
		++rc.top;
		rc.bottom -= 3;
		Draw::FillSolidRect(pDC,&rc,GetCtrlAreaBackColor(pCtrl));
		rc.top = rc.bottom;
		rc.bottom = pRect->bottom;
		Draw::DrawLine(pDC,rc.left+1,rc.top,rc.right-1,rc.top,::GetSysColor(COLOR_BTNTEXT));
		++rc.top;
		Draw::FillSolidRect(pDC,&rc,clrWndsAreaBack);
	}
	else
	{	rc.top += 3;
		--rc.bottom;
		Draw::DrawLine(pDC,rc.left+1,rc.bottom,rc.right-1,rc.bottom,clrWndsAreaBack);
		Draw::FillSolidRect(pDC,&rc,GetCtrlAreaBackColor(pCtrl));
		--rc.top;
		Draw::DrawLine(pDC,rc.left+1,rc.top,rc.right-1,rc.top,::GetSysColor(COLOR_BTNTEXT));
		rc.bottom = rc.top;
		rc.top = pRect->top;
		Draw::FillSolidRect(pDC,&rc,clrWndsAreaBack);
	}
	Draw::DrawLine(pDC,pRect->left,pRect->top,pRect->left,pRect->bottom,clrWndsAreaBack);
	Draw::DrawLine(pDC,pRect->right-1,pRect->top,pRect->right-1,pRect->bottom,clrWndsAreaBack);
}
// 
void TabCtrlStyle_VS2003_bars::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
	bool select = (pCtrl->GetSel()==hTab);
		// 
	CRect rc(pRect);
		// 
	if(select==true)
	{	CPen penBorder(PS_SOLID,1, GetTabBorderColor(pCtrl) );
		CPen *pOldPen = pDC->SelectObject(&penBorder);
			// 
		if(top==true)
		{	rc.DeflateRect(1,1,1,0);
			Draw::FillSolidRect(pDC,&rc,GetWndsAreaBackColor(pCtrl));	// fill background.
				// draw left-top border.
			rc.InflateRect(1,1,1,0);
			pDC->MoveTo(rc.left+1,rc.top);
			pDC->LineTo(rc.right-1,rc.top);
			pDC->LineTo(rc.right-1,rc.bottom);
				// draw right border.
			Draw::DrawLine(pDC,rc.left,rc.top,rc.left,rc.bottom-1,::GetSysColor(COLOR_BTNHIGHLIGHT));
		}
		else
		{	rc.DeflateRect(1,0,1,1);
			Draw::FillSolidRect(pDC,&rc,GetWndsAreaBackColor(pCtrl));	// fill background.
				// draw left-bottom border.
			rc.InflateRect(1,0,1,1);
			pDC->MoveTo(rc.left+1,rc.bottom-1);
			pDC->LineTo(rc.right-1,rc.bottom-1);
			pDC->LineTo(rc.right-1,rc.top-1);
				// draw right border.
			Draw::DrawLine(pDC,rc.left,rc.top+1,rc.left,rc.bottom,::GetSysColor(COLOR_BTNHIGHLIGHT));
		}
		pDC->SelectObject(pOldPen);
	}
	else
	{	CRect rcTabPadding = pCtrl->GetTabPadding();
		if(top==true)
			rc.DeflateRect(0,rcTabPadding.top,0,rcTabPadding.bottom+1);
		else
			rc.DeflateRect(0,rcTabPadding.top+1,0,rcTabPadding.bottom);
		Draw::DrawLine(pDC,rc.right,rc.top,rc.right,rc.bottom, ::GetSysColor(COLOR_BTNSHADOW) );	// draw right separator.
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2003_bars::GetTabBorderColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNTEXT);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2003_bars_custom1.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
CRect TabCtrlStyle_VS2003_bars_custom1::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(4,1,2,0) : CRect(4,0,2,1));
}
// 
CRect TabCtrlStyle_VS2003_bars_custom1::GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	bool bHideSingleTab = (pCtrl->GetCount()==1 && pCtrl->IsHideSingleTab()==true);
	if(bHideSingleTab==false)
		return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(1,0,1,1) : CRect(1,1,1,0));
	else
		return CRect(1,1,1,1);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	CRect rc(pRect);
	COLORREF clrBorder = GetTabBorderColor(pCtrl);
	COLORREF clrCtrlAreaBack = GetCtrlAreaBackColor(pCtrl);
		// 
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
	{	--rc.bottom;
		Draw::FillSolidRect(pDC,&rc,clrCtrlAreaBack);
		Draw::DrawLine(pDC,rc.left,rc.bottom,rc.right,rc.bottom,clrBorder);
	}
	else
	{	Draw::DrawLine(pDC,rc.left,rc.top,rc.right,rc.top,clrBorder);
		++rc.top;
		Draw::FillSolidRect(pDC,&rc,clrCtrlAreaBack);
	}
}
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	int count = pCtrl->GetCount();
		// 
	if(count==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	COLORREF clrBorder = GetTabBorderColor(pCtrl);
		bool bHideSingleTab = (count==1 && pCtrl->IsHideSingleTab()==true);
			// 
		if(bHideSingleTab==true)
			Draw::DrawRect(pDC,pRect,clrBorder);
		else
		{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
				// 
			CPen penBorder(PS_SOLID,1,clrBorder);
			CPen *pOldPen = pDC->SelectObject(&penBorder);
				// 
			if(top==true)
			{	pDC->MoveTo(pRect->left,pRect->top);
				pDC->LineTo(pRect->left,pRect->bottom-1);
				pDC->LineTo(pRect->right-1,pRect->bottom-1);
				pDC->LineTo(pRect->right-1,pRect->top-1);
			}
			else
			{	pDC->MoveTo(pRect->left,pRect->bottom-1);
				pDC->LineTo(pRect->left,pRect->top);
				pDC->LineTo(pRect->right-1,pRect->top);
				pDC->LineTo(pRect->right-1,pRect->bottom);
			}
			pDC->SelectObject(pOldPen);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	TabCtrlStyle_base::DrawButtonClose(pCtrl,pDC,pRect,bHover,bPushed);
}
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	TabCtrlStyle_base::DrawButtonMenu(pCtrl,pDC,pRect,bHover,bPushed,bPartialView);
}
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	TabCtrlStyle_base::DrawButtonScrollLeft(pCtrl,pDC,pRect,bHover,bPushed,bPartialView);
}
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView)
{	TabCtrlStyle_base::DrawButtonScrollRight(pCtrl,pDC,pRect,bHover,bPushed,bPartialView);
}
// 
void TabCtrlStyle_VS2003_bars_custom1::DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	TabCtrlStyle_base::DrawButtonFrame(pCtrl,pDC,pRect,bHover,bPushed);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2003_bars_custom1::GetTabBorderColor(TabCtrl * /*pCtrl*/)
{	return ::GetSysColor(COLOR_BTNSHADOW);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_client_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
ITabCtrlRecalc *TabCtrlStyle_VS2008_client_base::GetRecalcManager()
{	return this;
}
// 
ITabCtrlBehavior *TabCtrlStyle_VS2008_client_base::GetBehaviorManager()
{	return this;
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlStyle_VS2008_client_base::GetBorderWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 0;
}
// 
CRect TabCtrlStyle_VS2008_client_base::GetCtrlAreaPadding(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,0,3,0);
}
CRect TabCtrlStyle_VS2008_client_base::GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	bool bHideSingleTab = (pCtrl->GetCount()==1 && pCtrl->IsHideSingleTab()==true);
	if(bHideSingleTab==false)
		return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(5,3,5,5) : CRect(5,5,5,3));
	else
		return CRect(5,5,5,5);
}
// 
CRect TabCtrlStyle_VS2008_client_base::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(6,1/*border*/+2,6,3+2/*indent*/) : CRect(6,2/*indent*/+2,6,3+1/*border*/));
}
int TabCtrlStyle_VS2008_client_base::GetTabExtraWidth(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/, HTAB hTab)
{	if(pCtrl->GetIndex(hTab)!=0) return 0;	// it isn't first tab.
	return GetSlantWidth(pCtrl) - 6;
}
int TabCtrlStyle_VS2008_client_base::GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetTabMinWidth(pCtrl,base) + (GetSlantWidth(pCtrl)-6);
}
/////////////////////////////////////////////////////////////////////////////
// 
bool TabCtrlStyle_VS2008_client_base::IsStraightOrder(TabCtrl * /*pCtrl*/)
{	return false;
}
// 
void TabCtrlStyle_VS2008_client_base::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	bool allDisabled = (pCtrl->GetSel()==NULL);	// all tabs are disabled.
		// 
	Draw::FillSolidRect(pDC,pRect,GetCtrlAreaBackColor(pCtrl));
		// 
	CPen penBorder(PS_SOLID,1, GetTabBorderColor(pCtrl,true,allDisabled) );
	CPen penOutline(PS_SOLID,1, GetTabOutlineColor(pCtrl,true,false,allDisabled,true) );
		// 
	CPen *pOldPen = pDC->SelectObject(&penBorder);
		// 
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
	{	if(allDisabled==false)
		{		// draw border.
			pDC->MoveTo(pRect->left+1,pRect->bottom-1);
			pDC->LineTo(pRect->left+2,pRect->bottom-2);
			pDC->LineTo(pRect->right-3,pRect->bottom-2);
			pDC->LineTo(pRect->right-1,pRect->bottom);
				// draw outline.
			pDC->SelectObject(&penOutline);
			pDC->MoveTo(pRect->left+2,pRect->bottom-1);
			pDC->LineTo(pRect->right-2,pRect->bottom-1);
		}
		else
		{		// draw border.
			pDC->MoveTo(pRect->left,pRect->bottom-2);
			pDC->LineTo(pRect->right,pRect->bottom-2);
				// draw outline.
			pDC->SelectObject(&penOutline);
			pDC->MoveTo(pRect->left,pRect->bottom-1);
			pDC->LineTo(pRect->right,pRect->bottom-1);
		}
	}
	else
	{	if(allDisabled==false)
		{		// draw border.
			pDC->MoveTo(pRect->left+1,pRect->top);
			pDC->LineTo(pRect->left+2,pRect->top+1);
			pDC->LineTo(pRect->right-3,pRect->top+1);
			pDC->LineTo(pRect->right-1,pRect->top-1);
				// draw outline.
			pDC->SelectObject(&penOutline);
			pDC->MoveTo(pRect->left+2,pRect->top);
			pDC->LineTo(pRect->right-2,pRect->top);
		}
		else
		{		// draw border.
			pDC->MoveTo(pRect->left,pRect->top+1);
			pDC->LineTo(pRect->right,pRect->top+1);
				// draw outline.
			pDC->SelectObject(&penOutline);
			pDC->MoveTo(pRect->left,pRect->top);
			pDC->LineTo(pRect->right,pRect->top);
		}
	}
		// 
	pDC->SelectObject(pOldPen);
}
// 
void TabCtrlStyle_VS2008_client_base::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn)
{	bool selected = (pCtrl->GetSel()==hTab);
	bool hover = (pCtrl->GetHover()==hTab) && (pCtrl->GetPushed()==NULL);
	bool disabled = pCtrl->IsDisable(hTab);
	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		// 
	COLORREF clrBorder = GetTabBorderColor(pCtrl,selected,disabled);
	COLORREF clrBackLight = GetTabGradientLightColor(pCtrl,selected,hover,disabled);
	COLORREF clrBackDark = GetTabGradientDarkColor(pCtrl,selected,hover,disabled);
	COLORREF clrOutlineLeft = GetTabOutlineColor(pCtrl,selected,hover,disabled,true);
	COLORREF clrOutlineRight = GetTabOutlineColor(pCtrl,selected,hover,disabled,false);
		// 
	CPen penBorder(PS_SOLID,1,clrBorder);
	CPen penOutlineLeft(PS_SOLID,1,clrOutlineLeft);
	CPen penOutlineRight(PS_SOLID,1,clrOutlineRight);
		// 
	POINT pts[8];
	CRect rcFill;
	GetTabOutline(pCtrl,hTab,pRect,top,pts,&rcFill);
		// 
	CRgn rgn;
	rgn.CreatePolygonRgn(pts,sizeof(pts)/sizeof(POINT),WINDING);	// create clip region for gradient.
		// 
	RgnEx rgnRes;
	rgnRes.CombineRgn(&rgn,pRgn,RGN_AND);
		// 
	pDC->SelectClipRgn(&rgnRes,RGN_COPY);
		// 
		// draw back.
	if(top==true)
		Draw::DrawGradient(pDC,&rcFill,false,clrBackLight,clrBackDark);
	else
		Draw::DrawGradient(pDC,&rcFill,false,clrBackDark,clrBackLight);
		// 
		// left outline.
	CPen *pOldPen = pDC->SelectObject(&penOutlineLeft);
	pDC->MoveTo(pts[1].x+1,pts[1].y);
	pDC->LineTo(pts[2].x+1,pts[2].y);
	pDC->LineTo(pts[3].x+1,pts[3].y);
		// 
		// right outline.
	pOldPen = pDC->SelectObject(&penOutlineRight);
	pDC->MoveTo(pts[4].x-1,pts[4].y);
	pDC->LineTo(pts[5].x-1,pts[5].y);
	pDC->LineTo(pts[6].x-1,pts[6].y);
		// 
	pDC->SelectClipRgn(pRgn,RGN_COPY);	// restore clip region.
		// 
		// draw border.
	pOldPen = pDC->SelectObject(&penBorder);
	pDC->MoveTo(pts[1].x,pts[1].y);
	pDC->LineTo(pts[2].x,pts[2].y);
	pDC->LineTo(pts[3].x,pts[3].y);
	pDC->LineTo(pts[4].x,pts[4].y);
	pDC->LineTo(pts[5].x,pts[5].y);
	pDC->LineTo(pts[6].x,pts[6].y);
		// 
	pDC->SelectObject(pOldPen);
}
// 
void TabCtrlStyle_VS2008_client_base::DrawTabContext(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn)
{	CRect rc(pRect);
	if(pCtrl->GetSel()!=hTab)
		(pCtrl->GetLayout()==TAB_LAYOUT_TOP ? rc.top+=2 : rc.bottom-=2);
	rc.left += pCtrl->GetTabExtraWidth(hTab);
	TabCtrlStyle_base::DrawTabContext(pCtrl,pDC,hTab,&rc,pRgn);
}
// 
void TabCtrlStyle_VS2008_client_base::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	int count = pCtrl->GetCount();
		// 
	if(count==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		bool bHideSingleTab = (count==1 && pCtrl->IsHideSingleTab()==true);
			// 
		COLORREF clrBorder = GetTabBorderColor(pCtrl,true,false);
		COLORREF clrOutline = GetTabOutlineColor(pCtrl,true,false,false,true);
		COLORREF clrBack = GetTabGradientDarkColor(pCtrl,true,false,false);
		COLORREF clrBackOut = GetWndsAreaBackColor(pCtrl);
			// 
		CPen penBorder(PS_SOLID,1,clrBorder);
		CPen penOutline(PS_SOLID,1,clrOutline);
		CPen penBack(PS_SOLID,1,clrBack);
			// 
		CRect rc(pRect);
			// 
		CPen *pOldPen = pDC->SelectObject(&penBorder);
			// 
		if(bHideSingleTab==true)
		{		// outside border.
			Draw::DrawBeveledRect(pDC,&rc,2);
				// left-top corner.
			::SetPixelV(pDC->m_hDC,rc.left,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left+1,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left,rc.top+1,clrBackOut);
				// right-top corner.
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-2,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.top+1,clrBackOut);
				// left-bottom corner.
			::SetPixelV(pDC->m_hDC,rc.left,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left+1,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left,rc.bottom-2,clrBackOut);
				// right-bottom corner.
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-2,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.bottom-2,clrBackOut);
				// inside border.
			rc.DeflateRect(4,4);
			if(rc.IsRectEmpty()==0) Draw::DrawRect(pDC,&rc);
		}
		else if(top==true)
		{		// outside border.
			pDC->MoveTo(rc.left,rc.top);
			pDC->LineTo(rc.left,rc.bottom-3);
			pDC->LineTo(rc.left+2,rc.bottom-1);
			pDC->LineTo(rc.right-3,rc.bottom-1);
			pDC->LineTo(rc.right-1,rc.bottom-3);
			pDC->LineTo(rc.right-1,rc.top-1);
				// left-bottom corner.
			::SetPixelV(pDC->m_hDC,rc.left,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left+1,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left,rc.bottom-2,clrBackOut);
				// right-bottom corner.
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-2,rc.bottom-1,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.bottom-2,clrBackOut);
				// inside border.
			rc.DeflateRect(4,2,4,4);
			if(rc.IsRectEmpty()==0) Draw::DrawRect(pDC,&rc);
		}
		else
		{		// outside border.
			pDC->MoveTo(rc.left,rc.bottom-1);
			pDC->LineTo(rc.left,rc.top+2);
			pDC->LineTo(rc.left+2,rc.top);
			pDC->LineTo(rc.right-3,rc.top);
			pDC->LineTo(rc.right-1,rc.top+2);
			pDC->LineTo(rc.right-1,rc.bottom);
				// left-bottom corner.
			::SetPixelV(pDC->m_hDC,rc.left,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left+1,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.left,rc.top+1,clrBackOut);
				// right-bottom corner.
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-2,rc.top,clrBackOut);
			::SetPixelV(pDC->m_hDC,rc.right-1,rc.top+1,clrBackOut);
				// inside border.
			rc.DeflateRect(4,4,4,2);
			if(rc.IsRectEmpty()==0) Draw::DrawRect(pDC,&rc);
		}
			// 
			// draw back.
		pDC->SelectObject(&penBack);
			// 
		rc.InflateRect(1,1);
		if(rc.IsRectEmpty()==0) Draw::DrawRect(pDC,&rc);
		rc.InflateRect(1,1);
		if(rc.IsRectEmpty()==0) Draw::DrawRect(pDC,&rc);
			// 
			// draw outline;
		pDC->SelectObject(&penOutline);
			// 
		if(bHideSingleTab==true)
		{	rc.InflateRect(1,1);
			if(rc.IsRectEmpty()==0)
			{	pDC->MoveTo(rc.left,rc.top+1);
				pDC->LineTo(rc.left,rc.bottom-1);
				pDC->MoveTo(rc.left+1,rc.bottom-1);
				pDC->LineTo(rc.right-1,rc.bottom-1);
				pDC->MoveTo(rc.right-1,rc.bottom-2);
				pDC->LineTo(rc.right-1,rc.top);
				pDC->MoveTo(rc.right-2,rc.top);
				pDC->LineTo(rc.left,rc.top);
			}
		}
		else if(top==true)
		{	rc.InflateRect(1,0,1,1);
			if(rc.IsRectEmpty()==0)
			{	pDC->MoveTo(rc.left,rc.top);
				pDC->LineTo(rc.left,rc.bottom-1);
				pDC->MoveTo(rc.left+1,rc.bottom-1);
				pDC->LineTo(rc.right-1,rc.bottom-1);
				pDC->MoveTo(rc.right-1,rc.bottom-2);
				pDC->LineTo(rc.right-1,rc.top-1);
			}
		}
		else
		{	rc.InflateRect(1,1,1,0);
			if(rc.IsRectEmpty()==0)
			{	pDC->MoveTo(rc.left,rc.bottom-1);
				pDC->LineTo(rc.left,rc.top);
				pDC->MoveTo(rc.left+1,rc.top);
				pDC->LineTo(rc.right-1,rc.top);
				pDC->MoveTo(rc.right-1,rc.top+1);
				pDC->LineTo(rc.right-1,rc.bottom);
			}
		}
			// 
		pDC->SelectObject(pOldPen);
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrlStyle_VS2008_client_base::HitTest(TabCtrl *pCtrl, ITabCtrlBehavior * /*base*/, CPoint point)	// get tab in the given point.
{	if(CRect(pCtrl->GetTabsArea()).PtInRect(point)!=0)
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
			// 
		HTAB hTabSel = pCtrl->GetSel();
		if(hTabSel!=NULL && HitTest(pCtrl,hTabSel,top,point)==true) return hTabSel;
			// 
		for(int i=0, c=pCtrl->GetCount(); i<c; ++i)
		{	HTAB hTab = pCtrl->GetTab(i);
			if(hTab!=hTabSel && HitTest(pCtrl,hTab,top,point)==true) return hTab;
		}
	}
	return NULL;
}
// 
bool TabCtrlStyle_VS2008_client_base::HitTest(TabCtrl *pCtrl, HTAB hTab, bool top, CPoint point) const
{	CRect rc;
	POINT pts[8];
	pCtrl->GetTabRect(hTab,&rc);
	GetTabOutline(pCtrl,hTab,&rc,top,pts,NULL);
		// 
	CRgn rgn;
	rgn.CreatePolygonRgn(pts,sizeof(pts)/sizeof(POINT),WINDING);
	return rgn.PtInRegion(point)!=0;
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_client_base::GetTabOutline(TabCtrl *pCtrl, HTAB hTab, CRect const *rect, bool top, POINT pts[8]/*out*/, RECT *rcFill/*out*/) const
{	bool first = (pCtrl->GetIndex(hTab)==0);
	bool selected = (pCtrl->GetSel()==hTab);
	int iSlantWidth = GetSlantWidth(pCtrl);
		// 
	CRect rc(rect);
		// 
	if(selected==false)
	{	rc.top += 2;
		rc.bottom -= 2;
		iSlantWidth -= 4;
	}
		// 
	if(first==false)
		rc.left -= (iSlantWidth - 6);
	else if(selected==false)
		rc.left += 4;
		// 
	if(top==true)
	{	if(selected==false)
		{	pts[0].x = rc.left; pts[0].y = rc.bottom;
			pts[1].x = rc.left+1; pts[1].y = rc.bottom-1;
		}
		else
		{	pts[0].x = rc.left+2; pts[0].y = rc.bottom;
			pts[1].x = rc.left+2; pts[1].y = rc.bottom-2;
		}
		pts[2].x = rc.left+iSlantWidth-2; pts[2].y = rc.top+2;
		pts[3].x = rc.left+iSlantWidth+3; pts[3].y = rc.top;
		pts[4].x = rc.right-3; pts[4].y = rc.top;
		pts[5].x = rc.right-1; pts[5].y = rc.top+2;
		if(selected==false)
		{	pts[6].x = rc.right-1; pts[6].y = rc.bottom;
		}
		else
		{	pts[6].x = rc.right-1; pts[6].y = rc.bottom-1;
		}
		pts[7].x = rc.right-1; pts[7].y = rc.bottom;
	}
	else
	{	if(selected==false)
		{	pts[0].x = rc.left; pts[0].y = rc.top;
			pts[1].x = rc.left+1; pts[1].y = rc.top;
		}
		else
		{	pts[0].x = rc.left+2; pts[0].y = rc.top;
			pts[1].x = rc.left+2; pts[1].y = rc.top+1;
		}
		pts[2].x = rc.left+iSlantWidth-2; pts[2].y = rc.bottom-3;
		pts[3].x = rc.left+iSlantWidth+3; pts[3].y = rc.bottom-1;
		pts[4].x = rc.right-3; pts[4].y = rc.bottom-1;
		pts[5].x = rc.right-1; pts[5].y = rc.bottom-3;
		if(selected==false)
		{	pts[6].x = rc.right-1; pts[6].y = rc.top-1;
		}
		else
		{	pts[6].x = rc.right-1; pts[6].y = rc.top;
		}
		pts[7].x = rc.right-1; pts[7].y = rc.top;
	}
		// 
	if(rcFill!=NULL)
	{	*rcFill = rc;
		if(selected==false && first==false) 
			rcFill->left = rect->left;
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlStyle_VS2008_client_base::GetSlantWidth(TabCtrl *pCtrl) const
{	return CRect(pCtrl->GetTabsArea()).Height();
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_client_classic.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
 
COLORREF TabCtrlStyle_VS2008_client_classic::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool /*active*/, bool /*disable*/)
{	return ::GetSysColor(COLOR_BTNSHADOW);
}
COLORREF TabCtrlStyle_VS2008_client_classic::GetTabOutlineColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable, bool /*left*/)
{	if(active==true) return ::GetSysColor(COLOR_WINDOW);
	if(hover==true && disable==false) return ::GetSysColor(COLOR_WINDOW);
	if(disable==true) return ::GetSysColor(COLOR_BTNFACE);
	return ::GetSysColor(COLOR_BTNFACE);
}
COLORREF TabCtrlStyle_VS2008_client_classic::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return ::GetSysColor(COLOR_BTNHIGHLIGHT);
	if(hover==true && disable==false) return ::GetSysColor(COLOR_BTNHIGHLIGHT);
	return ::GetSysColor(COLOR_BTNFACE);
}
COLORREF TabCtrlStyle_VS2008_client_classic::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return ::GetSysColor(COLOR_BTNFACE);
	if(hover==true && disable==false) return ::GetSysColor(COLOR_BTNHIGHLIGHT);
	return ::GetSysColor(COLOR_BTNFACE);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_client_blue.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_client_blue::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool active, bool disable)
{	if(active==true) return RGB(105,161,191);
	if(disable==true) return RGB(145,150,162);
	return RGB(145,150,162);
}
COLORREF TabCtrlStyle_VS2008_client_blue::GetTabOutlineColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable, bool left)
{	if(active==true) return RGB(255,255,255);
	if(hover==true && disable==false) return RGB(255,255,255);
	if(disable==true) return RGB(140,171,204);
	return (left==true ? RGB(242,250,255) : RGB(140,171,204));
}
COLORREF TabCtrlStyle_VS2008_client_blue::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return RGB(252,253,254);
	if(hover==true && disable==false) return RGB(247,252,254);
	if(disable==true) return RGB(207,223,237);
	return RGB(236,245,252);
}
COLORREF TabCtrlStyle_VS2008_client_blue::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return RGB(210,230,250);
	if(hover==true && disable==false) return RGB(129,208,241);
	if(disable==true) return RGB(207,223,237);
	return RGB(152,180,210);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_client_silver.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_client_silver::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool active, bool disable)
{	if(active==true) return RGB(147,145,176);
	if(disable==true) return RGB(157,157,161);
	return RGB(157,157,161);
}
COLORREF TabCtrlStyle_VS2008_client_silver::GetTabOutlineColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable, bool left)
{	if(active==true) return RGB(255,255,255);
	if(hover==true && disable==false) return RGB(255,255,255);
	if(disable==true) return RGB(172,171,196);
	return (left==true ? RGB(255,255,255) : RGB(172,171,196));
}
COLORREF TabCtrlStyle_VS2008_client_silver::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return RGB(247,247,253);
	if(hover==true && disable==false) return RGB(247,247,253);
	if(disable==true) return RGB(203,205,217);
	return RGB(234,235,240);
}
COLORREF TabCtrlStyle_VS2008_client_silver::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return RGB(225,226,236);
	if(hover==true && disable==false) return RGB(182,185,201);
	if(disable==true) return RGB(203,205,217);
	return RGB(172,171,196);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_client_silver::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,227,173));
		Draw::DrawRect(pDC,pRect,RGB(74,73,107));
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,182,115));
		Draw::DrawRect(pDC,pRect,RGB(74,73,107));
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_client_olive.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_client_olive::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool active, bool disable)
{	if(active==true) return RGB(147,160,112);
	if(disable==true) return RGB(172,168,153);
	return RGB(172,168,153);
}
COLORREF TabCtrlStyle_VS2008_client_olive::GetTabOutlineColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable, bool left)
{	if(active==true) return RGB(255,255,255);
	if(hover==true && disable==false) return RGB(255,255,255);
	if(disable==true) return RGB(165,179,133);
	return (left==true ? RGB(255,255,255) : RGB(165,179,133));
}
COLORREF TabCtrlStyle_VS2008_client_olive::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return RGB(250,251,247);
	if(hover==true && disable==false) return RGB(245,247,240);
	if(disable==true) return RGB(208,217,181);
	return RGB(241,244,233);
}
COLORREF TabCtrlStyle_VS2008_client_olive::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool active, bool hover, bool disable)
{	if(active==true) return RGB(173,190,126);
	if(hover==true && disable==false) return RGB(197,210,165);
	if(disable==true) return RGB(208,217,181);
	return RGB(165,179,133);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_client_olive::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,227,173));
		Draw::DrawRect(pDC,pRect,RGB(118,128,95));
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,182,115));
		Draw::DrawRect(pDC,pRect,RGB(118,128,95));
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
ITabCtrlRecalc *TabCtrlStyle_VS2008_bars_base::GetRecalcManager()
{	return this;
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlStyle_VS2008_bars_base::GetBorderWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 0;
}
// 
CRect TabCtrlStyle_VS2008_bars_base::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(0,1,0,2) : CRect(0,2,0,1));
}
// 
CRect TabCtrlStyle_VS2008_bars_base::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? 
		CRect(1/*border*/+2,1+1/*border*/+2/*indent*/,3,1/*border*/+1) : 
		CRect(1/*border*/+2,1/*border*/+1,3,1+1/*border*/+2/*indent*/));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_bars_base::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect, GetCtrlAreaBackColor(pCtrl) );
		// 
	COLORREF clrBorder = GetTabBorderColor(pCtrl,false);
		// 
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
		Draw::DrawLine(pDC,pRect->left,pRect->bottom-3,pRect->right,pRect->bottom-3, clrBorder );
	else
		Draw::DrawLine(pDC,pRect->left,pRect->top+2,pRect->right,pRect->top+2, clrBorder );
}
// 
void TabCtrlStyle_VS2008_bars_base::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
	bool select = (pCtrl->GetSel()==hTab);
	int count = pCtrl->GetCount();
		// 
	COLORREF clrBorder = GetTabBorderColor(pCtrl,false);
		// 
	CPen penBorder(PS_SOLID,1,clrBorder);
	CPen *pOldPen = pDC->SelectObject(&penBorder);
		// 
	if(select==true)
	{	bool firstTab = (pCtrl->GetTab(0)==hTab);
		COLORREF clrBackSelected = GetTabSelectedBackColor(pCtrl);
			// 
			// draw back.
		Draw::FillSolidRect(pDC,pRect,clrBackSelected);	// fill background.
			// 
			// draw tab border.
		if(top==true)
		{	pDC->MoveTo(pRect->left,pRect->bottom - (firstTab==true ? 1 : 2));
			pDC->LineTo(pRect->left,pRect->top);
			pDC->LineTo(pRect->right-1,pRect->top);
			pDC->LineTo(pRect->right-1,pRect->bottom-1);
		}
		else
		{	pDC->MoveTo(pRect->left,pRect->top + (firstTab==true ? 0 : 1));
			pDC->LineTo(pRect->left,pRect->bottom-1);
			pDC->LineTo(pRect->right-1,pRect->bottom-1);
			pDC->LineTo(pRect->right-1,pRect->top);
		}
	}
	else	// tab isn't selected.
	{	bool hover = (pCtrl->GetHover()==hTab) && (pCtrl->GetPushed()==NULL);
		bool disabled = pCtrl->IsDisable(hTab);
			// 		
		COLORREF clrBorderHover = GetTabBorderColor(pCtrl,true);
		COLORREF clrBackLight = GetTabGradientLightColor(pCtrl,hover,disabled);
		COLORREF clrBackDark = GetTabGradientDarkColor(pCtrl,hover,disabled);
			// 
		HTAB tabSel = pCtrl->GetSel();
		int cmpRes = (tabSel!=NULL ? pCtrl->CmpTabs(hTab,tabSel) : -1);
			// 
		CRect rc(pRect);
			// 
		if(top==true)
		{	rc.DeflateRect(0,2,0,1);
				// 
				// draw back.
			Draw::DrawGradient(pDC,&rc,false,clrBackLight,clrBackDark);
				// 
			if(cmpRes<0)	// before selected tab or there isn't selected tab.
			{		// draw tab border.
				if(hover==true && disabled==false)
				{	Draw::DrawLine(pDC,rc.left,rc.bottom-1,rc.left,rc.top,clrBorderHover);
					Draw::DrawLine(pDC,rc.left,rc.top,rc.right,rc.top,clrBorderHover);
				}
				else
				{	pDC->MoveTo(rc.left,rc.bottom-1);
					pDC->LineTo(rc.left,rc.top);
					pDC->LineTo(rc.right,rc.top);
				}
					// draw outline.
				Draw::DrawLine(pDC,rc.left+1,rc.top+1,rc.left+1,rc.bottom,clrBackLight);
			}
			else	// after selected tab.
			{		// draw tab border.
				if(hover==true && disabled==false)
				{	Draw::DrawLine(pDC,rc.left,rc.top,rc.right-1,rc.top,clrBorderHover);
					Draw::DrawLine(pDC,rc.right-1,rc.top,rc.right-1,rc.bottom,clrBorderHover);
				}
				else
				{	pDC->MoveTo(rc.left,rc.top);
					pDC->LineTo(rc.right-1,rc.top);
					pDC->LineTo(rc.right-1,rc.bottom);
				}
					// draw outline.
				Draw::DrawLine(pDC,rc.right-2,rc.top+1,rc.right-2,rc.bottom,clrBackLight);
			}
		}
		else		// bottom.
		{	rc.DeflateRect(0,1,0,2);
				// 
				// draw back.
			Draw::DrawGradient(pDC,&rc,false,clrBackDark,clrBackLight);
				// 
			if(cmpRes<0)	// before selected tab or there isn't selected tab.
			{		// draw tab border.
				if(hover==true && disabled==false)
				{	Draw::DrawLine(pDC,rc.left,rc.top,rc.left,rc.bottom-1,clrBorderHover);
					Draw::DrawLine(pDC,rc.left,rc.bottom-1,rc.right,rc.bottom-1,clrBorderHover);
				}
				else
				{	pDC->MoveTo(rc.left,rc.top);
					pDC->LineTo(rc.left,rc.bottom-1);
					pDC->LineTo(rc.right,rc.bottom-1);
				}
					// draw outline.
				Draw::DrawLine(pDC,rc.left+1,rc.top,rc.left+1,rc.bottom-1,clrBackLight);
			}
			else	// after selected tab.
			{		// draw tab border.
				if(hover==true && disabled==false)
				{	Draw::DrawLine(pDC,rc.left,rc.bottom-1,rc.right-1,rc.bottom-1,clrBorderHover);
					Draw::DrawLine(pDC,rc.right-1,rc.bottom-1,rc.right-1,rc.top-1,clrBorderHover);
				}
				else
				{	pDC->MoveTo(rc.left,rc.bottom-1);
					pDC->LineTo(rc.right-1,rc.bottom-1);
					pDC->LineTo(rc.right-1,rc.top-1);
				}
					// draw outline.
				Draw::DrawLine(pDC,rc.right-2,rc.top,rc.right-2,rc.bottom-1,clrBackLight);
			}
		}
			// 
			// draw closing line.
		if(cmpRes<0)	// before selected tab or there isn't selected tab.
		{	bool lastTab = (pCtrl->GetTab(count-1)==hTab);
				// 
			if(lastTab==true)
				Draw::DrawLine(pDC,rc.right-1,rc.top,rc.right-1,rc.bottom);
		}
	}
	pDC->SelectObject(pOldPen);
}
// 
void TabCtrlStyle_VS2008_bars_base::DrawTabContext(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn)
{	CRect rc(pRect);
	if(pCtrl->GetSel()==hTab)
		(pCtrl->GetLayout()==TAB_LAYOUT_TOP ? rc.top-=2 : rc.bottom+=2);
	TabCtrlStyle_base::DrawTabContext(pCtrl,pDC,hTab,&rc,pRgn);
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_base::GetTabTextColor(TabCtrl *pCtrl, HTAB hTab)
{	if(pCtrl->IsDisable(hTab)==true)		// disabled.
		return TabCtrlStyle_base::GetTabTextColor(pCtrl,hTab);
	else if(pCtrl->GetSel()==hTab)	// selected.
		return ::GetSysColor(COLOR_BTNTEXT);
	else
		return Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(0,0,0),55);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_classic.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_classic::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? ::GetSysColor(COLOR_3DDKSHADOW) : ::GetSysColor(COLOR_BTNSHADOW));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_classic::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool /*hover*/, bool /*disable*/)
{	return ::GetSysColor(COLOR_WINDOW);
}
// 
COLORREF TabCtrlStyle_VS2008_bars_classic::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? 
		Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(255,255,255),75) :
		Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(255,255,255),55));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_blue.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_blue::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? RGB(60,127,177) : RGB(137,140,149));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_blue::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(250,253,254) : RGB(252,252,252));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_blue::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(167,217,245) : RGB(207,207,207));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_silver.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_silver::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? RGB(119,119,146) : RGB(137,140,149));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_silver::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(247,247,253) : RGB(252,252,252));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_silver::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(180,179,202) : RGB(207,207,207));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_bars_silver::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,227,173));
		Draw::DrawRect(pDC,pRect,RGB(74,73,107));
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,182,115));
		Draw::DrawRect(pDC,pRect,RGB(74,73,107));
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_olive.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_olive::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? RGB(143,158,116) : RGB(137,140,149));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_olive::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(250,251,247) : RGB(252,252,252));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_olive::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(182,198,141) : RGB(207,207,207));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_bars_olive::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,227,173));
		Draw::DrawRect(pDC,pRect,RGB(118,128,95));
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,182,115));
		Draw::DrawRect(pDC,pRect,RGB(118,128,95));
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_custom1_base.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
CRect TabCtrlStyle_VS2008_bars_custom1_base::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(0,1,0,0) : CRect(0,0,0,1));
}
// 
CRect TabCtrlStyle_VS2008_bars_custom1_base::GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	bool bHideSingleTab = (pCtrl->GetCount()==1 && pCtrl->IsHideSingleTab()==true);
	if(bHideSingleTab==false)
		return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? CRect(1,0,1,1) : CRect(1,1,1,0));
	else
		return CRect(1,1,1,1);
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_bars_custom1_base::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect, GetCtrlAreaBackColor(pCtrl) );
		// 
	COLORREF clrBorder = GetTabBorderColor(pCtrl,false);
		// 
	if(pCtrl->GetLayout()==TAB_LAYOUT_TOP)
		Draw::DrawLine(pDC,pRect->left,pRect->bottom-1,pRect->right,pRect->bottom-1, clrBorder );
	else
		Draw::DrawLine(pDC,pRect->left,pRect->top,pRect->right,pRect->top, clrBorder );
}
// 
void TabCtrlStyle_VS2008_bars_custom1_base::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	int count = pCtrl->GetCount();
		// 
	if(count==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	COLORREF clrBorder = GetTabBorderColor(pCtrl,false);
		bool bHideSingleTab = (count==1 && pCtrl->IsHideSingleTab()==true);
			// 
		if(bHideSingleTab==true)
			Draw::DrawRect(pDC,pRect,clrBorder);
		else
		{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
				// 
			CPen penBorder(PS_SOLID,1,clrBorder);
			CPen *pOldPen = pDC->SelectObject(&penBorder);
				// 
			if(top==true)
			{	pDC->MoveTo(pRect->left,pRect->top);
				pDC->LineTo(pRect->left,pRect->bottom-1);
				pDC->LineTo(pRect->right-1,pRect->bottom-1);
				pDC->LineTo(pRect->right-1,pRect->top-1);
			}
			else
			{	pDC->MoveTo(pRect->left,pRect->bottom-1);
				pDC->LineTo(pRect->left,pRect->top);
				pDC->LineTo(pRect->right-1,pRect->top);
				pDC->LineTo(pRect->right-1,pRect->bottom);
			}
			pDC->SelectObject(pOldPen);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_classic_custom1.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_classic_custom1::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? ::GetSysColor(COLOR_3DDKSHADOW) : ::GetSysColor(COLOR_BTNSHADOW));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_classic_custom1::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool /*hover*/, bool /*disable*/)
{	return ::GetSysColor(COLOR_WINDOW);
}
// 
COLORREF TabCtrlStyle_VS2008_bars_classic_custom1::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? 
		Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(255,255,255),75) :
		Draw::PixelAlpha(::GetSysColor(COLOR_BTNSHADOW),RGB(255,255,255),55));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_blue.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_blue_custom1::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? RGB(60,127,177) : RGB(137,140,149));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_blue_custom1::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(250,253,254) : RGB(252,252,252));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_blue_custom1::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(167,217,245) : RGB(207,207,207));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_silver.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_silver_custom1::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? RGB(119,119,146) : RGB(137,140,149));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_silver_custom1::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(247,247,253) : RGB(252,252,252));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_silver_custom1::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(180,179,202) : RGB(207,207,207));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_bars_silver_custom1::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,227,173));
		Draw::DrawRect(pDC,pRect,RGB(74,73,107));
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,182,115));
		Draw::DrawRect(pDC,pRect,RGB(74,73,107));
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2008_bars_olive.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2008_bars_olive_custom1::GetTabBorderColor(TabCtrl * /*pCtrl*/, bool hover)
{	return (hover==true ? RGB(143,158,116) : RGB(137,140,149));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_olive_custom1::GetTabGradientLightColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(250,251,247) : RGB(252,252,252));
}
// 
COLORREF TabCtrlStyle_VS2008_bars_olive_custom1::GetTabGradientDarkColor(TabCtrl * /*pCtrl*/, bool hover, bool disable)
{	return ((hover==true && disable==false) ? RGB(182,198,141) : RGB(207,207,207));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2008_bars_olive_custom1::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,227,173));
		Draw::DrawRect(pDC,pRect,RGB(118,128,95));
	}
	else if(bHover==true && bPushed==true)
	{	Draw::FillSolidRect(pDC,pRect,RGB(255,182,115));
		Draw::DrawRect(pDC,pRect,RGB(118,128,95));
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2010_client.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
ITabCtrlRecalc *TabCtrlStyle_VS2010_client::GetRecalcManager()
{	return this;
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlStyle_VS2010_client::GetBorderWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 0;
}
// 
CRect TabCtrlStyle_VS2010_client::GetCtrlAreaPadding(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,0,3,0);
}
// 
CRect TabCtrlStyle_VS2010_client::GetWndsAreaPadding(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,4,0,4);
}
// 
CRect TabCtrlStyle_VS2010_client::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? 
		CRect(5,1/*border*/+3,5,3) : CRect(5,3,5,3+1/*border*/));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2010_client::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect,GetCtrlAreaBackColor(pCtrl));
}
// 
void TabCtrlStyle_VS2010_client::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	if(pCtrl->IsDisable(hTab)==false)
	{	bool active = pCtrl->IsActive();
		bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		bool select = (pCtrl->GetSel()==hTab);
		bool hover = (pCtrl->GetHover()==hTab);
			// 
		DrawTabBack(pCtrl,pDC,pRect,top,active,select,hover);
	}
}
// 
void TabCtrlStyle_VS2010_client::DrawTabBack(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool top, bool active, bool select, bool hover)
{	CRect rc(pRect);
		// 
	if(top==true)
	{	if(select==true)
		{	if(active==true)
			{	rc.top = (rc.top + rc.bottom)/2;
				Draw::FillSolidRect(pDC,&rc,RGB(255,232,166));	// bottom path.
				rc.bottom = rc.top;
				rc.top = pRect->top;
				Draw::DrawGradient(pDC,&rc,false,RGB(255,252,242),RGB(255,243,207));	// top path.
					// left corner.
				pDC->SetPixelV(rc.left,rc.top,RGB(94,109,133));
				pDC->SetPixelV(rc.left+1,rc.top,RGB(195,198,199));
				pDC->SetPixelV(rc.left,rc.top+1,RGB(195,197,197));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.top,RGB(94,109,133));
				pDC->SetPixelV(rc.right-2,rc.top,RGB(195,198,199));
				pDC->SetPixelV(rc.right-1,rc.top+1,RGB(195,197,197));
			}
			else
			{	rc.top = (rc.top + rc.bottom)/2;
				Draw::FillSolidRect(pDC,&rc,RGB(206,212,223));	// bottom path.
				rc.bottom = rc.top;
				rc.top = pRect->top;
				Draw::DrawGradient(pDC,&rc,false,RGB(251,252,252),RGB(215,220,228));	// top path.
					// left corner.
				pDC->SetPixelV(rc.left,rc.top,RGB(93,109,135));
				pDC->SetPixelV(rc.left+1,rc.top,RGB(192,198,206));
				pDC->SetPixelV(rc.left,rc.top+1,RGB(190,196,204));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.top,RGB(93,109,135));
				pDC->SetPixelV(rc.right-2,rc.top,RGB(192,198,206));
				pDC->SetPixelV(rc.right-1,rc.top+1,RGB(190,196,204));
			}
		}
		else if(hover==true)
		{		// draw border.
			CPen pen(PS_SOLID,1,RGB(155,167,183));
			CPen *pOldPen = pDC->SelectObject(&pen);
			pDC->MoveTo(rc.left,rc.bottom);
			pDC->LineTo(rc.left,rc.top);
			pDC->LineTo(rc.right-1,rc.top);
			pDC->LineTo(rc.right-1,rc.bottom);
			pDC->SelectObject(pOldPen);
				// draw back.
			rc.DeflateRect(1,1,1,0);
			Draw::DrawGradient(pDC,&rc,false,RGB(111,119,118),RGB(79,95,116));
				// 
			rc.InflateRect(1,1,1,0);
				// left corner.
			pDC->SetPixelV(rc.left,rc.top,RGB(50,65,93));
			pDC->SetPixelV(rc.left+1,rc.top,RGB(120,135,157));
			pDC->SetPixelV(rc.left,rc.top+1,RGB(117,130,150));
			pDC->SetPixelV(rc.left+1,rc.top+1,RGB(115,126,139));
				// right corner.
			pDC->SetPixelV(rc.right-1,rc.top,RGB(50,65,93));
			pDC->SetPixelV(rc.right-2,rc.top,RGB(120,135,157));
			pDC->SetPixelV(rc.right-1,rc.top+1,RGB(117,130,150));
			pDC->SetPixelV(rc.right-2,rc.top+1,RGB(115,126,139));
		}
	}
	else	// bottom.
	{	if(select==true)
		{	if(active==true)
			{	rc.bottom = (rc.top + rc.bottom)/2;
				Draw::FillSolidRect(pDC,&rc,RGB(255,232,166));	// top path.
				rc.top = rc.bottom;
				rc.bottom = pRect->bottom;
				Draw::DrawGradient(pDC,&rc,false,RGB(255,243,207),RGB(255,252,242));	// bottom path.
					// left corner.
				pDC->SetPixelV(rc.left,rc.bottom-1,RGB(94,109,133));
				pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(195,198,199));
				pDC->SetPixelV(rc.left,rc.bottom-2,RGB(195,197,197));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(94,109,133));
				pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(195,198,199));
				pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(195,197,197));
			}
			else
			{	rc.bottom = (rc.top + rc.bottom)/2;
				Draw::FillSolidRect(pDC,&rc,RGB(206,212,223));	// bottom path.
				rc.top = rc.bottom;
				rc.bottom = pRect->bottom;
				Draw::DrawGradient(pDC,&rc,false,RGB(215,220,228),RGB(251,252,252));	// top path.
					// left corner.
				pDC->SetPixelV(rc.left,rc.bottom-1,RGB(93,109,135));
				pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(192,198,206));
				pDC->SetPixelV(rc.left,rc.bottom-2,RGB(190,196,204));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(93,109,135));
				pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(192,198,206));
				pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(190,196,204));
			}
		}
		else if(hover==true)
		{		// draw border.
			CPen pen(PS_SOLID,1,RGB(155,167,183));
			CPen *pOldPen = pDC->SelectObject(&pen);
			pDC->MoveTo(rc.left,rc.top);
			pDC->LineTo(rc.left,rc.bottom-1);
			pDC->LineTo(rc.right-1,rc.bottom-1);
			pDC->LineTo(rc.right-1,rc.top);
			pDC->SelectObject(pOldPen);
				// draw back.
			rc.DeflateRect(1,0,1,1);
			Draw::DrawGradient(pDC,&rc,false,RGB(79,95,116),RGB(111,119,118));
				// 
			rc.InflateRect(1,0,1,1);
				// left corner.
			pDC->SetPixelV(rc.left,rc.bottom-1,RGB(50,65,93));
			pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(120,135,157));
			pDC->SetPixelV(rc.left,rc.bottom-2,RGB(117,130,150));
			pDC->SetPixelV(rc.left+1,rc.bottom-2,RGB(115,126,139));
				// right corner.
			pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(50,65,93));
			pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(120,135,157));
			pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(117,130,150));
			pDC->SetPixelV(rc.right-2,rc.bottom-2,RGB(115,126,139));
		}
	}
}
// 
void TabCtrlStyle_VS2010_client::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	if(pCtrl->GetCount()==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	bool active = pCtrl->IsActive();
		bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		DrawWndsAreaBack(pCtrl,pDC,pRect,top,active);
	}
}
// 
void TabCtrlStyle_VS2010_client::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool top, bool active)
{	CRect rc;
	HTAB firstTab = (pCtrl->GetCount()>0 ? pCtrl->GetTab(0) : NULL);
	bool selectFirstTab = (pCtrl->GetSel()==firstTab);
	bool scaleMode = (pCtrl->GetBehavior()==TAB_BEHAVIOR_SCALE);
	COLORREF clr = (active==true ? RGB(255,232,166) : RGB(206,212,223));
		// 
		// draw top.
	rc.SetRect(pRect->left,pRect->top,pRect->right,pRect->top+4);
	Draw::FillSolidRect(pDC,&rc,clr);
		// 
	if(top==false || scaleMode==true)
	{		// left corner.
		if(top==false || selectFirstTab==false || (pCtrl->GetCount()==1 && pCtrl->IsHideSingleTab()==true))
			if(top==true && pCtrl->GetHover()==firstTab)	// tab is highlight.
			{	if(active==true)
				{	pDC->SetPixelV(rc.left,rc.top,RGB(175,180,180));
					pDC->SetPixelV(rc.left+1,rc.top,RGB(205,194,152));
					pDC->SetPixelV(rc.left,rc.top+1,RGB(227,214,171));
				}
				else
				{	pDC->SetPixelV(rc.left,rc.top,RGB(165,176,191));
					pDC->SetPixelV(rc.left+1,rc.top,RGB(170,179,193));
					pDC->SetPixelV(rc.left,rc.top+1,RGB(192,199,212));
				}
			}
			else	// tab isn't highlight.
				if(active==true)
				{	pDC->SetPixelV(rc.left,rc.top,RGB(86,95,105));
					pDC->SetPixelV(rc.left+1,rc.top,RGB(198,188,149));
					pDC->SetPixelV(rc.left,rc.top+1,RGB(195,184,144));
				}
				else
				{	pDC->SetPixelV(rc.left,rc.top,RGB(76,91,116));
					pDC->SetPixelV(rc.left+1,rc.top,RGB(163,173,190));
					pDC->SetPixelV(rc.left,rc.top+1,RGB(160,169,185));
				}
			// right corner.
		if(active==true)
		{	pDC->SetPixelV(rc.right-1,rc.top,RGB(100,106,109));
			pDC->SetPixelV(rc.right-2,rc.top,RGB(205,192,148));
			pDC->SetPixelV(rc.right-1,rc.top+1,RGB(195,184,144));
		}
		else
		{	pDC->SetPixelV(rc.right-1,rc.top,RGB(87,100,124));
			pDC->SetPixelV(rc.right-2,rc.top,RGB(168,176,192));
			pDC->SetPixelV(rc.right-1,rc.top+1,RGB(160,169,185));
		}
	}
		// 
		// draw bottom.
	rc.SetRect(pRect->left,pRect->bottom-4,pRect->right,pRect->bottom);
	Draw::FillSolidRect(pDC,&rc,clr);
		// 
	if(top==true || scaleMode==true)
	{		// left corner.
		if(top==true || selectFirstTab==false || (pCtrl->GetCount()==1 && pCtrl->IsHideSingleTab()==true))
			if(top==false && pCtrl->GetHover()==firstTab)	// tab is highlight.
			{	if(active==true)
				{	pDC->SetPixelV(rc.left,rc.bottom-1,RGB(175,180,180));
					pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(205,194,152));
					pDC->SetPixelV(rc.left,rc.bottom-2,RGB(227,214,171));
				}
				else
				{	pDC->SetPixelV(rc.left,rc.bottom-1,RGB(165,176,191));
					pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(170,179,193));
					pDC->SetPixelV(rc.left,rc.bottom-2,RGB(192,199,212));
				}
			}
			else	// tab isn't highlight.
				if(active==true)
				{	pDC->SetPixelV(rc.left,rc.bottom-1,RGB(86,95,105));
					pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(198,188,149));
					pDC->SetPixelV(rc.left,rc.bottom-2,RGB(195,184,144));
				}
				else
				{	pDC->SetPixelV(rc.left,rc.bottom-1,RGB(76,91,116));
					pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(163,173,190));
					pDC->SetPixelV(rc.left,rc.bottom-2,RGB(160,169,185));
				}
			// right corner.
		if(active==true)
		{	pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(100,106,109));
			pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(205,192,148));
			pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(195,184,144));
		}
		else
		{	pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(87,100,124));
			pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(168,176,192));
			pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(160,169,185));
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2010_client::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed || (bHover==true && bPushed==true))
	{	CRect rc(pRect);
		Draw::DrawRect(pDC,&rc,RGB(229,195,101));
			// 
		rc.DeflateRect(1,1);
		if(bHover==true && bPushed==true)
			Draw::FillSolidRect(pDC,&rc,RGB(255,232,166));
		else
			Draw::FillSolidRect(pDC,&rc,RGB(255,252,244));
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2010_client::GetCtrlAreaBackColor(TabCtrl * /*pCtrl*/)
{	return RGB(46,64,94);
}
// 
COLORREF TabCtrlStyle_VS2010_client::GetTabTextColor(TabCtrl *pCtrl, HTAB hTab)
{	return (pCtrl->GetSel()==hTab ? RGB(13,0,5) : RGB(248,255,255));
}
// 
COLORREF TabCtrlStyle_VS2010_client::GetEmptyWndsAreaBackColor(TabCtrl * /*pCtrl*/)
{	return RGB(46,64,94);
}
// 
COLORREF TabCtrlStyle_VS2010_client::GetButtonMarkerColor(TabCtrl * /*pCtrl*/, bool bHover, bool bPushed)
{	return ((bHover==true || bPushed==true) ? RGB(0,0,0) : RGB(206,212,221));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2010_client_custom1.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2010_client_custom1::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	if(pCtrl->IsDisable(hTab)==false)
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		bool select = (pCtrl->GetSel()==hTab);
		bool hover = (pCtrl->GetHover()==hTab);
			// 
		TabCtrlStyle_VS2010_client::DrawTabBack(pCtrl,pDC,pRect,top,true,select,hover);
	}
}
// 
void TabCtrlStyle_VS2010_client_custom1::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	if(pCtrl->GetCount()==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		TabCtrlStyle_VS2010_client::DrawWndsAreaBack(pCtrl,pDC,pRect,top,true);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2010_client_custom2.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2010_client_custom2::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	if(pCtrl->IsDisable(hTab)==false)
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		bool select = (pCtrl->GetSel()==hTab);
		bool hover = (pCtrl->GetHover()==hTab);
			// 
		TabCtrlStyle_VS2010_client::DrawTabBack(pCtrl,pDC,pRect,top,false,select,hover);
	}
}
// 
void TabCtrlStyle_VS2010_client_custom2::DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	if(pCtrl->GetCount()==0 || pCtrl->GetSel()==NULL)
		TabCtrlStyle_base::DrawWndsAreaBack(pCtrl,pDC,pRect);
	else
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		TabCtrlStyle_VS2010_client::DrawWndsAreaBack(pCtrl,pDC,pRect,top,false);
	}
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlStyle_VS2010_bars.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
ITabCtrlRecalc *TabCtrlStyle_VS2010_bars::GetRecalcManager()
{	return this;
}
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlStyle_VS2010_bars::GetBorderWidth(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return 0;
}
// 
CRect TabCtrlStyle_VS2010_bars::GetCtrlAreaPadding(TabCtrl * /*pCtrl*/, ITabCtrlRecalc * /*base*/)
{	return CRect(0,0,3,0);
}
// 
CRect TabCtrlStyle_VS2010_bars::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc * /*base*/)
{	return (pCtrl->GetLayout()==TAB_LAYOUT_TOP ? 
		CRect(5,1/*border*/+1,5,2) : CRect(5,2,5,1+1/*border*/));
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2010_bars::DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect)
{	Draw::FillSolidRect(pDC,pRect,GetCtrlAreaBackColor(pCtrl));
}
// 
void TabCtrlStyle_VS2010_bars::DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn * /*pRgn*/)
{	if(pCtrl->IsDisable(hTab)==false)
	{	bool top = (pCtrl->GetLayout()==TAB_LAYOUT_TOP);
		bool select = (pCtrl->GetSel()==hTab);
		bool hover = (pCtrl->GetHover()==hTab);
			// 
		CRect rc(pRect);
			// 
		if(top==true)
		{	if(select==true)
			{	Draw::FillSolidRect(pDC,&rc,RGB(255,255,255));	// back.
					// left corner.
				pDC->SetPixelV(rc.left,rc.top,RGB(103,116,138));
				pDC->SetPixelV(rc.left+1,rc.top,RGB(221,224,228));
				pDC->SetPixelV(rc.left,rc.top+1,RGB(197,202,210));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.top,RGB(103,116,138));
				pDC->SetPixelV(rc.right-2,rc.top,RGB(221,224,228));
				pDC->SetPixelV(rc.right-1,rc.top+1,RGB(197,202,210));
			}
			else if(hover==true)
			{		// draw border.
				CPen pen(PS_SOLID,1,RGB(155,167,183));
				CPen *pOldPen = pDC->SelectObject(&pen);
				pDC->MoveTo(rc.left,rc.top);
				pDC->LineTo(rc.left,rc.bottom-1);
				pDC->LineTo(rc.right-1,rc.bottom-1);
				pDC->LineTo(rc.right-1,rc.top);
				pDC->LineTo(rc.left,rc.top);
				pDC->SelectObject(pOldPen);
					// draw back.
				rc.DeflateRect(1,1);
				Draw::DrawGradient(pDC,&rc,false,RGB(76,92,116),RGB(111,119,118));
					// 
				rc.InflateRect(1,1);
					// left corner.
				pDC->SetPixelV(rc.left,rc.top,RGB(50,65,93));
				pDC->SetPixelV(rc.left+1,rc.top,RGB(120,135,157));
				pDC->SetPixelV(rc.left,rc.top+1,RGB(117,130,150));
				pDC->SetPixelV(rc.left+1,rc.top+1,RGB(115,126,139));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.top,RGB(50,65,93));
				pDC->SetPixelV(rc.right-2,rc.top,RGB(120,135,157));
				pDC->SetPixelV(rc.right-1,rc.top+1,RGB(117,130,150));
				pDC->SetPixelV(rc.right-2,rc.top+1,RGB(115,126,139));
			}
		}
		else	// bottom.
		{	if(select==true)
			{	Draw::FillSolidRect(pDC,&rc,RGB(255,255,255));	// back.
					// left corner.
				pDC->SetPixelV(rc.left,rc.bottom-1,RGB(103,116,138));
				pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(221,224,228));
				pDC->SetPixelV(rc.left,rc.bottom-2,RGB(197,202,210));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(103,116,138));
				pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(221,224,228));
				pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(197,202,210));
			}
			else if(hover==true)
			{		// draw border.
				CPen pen(PS_SOLID,1,RGB(155,167,183));
				CPen *pOldPen = pDC->SelectObject(&pen);
				pDC->MoveTo(rc.left,rc.top);
				pDC->LineTo(rc.left,rc.bottom-1);
				pDC->LineTo(rc.right-1,rc.bottom-1);
				pDC->LineTo(rc.right-1,rc.top);
				pDC->LineTo(rc.left,rc.top);
				pDC->SelectObject(pOldPen);
					// draw back.
				rc.DeflateRect(1,1);
				Draw::DrawGradient(pDC,&rc,false,RGB(111,119,118),RGB(76,92,116));
					// 
				rc.InflateRect(1,1);
					// left corner.
				pDC->SetPixelV(rc.left,rc.bottom-1,RGB(50,65,93));
				pDC->SetPixelV(rc.left+1,rc.bottom-1,RGB(120,135,157));
				pDC->SetPixelV(rc.left,rc.bottom-2,RGB(117,130,150));
				pDC->SetPixelV(rc.left+1,rc.bottom-2,RGB(115,126,139));
					// right corner.
				pDC->SetPixelV(rc.right-1,rc.bottom-1,RGB(50,65,93));
				pDC->SetPixelV(rc.right-2,rc.bottom-1,RGB(120,135,157));
				pDC->SetPixelV(rc.right-1,rc.bottom-2,RGB(117,130,150));
				pDC->SetPixelV(rc.right-2,rc.bottom-2,RGB(115,126,139));
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
void TabCtrlStyle_VS2010_bars::DrawButtonFrame(TabCtrl * /*pCtrl*/, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed)
{	if(bHover!=bPushed || (bHover==true && bPushed==true))
	{	CRect rc(pRect);
		Draw::DrawRect(pDC,&rc,RGB(229,195,101));
			// 
		rc.DeflateRect(1,1);
		if(bHover==true && bPushed==true)
			Draw::FillSolidRect(pDC,&rc,RGB(255,232,166));
		else
			Draw::FillSolidRect(pDC,&rc,RGB(255,252,244));
	}
}
/////////////////////////////////////////////////////////////////////////////
// 
COLORREF TabCtrlStyle_VS2010_bars::GetCtrlAreaBackColor(TabCtrl * /*pCtrl*/)
{	return RGB(46,64,94);
}
// 
COLORREF TabCtrlStyle_VS2010_bars::GetTabTextColor(TabCtrl *pCtrl, HTAB hTab)
{	return (pCtrl->GetSel()==hTab ? RGB(13,0,5) : RGB(248,255,255));
}
// 
COLORREF TabCtrlStyle_VS2010_bars::GetEmptyWndsAreaBackColor(TabCtrl * /*pCtrl*/)
{	return RGB(46,64,94);
}
// 
COLORREF TabCtrlStyle_VS2010_bars::GetButtonMarkerColor(TabCtrl * /*pCtrl*/, bool bHover, bool bPushed)
{	return ((bHover==true || bPushed==true) ? RGB(0,0,0) : RGB(206,212,221));
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlRecalcStub.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
int TabCtrlRecalcStub::GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetBorderWidth(pCtrl,NULL);
}
	// 
CRect TabCtrlRecalcStub::GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetCtrlAreaPadding(pCtrl,NULL);
}
CRect TabCtrlRecalcStub::GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetWndsAreaPadding(pCtrl,NULL);
}
	// 
CRect TabCtrlRecalcStub::GetTabHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetTabHorzMargin(pCtrl,NULL);
}
CRect TabCtrlRecalcStub::GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetTabPadding(pCtrl,NULL);
}
int TabCtrlRecalcStub::GetTabImageTextGap(TabCtrl *pCtrl, ITabCtrlRecalc *base)	// IMAGE<- gap ->TEXT.
{	return base->GetTabImageTextGap(pCtrl,NULL);
}
int TabCtrlRecalcStub::GetTabExtraWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base, HTAB hTab)
{	return base->GetTabExtraWidth(pCtrl,NULL,hTab);
}
int TabCtrlRecalcStub::GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetTabMinWidth(pCtrl,NULL);
}
	// 
CRect TabCtrlRecalcStub::GetLeftScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetLeftScrollButtonHorzMargin(pCtrl,NULL);
}
CRect TabCtrlRecalcStub::GetRightScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetRightScrollButtonHorzMargin(pCtrl,NULL);
}
CRect TabCtrlRecalcStub::GetMenuButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetMenuButtonHorzMargin(pCtrl,NULL);
}
CRect TabCtrlRecalcStub::GetCloseButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base)
{	return base->GetCloseButtonHorzMargin(pCtrl,NULL);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TabCtrlBehaviorStub.
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
HTAB TabCtrlBehaviorStub::HitTest(TabCtrl *pCtrl, ITabCtrlBehavior *base, CPoint point)
{	return base->HitTest(pCtrl,NULL,point);
}
// 
bool TabCtrlBehaviorStub::SetCursor(TabCtrl *pCtrl, ITabCtrlBehavior *base)
{	return base->SetCursor(pCtrl,NULL);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


























