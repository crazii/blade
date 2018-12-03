/********************************************************************
	Created:	5:6:2004   16:44
	Filename: 	BHTabedReBar
	File ext:	cpp (implementation file)
	Author:		Bahrudin Hrnjica
	e-mail:		bhrnjica@hotmail.com
	Web Page:	www.baha.dreamstation.com

	Disclaimer
	----------
	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
	RISK OF USING THIS SOFTWARE.

	Terms of use
	------------
	This code may be used in compiled form in any way you desire. This
	file may be redistributed unmodified by any means PROVIDING it is 
	not sold for profit without the authors written consent, and 
	providing that this notice and the authors name and all copyright 
	notices remains intact. 

	An email letting me know how you are using it would be nice as well. 

	This file is provided "as is" with no expressed or implied warranty.
	The author accepts no liability for any damage/loss of business that
	this product may cause.

	History:
	Version: 1.0.0.109 First Release Version. 
	This control is based on the Paul Selormey article "Automatic Tab Bar for MDI programs" 
	posted on August 1, 1999 on www.codeguru.com. 
	BHTabedReBar is similar to standard MFC 
	CReBar which colect toolbars, dialogbars and statusbars and shows them as Tabed panes.


	TODO:  
	1)Imeplementation  Ownerdraw TabCtrl
	2)Docking to any side of Frame Window
	3)Implementation of chevrons
	4)

	*********************************************************************/

/********************************************************************
	created:	2011/05/10
	filename: 	TabedReBar.cc
	author:		Bahrudin Hrnjica
	Licence:	CPOL
	url:		http://www.codeproject.com/KB/toolbars/TabedReBar.aspx
	modified by:Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <interface/IEventManager.h>
#include <interface/public/ui/UIEvents.h>
#include <ui/IconManager.h>
#include <ui/UIToolBox.h>
#include "TabedReBar.h"
#include "resource.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LEFT_EDGE (0)
#define RIGHT_EDGE (-1)
#define BOTTOM_EDGE (0)
#define CONTROLBAR_DEFLATE (1)
#define BAR_GAP	(1)
#define MIN_BUTTONS (2)		//minimal button count horizontal

//main definitions
#define IDC_TABED_REBAR						20360

namespace Blade
{

	// TabedReBar
	IMPLEMENT_DYNAMIC(TabedReBar, CDockBar)

	BEGIN_MESSAGE_MAP(TabedReBar, CDockBar)
		//{{AFX_MSG_MAP(TabedReBar)
		ON_WM_CREATE()
		ON_NOTIFY(TCN_SELCHANGE, IDC_TABED_REBAR, OnTabSelChange)
		ON_WM_WINDOWPOSCHANGED()
		ON_WM_ERASEBKGND()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	const TString TabedReBar::WIDGET_TYPE = BTString(BLANG_TOOL);

	//////////////////////////////////////////////////////////////////////////
	TabedReBar::TabedReBar()
		:CDockBar()
		, DockWindow(WIDGET_TYPE)
		, m_bRecalcLayout(FALSE)
		, m_bSizeChanged(FALSE)
		, mTotalBarCount(0)
		, m_nWidthButtons(MIN_BUTTONS)
	{
		m_rectClient.SetRectEmpty();
		m_nActiveTab = -1;
		m_bAutoDelete = false;
	}

	//////////////////////////////////////////////////////////////////////////
	TabedReBar::~TabedReBar()
	{
		m_tabctrl.SetImageList(NULL);
		this->RemoveAll();

		//HACK: restore the original left dock bar
		ASSERT_KINDOF(CFrameWnd, ::AfxGetMainWnd());
		CFrameWnd* mainWnd = static_cast<CFrameWnd*>(::AfxGetMainWnd());
		CPtrList& controlBarList = mainWnd->m_listControlBars;
		POSITION pos = controlBarList.GetHeadPosition();

		bool found = false;
		while (pos != NULL)
		{
			CControlBar* pBar = (CControlBar*)controlBarList.GetNext(pos);	//++pos
			ASSERT(pBar != NULL);
			if (pBar == this)
			{
				//back to previous
				controlBarList.GetPrev(pos);	//--pos
				controlBarList.RemoveAt(pos);
				found = true;
				break;
			}
		}

		assert(found);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TabedReBar::createWindow()
	{
		if (this->Create(::AfxGetMainWnd(), WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT, AFX_IDW_DOCKBAR_LEFT))
		{
			//HACK: add to control bar list
			ASSERT_KINDOF(CFrameWnd, ::AfxGetMainWnd());
			CFrameWnd* mainWnd = static_cast<CFrameWnd*>(::AfxGetMainWnd());
			CPtrList& controlBarList = mainWnd->m_listControlBars;
			controlBarList.AddHead(this);

			this->SetBarStyle(this->GetBarStyle() | CBRS_SIZE_DYNAMIC);
			this->EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_TOP);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TabedReBar::setupCategory(const TString& defCategory, IconIndex defIcon, const TString* categories, IconIndex* icons, size_t count)
	{
		bool ret = true;
		for(size_t i = 0; i < count; ++i)
		ret = mCategoryInfo.insert(std::make_pair(categories[i], icons[i])).second && ret;

		mDefaultCategory = defCategory;
		ret = mCategoryInfo.insert(std::make_pair(defCategory, defIcon)).second && ret;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TabedReBar::addToolBox(const TStringParam& categories, IUIToolBox* toolbox, bool allowMove/* = true*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		CToolBar* tb = static_cast<UIToolBox*>(toolbox)->getToolbar();
		ASSERT_KINDOF(CUIToolBar, tb);
		CUIToolBar* toolbar = STATIC_DOWNCAST(CUIToolBar, tb);
		TBINFO& info = mToolbarInfo[toolbar];
		info.allowMove = allowMove;
		info.categories = categories;

		ASSERT_KINDOF(CFrameWnd, ::AfxGetMainWnd());
		CFrameWnd* mainWnd = static_cast<CFrameWnd*>(::AfxGetMainWnd());
		mainWnd->ReDockControlBar(tb, this);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TabedReBar::switchCategory(const TString& category)
	{
		ToolbarCategory::const_iterator i = std::find(mAllCategories.begin(), mAllCategories.end(), Category(category));
		if (i != mAllCategories.end())
		{
			//note: m_tabctrl will switch to m_nActiveTab at OnUpdateCmdUI
			if (m_nActiveTab != (int)(i - mAllCategories.begin()))
				m_nActiveTab = (int)(i - mAllCategories.begin());

			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TabedReBar::SetWidth(int nHorzButtonCount)
	{
		if (nHorzButtonCount <= 0)
			return;
		m_nWidthButtons = nHorzButtonCount;
		m_nCurrentButtons = nHorzButtonCount;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TabedReBar::RecalcLayout()
	{
		//After repositioning the tab control we have to activate selected tab pane
		assert(m_nActiveTab < m_tabctrl.GetItemCount());
		if (m_nActiveTab >= 0)
		{
			//un dock all bars to re-arrange
			for (size_t i = 0; i < mAllCategories.size(); ++i)
			{
				for (size_t j = 0; j < mAllCategories[i].size(); ++j)
				{
					CControlBar* pBar = mAllCategories[i].get(j);
					//reset window size to force refresh NC area
					pBar->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE);
					pBar->ShowWindow(SW_HIDE);
				}
			}
			//this->GetParentFrame()->RecalcLayout();

			Category& category = mAllCategories[(size_t)m_nActiveTab];
			assert(category.size() != 0);

			//for tool bars with empty category
			//make them always visible
			int y = 0;
			for (ToolbarInfoMap::iterator i = mToolbarInfo.begin(); i != mToolbarInfo.end(); ++i)
			{
				if(!i->second.categories.empty())
					continue;

				CUIToolBar* pBar = i->first;
				pBar->EnableWindow(TRUE);
				pBar->ShowWindow(SW_SHOW);
				CSize size = pBar->CalcFixedLayout(FALSE, FALSE);
				CRect rectBar(0, y, size.cx, y + size.cy);
				this->ClientToScreen(&rectBar);

				if (m_nWidth < rectBar.Width())
					m_nWidth = rectBar.Width();

				this->GetParentFrame()->DockControlBar(pBar, this, &rectBar); //screen coordinates
				//this->GetParentFrame()->RecalcLayout();
				y += rectBar.Height();
			}

			for (size_t i = 0; i < category.size(); ++i)
			{
				CUIToolBar* pBar = category.get(i);
				//skip the already shown
				TBINFO& info = mToolbarInfo[pBar];
				if(info.categories.empty())
					continue;

				if (y >= m_nHeight - m_tabctrl.GetMaxTabHeight())
				{
					//increase width and try again
					this->RemoveAll();
					++m_nCurrentButtons;
					return;
				}

				pBar->EnableWindow(TRUE);
				pBar->ShowWindow(SW_SHOW);
				CSize size = pBar->CalcFixedLayout(FALSE, FALSE);
				CRect rectBar(0, y, size.cx, y + size.cy);
				this->ClientToScreen(&rectBar);

				if (m_nWidth < rectBar.Width())
					m_nWidth = rectBar.Width();

				this->GetParentFrame()->DockControlBar(pBar, this, &rectBar);
				//this->GetParentFrame()->RecalcLayout();
				y += rectBar.Height();
			}
		}
		this->GetParentFrame()->RecalcLayout();
		m_bRecalcLayout = FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TabedReBar::UpdateTabData(CControlBar* pBar)
	{
		ASSERT_KINDOF(CUIToolBar, pBar);
		CUIToolBar* pToolbar = STATIC_DOWNCAST(CUIToolBar, pBar);

		BOOL bBarExist = this->FindBar(pBar) != -1;
		if (bBarExist)
			return;

		for (size_t i = 0; i < mAllCategories.size(); ++i)
		{
			Category& TabedBars = mAllCategories[i];
			index_t index = TabedBars.find(pToolbar);
			if (index != INVALID_INDEX)
			{
				TabedBars.erase(index);
				if (TabedBars.size() == 0)
				{
					//delete category and tab item
					mAllCategories.erase(mAllCategories.begin() + (indexdiff_t)i);
					m_tabctrl.DeleteItem((int)i);

					if (mAllCategories.size() == 0)
						m_nActiveTab = -1;
					else
					{
						//last tab removed?
						if (m_nActiveTab == (int)mAllCategories.size())
							--m_nActiveTab;

						m_tabctrl.SetCurSel(m_nActiveTab);
						this->RecalcLayout();
					}
				}

				//re-check left category
				//-1 is OK, because ++i, 
				//although unsigned, 0xFFFFFFFF(or 0xFFFFFFFFFFFFFFFF) has overflow to 0
				i = INVALID_INDEX;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL TabedReBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
	{
		dwStyle &= ~(CBRS_TOP | CBRS_BOTTOM);
		return __super::Create(pParentWnd, dwStyle | CBRS_ALIGN_LEFT, nID);
	}

	//////////////////////////////////////////////////////////////////////////
	void TabedReBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
	{
		__super::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);

		int nCount = (int)this->m_arrBars.GetSize();
		size_t  nValidCount = 0;

		for (int i = 0; i < nCount; ++i)
		{
			CControlBar* pbar = GetDockedControlBar(i);
			if (pbar == NULL)
				continue;

			if (!pbar->IsKindOf(RUNTIME_CLASS(CUIToolBar)))
				pbar->m_pDockContext->ToggleDocking();
			else
				++nValidCount;
		}

		if (nValidCount != mTotalBarCount || m_nActiveTab != m_tabctrl.GetCurSel() || m_bSizeChanged)
		{
			assert(m_tabctrl.GetItemCount() == (int)mAllCategories.size());

			//note: re-add all tabs will cause m_nActiveTab mismatch the new index, use name instead
			TString activeTabLabel;
			if (m_nActiveTab > 0 && m_nActiveTab < (int)mAllCategories.size())
				activeTabLabel = mAllCategories[(size_t)m_nActiveTab].mName;

			this->RemoveAll();

			nCount = (int)this->m_arrBars.GetSize();
			for (int i = 0; i < nCount; ++i)
			{
				if (i >= this->m_arrBars.GetSize())
					break;

				CControlBar* pbar = GetDockedControlBar(i);
				if (pbar != NULL && pbar->IsKindOf(RUNTIME_CLASS(CUIToolBar)))
				{
					CUIToolBar* pToolBar = STATIC_DOWNCAST(CUIToolBar, pbar);
					this->AddBar(pToolBar);
					const TBINFO& info = mToolbarInfo[pToolBar];
					
					if (m_nActiveTab < 0 && info.categories.empty() )
						activeTabLabel = mDefaultCategory;
				}
			}

			if (m_nActiveTab >= (int)nValidCount)
				m_nActiveTab = (int)nValidCount - 1;

			ToolbarCategory::const_iterator iter = std::find(mAllCategories.begin(), mAllCategories.end(), Category(activeTabLabel));
			if (iter != mAllCategories.end())
				m_nActiveTab = (int)(iter - mAllCategories.begin());

			if (m_nActiveTab != m_tabctrl.GetCurSel())
				m_tabctrl.SetCurSel(m_nActiveTab);

			m_bRecalcLayout = TRUE;
			m_bSizeChanged = FALSE;
		}

		if (m_bRecalcLayout)
			this->RecalcLayout();

	}

	//////////////////////////////////////////////////////////////////////////
	int TabedReBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (__super::OnCreate(lpCreateStruct) == -1)
			return -1;

		//modify CSizingControlBar style
		//m_dwStyle &= ~SCBS_SHOWEDGES;

		//Create the Tab Control
		CRect rect;
		m_tabctrl.Create(WS_CHILD | WS_VISIBLE | TCS_BOTTOM | TCS_FOCUSONBUTTONDOWN | TCS_MULTILINE | TCS_FIXEDWIDTH,
			rect, this, IDC_TABED_REBAR);

		m_tabctrl.SetMinTabWidth(16);
		m_tabctrl.EnableCustomLook(TRUE, ETC_FLAT/*|ETC_SELECTION*/| ETC_COLOR);
		m_tabctrl.EnableDraw();
		m_tabctrl.SetImageList(IconManager::getSingleton().getIconList(IS_16));

		if(m_hReBarTheme != NULL)
		{
			::CloseThemeData(m_hReBarTheme);
			m_hReBarTheme = NULL;
		}
		return 0;
	}

	//Notification message of tab selection
	//////////////////////////////////////////////////////////////////////////
	void TabedReBar::OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult)
	{
		BLADE_UNREFERENCED(pNMHDR);
		*pResult = NULL;

		if (m_tabctrl.GetCurSel() != -1 && m_tabctrl.GetCurSel() != m_nActiveTab)
		{
			if (m_nActiveTab > 0)
			{
				Category& oldBars = mAllCategories[(size_t)m_nActiveTab];
				for (size_t i = 0; i < oldBars.size(); ++i)
				{
					CControlBar* pBar = oldBars.get(i);
					pBar->EnableWindow(FALSE);
					pBar->ShowWindow(SW_HIDE);
				}

			}
			m_nActiveTab = m_tabctrl.GetCurSel();
			this->RecalcLayout();
		}

	}

	//After the control changed size, position, or Z-order, it is time to 
	//reposition the tab control
	//////////////////////////////////////////////////////////////////////////
	void TabedReBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
	{
		__super::OnWindowPosChanged(lpwndpos);

		if (!(lpwndpos->flags&SWP_NOSIZE))
		{
			mData.mSize.x = lpwndpos->cx;
			mData.mSize.y = lpwndpos->cy;
		}

		if (!(lpwndpos->flags&SWP_NOMOVE))
		{
			mData.mPos.x = lpwndpos->x;
			mData.mPos.y = lpwndpos->y;
		}

		if (!(lpwndpos->flags&SWP_NOSIZE))
		{
			m_nHeight = lpwndpos->cy;

			CRect rect;
			int nWidth;
			int nHeight;
			nWidth = lpwndpos->cx;
			nHeight = lpwndpos->cy;

			rect.SetRect(0, 0, nWidth, nHeight);

			// calculating the size and position of the tab control
			if (m_dwStyle & CBRS_FLOATING)   // If the ControlBar is Floating
			{
				m_tabctrl.MoveWindow(rect);
				rect.left += LEFT_EDGE;
				rect.right -= RIGHT_EDGE;
				rect.bottom -= m_tabctrl.GetMaxTabHeight() + BOTTOM_EDGE;
				m_rectClient = rect;
			}
			else if ((m_dwStyle & (CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT)))// If the ControlBar is Left aligned
			{
				if (m_tabctrl.GetStyle()& TCS_BOTTOM)
				{
					m_rectClient.left = m_rectClient.top = 0;
					m_rectClient.right = nWidth - RIGHT_EDGE;
					m_rectClient.left += LEFT_EDGE;
					m_rectClient.bottom = nHeight - m_tabctrl.GetMaxTabHeight() - BOTTOM_EDGE;
					m_tabctrl.MoveWindow(&m_rectClient);
				}
				else
				{
					m_rectClient.left = LEFT_EDGE;
					m_rectClient.top = 0;
					m_rectClient.right = nWidth - RIGHT_EDGE;
					m_rectClient.bottom = nHeight;
					m_tabctrl.MoveWindow(&m_rectClient);
					m_rectClient.top += m_tabctrl.GetMaxTabHeight();
				}
			}

			::SetWindowPos(m_tabctrl.m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			if (m_tabctrl.GetItemCount() <= 1)
				m_tabctrl.SetMinTabWidth(nWidth - 4);
			else if (m_tabctrl.GetItemCount() > 1)
				m_tabctrl.SetMinTabWidth(nWidth / m_tabctrl.GetItemCount());

			if (mToolbarInfo.size() > 0)
			{
				CUIToolBar* tb = mToolbarInfo.begin()->first;
				DWORD buttonSize = tb->GetToolBarCtrl().GetButtonSize();
				WORD buttonWidth = LOWORD(buttonSize);
				TBMETRICS metric;
				::ZeroMemory(&metric, sizeof(metric));
				metric.cbSize = sizeof(metric);
				metric.dwMask = TBMF_BUTTONSPACING;
				tb->GetToolBarCtrl().GetMetrics(&metric);
				int buttonCount = nWidth / (buttonWidth + metric.cxButtonSpacing);
				if (buttonCount != m_nWidthButtons)
				{
					m_nWidthButtons = buttonCount;
					m_bSizeChanged = TRUE;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL	TabedReBar::OnEraseBkgnd(CDC* pDC)
	{
#if ENABLE_THEME
		CRect rect;
		this->GetClientRect(&rect);
		pDC->FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
		return TRUE;
#else
		return __super::OnEraseBkgnd(pDC);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	HBRUSH	TabedReBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
	{
#if ENABLE_THEME
		BLADE_UNREFERENCED(pWnd);
		BLADE_UNREFERENCED(nCtlColor);
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(THEMECOLOR(TC_TEXT));
		return THEMEBRUSH(TC_WINBK);
#else
		return __super::OnCtlColor(pDC, pWnd, nCtlColor);
#endif
	}

	// This function Adds every bar 
	//////////////////////////////////////////////////////////////////////////
	BOOL TabedReBar::AddBar(CUIToolBar* pToolBar)
	{
		const TBINFO& info = mToolbarInfo[pToolBar];
		TStringList categories = info.categories;

		if (categories.empty())
			categories.push_back(mDefaultCategory);

		for (size_t i = 0; i < categories.size(); ++i)
		{
			const TString& category = categories[i];
			CategoryInfo::const_iterator catIter = mCategoryInfo.find(category);
			IconIndex icon = catIter != mCategoryInfo.end() ? catIter->second : INVALID_ICON_INDEX;

			ToolbarCategory::iterator iter = std::find(mAllCategories.begin(), mAllCategories.end(), Category(category));

			//new category
			if (iter == mAllCategories.end())
			{
				int nTabs = (int)mAllCategories.size();

				TC_ITEM tci;
				tci.mask = TCIF_TEXT | TCIF_IMAGE;

				const TString& title = BTString2Lang(category);
				tci.pszText = (LPTSTR)(LPCTSTR)title.c_str();
				tci.iImage = icon;

				m_tabctrl.InsertItem(nTabs, &tci);

				mAllCategories.push_back(Category());
				Category& newCategory = mAllCategories[mAllCategories.size() - 1];
				newCategory.mName = category;

				newCategory.push_back(pToolBar);
			}
			else
				//existing category
				iter->push_back(pToolBar);
		}

		pToolBar->SetColumnCount((uint8)m_nCurrentButtons);
		pToolBar->EnableWindow(FALSE);
		pToolBar->ShowWindow(SW_HIDE);

		++mTotalBarCount;
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TabedReBar::RemoveAll()
	{
		mAllCategories.clear();
		mTotalBarCount = 0;
		m_nCurrentButtons = m_nWidthButtons;
		m_nWidth = 0;

		if (m_tabctrl.GetSafeHwnd() != NULL)
			m_tabctrl.DeleteAllItems();
	}
	
}//namespace Blade