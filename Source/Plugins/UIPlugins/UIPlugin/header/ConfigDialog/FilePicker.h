#pragma once
#include "afxwin.h"
#include "resource.h"
#include "OptionControls.h"

// CFilePicker dialog

class CFilePicker : public CCompositedControl, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];

public:
	CFilePicker(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFilePicker();

// Dialog Data
	enum { IDD = IDD_FILE_PICKER };

	/** @brief  */
	virtual BOOL	Create(const RECT& rect,CWnd* pParent,UINT nID,Blade::IConfig* config);

	/** @brief set file extension if type is a file,otherwise unused. */
	void			SetFileExtension(const Blade::TString& fileExt);

	/** @brief  */
	void			SetTextWidth(int cx);

protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);

	virtual BOOL	OnInitDialog();
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT	DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL	PreTranslateMessage(MSG* pMsg);
	virtual void	OnOK();
	virtual void	OnCancel();

	DECLARE_MESSAGE_MAP()
	CString			mPathString;
	Blade::TString	mFileExtension;

	//////////////////////////////////////////////////////////////////////////
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnBnClickedBrowseButton();
	afx_msg void OnEnKillfocusFilePath();
	afx_msg LRESULT	OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};
