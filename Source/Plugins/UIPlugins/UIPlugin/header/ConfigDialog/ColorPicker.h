#pragma once

#include "resource.h"
#include "OptionControls.h"
#include <utility/BladeContainer.h>

enum EColorPackFormat
{
	CPF_UCHAR,
	CPF_FLOAT,
};

// CColorPicker dialog
class CColorPicker : public CCompositedControl, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];


public:
	CColorPicker(CWnd* pParent = NULL);   // standard constructor
	virtual ~CColorPicker();

	/** @brief  */
	virtual BOOL Create(const RECT& rect,CWnd* pParent,UINT nID,Blade::IConfig* config);

	/** @brief  set format default floats */
	void		SetOutputFormat(EColorPackFormat format);

// Dialog Data
	enum { IDD = IDD_COLOR_PICKER };

protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);

	/** @brief  */
	bool			ValidateColorString();

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual LRESULT	DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void	OnOK();
	virtual void	OnCancel();

	DECLARE_MESSAGE_MAP()

	COLORREF			mColor;
	EColorPackFormat	mFormat;

	//////////////////////////////////////////////////////////////////////////
	afx_msg void	OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void	OnBnClickedColorButton();
	afx_msg void	OnPaint();
	afx_msg void	OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void	OnEnKillfocusColorPicker();
	afx_msg LRESULT	OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void	OnSetFocus(CWnd* pOldWnd);
};
