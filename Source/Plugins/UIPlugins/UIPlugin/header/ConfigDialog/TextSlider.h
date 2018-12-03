#pragma once
#include "afxcmn.h"
#include <resource.h>
#include "OptionControls.h"
#include "ConfigControlUtil.h"


class CTextSlider : public CCompositedControl, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	CTextSlider();   // standard constructor
	virtual ~CTextSlider();

	virtual BOOL	Create(DWORD style,const RECT& prect,CWnd* parent,UINT nID,Blade::IConfig* config);

	/*
	@describe
	@param
	@return
	*/
	void			InitValues(const OptionValueList& strings,UINT curvalIndex = 0);

// Dialog Data
	enum { IDD = IDD_SLIDER_SHOW };

protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);

	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void	OnSliderDataChange(NMHDR *pNMHDR, LRESULT *pResult);

	CSliderCtrl		mSliderCtrl;
	CString			mInfoText;
	UINT			mCurIndex;
	typedef Blade::Vector<Blade::TString> TextList;
	TextList		mTStringValues;

	virtual void	OnOK();
	virtual void	OnCancel();

	afx_msg void	OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void	OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void	OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT	OnSetFont(WPARAM wParam, LPARAM lParam);
};
