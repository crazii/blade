/********************************************************************
	created:	2012/08/20
	filename: 	LayoutManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "resource.h"
#include <interface/public/ui/UIWidgetLayout.h>
#include <ui/LayoutManager.h>
#include <ui/DockWindow.h>
#ifdef max
#undef max
#endif
namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	typedef struct SPaneItem
	{
		HPANE			first;
		UILayoutNode*	second;
		bool			this_node;	//post-order mark

		SPaneItem() {}
		SPaneItem(HPANE pane, UILayoutNode* node, bool skip_child)
			:first(pane), second(node), this_node(skip_child) {}
	}PaneItem;

	const TString LAYOUT_FILE = BTString("cwd:last.uilayout");

	//////////////////////////////////////////////////////////////////////////
	class CEmpty : public CWnd, public StaticAllocatable
	{
	public:
		using StaticAllocatable::operator new;
		using StaticAllocatable::operator delete;
	public:
		virtual ~CEmpty() {}
		DECLARE_MESSAGE_MAP()
		afx_msg void OnPaint()
		{
			CPaintDC dc(this);
			CRect rect;
			this->GetClientRect(&rect);
			dc.FillSolidRect(&rect, THEMECOLOR(TC_EMPTYBK));
		}
	};

	BEGIN_MESSAGE_MAP(CEmpty, CWnd)
		ON_WM_PAINT()
	END_MESSAGE_MAP()

#define DOCKWINDOW_IDBASE	(3000)

	//////////////////////////////////////////////////////////////////////////
	struct MarkersLayoutA : public DockingMarkers::LayoutPane, public NonAssignable
	{
		MarkersLayoutA() : LayoutPane(107, 105,		// total size of marker's group.
			DockingMarkers::MarkerPane(CPoint(0, 35), IDB_BITMAP_LEFT, CRect(2, 2, 34, 33)),	// left marker.
			DockingMarkers::MarkerPane(CPoint(36, 0), IDB_BITMAP_TOP, CRect(2, 2, 33, 34)),		// top marker.
			DockingMarkers::MarkerPane(CPoint(71, 35), IDB_BITMAP_RIGHT, CRect(2, 2, 34, 33)),	// right marker.
			DockingMarkers::MarkerPane(CPoint(36, 69), IDB_BITMAP_BOTTOM, CRect(2, 2, 33, 34)),	// bottom marker.
			DockingMarkers::MarkerPane(CPoint(15, 15), IDB_BITMAP_TABS, CRect(2, 2, 75, 73)),	// marker of tabs.
			RGB(255, 0, 255))	// color of mask (pixels which don't show).
		{
		}
	};

	//////////////////////////////////////////////////////////////////////////
	LayoutManager::LayoutManager()
	{
		mInitChild = NULL;
		mCenterPane = NULL;
		mLayout = BLADE_NEW UIWidgetLayout();
	}

	//////////////////////////////////////////////////////////////////////////
	LayoutManager::~LayoutManager()
	{
		BLADE_DELETE mLayout;
	}

	/************************************************************************/
	/* IUILayoutManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IUIWindow*		LayoutManager::createWindow(IconIndex icon, const TString& wtype, const TString& caption/* = TString::EMPTY*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		IUIWindow* child = BLADE_FACTORY_CREATE(IUIWindow, wtype);
		TabCtrl* tab = mPanelCtrl.GetTabCtrl(mCenterPane);
		uintptr_t parent = (uintptr_t)(HWND)*tab;
		bool ret = child->initWindow(parent, icon, caption);
		if (!ret)
		{
			assert(false);
			BLADE_DELETE child;
			return NULL;
		}
		child->getWindow()->addEventHandler(this);

		if (mChildWindows.size() == 0)
		{
			//remove init child from tab
			HTAB tabItem = tab->GetTabWithWindow(*mInitChild);
			assert(tabItem != NULL);
			tab->Delete(tabItem);
		}

		HWND hWnd = (HWND)(void*)child->getWindow()->getNativeHandle();
		HTAB exist = tab->GetTabWithWindow(hWnd);
		assert(exist == NULL);
		exist = tab->Add(hWnd, caption.c_str(), icon);
		tab->SetTabData(exist, (intptr_t)child);
		tab->SetSel(exist);
		tab->Update();
		mChildWindows.push_back(child);
		return child;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIWindow*		LayoutManager::getActiveWindow() const
	{
		if (mChildWindows.size() == 0)
			return NULL;
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		TabCtrl* tab = mPanelCtrl.GetTabCtrl(mCenterPane);
		HTAB item = tab->GetSel();
		if (item == NULL)
			return NULL;

		intptr_t ptr = (intptr_t)tab->GetTabData(item);
		return (IUIWindow*)ptr;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIWidget*		LayoutManager::createWidget(const WIDGET_DATA& widgetData)
	{
		if (widgetData.mName.empty() || this->getWidget(widgetData.mName) != NULL)
		{
			assert(false);
			return NULL;
		}

		IUIWidget* widget = NULL;
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		widget = BLADE_FACTORY_CREATE(IUIWidget, widgetData.mType);
		if (!widget->initWidget(widgetData))
		{
			BLADE_DELETE widget;
			widget = NULL;
		}

		if (widget != NULL)
		{
			bool ret = this->addWidget(widget);
			assert(ret);
			BLADE_UNREFERENCED(ret);
		}
		return widget;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LayoutManager::applyLayout(const UIWidgetLayout& layout)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		if (&layout != mLayout)
		{
			*mLayout = layout;
			const fp32* rootSize = mLayout->getRoot().getSize();
			if (rootSize[0] >= 0.99f && rootSize[1] >= 0.95f)
				::AfxGetMainWnd()->ShowWindow(SW_MAXIMIZE);
			else
			{
				int cx = (int)(rootSize[0] * screenWidth);
				int cy = (int)(rootSize[1] * screenHeight);
				::AfxGetMainWnd()->SetWindowPos(NULL, mLayout->getRootPosition().x, mLayout->getRootPosition().y, cx, cy, SWP_NOZORDER);
				::AfxGetMainWnd()->ShowWindow(SW_SHOW);
			}
		}
		mLayout->getRoot().adjustSize(SIZE2I(screenWidth, screenHeight));

		IUIWindow* activeChild = this->getActiveWindow();
		HTAB hActiveTab = NULL;

		mCenterPane = NULL;
		mPanelCtrl.DeleteAll();
		for (WidgetList::iterator i = mWidgets.begin(); i != mWidgets.end(); ++i)
		{
			DockWindow<IUIWidgetLeaf>* widget = static_cast<DockWindow<IUIWidgetLeaf>*>(*i);
			widget->setVisible(false);
		}
		CSize minSize(32, 32);

		typedef std::pair<HPANE, UILayoutNode*> PaneNode;
		typedef TempQueue<PaneNode> NodeQueue;
		NodeQueue queue;
		queue.push_back(PaneNode(mPanelCtrl.GetRoot(), &mLayout->getRoot()));
		while (!queue.empty())
		{
			PaneNode item = queue.front();
			queue.pop();

			HPANE pane = item.first;
			UILayoutNode* node = item.second;
			mPanelCtrl.SetData(pane, (intptr_t)node);

			size_t childCount = node->getChildCount();

			//widgets
			if (node->getWidgetCount() == 0 && childCount == 0)
				assert(false);
			size_t widgetCount = 0;
			bool content = false;

			//add sub tabs for child node
			for (index_t i = 0; i < node->getWidgetCount(); ++i)
			{
				const UILayoutNode::SWidgetInfo* widgetInfo = node->getWidgetInfo(i);
				assert(widgetInfo != NULL);
				const TString& name = widgetInfo->name;

				HWND hWnd = NULL;
				int icon = -1;
				IUIWidget* widget = NULL;

				if (name == IUILayoutManager::WORKING_AREA)
				{
					mCenterPane = pane;
					hWnd = *mInitChild;
					if (mChildWindows.size() == 0)
					{
						//add init child
						HWND hInitWnd = *mInitChild;
						hActiveTab = mPanelCtrl.AddTab(pane, hInitWnd, IUILayoutManager::WORKING_AREA.c_str(), INVALID_ICON_INDEX);
						mPanelCtrl.GetTabCtrl(pane)->SetTabData(hActiveTab, (intptr_t)NULL);
					}
					//add child window
					for (ChildWindowList::const_iterator j = mChildWindows.begin(); j != mChildWindows.end(); ++j)
					{
						assert(widget == NULL);
						IUIWindow* childWindow = *j;
						icon = childWindow->getIcon();
						IWindow* window = childWindow->getWindow();
						HWND hChildWnd = (HWND)(void*)window->getNativeHandle();
						HTAB hTab = mPanelCtrl.AddTab(pane, hChildWnd, window->getCaption().c_str(), icon);
						mPanelCtrl.GetTabCtrl(pane)->SetTabData(hTab, (intptr_t)childWindow);
						if (childWindow == activeChild)
							hActiveTab = hTab;
					}
					content = true;
				}
				else
				{
					//if (widgetInfo->flags&WMF_FIXED)
					//{
					//	if (node->getLayoutType() == UILayoutNode::LT_HORIZONTAL)
					//		minSize.cx = std::max((int)minSize.cx, widgetInfo->size.x);
					//	else
					//		minSize.cy = std::max((int)minSize.cy, widgetInfo->size.y);
					//}

					widget = this->getWidget(name);
					if (widget != NULL)
					{
						if ((widgetInfo->flags&WMF_VISIBLE))
							widget->setVisible(true);

						hWnd = *(static_cast<DockWindow<IUIWidgetLeaf>*>(widget)->getCWnd());
						icon = widget->getIcon();
						++widgetCount;
						if (hWnd != NULL && (node->getWidgetVisibility(i) || node->getActiveWidget() == i))
						{
							//panels
							HTAB hTab = mPanelCtrl.AddTab(pane, hWnd, widget->getCaption().c_str(), icon);
							mPanelCtrl.GetTabCtrl(pane)->SetTabData(hTab, (intptr_t)widget);
						}
					}
				}
			}

			//no valid window found (maybe plugin not installed on this time)
			if (widgetCount == 0 && childCount == 0 && !content)
				mPanelCtrl.Delete(pane, SPACE_BETWEEN);

			//show active window
			if (widgetCount != 0 && !content)
			{
				IUIWidget* widget = NULL;
				indexdiff_t dir = -1;	//backwards
				index_t active = node->getActiveWidget();
				if (active == INVALID_INDEX)
					active = 0;
				while (widget == NULL)
				{
					const TString& name = node->getWidget(active);
					widget = this->getWidget(name);
					if (widget == NULL)
					{
						active += dir;
						if (active == INVALID_INDEX)
						{
							assert(dir == -1);
							dir = 1;	//turn forwards
							active = node->getActiveWidget();
							if (active == INVALID_INDEX)
								++active;
						}
						assert(active < node->getWidgetCount());//to the end, no valid window found: impossible
					}
				}
				assert(widget != NULL);
				node->setActiveWidget(active);
				widget->setActive(true);
				TabCtrl* tabctrl = mPanelCtrl.GetTabCtrl(pane);
				for (int i = 0; i < tabctrl->GetCount(); ++i)
				{
					HTAB hTab = tabctrl->GetTab(i);
					if (tabctrl->GetTabData(hTab) == (intptr_t)widget)
					{
						tabctrl->SetSel(hTab);
						break;
					}
				}
			}

			//children
			if (childCount != 0)
			{
				assert(node->getLayoutType() != UILayoutNode::LT_UNDEFINED);

				//find first visible child
				size_t firstChild = 0;
				while (firstChild < node->getChildCount() && !node->getChild(firstChild)->isVisible()) ++firstChild;

				//all sub nodes in-visible: this is impossible because if all sub nodes invisible, \n
				//then this node should be invisible and the code should not run to here
				assert(firstChild < node->getChildCount());
				UILayoutNode* firstNode = node->getChild(firstChild);
				assert(firstNode != NULL);

				HPANE firstPane = mPanelCtrl.ConvertToLine(pane, node->getLayoutType() == UILayoutNode::LT_HORIZONTAL);

				TempVector<int> portions;
				for (index_t i = firstChild; i < childCount; ++i)
				{
					UILayoutNode* subNode = node->getChild(i);
					if (subNode->isVisible())
					{
						const fp32* sizes = subNode->getSize();
						portions.push_back( (int)(mPanelCtrl.IsHorizontal(pane) ? sizes[0] * 100 + 0.5f : sizes[1] * 100 + 0.5f) );

						HPANE newPane = subNode == firstNode ? firstPane : mPanelCtrl.Add(pane);
						queue.push_back(PaneNode(newPane, subNode));
					}
				}
				if (portions.size() > 1)
				{
					int sum = 0;
					for (size_t i = 0; i < portions.size() - 1; ++i)
						sum += portions[i];
					//make multi pane control happy
					portions.back() = 100 - sum;
					mPanelCtrl.SetPortions(pane, &portions[0]);
				}
			}

		}
		//cannot set min size for each pane. TODO:
		mPanelCtrl.SetMinSize(minSize);

		assert(mCenterPane != NULL);
		assert(hActiveTab != NULL);
		mPanelCtrl.GetTabCtrl(mCenterPane)->SetSel(hActiveTab);
		mPanelCtrl.Update();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LayoutManager::loadLastLayout()
	{
		UIWidgetLayout layout;
		return layout.load(LAYOUT_FILE) && this->applyLayout(layout);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LayoutManager::saveLayout() const
	{
		//update top level window size & pos
		CRect rect;
		::AfxGetMainWnd()->GetWindowRect(&rect);
		mLayout->setRootPosition(POINT2I(rect.left, rect.top));
		fp32 x = (fp32)rect.Width() / (fp32)::GetSystemMetrics(SM_CXSCREEN);
		fp32 y = (fp32)rect.Height() / (fp32)::GetSystemMetrics(SM_CYSCREEN);
		mLayout->getRoot().setSize(x, y);

		//update widget's pos/size/visibility
		for (WidgetList::const_iterator i = mWidgets.begin(); i != mWidgets.end(); ++i)
		{
			bool visible = static_cast<DockWindow<IUIWidgetLeaf>*>(*i)->shouldBeVisible();
			(*i)->setVisible(visible);
			mLayout->updateWidget(*i);
		}

		//update all panes' sizes
		for(HPANE pane = mPanelCtrl.GetFirst(); pane != NULL; pane = mPanelCtrl.GetNext(pane))
		{
			UILayoutNode* parentNode = (UILayoutNode*)(mPanelCtrl.GetParent(pane) != NULL ? mPanelCtrl.GetData(mPanelCtrl.GetParent(pane)) : NULL);
			if(parentNode == NULL)
				continue;

			UILayoutNode* node = (UILayoutNode*)mPanelCtrl.GetData(pane);
			if (node == NULL)
				continue;
			const fp32* sizes = node->getSize();
			x = sizes[0];
			y = sizes[1];
			if (parentNode->getLayoutType() == UILayoutNode::LT_HORIZONTAL)
				x = (fp32)mPanelCtrl.GetPortion(pane);
			else
				y = (fp32)mPanelCtrl.GetPortion(pane);
			node->setSize(x, y);

			if (!mPanelCtrl.IsLine(pane) && node->findWidget(WORKING_AREA) == INVALID_INDEX)
			{
				TabCtrl* tabCtrl = mPanelCtrl.GetTabCtrl(pane);
				HTAB tab = tabCtrl->GetSel();
				int sel = tabCtrl->GetIndex(tab);
				node->setActiveWidget((index_t)sel);
			}
		}

		return mLayout->save(LAYOUT_FILE);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LayoutManager::toggleWidget(IUIWidget* widget, bool visible)
	{
		UIWidgetLayout::OutputBranch output;
		if (!mLayout->findWidget(widget->getName(), output))
			return false;

		widget->setVisible(visible);
		//update info, including parent node
		mLayout->updateWidget(widget);

		this->applyLayout(*mLayout);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		LayoutManager::activateWidget(IUIWidget* widget, bool active)
	{
		bool ret = IUILayoutManager::activateWidget(widget, active);
		if (!ret)
			return ret;

		UIWidgetLayout::OutputBranch output;
		if (!mLayout->findWidget(widget->getName(), output))
			return false;

		UILayoutNode* node = output[output.size() - 1];
		for (size_t i = 0; i < node->getWidgetCount(); ++i)
		{
			const TString& name = node->getWidget(i);
			if (name == widget->getName())
			{
				if(active)
					node->setActiveWidget(i);
				continue;
			}
			IUIWidget* other = this->getWidget(name);
			if(other == NULL || !static_cast<DockWindow<IUIWidgetLeaf>*>(other)->shouldBeVisible() )	//widget may not installed this time
				continue;
			other->setActive(!active);
			if (!active)
			{
				node->setActiveWidget(i);
				break;
			}
		}

		TPointerParam<UILayoutNode::SWidgetInfo> visibleWidgets;
		node->getVisibleWidgets(visibleWidgets);
		const TString& activeName = node->getWidget(node->getActiveWidget());

		index_t index = INVALID_INDEX;
		for (size_t i = 0; i != visibleWidgets.size(); ++i)
		{
			if (activeName == visibleWidgets[i]->name && (active || activeName != widget->getName()) )
			{
				index = i;
				break;
			}
		}

		if (index != INVALID_INDEX)
		{
			HPANE pane = this->findPane(node);
			assert(pane != NULL && !mPanelCtrl.IsLine(pane));
			TabCtrl* tabCtrl = mPanelCtrl.GetTabCtrl(pane);
			tabCtrl->SetSel(tabCtrl->GetTab((int)index));
		}
		mPanelCtrl.Update();
		return true;
	}

	/************************************************************************/
	/* IWindowEventHandler interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			LayoutManager::onWindowDestroy(IWindow* window)
	{
		ChildWindowList::const_iterator iter = mChildWindows.end();
		for (ChildWindowList::const_iterator i = mChildWindows.begin(); i != mChildWindows.end(); ++i)
		{
			IUIWindow* childWindow = *i;
			if (childWindow == NULL)
			{
				assert(false);
				continue;
			}
			if (window == childWindow->getWindow())
			{
				iter = i;
				break;
			}
		}

		if (iter == mChildWindows.end())
		{
			assert(false);
			return;
		}

		TabCtrl* tab = mPanelCtrl.GetTabCtrl(mCenterPane);
		HWND hWnd = (HWND)(void*)window->getNativeHandle();
		HTAB tabItem = tab->GetTabWithWindow(hWnd);
		assert(tabItem != NULL);
		tab->Delete(tabItem);
		//BLADE_DELETE child;	//PostNcDestroy() will delete the window, WE DON'T NEED TO DELETE IT

		mChildWindows.erase(iter);
		if (mChildWindows.size() == 0)
			mPanelCtrl.AddTab(mCenterPane, *mInitChild, IUILayoutManager::WORKING_AREA.c_str(), -1);
		tab->Update();
	}

	//////////////////////////////////////////////////////////////////////////
	void			LayoutManager::onWindowCaptionChanged(IWindow* window)
	{
		IUIWindow* child = this->findChildWindow(window);
		if (child == NULL)
		{
			assert(false);
			return;
		}

		TabCtrl* tab = mPanelCtrl.GetTabCtrl(mCenterPane);
		HWND hWnd = (HWND)(void*)window->getNativeHandle();
		HTAB tabItem = tab->GetTabWithWindow(hWnd);
		if (tabItem != NULL)
		{
			tab->SetTabText(tabItem, window->getCaption().c_str());
			tab->SetTabTooltipText(tabItem, window->getCaption().c_str());
			tab->Update();
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			LayoutManager::initialize(CFrameWnd *parent, DWORD style, RECT const &rect, CImageList* iconList, CImageList* grayIconList)
	{
		//init panel ctrl
		mPanelCtrl.Create(parent, style, rect, AFX_IDW_PANE_FIRST);
		mPanelCtrl.SetTabBehavior(TAB_BEHAVIOR_SCALE);
		mPanelCtrl.RemoveTabEnable(true);
		mPanelCtrl.DragTabEnable(true);
		mPanelCtrl.ShowTabScrollButtons(false);
		mPanelCtrl.ShowTabMenuButton(false);
		mPanelCtrl.ShowTabCloseButton(true);
		mPanelCtrl.TabToolTipEnable(true);
		mPanelCtrl.HideSingleTab(false);
		mPanelCtrl.SetDockMarkers(MarkersLayoutA(), DockingMarkers::Params(25, true, 14), 61);

		CImageList imagelist;
		CBitmap bmpBuiltin;
		imagelist.Create(14, 14, ILC_COLOR24 | ILC_MASK, 7, 0);
		bmpBuiltin.LoadBitmap(IDB_BUILTIN);
		imagelist.Add(&bmpBuiltin, RGB(255, 0, 255));
		mPanelCtrl.SetSystemImageList(&imagelist);
		mPanelCtrl.SetImageListsRef(iconList, grayIconList);
		mPanelCtrl.SetNotifyManager(this);
		mPanelCtrl.SetAbilityManager(this);
		//pre-create child
		this->createInitChild();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LayoutManager::shutdown()
	{
		for (WidgetList::iterator i = mWidgets.begin(); i != mWidgets.end(); ++i)
		{
			IUIWidget* widget = *i;
			BLADE_DELETE widget;
		}
		mWidgets.clear();

		//note: ChildWindow will be auto destroyed by MFC, via PostNcDestroy
		//for(ChildWindowList::iterator i = mChildWindows.begin(); i != mChildWindows.end(); ++i)
		//{
		//	IUIWindow* window = *i;
		//	BLADE_DELETE window;
		//}
		BLADE_DELETE mInitChild;
		mInitChild = NULL;
		return true;
	}

	/************************************************************************/
	/* child window management                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IUIWindow*		LayoutManager::findChildWindow(IWindow* window) const
	{
		for (ChildWindowList::const_iterator i = mChildWindows.begin(); i != mChildWindows.end(); ++i)
		{
			IUIWindow* childWindow = *i;
			if (childWindow == NULL)
			{
				assert(false);
				continue;
			}
			if (window == childWindow->getWindow())
				return childWindow;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LayoutManager::createInitChild()
	{
		if (mChildWindows.size() != 0 || mInitChild != NULL)
		{
			assert(false);
			return false;
		}
		mInitChild = BLADE_NEW CEmpty();
		CRect rect(0, 0, 9999, 9999);
		mInitChild->Create(TEXT("STATIC"), IUILayoutManager::WORKING_AREA.c_str(), WS_CHILD | WS_VISIBLE, rect, mPanelCtrl.CWnd::GetParent(), UINT(-1));
		return true;
	}

	/************************************************************************/
	/* widget (dock window) management                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			LayoutManager::addWidget(IUIWidget* widget)
	{
		if (widget == NULL)
			return false;
		else
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			DockWindow<IUIWidgetLeaf>* dockWindow = static_cast<DockWindow<IUIWidgetLeaf>*>(widget);
			bool ret = mWidgets.insert(widget).second;
			if (ret && dockWindow->getCWnd()->GetDlgCtrlID() == UINT(-1))
				dockWindow->getCWnd()->SetDlgCtrlID(DOCKWINDOW_IDBASE + (int)mWidgets.size() - 1);
			else if (dockWindow->getCWnd()->GetDlgCtrlID() == AFX_IDW_DOCKBAR_LEFT) {}
			else
				assert(false);
			return ret;
		}
	}

	/************************************************************************/
	/* PaneCtrl callbacks                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool LayoutManager::CanDrop(MultiPaneCtrl *pCtrl, HTAB hTabSrc, DOCKSIDE side, HPANE hPaneDst)
	{
		BLADE_UNREFERENCED(side);

		if (hPaneDst == mCenterPane && side == DOCKSIDE_TABS)
			return false;

		HPANE pane = pCtrl->GetPane(hTabSrc);
		if (pane == mCenterPane)
			return false;

		TabCtrl* tabCtrl = pCtrl->GetTabCtrl(pane);
		//be careful of the pointer used here
		IUIWidget* widget = (IUIWidget*)tabCtrl->GetTabData(hTabSrc);
		assert(widget != NULL);
		//TODO: determine the new dock place of widget
		return (widget->getDockFlag() & WDF_MOVABLE) != 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void LayoutManager::OnTabCloseButtonClicked(MultiPaneCtrl *pPaneCtrl, TabCtrl *pTabCtrl, CRect const *pRect, CPoint ptScr)
	{
		BLADE_UNREFERENCED(ptScr);
		BLADE_UNREFERENCED(pRect);

		TabCtrl* center = pPaneCtrl->GetTabCtrl(mCenterPane);
		if (pTabCtrl == center)
		{
			IUIWindow* window = this->getActiveWindow();
			if (window != NULL)
				window->getWindow()->closeWindow();
			pPaneCtrl->Update();
		}
		else
		{
			HTAB item = pTabCtrl->GetSel();
			if (item == NULL)
				return;
			IUIWidget* widget = (IUIWidget*)pTabCtrl->GetTabData(item);
			assert(widget != NULL);
			this->toggleWidget(widget, false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void LayoutManager::OnTabFinishDrag(MultiPaneCtrl* pPaneCtrl, TabCtrl* pTabCtrl, HTAB hTab, bool cancel)
	{
		if (cancel)
			return;

		if (hTab == NULL || pTabCtrl == NULL) //tab moved within the pane, not moved at all
			return;
		pPaneCtrl->Update();

		IUIWidget* widget = (IUIWidget*)pTabCtrl->GetTabData(hTab);
		//remove window from its original parent
		bool ret = mLayout->removeWidget(widget);
		assert(ret); BLADE_UNREFERENCED(ret);

		HPANE targetPane = pPaneCtrl->GetPane(pTabCtrl);
		UILayoutNode* targetNode = (UILayoutNode*)pPaneCtrl->GetData(targetPane);
		//target pane may be a new created one, or existing one,\n
		//but its parent should be already exist
		if (targetNode == NULL) //newly created pane
		{
			HPANE parentPane = pPaneCtrl->GetParent(targetPane);
			UILayoutNode* parentNode = (UILayoutNode*)pPaneCtrl->GetData(parentPane);
			assert(parentNode != NULL);

			if(parentNode->getWidgetCount() != 0 )
			{
				parentPane = pPaneCtrl->GetParent(parentPane);
				parentNode = (UILayoutNode*)pPaneCtrl->GetData(parentPane);
			}

			assert(parentNode->getWidgetCount() == 0);
			targetNode = BLADE_NEW UILayoutNode();
			targetNode->setLayoutType(parentNode->getLayoutType() == UILayoutNode::LT_HORIZONTAL ? UILayoutNode::LT_VERTICAL : UILayoutNode::LT_HORIZONTAL);
			parentNode->addChild(targetNode);
		}
		pPaneCtrl->SetData(targetPane, (intptr_t)targetNode);
		targetNode->setVisible(true);
		index_t index = targetNode->addWidget(widget);
		assert(index != INVALID_INDEX);
		targetNode->setActiveWidget(index);
	}

}//namespace Blade