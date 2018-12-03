#pragma once

#include "resource.h"
#include "afxcmn.h"
#include "OptionControls.h"
#include <ui/private/UISlider.h>
// CSliderInput dialog

class CSliderInput : public CCompositedControl, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	CSliderInput(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSliderInput();

	//create control
	//@param dwStyle :  default SIS_EDIT
	virtual BOOL	Create(const RECT& rect,CWnd* pParent,UINT nID,Blade::IConfig* config,bool edge = true);

// Dialog Data
	enum { IDD = IDD_SLIDER_INPUT };

protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual void	OnOK();
	virtual void	OnCancel();

	DECLARE_MESSAGE_MAP()

	CUISlider	mInputSlider;
	CString		mInputString;
	float		mRangeMin;
	float		mRangeMax;
	float		mRangeStep;
	int			mSliderPos;

	afx_msg void	OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void	OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void	OnEnKillfocusSliderEdit();
	afx_msg void	OnPaint();
	afx_msg void	OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT	OnSetFont(WPARAM wParam, LPARAM lParam);
};
