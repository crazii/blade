//==========================================================
// Author: Borodenko Oleg
// 05/08/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include "Tools.h"
/////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4290)	// C++ exception specification ignored...
#pragma warning(disable : 4265)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
typedef struct _HTAB {} *HTAB;
// 
enum TAB_LAYOUT
{	TAB_LAYOUT_TOP,
	TAB_LAYOUT_BOTTOM
};
// 
enum TAB_BEHAVIOR
{	TAB_BEHAVIOR_SCALE,
	TAB_BEHAVIOR_SCROLL
};
/////////////////////////////////////////////////////////////////////////////
// 
class TabCtrl;
// 
struct TabCtrlDraw
{	virtual bool IsStraightOrder(TabCtrl * /*pCtrl*/) { return true; }	// true - paint tabs left to right, false - reverse order.
	virtual void DrawBegin(TabCtrl * /*pCtrl*/, CDC * /*pDC*/) {}
	virtual void DrawBorder(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/) {}
	virtual void DrawCtrlAreaBack(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/) {}
	virtual void DrawTab(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, HTAB /*hTab*/, CRgn * /*pRgn*/) {}
	virtual void DrawButtonClose(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/, bool /*bHover*/, bool /*bPushed*/) {}
	virtual void DrawButtonMenu(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/, bool /*bHover*/, bool /*bPushed*/, bool /*bPartialView*/) {}
	virtual void DrawButtonScrollLeft(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/, bool /*bHover*/, bool /*bPushed*/, bool /*bPartialView*/) {}
	virtual void DrawButtonScrollRight(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/, bool /*bHover*/, bool /*bPushed*/, bool /*bPartialView*/) {}
	virtual void DrawWndsAreaBack(TabCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/) {}
	virtual void DrawEnd(TabCtrl * /*pCtrl*/, CDC * /*pDC*/) {}
};
// 
interface ITabCtrlRecalc
{	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;
		// 
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;
		// 
	virtual CRect GetTabHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// uses only CRect::left and CRect::right.
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;
	virtual int GetTabImageTextGap(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// IMAGE<- gap ->TEXT.
	virtual int GetTabExtraWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base, HTAB hTab) = 0;	// additional width of tab.
	virtual int GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// minimal width of tab.
		// 
	virtual CRect GetLeftScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// uses only CRect::left and CRect::right.
	virtual CRect GetRightScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// uses only CRect::left and CRect::right.
	virtual CRect GetMenuButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// uses only CRect::left and CRect::right.
	virtual CRect GetCloseButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base) = 0;	// uses only CRect::left and CRect::right.
};
// 
interface ITabCtrlBehavior
{	virtual HTAB HitTest(TabCtrl *pCtrl, ITabCtrlBehavior *base, CPoint point) = 0;		// get tab in the given point.
	virtual bool SetCursor(TabCtrl *pCtrl, ITabCtrlBehavior *base) = 0;	// return true if you set cursor.
};
// 
interface ITabCtrlToolTip
{	virtual CToolTipCtrl *CreateToolTip(TabCtrl *pCtrl, ITabCtrlToolTip *base) = 0;
};
// 
struct TabCtrlUserAbility
{	virtual bool CanShowCloseButton(TabCtrl * /*pCtrl*/) { return true; }
	virtual bool CanShowMenuButton(TabCtrl * /*pCtrl*/) { return true; }
	virtual bool CanShowScrollButtons(TabCtrl * /*pCtrl*/) { return true; }
};
// 
struct TabCtrlNotify
{	virtual void OnTabPreCreate(TabCtrl * /*pCtrl*/, HWND /*hWnd*/, TCHAR const * /*text*/, int /*image*/) {}
	virtual void OnTabPostCreate(TabCtrl * /*pCtrl*/, HTAB /*hTab*/) {}
	virtual void OnTabPreDestroy(TabCtrl * /*pCtrl*/, HTAB /*hTab*/) {}
		// 
	virtual void OnCloseButtonClicked(TabCtrl * /*pCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnMenuButtonClicked(TabCtrl * /*pCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnTabSelected(TabCtrl * /*pCtrl*/, HTAB /*hTab*/) {}
	virtual void OnLButtonDown(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnLButtonDblClk(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnRButtonDown(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}		// ptScr - in screen space, hTab can be NULL.
	virtual void OnRButtonUp(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space, hTab can be NULL.
		// 
	virtual void OnStartDrag(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnDrag(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/, bool /*outside*/) {}	// ptScr - in screen space, outside==true - dragging out of tabs area.
	virtual void OnFinishDrag(TabCtrl * /*pCtrl*/, HTAB /*hTab*/, bool /*cancel*/) {}	// cancel==false - dragging was finished using left button up.
};
/////////////////////////////////////////////////////////////////////////////
// 
interface ITabCtrlStyle
{	virtual TabCtrlDraw *GetDrawManager() = 0;
	virtual ITabCtrlRecalc *GetRecalcManager() = 0;
	virtual ITabCtrlBehavior *GetBehaviorManager() = 0;
	virtual ITabCtrlToolTip *GetToolTipManager() = 0;
};
/////////////////////////////////////////////////////////////////////////////
// 
class TabCtrl : public CWnd, 
	private TabCtrlUserAbility,
	private ActivityHook::INotify, private KeyboardHook::Notify,
	public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	TabCtrl();
	~TabCtrl();

///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
public:
	bool Create(CWnd *parent, DWORD dwStyle, RECT const &rect, UINT nID);
	HTAB Add(HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc);	// 'image'=-1 for tab without image.
	HTAB Insert(HTAB hBefore, HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc);	// 'image'=-1 for tab without image.
	void RemoveBefore(HTAB hBefore, HTAB hSrc) throw(std::bad_alloc);
	void RemoveAfter(HTAB hAfter, HTAB hSrc) throw(std::bad_alloc);
	void Delete(HTAB hTab);
	void DeleteAll();
		// 
	void Update(bool redraw = true);	// recalculate and redraw control.
		// 
	void InstallStyle(ITabCtrlStyle *style);
		// 
	void SetDrawManager(TabCtrlDraw *p);
	TabCtrlDraw *GetDrawManager() const;
	void SetRecalcManager(ITabCtrlRecalc *p);	// or NULL for default manager.
	ITabCtrlRecalc *GetRecalcManager() const;
	void SetBehaviorManager(ITabCtrlBehavior *p);	// or NULL for default manager.
	ITabCtrlBehavior *GetBehaviorManager() const;
	void SetToolTipManager(ITabCtrlToolTip *p);	// or NULL for default manager.
	ITabCtrlToolTip *GetToolTipManager() const;
	void SetAbilityManager(TabCtrlUserAbility *p);	// or NULL for default manager.
	TabCtrlUserAbility *GetAbilityManager() const;
	void SetNotifyManager(TabCtrlNotify *p);
	TabCtrlNotify *GetNotifyManager() const;
		// 
	void SetLayout(TAB_LAYOUT layout);
	TAB_LAYOUT GetLayout() const;
	void SetBehavior(TAB_BEHAVIOR behavior);
	TAB_BEHAVIOR GetBehavior() const;
		// 
	bool SetImageLists(CImageList *pImageList, CImageList *pImageListDisabled);
	void SetImageListsRef(CImageList *pImageList, CImageList *pImageListDisabled);	// set references to other ImageLists.
	void GetImageLists(CImageList **pImageList/*out*/, CImageList **pImageListDisabled/*out*/);	// 'pImageList' or 'pImageListDisabled' ban be NULL.
	void GetImageSizes(CSize *pszImage/*out*/, CSize *pszImageDisabled/*out*/) const;	// 'pszImage' or 'pszImageDisabled' can be NULL.
		// 
	bool SetSystemImageList(CImageList *pImageList);	// system images must have the same order as IMAGE enum (see below).
	void SetSystemImageListRef(CImageList *pImageList);	// set reference to another ImageList.
	CImageList *GetSystemImageList() const;
	CSize GetSystemImageSize() const;
		// 
	bool SetCursor(UINT uId);
	bool SetCursor(HMODULE hModule, UINT uId);
	bool SetCursor(HCURSOR hCursor);
	void SetCursorRef(HCURSOR *phCursor);	// set reference to another cursor.
	HCURSOR GetCursor() const;
		// 
	bool SetFont(CFont *pFont);
	void SetFontRef(CFont *pFont);	// set reference to another font.
	bool SetFont(LOGFONT const *pLf);
	CFont *GetFont();
		// 
	bool SetFontSelect(CFont *pFont);
	void SetFontSelectRef(CFont *pFont);	// set reference to another font.
	bool SetFontSelect(LOGFONT const *pLf);
	CFont *GetFontSelect();
		// 
	void ToolTipEnable(bool enable);
	bool IsToolTipEnable() const;
	void SetTabTooltipText(HTAB hTab, TCHAR const *text) throw(std::bad_alloc);
	CString GetTabTooltipText(HTAB hTab) const;
	void SetCloseButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetCloseButToolTipText() const;
	void SetMenuButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetMenuButToolTipText() const;
	void SetLeftScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetLeftScrollButToolTipText() const;
	void SetRightScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetRightScrollButToolTipText() const;
		// 
	void SetTabText(HTAB hTab, TCHAR const *text) throw(std::bad_alloc);
	CString GetTabText(HTAB hTab) const;
	void SetTabImage(HTAB hTab, int image);	// 'image'=-1 for tab without image.
	int GetTabImage(HTAB hTab) const;
	void SetTabWnd(HTAB hTab, HWND hWnd);
	HWND GetTabWnd(HTAB hTab) const;
	void SetTabData(HTAB hTab, __int64 data);	// set any user data for the tab.
	__int64 GetTabData(HTAB hTab) const;
		// 
	void CopyTab(HTAB hDst, TabCtrl const *pTabCtrlSrc, HTAB hSrc) throw(std::bad_alloc);	// copy: text, image, data, tooltip text and disable/enable state.
		// 
	int GetCount() const;	// get number of tabs in the control.
	HTAB GetTab(int idx) const;	// idx - index of tab (>=0).
		// 
	HTAB GetFirstEnableTab() const;
	HTAB GetPrevEnableTab(HTAB hTab) const;
	HTAB GetNextEnableTab(HTAB hTab) const;
		// 
	void SetSel(HTAB hTab);	// select tab.
	HTAB GetSel() const;		// get handle of current active tab (whose child window is visible).
	HTAB GetHover() const;	// get tab under cursor.
	HTAB GetPushed() const;	// get handle of pushed tab.
		// 
	void Disable(HTAB hTab, bool disable) const;	// disable/enable tab.
	bool IsDisable(HTAB hTab) const;
		// 
	HTAB HitTest(CPoint point) const;	// get tab in the given point or NULL, 'point' - in screen space.
	int GetIndex(HTAB hTab) const;	// get index of tab (return value is >=0).
	bool IsExist(HTAB hTab) const;	// return true - tab with this handle exists in the control.
	HTAB GetTabWithWindow(HWND hWnd) const;
	int CmpTabs(HTAB hTab1, HTAB hTab2) const;	// return <0 - index of hTab1 less than hTab2, 0 - indexes are equal.
		// 
	void GetTabRect(HTAB hTab, RECT *rc/*out*/) const;
	bool IsTabVisible(HTAB hTab, bool *partially/*out*/) const;	// 'partially' can be NULL.
	void EnsureVisible(HTAB hTab);	// shift tab in a visible area (only for the TAB_BEHAVIOR_SCROLL mode).
		// 
	void ScrollToBegin();	// shift to show first (left) tab (only for the TAB_BEHAVIOR_SCROLL mode).
	void ScrollToEnd();	// shift to show last (right) tab (only for the TAB_BEHAVIOR_SCROLL mode).
		// 
	void SetScrollingStep(int step);	// width of one step for scrolling (in pixels >=1) (only for the TAB_BEHAVIOR_SCROLL mode).
	int GetScrollingStep() const;
		// 
	void ShowBorder(bool show);	// border is visible if IsShowBorder()==true and ITabCtrlRecalc::GetBorderWidth(...) returns >0.
	bool IsShowBorder() const;
	void EqualTabsSize(bool equal);	// true - all tabs will have the same width.
	bool IsEqualTabsSize() const;
	void RemoveTabEnable(bool enable);	// true - you can change positions of tabs using mouse.
	bool IsRemoveTabEnable() const;
	void HideSingleTab(bool hide);	// true - hide control area if control has one tab. 
	bool IsHideSingleTab() const;
		// 
	void ShowCloseButton(bool show);
	bool IsShowCloseButton() const;
	void ShowMenuButton(bool show);
	bool IsShowMenuButton() const;
	void ShowScrollButtons(bool show);
	bool IsShowScrollButtons() const;
		// 
	bool IsDragging() const;	// return true - user is removing a tab.
	void CancelDragging();
		// 
	void WatchActivityCtrl(bool watch);	// true - control saves activity state which you can get using IsActive().
	bool IsWatchActivityCtrl() const;
		// 
	bool IsActive() const;	// return true - one of child windows is active (has focus), works only if IsWatchActivityCtrl()==true.
		// 
	RECT GetCtrlArea() const;	// control area includes tabs area and control (system) buttons.
	RECT GetTabsArea() const;
	RECT GetWindowsArea() const;	// returns area for child windows.
		// 
	int CalcCtrlAreaHeight();	// return necessary height of control area.
		// 
	RECT GetCloseButtonRect() const;
	RECT GetMenuButtonRect() const;
	RECT GetScrollLeftRect() const;
	RECT GetScrollRightRect() const;
		// 
	bool LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry);	// load state from registry.
	bool SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const;	// save state in registry.
	bool LoadState(CArchive *ar);
	bool SaveState(CArchive *ar) const;

public:	// functions of ITabCtrlRecalc interface, return information from current recalc manager.
	int GetBorderWidth();
		// 
	CRect GetCtrlAreaPadding();
	CRect GetWndsAreaPadding();
		// 
	CRect GetTabHorzMargin();
	CRect GetTabPadding();
	int GetTabImageTextGap();	// IMAGE<- gap ->TEXT.
	int GetTabExtraWidth(HTAB hTab);
	int GetTabMinWidth();
		// 
	CRect GetCloseButtonHorzMargin();
	CRect GetMenuButtonHorzMargin();
	CRect GetLeftScrollButtonHorzMargin();
	CRect GetRightScrollButtonHorzMargin();

public:
	CToolTipCtrl *GetToolTip() const;	// get used tooltip object (NULL if tooltip wasn't created).

public:
	enum IMAGE	// id of images of system buttons (close, menu, left scroll, right scroll).
	{	IMAGE_CLOSE,
		IMAGE_MENU, IMAGE_MENU_PARTIAL,
		IMAGE_ARROW_LEFT, IMAGE_ARROW_LEFT_DISABLE,
		IMAGE_ARROW_RIGHT, IMAGE_ARROW_RIGHT_DISABLE
	};

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	CImageList m_ImageList, m_ImageListDisabled;
	CSize m_szImage, m_szImageDisabled;
	CImageList m_SysImageList;
	CSize m_szSysImage;
	CFont m_Font, m_FontSelect;
	HCURSOR m_hCursor;
	CString m_sToolTip;
	CString m_sToolTipButClose, m_sToolTipButMenu, m_sToolTipButLeftScroll,m_sToolTipButRightScroll;
		// 

	CImageList* m_pImageListRef;
	CImageList* m_pImageListDisabledRef;
	CImageList* m_pSysImageListRef;
	CFont* m_pFontRef;
	CFont* m_pFontSelectRef;
	HCURSOR* m_phCursorRef;

	TabCtrlDraw *m_pDrawManager;
	ITabCtrlRecalc *m_pRecalcManager;
	ITabCtrlBehavior *m_pBehaviorManager;
	ITabCtrlToolTip *m_pToolTipManager;
	TabCtrlUserAbility *m_pAbilityManager;
	TabCtrlNotify *m_pNotifyManager;
		// 
	const HTAB m_hButClose, m_hButMenu, m_hButLeftScroll,m_hButRightScroll;

	int m_iScrollingStep;
	bool m_bShowBorder;
	bool m_bEqualTabsSize;
	bool m_bRemoveTabEnable;
	bool m_bHideSingleTab;
	bool m_bShowCloseButton;
	bool m_bShowMenuButton;
	bool m_bShowScrollButtons;
	bool m_bToolTipEnable;
	// 
	TAB_LAYOUT m_Layout;
	TAB_BEHAVIOR m_Behavior;
private:
	struct Tab : public _HTAB , public Blade::Allocatable
	{	HWND hWnd;
		int image;
		CString text, tooltipText;
		bool disable;
		__int64 data;
			// 
		CRect rc;
		int width;
	};
		// 
	HTAB m_hCurTab;
	HTAB m_hHoverArea, m_hPushedArea;
	int m_iTabsOffset, m_iMaxTabsOffset;

	CPoint m_ptStartTabDrag;
		// 
	// 
	bool m_bPartialView, m_bScrollLeftAllow,m_bScrollRightAllow;
	bool m_bTabDrag;
	bool m_bWatchActivityCtrl;
	bool m_bActive;
		// 
	CRect m_rcCtrlArea, m_rcTabs, m_rcWindows;
	CRect m_rcCloseButton, m_rcMenuButton, m_rcScrollLeft,m_rcScrollRight;
		// 
	CToolTipCtrl *m_pToolTip;
	Blade::Vector<Tab *> m_vtrtabs;
	typedef Blade::Vector<Tab *>::iterator m_i_vtrtabs;
	typedef Blade::Vector<Tab *>::reverse_iterator m_ri_vtrtabs;
	typedef Blade::Vector<Tab *>::const_iterator m_ci_vtrtabs;
		// 
	ActivityHook m_ActivityHook;
	KeyboardHook m_KeyboardHook;
	Blade::Vector<Tab *> m_vtrtabs_dragstore;
		// 
	bool *m_pLifeStatus;

private:
	struct Service : public ITabCtrlRecalc, public ITabCtrlBehavior, public ITabCtrlToolTip
	{		// ITabCtrlRecalc.
		virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
			// 
		virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
			// 
		virtual CRect GetTabHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual int GetTabImageTextGap(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual int GetTabExtraWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base, HTAB hTab);
		virtual int GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
			// 
		virtual CRect GetLeftScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual CRect GetRightScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual CRect GetMenuButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		virtual CRect GetCloseButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
			// 
			// ITabCtrlBehavior.
		virtual HTAB HitTest(TabCtrl *pCtrl, ITabCtrlBehavior *base, CPoint point);
		virtual bool SetCursor(TabCtrl *pCtrl, ITabCtrlBehavior *base);
			// 
			// ITabCtrlToolTip.
		virtual CToolTipCtrl *CreateToolTip(TabCtrl *pCtrl, ITabCtrlToolTip *base);
			// 
		CToolTipCtrl m_ToolTipCtrl;
	} m_Service;

private:
		// ActivityHook::INotify.
	virtual void OnActive(bool active, HWND wnd);
		// KeyboardHook::Notify.
	virtual void OnKeyDown(UINT keyCode, UINT msgFlag);

private:
	HTAB Insert(m_i_vtrtabs before, HWND hWnd, TCHAR const *text, int image) throw(std::bad_alloc);
	void Recalc(bool redraw);	// recalculate control.
	int GetFullTabsWidth();
	void RecalcScale(int iVisibleTabsWidth, int iFullTabsWidth);
	void RecalcScroll();
	void CalcTabsWidth();
	int CalcTabWidth(HTAB hTab);
	HTAB GetTabWithWindow(HWND hWnd, HTAB exceptTab) const;
	bool GetTabAndIndex(int id, HTAB *tab/*out*/, int *idx/*out*/) const;
	void LButtonDown(CPoint point);
	bool IsSystemButton(HTAB hTab) const;
	void StepLeft();
	void StepRight();
	void StopScrolling();
	void StartDragging(CPoint point);
	void StopDragging(bool cancel);
	void AssignHoverArea(CPoint point);
	void SetFocusInChildWnd();
	bool LoadStateInner(CArchive *ar);
	void SaveStateInner(CArchive *ar) const;
	void TtnNeedText(NMTTDISPINFO *pTT);

private:
	DECLARE_DYNCREATE(TabCtrl)

///////////////////////////////////////
// PROTECTED
///////////////////////////////////////
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	LRESULT OnMouseLeave(WPARAM wp, LPARAM lp);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlRecalcStub : public ITabCtrlRecalc
{	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		// 
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		// 
	virtual CRect GetTabHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual int GetTabImageTextGap(TabCtrl *pCtrl, ITabCtrlRecalc *base);	// IMAGE<- gap ->TEXT.
	virtual int GetTabExtraWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base, HTAB hTab);
	virtual int GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
		// 
	virtual CRect GetLeftScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetRightScrollButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetMenuButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetCloseButtonHorzMargin(TabCtrl *pCtrl, ITabCtrlRecalc *base);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlBehaviorStub : public ITabCtrlBehavior
{	virtual HTAB HitTest(TabCtrl *pCtrl, ITabCtrlBehavior *base, CPoint point);		// get tab in the given point.
	virtual bool SetCursor(TabCtrl *pCtrl, ITabCtrlBehavior *base);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
struct TabCtrlStyle_base : public ITabCtrlStyle, public TabCtrlDraw
{		// ITabCtrlStyle.
	virtual TabCtrlDraw *GetDrawManager();
	virtual ITabCtrlRecalc *GetRecalcManager();
	virtual ITabCtrlBehavior *GetBehaviorManager();
	virtual ITabCtrlToolTip *GetToolTipManager();

protected:
		// TabCtrlDraw.
	virtual void DrawBorder(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawTab(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRgn *pRgn);
	virtual void DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
	virtual void DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:
	virtual COLORREF GetBorderColor(TabCtrl *pCtrl);
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl);
	virtual COLORREF GetCtrlAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetWndsAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetTabSelectedBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetTabTextColor(TabCtrl *pCtrl, HTAB hTab);
	virtual COLORREF GetButtonMarkerColor(TabCtrl *pCtrl, bool bHover, bool bPushed);
	virtual COLORREF GetChildWndBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetEmptyWndsAreaBackColor(TabCtrl *pCtrl);

protected:
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);	// draw background of tab.
	virtual void DrawTabContext(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);	// draw image and text.
	virtual void DrawTabImage(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual void DrawTabText(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);	// draw close, menu or scroll button without image.
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2003_base : public TabCtrlStyle_base, public TabCtrlRecalcStub
{		// ITabCtrlStyle.
	virtual ITabCtrlRecalc *GetRecalcManager();

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
	virtual void DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
		// 
	virtual COLORREF GetCtrlAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetTabTextColor(TabCtrl *pCtrl, HTAB hTab);
	virtual COLORREF GetButtonMarkerColor(TabCtrl *pCtrl, bool bHover, bool bPushed);
		// 
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2003_client : public TabCtrlStyle_VS2003_base
{protected:	// ITabCtrlRecalc.
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl);
};
// 
struct TabCtrlStyle_VS2003_client_custom1 : public TabCtrlStyle_VS2003_client
{protected:	// TabCtrlStyle_base.
	virtual void DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
	virtual void DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
		// 
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2003_bars : public TabCtrlStyle_VS2003_base
{protected:	// ITabCtrlRecalc.
	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl);
};
// 
struct TabCtrlStyle_VS2003_bars_custom1 : public TabCtrlStyle_VS2003_bars
{protected:	// ITabCtrlRecalc.
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonClose(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
	virtual void DrawButtonMenu(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollLeft(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
	virtual void DrawButtonScrollRight(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed, bool bPartialView);
		// 
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2008_client_base : public TabCtrlStyle_base, public TabCtrlRecalcStub, public TabCtrlBehaviorStub
{		// ITabCtrlStyle.
	virtual ITabCtrlRecalc *GetRecalcManager();
	virtual ITabCtrlBehavior *GetBehaviorManager();

protected:	// ITabCtrlRecalc.
	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual int GetTabExtraWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base, HTAB hTab);
	virtual int GetTabMinWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// ITabCtrlBehavior.
	virtual HTAB HitTest(TabCtrl *pCtrl, ITabCtrlBehavior *base, CPoint point);		// get tab in the given point.

protected:	// TabCtrlDraw.
	virtual bool IsStraightOrder(TabCtrl *pCtrl);
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual void DrawTabContext(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);	// draw image and text.

protected:
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool active, bool disable) = 0;
	virtual COLORREF GetTabOutlineColor(TabCtrl *pCtrl, bool active, bool hover, bool disable, bool left) = 0;
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool active, bool hover, bool disable) = 0;
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool active, bool hover, bool disable) = 0;

protected:
	void GetTabOutline(TabCtrl *pCtrl, HTAB hTab, CRect const *rect, bool top, POINT pts[8]/*out*/, RECT *rcFill/*out*/) const;
	bool HitTest(TabCtrl *pCtrl, HTAB hTab, bool top, CPoint point) const;
	int GetSlantWidth(TabCtrl *pCtrl) const;
};
// 
struct TabCtrlStyle_VS2008_client_classic : public TabCtrlStyle_VS2008_client_base
{protected:	// TabCtrlStyle_VS2008_client_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool active, bool disable);
	virtual COLORREF GetTabOutlineColor(TabCtrl *pCtrl, bool active, bool hover, bool disable, bool left);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);
};
// 
struct TabCtrlStyle_VS2008_client_blue : public TabCtrlStyle_VS2008_client_base
{protected:	// TabCtrlStyle_VS2008_client_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool active, bool disable);
	virtual COLORREF GetTabOutlineColor(TabCtrl *pCtrl, bool active, bool hover, bool disable, bool left);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);
};
// 
struct TabCtrlStyle_VS2008_client_silver : public TabCtrlStyle_VS2008_client_base
{protected:	// TabCtrlStyle_VS2008_client_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool active, bool disable);
	virtual COLORREF GetTabOutlineColor(TabCtrl *pCtrl, bool active, bool hover, bool disable, bool left);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
// 
struct TabCtrlStyle_VS2008_client_olive : public TabCtrlStyle_VS2008_client_base
{protected:	// TabCtrlStyle_VS2008_client_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool active, bool disable);
	virtual COLORREF GetTabOutlineColor(TabCtrl *pCtrl, bool active, bool hover, bool disable, bool left);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool active, bool hover, bool disable);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2008_bars_base : public TabCtrlStyle_base, public TabCtrlRecalcStub
{		// ITabCtrlStyle.
	virtual ITabCtrlRecalc *GetRecalcManager();

protected:	// ITabCtrlRecalc.
	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual void DrawTabContext(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);	// draw image and text.
	virtual COLORREF GetTabTextColor(TabCtrl *pCtrl, HTAB hTab);

protected:
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover) = 0;
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable) = 0;
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable) = 0;
};
// 
struct TabCtrlStyle_VS2008_bars_classic : public TabCtrlStyle_VS2008_bars_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);
};
// 
struct TabCtrlStyle_VS2008_bars_blue : public TabCtrlStyle_VS2008_bars_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);
};
// 
struct TabCtrlStyle_VS2008_bars_silver : public TabCtrlStyle_VS2008_bars_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
// 
struct TabCtrlStyle_VS2008_bars_olive : public TabCtrlStyle_VS2008_bars_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2008_bars_custom1_base : public TabCtrlStyle_VS2008_bars_base
{protected:	// ITabCtrlRecalc.
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
};
// 
struct TabCtrlStyle_VS2008_bars_classic_custom1 : public TabCtrlStyle_VS2008_bars_custom1_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);
};
// 
struct TabCtrlStyle_VS2008_bars_blue_custom1 : public TabCtrlStyle_VS2008_bars_custom1_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);
};
// 
struct TabCtrlStyle_VS2008_bars_silver_custom1 : public TabCtrlStyle_VS2008_bars_custom1_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
// 
struct TabCtrlStyle_VS2008_bars_olive_custom1 : public TabCtrlStyle_VS2008_bars_custom1_base
{		// TabCtrlStyle_VS2008_bars_base.
	virtual COLORREF GetTabBorderColor(TabCtrl *pCtrl, bool hover);
	virtual COLORREF GetTabGradientLightColor(TabCtrl *pCtrl, bool hover, bool disable);
	virtual COLORREF GetTabGradientDarkColor(TabCtrl *pCtrl, bool hover, bool disable);

protected:	// TabCtrlStyle_base.
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2010_client : public TabCtrlStyle_base, public TabCtrlRecalcStub
{		// ITabCtrlStyle.
	virtual ITabCtrlRecalc *GetRecalcManager();

protected:	// ITabCtrlRecalc.
	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetWndsAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual COLORREF GetCtrlAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetTabTextColor(TabCtrl *pCtrl, HTAB hTab);
	virtual COLORREF GetEmptyWndsAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetButtonMarkerColor(TabCtrl *pCtrl, bool bHover, bool bPushed);
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);

protected:
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool top, bool active, bool select, bool hover);
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool top, bool active);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2010_client_custom1 : public TabCtrlStyle_VS2010_client
{		// TabCtrlStyle_VS2010_client.
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2010_client_custom2 : public TabCtrlStyle_VS2010_client
{		// TabCtrlStyle_VS2010_client.
	virtual void DrawWndsAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
};
/////////////////////////////////////////////////////////////////////////////
// 
struct TabCtrlStyle_VS2010_bars : public TabCtrlStyle_base, public TabCtrlRecalcStub
{		// ITabCtrlStyle.
	virtual ITabCtrlRecalc *GetRecalcManager();

protected:	// ITabCtrlRecalc.
	virtual int GetBorderWidth(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetCtrlAreaPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);
	virtual CRect GetTabPadding(TabCtrl *pCtrl, ITabCtrlRecalc *base);

protected:	// TabCtrlDraw.
	virtual void DrawCtrlAreaBack(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:	// TabCtrlStyle_base.
	virtual COLORREF GetCtrlAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetTabTextColor(TabCtrl *pCtrl, HTAB hTab);
	virtual COLORREF GetEmptyWndsAreaBackColor(TabCtrl *pCtrl);
	virtual COLORREF GetButtonMarkerColor(TabCtrl *pCtrl, bool bHover, bool bPushed);
	virtual void DrawTabBack(TabCtrl *pCtrl, CDC *pDC, HTAB hTab, CRect const *pRect, CRgn *pRgn);
	virtual void DrawButtonFrame(TabCtrl *pCtrl, CDC *pDC, CRect const *pRect, bool bHover, bool bPushed);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
template<typename CLASS_STYLE>
struct TabCtrlEx : public TabCtrl
{	TabCtrlEx()
	{	InstallStyle(&style);
	}
	CLASS_STYLE style;
};
/////////////////////////////////////////////////////////////////////////////
#pragma warning(pop)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////








