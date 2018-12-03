//==========================================================
// Author: Borodenko Oleg
// 17/12/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable:4265)
#pragma warning(disable:4365)
#pragma warning(disable:4512)
#pragma warning(disable:4640)
#pragma warning(disable : 4290)	// C++ exception specification ignored...
/////////////////////////////////////////////////////////////////////////////
#include "TabCtrl.h"
#include "DockingMarkers.h"
#include "Tree.h"
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
typedef HNODE HPANE;
// 
enum DOCKSIDE
{	DOCKSIDE_NONE,
	DOCKSIDE_LEFT, DOCKSIDE_TOP, DOCKSIDE_RIGHT, DOCKSIDE_BOTTOM, DOCKSIDE_TABS
};
// 
enum SPACE		// algorithm of insertion or deletion pane.
{	SPACE_BETWEEN,		// takes/returns space from/to previous and next panes.
	SPACE_PREVIOUS,	// takes/returns space from/to previous pane.
	SPACE_NEXT			// takes/returns space from/to next pane.
};
// 
enum SPLITTER_DRAGGING
{	SPLITTER_DRAGGING_STATIC,
	SPLITTER_DRAGGING_DYNAMIC
};
// 
class MultiPaneCtrl;
// 
struct MultiPaneCtrlDraw
{	virtual void DrawBegin(MultiPaneCtrl * /*pCtrl*/, CDC * /*pDC*/) {}
	virtual void DrawSplitter(MultiPaneCtrl * /*pCtrl*/, CDC * /*pDC*/, bool /*horz*/, CRect const * /*pRect*/) {}
	virtual void DrawSplitterDragRect(MultiPaneCtrl * /*pCtrl*/, CDC * /*pDC*/, bool /*horz*/, CRect const * /*pRectNew*/, CRect const * /*pRectOld*/) {}
	virtual void DrawBorder(MultiPaneCtrl * /*pCtrl*/, CDC * /*pDC*/, CRect const * /*pRect*/) {}
	virtual void DrawEnd(MultiPaneCtrl * /*pCtrl*/, CDC * /*pDC*/) {}
};
// 
interface IMultiPaneCtrlRecalc
{	virtual int GetBorderWidth(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base) = 0;
	virtual CSize GetSplitterSize(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base) = 0;	// width (CSize::cx) for vertical and height (CSize::cy) for horizontal splitter.
};
// 
struct MultiPaneCtrlUserAbility
{		// function is called during Drag and Drop operation.
	virtual bool CanDrop(MultiPaneCtrl * /*pCtrl*/, HTAB /*hTabSrc*/, DOCKSIDE /*sideDst*/, HPANE /*hPaneDst*/) { return true; }	// permission to insert the panel using a marker under the cursor.
		// functions are called during the recalculation of the control (WM_SIZE or Update()).
	virtual bool CanShowCloseButton(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pCtrl*/) { return true; }
	virtual bool CanShowMenuButton(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pCtrl*/) { return true; }
	virtual bool CanShowScrollButtons(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pCtrl*/) { return true; }
};
// 
struct MultiPaneCtrlNotify
{	virtual void OnTabPreCreate(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HWND /*hWnd*/, TCHAR const * /*text*/, int /*image*/) {}
	virtual void OnTabPostCreate(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/) {}
	virtual void OnTabPreDestroy(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/) {}
		// 
	virtual void OnTabCloseButtonClicked(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnTabMenuButtonClicked(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, CRect const * /*pRect*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnTabSelected(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/) {}
	virtual void OnTabLButtonDown(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnTabLButtonDblClk(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnTabRButtonDown(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space, hTab can be NULL.
	virtual void OnTabRButtonUp(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space, hTab can be NULL.
		// 
	virtual void OnTabStartDrag(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/) {}	// ptScr - in screen space.
	virtual void OnTabDrag(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, CPoint /*ptScr*/, bool /*outside*/) {}	// ptScr - in screen space, outside==true - dragging out of tabs area.
	virtual void OnTabFinishDrag(MultiPaneCtrl * /*pPaneCtrl*/, TabCtrl * /*pTabCtrl*/, HTAB /*hTab*/, bool /*cancel*/) {}	// cancel==false - dragging was finished using left button up.
		// 
	virtual void OnPanePostCreate(MultiPaneCtrl * /*pPaneCtrl*/, HPANE /*hPane*/) {}	// new pane has been created.
	virtual void OnPanePreDestroy(MultiPaneCtrl * /*pPaneCtrl*/, HPANE /*hPane*/) {}	// before pane delete.

	//added by Crazii @1/14/2013
	virtual void OnSplitterFinishDrag(MultiPaneCtrl* /*pPaneCtrl*/)			{}
};
/////////////////////////////////////////////////////////////////////////////
// 
interface IMultiPaneCtrlStyle
{	virtual ITabCtrlStyle *GetTabStyle() = 0;
	virtual DockingMarkersDraw *GetDockMarkersDraw() = 0;
		// 
	virtual MultiPaneCtrlDraw *GetDrawManager() = 0;
	virtual IMultiPaneCtrlRecalc *GetRecalcManager() = 0;
};
/////////////////////////////////////////////////////////////////////////////
// 
class MultiPaneCtrl : public CWnd, 
	private ActivityHook::INotify, private KeyboardHook::Notify,
	private IMultiPaneCtrlRecalc, private MultiPaneCtrlUserAbility,
	private TabCtrlUserAbility, private TabCtrlNotify,
	private DockingMarkersAbility
{
public:
	MultiPaneCtrl();
	~MultiPaneCtrl();

///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
public:
	bool Create(CWnd *parent, DWORD style, RECT const &rect, UINT id);
		// 
	HPANE ConvertToLine(HPANE pane, bool horz) throw(std::bad_alloc);	// convert pane (empty or with tabs) to line, return new created child pane for the passed pane.
	HPANE Add(HPANE parent) throw(std::bad_alloc);	// add empty pane to the end of line.
	HPANE Insert(HPANE before, SPACE space) throw(std::bad_alloc);	// insert empty pane to the line.
	void Remove(HPANE before, HPANE src);	// change position of 'src' pane in the line.
	void Delete(HPANE pane, SPACE space);	// delete the pane with the redistribution of its space.
	void DeleteAll();	// remove all panes except the root pane.
		// 
	HPANE InsertIntermediate(HPANE parent, bool horz) throw(std::bad_alloc);	// insert intermediate pane in hierarchical branch as child pane for the 'parent'.
	void DeleteIntermediate(HPANE pane);
		// 
	void Update();		// recalculate and update control (include call UpdateAllTabs()).
	void Update(HPANE pane);	// without recalculate pane, only update all its child controls (and descendants).
	void UpdateAllTabs();	// update all child TabCtrl in the control.
		// 
	void InstallStyle(IMultiPaneCtrlStyle *style);
		// 
	void SetDrawManager(MultiPaneCtrlDraw *p);
	MultiPaneCtrlDraw *GetDrawManager() const;
	void SetRecalcManager(IMultiPaneCtrlRecalc *p);		// or NULL for default manager.
	IMultiPaneCtrlRecalc *GetRecalcManager() const;
	void SetAbilityManager(MultiPaneCtrlUserAbility *p);	// or NULL for default manager.
	MultiPaneCtrlUserAbility *GetAbilityManager() const;
	void SetNotifyManager(MultiPaneCtrlNotify *p);
	MultiPaneCtrlNotify *GetNotifyManager() const;
		// 
	bool SetDockMarkers(DockingMarkers::LayoutPane const &layout, DockingMarkers::Params const &params, int transpSelectBar/*1...100%*/);	// transpSelectBar - transparency of the window-marker that indicates the position of the pane to insert.
	void DisableDockMarkers();
	bool IsDockMarkersEnable() const;
		// 
	bool SetImageLists(CImageList *pImageList, CImageList *pImageListDisabled);	// pImageList and/or pImageListDisabled can be NULL.
	void SetImageListsRef(CImageList *pImageList, CImageList *pImageListDisabled);	// set references to other ImageList.
	void GetImageLists(CImageList **pImageList/*out*/, CImageList **pImageListDisabled/*out*/);	// any pointer can be NULL.
		// 
	bool SetSystemImageList(CImageList *pImageList);
	void SetSystemImageListRef(CImageList *pImageList);	// set reference to another ImageList.
	CImageList *GetSystemImageList() const;
		// 
	bool SetCursors(UINT tab, UINT splitterHorz, UINT splitterVert, UINT dragEnable, UINT dragDisable);	// 0 if you do not use cursor.
	bool SetCursors(HMODULE module, UINT tab, UINT splitterHorz, UINT splitterVert, UINT dragEnable, UINT dragDisable);	// 'module' - module containing the resource of cursor, use 0 for any id if you do not use this cursor.
	bool SetCursors(HCURSOR tab, HCURSOR splitterHorz, HCURSOR splitterVert, HCURSOR dragEnable, HCURSOR dragDisable);	// NULL if you do not use cursor.
	void SetCursorsRef(HCURSOR *tab, HCURSOR *splitterHorz, HCURSOR *splitterVert, HCURSOR *dragEnable, HCURSOR *dragDisable);	// set references to other cursors.
	void GetCursors(HCURSOR *tab/*out*/, HCURSOR *splitterHorz/*out*/, HCURSOR *splitterVert/*out*/, HCURSOR *dragEnable/*out*/, HCURSOR *dragDisable/*out*/) const;	// any pointer can be NULL.
		// 
	bool SetFont(CFont *pFont);
	void SetFontRef(CFont *pFont);	// set reference to another font.
	bool SetFont(LOGFONT const *pLf);
	CFont *GetFont();
		// 
	bool SetFontSelect(CFont *pFont);
	void SetFontSelectRef(CFont *pFont);	// set reference to another select font.
	bool SetFontSelect(LOGFONT const *pLf);
	CFont *GetFontSelect();
		// 
	void SetData(HPANE pane, __int64 data);
	__int64 GetData(HPANE pane) const;
		// 
	HPANE GetRoot() const;		// for all functions can be used NULL instead GetRoot().
	HPANE GetParent(HPANE pane) const;	// return NULL if 'pane' is root pane.
	HPANE GetPane(HPANE parent, int idx) const;	// get pane with the index 'idx' in the line.
	int GetIndex(HPANE pane) const;	// get index of pane in the line.
	int GetCount(HPANE parent) const;	// get the number of panes in the line.
	bool IsExist(HPANE pane) const;	// return true - pane with this handle exists in the control.
		// 
	HPANE GetFirstChild(HPANE parent) const;	// get first (top for vertical and left for horizontal line) pane in the line.
	HPANE GetPrevChild(HPANE pane) const;	// get previous pane in the line or NULL if 'pane' is the first pane.
	HPANE GetNextChild(HPANE pane) const;	// get next pane in the line or NULL if 'pane' is the last pane.
	HPANE GetLastChild(HPANE parent) const;	// get last (bottom for vertical and right for horizontal line) pane in the line.
		// 
	HPANE GetFirst() const;		// get top pane in the tree of panes (always return the root pane (GetRoot())).
	HPANE GetPrev(HPANE pane) const;	// get previous pane in the tree of panes (recursively).
	HPANE GetNext(HPANE pane) const;	// get next pane in the tree of panes (recursively).
	HPANE GetLast() const;	// get bottom pane in the tree of panes.
		// 
	HPANE HitTest(CPoint ptScr) const;	// get pane in the given point or NULL, 'point' - in screen space.
	bool IsLine(HPANE pane) const;	// return true - pane has child panes, return false - pane is empty or has tabs.
	bool IsHorizontal(HPANE pane) const;	// get orientation of line (must be IsLine(pane)==true).
	void SetOrientation(HPANE pane, bool horz);	// set orientation of line. 
		// 
	void ShowBorder(bool show);	// border is visible if IsShowBorder()==true and IMultiPaneCtrlRecalc::GetBorderWidth(...) returns >0.
	bool IsShowBorder() const;
		// 
	void DragTabEnable(bool enable);	// enable/disable Drag and Drop operation, drag of tabs works only if IsDragTabEnable()==true and RemoveTabEnable()==true.
	bool IsDragTabEnable() const;
	void DropOptimization(bool optimiz);	// true - use optimization layout of panes after Drag and Drop operation (some panes can be deleted or transformed).
	bool IsDropOptimization() const;
		// 
	void SetMinSizeForDrop(CSize const &size);	// Drag and Drop operation is allowed only if the size of the pane under the cursor more than 'size'.
	CSize GetMinSizeForDrop() const;
		// 
	void SetPortions(HPANE parent, int *percent/*in*/);	// set proportions of panes in the line in percents (count items in 'percent'==GetCount(parent) and total number of percents must be 100).
	CRect GetRect(HPANE pane) const;		// obtain the coordinates of the 'pane'. 
	void SetPortion(HPANE pane, double portion, SPACE space) const;
	double GetPortion(HPANE pane) const;	// obtain the part of the 'pane' in the parent line in the range (0.0,1.0].
	void CopyPortions(HPANE dst, MultiPaneCtrl const *paneCtrlSrc, HPANE src);	// copy proportions of child panes from 'src' of 'paneCtrlSrc' control to 'dst' of this control.
		// 
	void SetEqualPaneSize(HPANE parent);	// set equal size (width for horizontal and height for vertical) for all child panes of 'parent'.
	void SetEqualPaneSize();	// set equal size (width for horizontal and height for vertical) for every line in the control severally.
		// 
	void SetMinSize(CSize const &size);	// set min width and height of panes in the control.
	CSize GetMinSize() const;
		// 
	void WatchActivityCtrl(bool on);	// true - control saves activity state which you can get using IsActive().
	bool IsWatchActivityCtrl() const;
		// 
	bool IsActive() const;	// return true - one of child windows is active (has focus), works only if IsWatchActivityCtrl()==true.
		// 
		// 
		// work with splitters.
	void SetSplitterDraggingMode(SPLITTER_DRAGGING mode);
	SPLITTER_DRAGGING GetSplitterDraggingMode() const;
		// 
	bool IsSplitterDragging(bool *horz/*out*/) const;
	void CancelDragging();
		// 
	void ActiveSplitter(HPANE pane, bool active);	// set activity for splitter which is right/bottom of pane.
	bool IsActiveSplitter(HPANE pane) const;
		// 
	CRect GetSplitterRect(HPANE pane) const;	// splitter which is right/bottom of pane.
		// 
		// 
		// for getting information about tabs during load state operation.
	struct Tab
	{	HWND GetHWND() const;
		CString GetText() const;
		int GetImage() const;
		CString GetToolTipText() const;
		bool IsDisable() const;
		__int64 GetData() const;
	};
	struct Tabs : public TOKEN_OWNER
	{	Tabs();
		~Tabs();
		int Add(HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc);	// 'image'=-1 for tab without image.
		int Add(HWND wnd, TCHAR const *text, int image, TCHAR const *tooltip, bool disable, __int64 data) throw(std::bad_alloc);	// 'image'=-1 for tab without image.
		Tab const *operator[](int idx);
		int GetCount() const;
	};
		// 
		// 
		// work with tabs.
	HTAB AddTab(HPANE pane, HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc);	// 'image'=-1 for tab without image.
	HTAB AddTab(HPANE pane, Tab const *tab) throw(std::bad_alloc);
	HTAB InsertTab(HPANE pane, HTAB before, HWND wnd, TCHAR const *text, int image) throw(std::bad_alloc);	// 'image'=-1 for tab without image.
	HTAB InsertTab(HPANE pane, HTAB before, Tab const *tab) throw(std::bad_alloc);
		// 
	TabCtrl *GetTabCtrl(HPANE pane) const;	// only for pane which hasn't child panes.
	HPANE GetPane(TabCtrl const *ctrl) const;
	HPANE GetPane(HTAB tab) const;
	HTAB GetTabWithWindow(HWND hWnd, HPANE *pane/*out*/) const;	// get tab whose window has 'id'.
	bool IsSinglePaneWithTabs() const;	// true - there is only one pane with tabs.
	HPANE GetFirstPaneWithTabs() const;	// recursive search from GetFirst() to GetLast().
		// 
	void ActivateTab(HPANE pane);	// set focus into selected tab in this pane.
	void ActivateTab(HPANE pane, HTAB tab);	// select and set focus into 'tab'.
		// 
	void SetTabLayout(TAB_LAYOUT layout);
	TAB_LAYOUT GetTabLayout() const;
	void SetTabBehavior(TAB_BEHAVIOR behavior);
	TAB_BEHAVIOR GetTabBehavior() const;
		// 
	void SetTabScrollingStep(int step);	// width of one step for scrolling (in pixels >=1) (only for the TAB_BEHAVIOR_SCROLL mode).
	int GetTabScrollingStep() const;
		// 
	void ShowTabBorder(bool show);	// border is visible if IsShowTabBorder()==true and IMultiPaneCtrlRecalc::GetBorderWidth(...) returns >0.
	bool IsShowTabBorder() const;
	void EqualTabsSize(bool equal);	// true - all tabs will have the same width.
	bool IsEqualTabsSize() const;
	void RemoveTabEnable(bool enable);	// true - you can change positions of tabs using mouse.
	bool IsRemoveTabEnable() const;
	void HideSingleTab(bool hide);	// true - hide control area of TabCtrl if it has one tab. 
	bool IsHideSingleTab() const;
		// 
	void ShowTabCloseButton(bool show);
	bool IsShowTabCloseButton() const;
	void ShowTabMenuButton(bool show);
	bool IsShowTabMenuButton() const;
	void ShowTabScrollButtons(bool show);
	bool IsShowTabScrollButtons() const;
		// 
	void TabToolTipEnable(bool enable);
	bool IsTabToolTipEnable() const;
	void SetTabCloseButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetTabCloseButToolTipText() const;
	void SetTabMenuButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetTabMenuButToolTipText() const;
	void SetTabLeftScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetTabLeftScrollButToolTipText() const;
	void SetTabRightScrollButToolTipText(TCHAR const *text) throw(std::bad_alloc);
	CString GetTabRightScrollButToolTipText() const;
		// 
		// 
		// save/load state.
	bool SaveTabs(Tabs *tabs/*out*/) const;	// load information (in the 'tabs') about tabs exist in the control.
	bool LoadState(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack);	// load state from registry.
	bool LoadStateAndUpdate(CWinApp *app, TCHAR const *section, TCHAR const *entry, Tabs const *tabs, bool ignoreLack);	// load state from registry + update control.
	bool SaveState(CWinApp *app, TCHAR const *section, TCHAR const *entry) const;	// save state in registry.
	bool LoadState(CArchive *ar, Tabs const *tabs, bool ignoreLack);
	bool LoadStateAndUpdate(CArchive *ar, Tabs const *tabs, bool ignoreLack);	// load state + update control.
	bool SaveState(CArchive *ar) const;
		// ignoreLack=true - ignore and not to load the pane and continue to load state if there is information about pane in the registry/archive, but not in the 'tabs'.

public:	// functions of IMultiPaneCtrlRecalc interface, return information from current recalc manager.
	int GetBorderWidth();
	CSize GetSplitterSize();

public:
	HPANE GetDragDstPane(HTAB tabSrc/*or NULL*/, CPoint ptScr);	// get destination pane for drag operation.
	void DeleteOptimizDown(HPANE pane);	// recursive removal of the pane down to the root with optimization layout of panes.

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
		// ActivityHook::INotify.
	virtual void OnActive(bool active, HWND wnd);
		// KeyboardHook::Notify.
	virtual void OnKeyDown(UINT keyCode, UINT msgFlag);
		// 
		// IMultiPaneCtrlRecalc.
	virtual int GetBorderWidth(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base);
	virtual CSize GetSplitterSize(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base);
		// 
		// TabCtrlUserAbility.
	virtual bool CanShowCloseButton(TabCtrl *pCtrl);
	virtual bool CanShowMenuButton(TabCtrl *pCtrl);
	virtual bool CanShowScrollButtons(TabCtrl *pCtrl);
		// 
		// TabCtrlNotify.
	virtual void OnTabPreCreate(TabCtrl *pCtrl, HWND hWnd, TCHAR const *text, int image);
	virtual void OnTabPostCreate(TabCtrl *pCtrl, HTAB hTab);
	virtual void OnTabPreDestroy(TabCtrl *pCtrl, HTAB hTab);
		// 
	virtual void OnCloseButtonClicked(TabCtrl *pCtrl, CRect const *pRect, CPoint ptScr);
	virtual void OnMenuButtonClicked(TabCtrl *pCtrl, CRect const *pRect, CPoint ptScr);
	virtual void OnTabSelected(TabCtrl *pCtrl, HTAB hTab);
	virtual void OnLButtonDown(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr);
	virtual void OnLButtonDblClk(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr);
	virtual void OnRButtonDown(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr);
	virtual void OnRButtonUp(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr);
		// 
	virtual void OnStartDrag(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr);
	virtual void OnDrag(TabCtrl *pCtrl, HTAB hTab, CPoint ptScr, bool outside);
	virtual void OnFinishDrag(TabCtrl *pCtrl, HTAB hTab, bool cancel);
		// 
		// DockingMarkersAbility.
	virtual bool IsMarkerEnable(DockingMarkers *pObj, DOCKMARKER marker);

private:
	struct Pane
	{	Pane();
		~Pane();
			// 
		TabCtrl *tab;	// NULL if there are child panes, otherwise pane is empty or has tabs.
		bool horz;		// orientation of line.
		__int64 data;	// data of pane.
			// 
		MultiPaneCtrl *owner;
		HPANE pane;
			// 
		struct State
		{	CRect rc, rcSplitter;
			double factor;
		} real, store;
			// 
		bool activeSplitter;
	};
	friend Pane;
	tree<Pane> m_Tree;
		// 
	ITabCtrlStyle *m_pTabStyle;
	TabCtrl m_TabState;
		// 
	MultiPaneCtrlDraw *m_pDrawManager;
	IMultiPaneCtrlRecalc *m_pRecalcManager;
	MultiPaneCtrlUserAbility *m_pAbilityManager;
	MultiPaneCtrlNotify *m_pNotifyManager;
		// 
	DockingMarkers m_DockMarkMngr;
	CSize m_szMinSize;
	bool m_bShowBorder;
	bool m_bDragTabEnable;
	bool m_bDropOptimiz;
	CSize m_szMinSizeForDrop;
		// 
	SPLITTER_DRAGGING m_SplitterDragMode;
	HPANE m_hSplitDragPane;
	CPoint m_ptSplitDragStart;
		// 
	CImageList m_ImageList,*m_pImageListRef, m_ImageListDisabled,*m_pImageListDisabledRef;
	CImageList m_SysImageList, *m_pSysImageListRef;
	HCURSOR m_hCurTab, m_hCurSplitterHorz,m_hCurSplitterVert, m_hCurDragEnable,m_hCurDragDisable;
	HCURSOR *m_hpCurTabRef, *m_hpCurSplitterHorzRef,*m_hpCurSplitterVertRef, 
		*m_hpCurDragEnableRef,*m_hpCurDragDisableRef;
	CFont m_Font,*m_pFontRef, m_FontSelect,*m_pFontSelectRef;
		// 
	ActivityHook m_ActivityHook;
	KeyboardHook m_KeyboardHook;
		// 
	bool m_bActive;
	HWND m_hLastActiveWnd;
		// 
	HTAB m_hDraggingTab;
	HPANE m_hMarkedPane;
	DOCKMARKER m_SelectedMarker;
	bool m_bDragFinishing;

private:
	void PreCreateTabCtrl(TabCtrl *tab);
	bool CreatePane(HPANE pane, bool createTab);
	void DestroyPane(HPANE pane);
	void Recalc(bool redraw);
	void Recalc(DeferWindow *dw, HPANE pane, CRect rc, bool redraw);
	HPANE HitTestSplitter(CPoint const *pt);
	void StartSplitterDragging(CPoint point);
	void StopSplitterDragging(bool reset);
	void SetDraggingWidth(HPANE pane, int offset, CRect *rcdrag/*out*/);
	void SetDraggingHeight(HPANE pane, int offset, CRect *rcdrag/*out*/);
	void DrawSplitterDragRect(CRect const *pRect, bool horz);
	bool IsSplitterHorizontal(HPANE pane) const;
	bool CanSplitterDrag(HPANE pane);
	void HalveWidthPanes(HPANE prev, HPANE next);
	void HalveHeightPanes(HPANE prev, HPANE next);
	void SaveAllPanesState();
	void RestoreAllPanesState();
	void UpdateAdjacentPanes(HPANE pane1, HPANE pane2);
	DOCKMARKER RelayMouseMove(TabCtrl *pTabDst, CPoint ptScr);
	DOCKSIDE MarkerToSide(DOCKMARKER marker) const;
	void DeleteSrcTab(HPANE paneSrc, TabCtrl *pTabSrc, HTAB hTabSrc, HPANE paneMarked, DOCKMARKER marker);
	void DeleteOptimizDown(HPANE paneSrc, bool optimize, SPACE space);
	void SaveStateInner(CArchive *ar) const;
	void SaveStateInner(CArchive *ar, HPANE pane) const;
	bool LoadStateInner(CArchive *ar, Tabs const *tabs, bool ignoreLack) throw(std::bad_alloc);
	bool LoadStateInner(CArchive *ar, HPANE parent, Tabs const *tabs, bool ignoreLack, Blade::List<HPANE> *panes) throw(std::bad_alloc);

private:
	DECLARE_DYNCREATE(MultiPaneCtrl)

///////////////////////////////////////
// PROTECTED
///////////////////////////////////////
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlRecalcStub : public IMultiPaneCtrlRecalc
{	virtual int GetBorderWidth(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base);
	virtual CSize GetSplitterSize(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlDrawBase : public MultiPaneCtrlDraw
{		// MultiPaneCtrlDraw.
	virtual void DrawSplitter(MultiPaneCtrl *pCtrl, CDC *pDC, bool horz, CRect const *pRect);
	virtual void DrawSplitterDragRect(MultiPaneCtrl *pCtrl, CDC *pDC, bool horz, CRect const *pRectNew, CRect const *pRectOld);
	virtual void DrawBorder(MultiPaneCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:
	virtual COLORREF GetSplitterColor(MultiPaneCtrl *pCtrl);
	virtual COLORREF GetBorderColor(MultiPaneCtrl *pCtrl);

protected:
	virtual void DrawDragRect(MultiPaneCtrl *pCtrl, CDC *pDC, CRect const *pRect);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_base : public IMultiPaneCtrlStyle, public MultiPaneCtrlDrawBase
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }
	virtual DockingMarkersDraw *GetDockMarkersDraw() { return &dockMarkDraw; }
	virtual MultiPaneCtrlDraw *GetDrawManager() { return this; }
	virtual IMultiPaneCtrlRecalc *GetRecalcManager() { return NULL; }	// use default manager.

protected:
	TabCtrlStyle_base tabStyle;
	DockingMarkersDrawBase dockMarkDraw;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_VS2003_base : public IMultiPaneCtrlStyle, public MultiPaneCtrlDrawBase
{		// IMultiPaneCtrlStyle.
	virtual DockingMarkersDraw *GetDockMarkersDraw() { return &dockMarkDraw; }
	virtual MultiPaneCtrlDraw *GetDrawManager() { return this; }
	virtual IMultiPaneCtrlRecalc *GetRecalcManager() { return NULL; }	// use default manager.

protected:
	DockingMarkersDraw_VS2003 dockMarkDraw;
};
// 
struct MultiPaneCtrlStyle_VS2003_client : public MultiPaneCtrlStyle_VS2003_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2003_client tabStyle;
};
// 
struct MultiPaneCtrlStyle_VS2003_client_custom1 : public MultiPaneCtrlStyle_VS2003_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2003_client_custom1 tabStyle;
};
// 
struct MultiPaneCtrlStyle_VS2003_bars : public MultiPaneCtrlStyle_VS2003_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2003_bars tabStyle;
};
// 
struct MultiPaneCtrlStyle_VS2003_bars_custom1 : public MultiPaneCtrlStyle_VS2003_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2003_bars_custom1 tabStyle;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_VS2008_base : public IMultiPaneCtrlStyle, public MultiPaneCtrlDrawBase
{		// IMultiPaneCtrlStyle.
	virtual DockingMarkersDraw *GetDockMarkersDraw() { return &dockMarkDraw; }
	virtual MultiPaneCtrlDraw *GetDrawManager() { return this; }
	virtual IMultiPaneCtrlRecalc *GetRecalcManager() { return NULL; }	// use default manager.

protected:
	DockingMarkersDraw_VS2008 dockMarkDraw;
};
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_VS2008_client_classic : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_client_classic tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_client_blue : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_client_blue tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_client_silver : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_client_silver tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_client_olive : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_client_olive tabStyle;
};
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_VS2008_bars_classic : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_classic tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_bars_blue : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_blue tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_bars_silver : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_silver tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_bars_olive : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_olive tabStyle;
};
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_VS2008_bars_classic_custom1 : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_classic_custom1 tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_bars_blue_custom1 : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_blue_custom1 tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_bars_silver_custom1 : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_silver_custom1 tabStyle;
};
struct MultiPaneCtrlStyle_VS2008_bars_olive_custom1 : public MultiPaneCtrlStyle_VS2008_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2008_bars_olive_custom1 tabStyle;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MultiPaneCtrlStyle_VS2010_base : public IMultiPaneCtrlStyle, 
	public MultiPaneCtrlRecalcStub, public MultiPaneCtrlDrawBase
{
		// IMultiPaneCtrlStyle.
	virtual DockingMarkersDraw *GetDockMarkersDraw() { return &dockMarkDraw; }
	virtual MultiPaneCtrlDraw *GetDrawManager() { return this; }
	virtual IMultiPaneCtrlRecalc *GetRecalcManager() { return this; }

protected:	// IMultiPaneCtrlRecalc.
	virtual CSize GetSplitterSize(MultiPaneCtrl *pCtrl, IMultiPaneCtrlRecalc *base);

protected:	// MultiPaneCtrlDrawBase.
	virtual COLORREF GetSplitterColor(MultiPaneCtrl *pCtrl);
	virtual void DrawDragRect(MultiPaneCtrl *pCtrl, CDC *pDC, CRect const *pRect);

protected:
	DockingMarkersDraw_VS2010 dockMarkDraw;
};
// 
struct MultiPaneCtrlStyle_VS2010_client : public MultiPaneCtrlStyle_VS2010_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2010_client tabStyle;
};
// 
struct MultiPaneCtrlStyle_VS2010_bars : public MultiPaneCtrlStyle_VS2010_base
{		// IMultiPaneCtrlStyle.
	virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

protected:
	TabCtrlStyle_VS2010_bars tabStyle;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
template<typename CLASS_STYLE>
struct MultiPaneCtrlEx : public MultiPaneCtrl
{	MultiPaneCtrlEx()
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











