#pragma once
#include "afxwin.h"
#include "resource.h"
#include "OptionControls.h"
#include <interface/IMediaLibrary.h>

// CFilePicker dialog

class CImagePicker : public CButton, public ConfigControl, public Blade::IMediaLibrary::ISelectionReceiver , public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];


public:
	CImagePicker(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImagePicker();

	/** @brief tooltip helper */
	virtual void	AddToolTip(CToolTipCtrl* tooltip, const Blade::TString& desc)
	{
		tooltip->AddTool(this, desc.c_str());
	}

	/************************************************************************/
	/* IMediaLibrary::ISelectionReceiver interface                                                                     */
	/************************************************************************/
	/** @brief  */
	virtual void	setSelectedTarget(Blade::index_t mediaIndex, const Blade::IMediaLibrary::MediaFile& media);

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	/** @brief  */
	virtual BOOL	Create(const RECT& rect,CWnd* pParent,UINT nID,Blade::IConfig* config);

protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);

	DECLARE_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	afx_msg void	OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void	OnBnClickedMediaImage();
	afx_msg void	OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL	OnClicked();

	virtual void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	bool			mIniting;
	CBitmap			mBitmap;
};
