//==========================================================
// Author: Borodenko Oleg
// 05/08/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
#include "Tools.h"
#pragma warning(push)
#pragma warning(disable : 4512)
#pragma warning(disable : 4265)
#pragma warning(disable : 4062)

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
enum DOCKMARKER
{	DOCKMARKER_NONE,
		// pane markers.
	DOCKMARKER_PANE_LEFT, DOCKMARKER_PANE_TOP, DOCKMARKER_PANE_RIGHT, DOCKMARKER_PANE_BOTTOM, DOCKMARKER_PANE_TABS,
		// frame markers.
	DOCKMARKER_FRAME_LEFT, DOCKMARKER_FRAME_TOP, DOCKMARKER_FRAME_RIGHT, DOCKMARKER_FRAME_BOTTOM
};
// 
class DockingMarkers;
// 
interface DockingMarkersAbility
{	virtual bool IsMarkerEnable(DockingMarkers * /*pObj*/, DOCKMARKER /*marker*/) { return true; }
};
// 
struct DockingMarkersDraw
{	virtual void DrawMarker(DockingMarkers * /*pObj*/, CDC * /*pDC*/, CRect const * /*pRect*/, DOCKMARKER /*marker*/, CImageList * /*pImageList*/, bool /*bHighlight*/, bool /*bDisable*/) {}
	virtual void DrawSelectWindow(DockingMarkers * /*pObj*/, CDC * /*pDC*/, CRect const * /*pRect*/, bool /*bTab*/, bool /*bTop*/, int /*iTabHeight*/) {}
	virtual void DrawSelectFrame(DockingMarkers * /*pObj*/, CDC * /*pDC*/, CRect const * /*pRectNew*/, CRect const * /*pRectOld*/, bool /*bTab*/, bool /*bTop*/, int /*iTabHeight*/) {}	// uses when WasCreated()==false -> DockingMarkers wasn't created.
};
// 
class DockingMarkers : 
	public DockingMarkersAbility
{
///////////////////////////////////////
// PUBLIC
///////////////////////////////////////
public:
	struct MarkerPane
	{	MarkerPane(POINT const &ptPos, UINT uId, RECT const &rcArea);
		const CPoint pos;	// position of bitmap.
		const UINT id;		// ID of bitmap in resources.
		const CRect area;	// sensitive area (relative to this marker).
	};
	struct LayoutPane		// layout for pane's markers.
	{	LayoutPane(int iWidthTotal, int iHeightTotal,
			MarkerPane const &left, MarkerPane const &top, MarkerPane const &right, MarkerPane const &bottom, MarkerPane const &tabs, 
			COLORREF clrMask);
			// 
		const int m_iWidthTotal, m_iHeightTotal;	// total size of marker's group.
		const MarkerPane m_left, m_top, m_right, m_bottom, m_tabs;
		const COLORREF m_clrMask;		// color of not visible pixels.
	};
		// 
	struct MarkerFrame
	{	MarkerFrame(UINT uId, RECT const &rcArea);
		const UINT id;		// ID of bitmap in resources.
		const CRect area;	// sensitive area (relative to this marker).
	};
	struct LayoutFrame		// layout for pane's and frame's markers.
	{	LayoutFrame(int iWidthTotal, int iHeightTotal,
			MarkerPane const &leftPane, MarkerPane const &topPane, MarkerPane const &rightPane, MarkerPane const &bottomPane, MarkerPane const &tabsPane, 
			MarkerFrame const &leftFrame, MarkerFrame const &topFrame, MarkerFrame const &rightFrame, MarkerFrame const &bottomFrame, 
			COLORREF clrMask);
			// 
		const int m_iWidthTotal, m_iHeightTotal;	// total size of marker's group (for markers of pane).
		const MarkerPane m_leftPane, m_topPane, m_rightPane, m_bottomPane, m_tabsPane;
		const MarkerFrame m_leftFrame, m_topFrame, m_rightFrame, m_bottomFrame;
		const COLORREF m_clrMask;		// color of not visible pixels.
	};
		// 
	struct Params
	{	Params(int iTransparence, bool bAnimation, int iAnimationStep);
		const int transparence;		// 0...99% - transparence of markers.
		const bool animation;		// enable/disable animation.
		const int animationStep;	// 1...100% - step of transparence changing during animation.
	};

public:
	DockingMarkers();

public:
	bool Create(LayoutPane const &layout, Params const &params, int transpSelectBar/*1...100%*/);
	bool Create(LayoutFrame const &layout, Params const &params, int transpSelectBar/*1...100%*/);
	void Destroy();
		// 
	bool WasCreated() const;
		// 
	void ShowPaneMarkers(CRect const *rcScr);
	void HidePaneMarkers();
	void ShowFrameMarkers(CRect const *rcScr);
	void HideFrameMarkers();
		// 
	void ShowSelectBar(CWnd *pParentWnd, CRect const *rcScr, bool bTab, bool bTop, int iTabHeight);
	void HideSelectBar(CWnd *pParentWnd);
		// 
	DOCKMARKER RelayMouseMove(CPoint pt, bool testPaneMarkers, bool testFrameMarkers);
		// 
	static bool IsPaneMarker(DOCKMARKER marker);
	static bool IsFrameMarker(DOCKMARKER marker);
		// 
	void SetAbilityManager(DockingMarkersAbility *p);
	DockingMarkersAbility *GetAbilityManager() const;
	void SetDrawManager(DockingMarkersDraw *p);
	DockingMarkersDraw *GetDrawManager() const;

///////////////////////////////////////
// PRIVATE
///////////////////////////////////////
private:
	struct MarkerWnd : public CWnd
	{	MarkerWnd(DockingMarkers *p, DOCKMARKER marker);
		~MarkerWnd();
			// 
		bool Create(UINT id, BOOL (__stdcall *pSetLayerWndAttr)(HWND,COLORREF,BYTE,DWORD), COLORREF clrMask);
		void Destroy();
			// 
		void SetPos(POINT pt);
		void Disable(bool disable);
		void Highlight(bool highlight);
		bool IsHighlight() const;
		void SetTransparence(int transparence/*percents*/);
			// 
		void SetDrawManager(DockingMarkersDraw *p);
		CSize GetSize() const;

	private:
		const DockingMarkers *m_pDockingMarkers;
		const DOCKMARKER m_Marker;
			// 
		DockingMarkersDraw *m_pDrawManager;
		CImageList m_Imagelist;
		BOOL (__stdcall *m_pSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
		COLORREF m_clrMask;
			// 
		bool m_bDisable;
		bool m_bHighlight;
			// 
		DECLARE_MESSAGE_MAP()
		LRESULT OnPaint(WPARAM wp, LPARAM lp);
	};
		// 
	struct TimerObj : public Timer::INotify
	{	void Init(Params const &params);
		bool Add(MarkerWnd *wnd);
		void Destroy();
			// 
		void Show();
		void Hide();
		void Highlight(MarkerWnd *wnd, bool highlight);

	private:	// Timer::INotify.
		virtual void OnTimer(Timer *timer, int id, int time);

	private:
		Timer timer;
			// 
		int m_iMinTransparence;
		bool m_bAnimation;
		int m_iAnimationStep;
			// 
		int m_iTransparence;
		Blade::List<MarkerWnd *> lst_markWnds;
		typedef Blade::List<MarkerWnd *>::iterator i_lst_markWnds;
	};
		// 
	struct Pane
	{	Pane(DockingMarkers *p);
		MarkerWnd left, top, right, bottom, tabs;
		TimerObj timer;
		enum TYPE { TYPE_FIRST, TYPE_SECOND };
		CPoint basePoint;
	} pane1, pane2;
		// 
	struct Frame
	{	Frame(DockingMarkers *p);
		MarkerWnd left, top, right, bottom;
		TimerObj timer;
		CPoint baseLeft, baseTop, baseRight, baseBottom;
	} frame;
		// 
	DockingMarkersAbility *m_pAbilityManager;
		// 
	enum LAYOUT { LAYOUT_PANE, LAYOUT_FRAME } m_LayoutType;
	LayoutFrame m_Layout;
		// 
	bool m_bTab, m_bTop;
	int m_iTabHeight;
		// 
	Pane::TYPE m_ShownPane;

private:
	struct SelectWindow : public CWnd
	{	SelectWindow(DockingMarkers *p);
		~SelectWindow();
			// 
		bool Create(BOOL (__stdcall *pSetLayerWndAttr)(HWND,COLORREF,BYTE,DWORD), COLORREF clrMask, int iTransparence);
		void Destroy();
			// 
		void SetParams(bool bTab, bool bTop, int iTabHeight);
		void Show(HWND hWndAfter, CRect const *rcScr);
		void Hide();
			// 
		void SetDrawManager(DockingMarkersDraw *p);
		DockingMarkersDraw *GetDrawManager() const;

	private:
		const DockingMarkers *m_pDockingMarkers;
			// 
		DockingMarkersDraw *m_pDrawManager;
		COLORREF m_clrMask;
		bool m_bTab, m_bTop;
		int m_iTabHeight;
			// 
		DECLARE_MESSAGE_MAP()
		LRESULT OnPaint(WPARAM wp, LPARAM lp);
	} m_SelectWnd;

private:
	Pane *GetWorkingPane();
	Pane *GetNotWorkingPane();
	DOCKMARKER HitTest(CPoint pt, bool testPaneMarkers, bool testFrameMarkers);
	bool CreatePanesMarkerWnd(LayoutFrame const &layout, BOOL (__stdcall *ptr)(HWND,COLORREF,BYTE,DWORD), Params const &params);
	bool CreateFrameMarkerWnd(LayoutFrame const &layout, BOOL (__stdcall *ptr)(HWND,COLORREF,BYTE,DWORD), Params const &params);
	void DrawSelectFrame(CWnd *pParentWnd, CRect const *pRect);
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
class DockingMarkersDrawBase : public DockingMarkersDraw
{protected:
		// DockingMarkersDraw.
	virtual void DrawMarker(DockingMarkers *pObj, CDC *pDC, CRect const *pRect, DOCKMARKER marker, CImageList *pImageList, bool bHighlight, bool bDisable);
	virtual void DrawSelectWindow(DockingMarkers *pObj, CDC *pDC, CRect const *pRect, bool bTab, bool bTop, int iTabHeight);
	virtual void DrawSelectFrame(DockingMarkers *pObj, CDC *pDC, CRect const *pRectNew, CRect const *pRectOld, bool bTab, bool bTop, int iTabHeight);	// uses when WasCreated()==false -> DockingMarkers wasn't created.

protected:
	virtual int GetSelectBarBorderWidth(DockingMarkers *pObj);
	virtual int GetSelectFrameWidth(DockingMarkers *pObj);

protected:
	virtual COLORREF GetSelectBarInsideColor(DockingMarkers *pObj);
	virtual COLORREF GetSelectBarBorderColor(DockingMarkers *pObj);

protected:
	virtual void DrawSelectBarInside(DockingMarkers *pObj, CDC *pDC, CRect const *pRect);
	virtual void DrawSelectBarBorder(DockingMarkers *pObj, CDC *pDC, CRect const *pRect);
	virtual void DrawSelectFrame(DockingMarkers *pObj, CDC *pDC, CRect const *pRect);

protected:
	virtual void GetOuterOutline(DockingMarkers *pObj, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/);
	virtual void GetInnerOutline(DockingMarkers *pObj, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/);
};
/////////////////////////////////////////////////////////////////////////////
//
class DockingMarkersDraw_custom1 : public DockingMarkersDrawBase
{protected:
	virtual int GetSelectBarBorderWidth(DockingMarkers *pObj);
	virtual int GetSelectFrameWidth(DockingMarkers *pObj);

protected:
	virtual COLORREF GetSelectBarBorderColor(DockingMarkers *pObj);

protected:
	virtual void DrawSelectBarInside(DockingMarkers *pObj, CDC *pDC, CRect const *pRect);
	virtual void DrawSelectFrame(DockingMarkers *pObj, CDC *pDC, CRect const *pRect);
};
/////////////////////////////////////////////////////////////////////////////
//
class DockingMarkersDraw_VS2003 : public DockingMarkersDrawBase
{protected:
	virtual int GetSelectFrameWidth(DockingMarkers *pObj);
};
/////////////////////////////////////////////////////////////////////////////
// 
class DockingMarkersDraw_VS2005 : public DockingMarkersDrawBase
{protected:
	virtual int GetSelectBarBorderWidth(DockingMarkers *pObj);
};
/////////////////////////////////////////////////////////////////////////////
// 
class DockingMarkersDraw_VS2008 : public DockingMarkersDrawBase
{
};
/////////////////////////////////////////////////////////////////////////////
//
class DockingMarkersDraw_VS2010 : public DockingMarkersDrawBase
{protected:
	virtual int GetSelectBarBorderWidth(DockingMarkers *pObj);
	virtual COLORREF GetSelectBarInsideColor(DockingMarkers *pObj);

protected:
	virtual void GetOuterOutline(DockingMarkers *pObj, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/);
	virtual void GetInnerOutline(DockingMarkers *pObj, CRect const *rect, bool top, int tabHeight, int width, POINT pts[8]/*out*/);
};
#pragma warning(pop)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////













