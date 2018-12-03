/********************************************************************
	Created:	5:6:2004   16:44
	Filename: 	BHTabedReBar
	File ext:	h
	Author:		Bahrudin Hrnjica
	e-mail:		bhrnjica@hotmail.com
	Web Page:	www.baha.dreamstation.com

	Disclaimer
	----------
	THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
	ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
	DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
	RISK OF USING THIS SOFTWARE.
*********************************************************************/

/********************************************************************
	created:	2011/05/10
	filename: 	TabedReBar.h
	author:		Bahrudin Hrnjica
	Licence:	CPOL
	url:		http://www.codeproject.com/KB/toolbars/TabedReBar.aspx
	purpose:
*********************************************************************/
#pragma once
#include <afxpriv.h>
#include <afxtempl.h>
#include <ui/private/UITabCtrl.h>
#include <ui/private/UIToolBar.h>
#include <ui/DockWindow.h>
#include <interface/public/ui/widgets/IUIToolPanel.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	typedef List<CUIToolBar*> ToolbarList;

	class Category
	{
	public:
		Category() {}
		Category(const TString name) :mName(name) {}

		/** @brief  */
		void	push_back(CUIToolBar* p) { mToolbars.push_back(p); }

		/** @brief  */
		size_t	size() const { return mToolbars.size(); }

		/** @brief  */
		bool	operator==(const Category& rhs) const { return mName == rhs.mName; }

		/** @brief  */
		CUIToolBar*	get(size_t index)
		{
			if (index > mToolbars.size())
				return NULL;
			ToolbarList::iterator i = mToolbars.begin();
			std::advance(i, index);
			return *i;
		}

		/** @brief  */
		void			erase(index_t index)
		{
			if (index > mToolbars.size())
				return;
			ToolbarList::iterator i = mToolbars.begin();
			std::advance(i, index);
			mToolbars.erase(i);
		}

		/** @brief  */
		index_t	find(CUIToolBar* p)
		{
			ToolbarList::iterator i = std::find(mToolbars.begin(), mToolbars.end(), p);
			if (i == mToolbars.end())
				return INVALID_INDEX;
			else
			{
				index_t ret = 0;
				ToolbarList::iterator beg = mToolbars.begin();
				while (beg++ != i)	++ret;
				return ret;
			}
		}

		TString			mName;
		ToolbarList		mToolbars;
	};

	// TabedReBar
	class TabedReBar : public CDockBar, public DockWindow<IUIToolPanel>, public Singleton<TabedReBar>
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		using Singleton<TabedReBar>::operator new;
		using Singleton<TabedReBar>::operator delete;
		using Singleton<TabedReBar>::operator new[];
		using Singleton<TabedReBar>::operator delete[];

		DECLARE_DYNAMIC(TabedReBar)
		DECLARE_MESSAGE_MAP()
	public:

		TabedReBar();
		virtual ~TabedReBar();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual CWnd*	getCWnd() const { return (CDockBar*)this; }
		/** @brief  */
		virtual bool	createWindow();

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual bool	setupCategory(const TString& defCategory, IconIndex defIcon, const TString* categories, IconIndex* icons, size_t count);

		/** @brief  */
		virtual bool	addToolBox(const TStringParam& categories, IUIToolBox* toolbox, bool allowMove = true);

		/** @brief  */
		virtual bool	switchCategory(const TString& category);

		/** @brief  */
		virtual const TString& getDefaultCategory() const { return mDefaultCategory; }

		/** @brief  */
		void	SetWidth(int nHorzButtonCount);

		/** @brief  */
		void	RecalcLayout();

		/** @brief remove un-docked control bar */
		void	UpdateTabData(CControlBar* pBar);

		// Overrides
	public:
		virtual BOOL	Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
		virtual void	OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

		// Generated message map functions
	protected:
		//{{AFX_MSG(TabedReBar)
		afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void	OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void	OnWindowPosChanged(WINDOWPOS* lpwndpos);
		afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
		afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		//}}AFX_MSG

		// Implementation
	public:
		/** @brief  */
		BOOL			AddBar(CUIToolBar* pBar);//Adds any bars derived from CWnd

												  //Data Members
	protected:
		/** @brief  */
		void			RemoveAll();

		CUITabCtrl		m_tabctrl;		   // Tab Control
		int				m_nActiveTab;	   // currently active tab
		CRect			m_rectClient;	   // client rect of the control
		BOOL			m_bRecalcLayout;
		BOOL			m_bSizeChanged;

		int				m_nWidthButtons;	//width in buttons
		int				m_nCurrentButtons;
		int				m_nWidth;
		int				m_nHeight;			//height in pixels

		typedef struct SToolbarInfo
		{
			TStringList categories;
			bool		allowMove;
		}TBINFO;
		typedef Map<CUIToolBar*, TBINFO>	ToolbarInfoMap;
		typedef TStringMap<IconIndex>		CategoryInfo;

		typedef Vector<Category> ToolbarCategory;

		TString			mDefaultCategory;
		CategoryInfo	mCategoryInfo;
		ToolbarInfoMap	mToolbarInfo;

		ToolbarCategory	mAllCategories;
		size_t			mTotalBarCount;
	};

}//namespace Blade

