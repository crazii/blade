/********************************************************************
	created:	2012/08/20
	filename: 	LayoutManager.h
	author:		Crazii
	purpose:	window manager for all children layout and side panels
*********************************************************************/
#ifndef __Blade_LayoutManager_h__
#define __Blade_LayoutManager_h__
#include <ui/private/MultiPaneCtrl/MultiPaneCtrl.h>
#include <interface/public/ui/IUIWindow.h>
#include <interface/public/ui/IUIWidget.h>
#include <interface/public/ui/IUILayoutManager.h>

namespace Blade
{
#if ENABLE_THEME
	struct TabCtrlStyle_VS2012_bars_black : public TabCtrlStyle_VS2008_bars_base
	{		// TabCtrlStyle_VS2008_bars_base.
		virtual COLORREF GetBorderColor(TabCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetCtrlAreaBackColor(TabCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetTabBorderColor(TabCtrl* /*pCtrl*/, bool hover)
		{
			return hover ? THEMECOLOR(TC_HILIGHTBK) : THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetTabGradientLightColor(TabCtrl* /*pCtrl*/, bool hover, bool /*disable*/)
		{
			return hover ? THEMECOLOR(TC_HILIGHTBK) : THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetTabGradientDarkColor(TabCtrl* /*pCtrl*/, bool hover, bool /*disable*/)
		{
			return hover ? THEMECOLOR(TC_HILIGHTBK) : THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetTabSelectedBackColor(TabCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_HILIGHTBK);
		}
		virtual COLORREF GetWndsAreaBackColor(TabCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetTabTextColor(TabCtrl* /*pCtrl*/, HTAB /*hTab*/)
		{
			return THEMECOLOR(TC_TEXT);
		}
		virtual COLORREF GetButtonMarkerColor(TabCtrl* /*pCtrl*/, bool /*bHover*/, bool bPushed)
		{
			return bPushed ? RGB(0,0,0) : THEMECOLOR(TC_TEXT);
		}
		virtual COLORREF GetChildWndBackColor(TabCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_WINBK);
		}
		virtual COLORREF GetEmptyWndsAreaBackColor(TabCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_WINBK);
		}
	};
	struct MultiPaneCtrlStyle_VS2012_bars_black : public MultiPaneCtrlStyle_VS2008_base
	{		// IMultiPaneCtrlStyle.
		virtual ITabCtrlStyle *GetTabStyle() { return &tabStyle; }

		virtual COLORREF GetSplitterColor(MultiPaneCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_SPLITTER);
		}
		virtual COLORREF GetBorderColor(MultiPaneCtrl* /*pCtrl*/)
		{
			return THEMECOLOR(TC_FRAME);
		}
	protected:
		TabCtrlStyle_VS2012_bars_black tabStyle;
	};
#else
	typedef TabCtrlStyle_VS2008_bars_classic TabCtrlStyle_VS2012_bars_black;
	typedef MultiPaneCtrlStyle_VS2008_bars_classic MultiPaneCtrlStyle_VS2012_bars_black;
#endif

	class UIWidgetLayout;
	class UILayoutNode;

	class LayoutManager : public IUILayoutManager, public IWindowEventHandler,
		public MultiPaneCtrlUserAbility, public MultiPaneCtrlNotify,	//multi-pane control
		public StaticAllocatable
	{
	public:
		typedef List<IUIWindow*>					ChildWindowList;
		typedef Set<IUIWidget*, FnWidgetNameLess>	WidgetList;
	public:
		LayoutManager();
		virtual ~LayoutManager();

		/************************************************************************/
		/* IUILayoutManager interface                                                                     */
		/************************************************************************/
		/** @brief  create a window, if caption not specified,use name as caption */
		virtual IUIWindow*		createWindow(IconIndex icon, const TString& wtype, const TString& caption = TString::EMPTY);

		/** @brief get the active window */
		virtual IUIWindow*		getActiveWindow() const;

		/** @brief  */
		virtual IUIWidget*	createWidget(const WIDGET_DATA& widgetData);

		/** @brief  */
		inline virtual IUIWidget*	getWidget(const TString& name) const
		{
			UIWidgetFinder finder(name);

			WidgetList::const_iterator i = mWidgets.find(&finder);
			return i != mWidgets.end() ? *i : NULL;
		}

		/** @brief get top level widget count */
		inline virtual size_t		getWidgetCount() const { return mWidgets.size(); }

		/** @brief get top level widget */
		inline virtual IUIWidget*	getWidget(index_t index) const
		{
			if (index < mWidgets.size())
			{
				WidgetList::const_iterator i = mWidgets.begin();
				std::advance(i, index);
				return *i;
			}
			return NULL;
		}

		/** @brief  */
		virtual bool		applyLayout(const UIWidgetLayout& layout);

		/** @brief  */
		virtual bool		loadLastLayout();

		/** @brief  */
		virtual bool		saveLayout() const;

		/** @brief  */
		virtual bool		toggleWidget(IUIWidget* widget, bool visible);

		/** @brief  */
		virtual bool		activateWidget(IUIWidget* widget, bool active);

		/************************************************************************/
		/* IWindowEventHandler interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void onWindowDestroy(IWindow* window);
		/** @brief  */
		virtual void onWindowCaptionChanged(IWindow* window);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool			initialize(CFrameWnd *parent, DWORD style, RECT const &rect, CImageList* iconList, CImageList* grayIconList);

		/** @brief  */
		bool			shutdown();

		/************************************************************************/
		/* child window management                                                                     */
		/************************************************************************/
		/** @brief  */
		IUIWindow*		findChildWindow(IWindow* window) const;

		/** @brief  */
		bool			createInitChild();

		/************************************************************************/
		/* widget (dock window) management                                                                     */
		/************************************************************************/
		/** @brief  */
		bool			addWidget(IUIWidget* widget);

		/************************************************************************/
		/* PaneCtrl callbacks                                                                     */
		/************************************************************************/
		// MultiPaneCtrlUserAbility.
		virtual bool CanDrop(MultiPaneCtrl* pCtrl, HTAB hTabSrc, DOCKSIDE side, HPANE hPaneDst);
		// MultiPaneCtrlNotify.
		virtual void OnTabCloseButtonClicked(MultiPaneCtrl* pPaneCtrl, TabCtrl* pTabCtrl, CRect const* pRect, CPoint ptScr);
		virtual void OnTabFinishDrag(MultiPaneCtrl* pPaneCtrl, TabCtrl* pTabCtrl, HTAB hTab, bool cancel);

		/** @brief  */
		inline HPANE findPane(UILayoutNode* node)
		{
			HPANE ret = NULL;
			for (HPANE pane = mPanelCtrl.GetFirst(); pane != NULL; pane = mPanelCtrl.GetNext(pane))
			{
				if ((UILayoutNode*)mPanelCtrl.GetData(pane) == node)
				{
					ret = pane;
					break;
				}
			}
			return ret;
		}

		//layouts
		MultiPaneCtrlEx<MultiPaneCtrlStyle_VS2012_bars_black>	mPanelCtrl;

		//child windows(center window)
		ChildWindowList			mChildWindows;
		CWnd*					mInitChild;	//dummy child

		UIWidgetLayout*			mLayout;	//panels (side window)
		WidgetList				mWidgets;
		HPANE					mCenterPane;
	};//class LayoutManager
	
}//namespace Blade

#endif //  __Blade_LayoutManager_h__