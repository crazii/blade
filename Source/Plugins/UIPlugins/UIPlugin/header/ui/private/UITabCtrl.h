//_ **********************************************************
//_ 
//_ Name: EnTabCtrl.h 
//_ Purpose: 
//_ Created: 15 September 1998 
//_ Author: D.R.Godson
//_ Modified By: 
//_ 
//_ Copyright (c) 1998 Brilliant Digital Entertainment Inc. 
//_ 
//_ **********************************************************

/********************************************************************
	added:		2011/05/09
	filename: 	UITabCtrl.h (renamed)
	modifier:	Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UITabCtrl_h__
#define __Blade_UITabCtrl_h__
#include "UIBaseTabCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CUITabCtrl window

// custom look
enum
{
	ETC_FLAT = 1, 
	ETC_COLOR = 2, // draw tabs with color
	ETC_SELECTION = 4, // highlight the selected tab
	ETC_GRADIENT = 8, // draw colors with a gradient
	ETC_BACKTABS = 16,
};

class CUITabCtrl : public CUIBaseTabCtrl
{
// Construction
public:
	CUITabCtrl();

	void EnableCustomLook(BOOL bEnable = TRUE, DWORD dwStyle = ETC_FLAT | ETC_COLOR);

// Attributes
public:

protected:
	DWORD m_dwCustomLook;

// Operations
public:
	virtual BOOL Create(DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUITabCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CUITabCtrl();

	// Generated message map functions
protected:
	/** @brief  */
	void	DrawItemRect(CDC* pDC,const CRect& rect,BOOL bUseColor,BOOL bSelected);

	virtual void DrawMainBorder(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void DrawItemBorder(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual COLORREF GetTabColor(BOOL bSelected = FALSE);
	virtual COLORREF GetTabTextColor(BOOL bSelected = FALSE);

};

/////////////////////////////////////////////////////////////////////////////


#endif // __Blade_UITabCtrl_h__