/********************************************************************
	created:	2011/05/09
	filename: 	UIBaseTabCtrl.h
	author:		.dan.g.@codeproject.com
	purpose:	original code from : http://www.codeproject.com/KB/tabs/customtab.aspx?msg=889315
*********************************************************************/
#ifndef __Blade_UIBaseTabCtrl_h__
#define __Blade_UIBaseTabCtrl_h__
enum
{
	BTC_NONE = 0,
	BTC_TABS = 1,
	BTC_ALL = 2,
};

class CUIBaseTabCtrl : public CTabCtrl
{
// Construction
public:
	CUIBaseTabCtrl(int nType = BTC_NONE);
	BOOL EnableDraw(int nType = BTC_ALL);
	void SetBkgndColor(COLORREF color);

	/** @brief  */
	int GetMaxTabHeight() const;
	/** @brief  */
	int	GetMaxTabWidth() const;

protected:
	COLORREF m_crBack;
	int m_nDrawType;
	mutable int		mTabHeight;
	mutable	int		mTabWidth;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIBaseTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUIBaseTabCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUIBaseTabCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual void DrawMainBorder(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DrawItemBorder(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual COLORREF GetTabColor(BOOL bSelected = FALSE);
	virtual COLORREF GetTabTextColor(BOOL bSelected = FALSE);
	virtual void PreSubclassWindow();
};

/////////////////////////////////////////////////////////////////////////////

#endif // __Blade_UIBaseTabCtrl_h__
