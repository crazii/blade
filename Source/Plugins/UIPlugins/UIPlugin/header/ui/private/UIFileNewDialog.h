/********************************************************************
	created:	2010/05/27
	filename: 	UIFileNewDialog.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UIFileNewDialog_h__
#define __Blade_UIFileNewDialog_h__
#include <ConfigTypes.h>
#include <interface/public/ui/IUIMainWindow.h>
#include <ConfigDialog/OptionList.h>
#include <ConfigDialog/FilePicker.h>
#include <ui/private/UIDialog.h>
#include <ui/private/UIListCtrl.h>
#include "resource.h"


class CUIFileNewDialog : public CUIDialog , public Blade::TempAllocatable
{
public:
	using Blade::TempAllocatable::operator new;
	using Blade::TempAllocatable::operator delete;
	using Blade::TempAllocatable::operator new[];
	using Blade::TempAllocatable::operator delete[];
public:
	CUIFileNewDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUIFileNewDialog();

	enum { IDD = IDD_FILE_NEW };

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	/** @brief  */
	void			AddFileTypes(const Blade::IUIMainWindow::NEW_FILE_DESC* desc);

	/** @brief  */
	const Blade::IUIMainWindow::NEW_FILE_DESC*	GetFileType() const;

	/** @brief  */
	Blade::TString	GetFileName() const;

	/** @brief  */
	const Blade::TString&	GetFileDirectory() const;

	/** @brief  */
	void			RefreshOption(Blade::index_t fileOptIndex);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();


protected:
	typedef Blade::TempVector<const Blade::IUIMainWindow::NEW_FILE_DESC*>	FileInfoList;

	COptionList		mFileOption;
	CUIListCtrl		mFileTypeList;
	FileInfoList	mFileInfo;
	Blade::index_t	mSelectFile;
	CString			mNewFileName;
	Blade::ConfigAtom	mFilePathConfig;
	CFilePicker		mDirectoryPicker;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMClickFileTypeList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
};


#endif //__Blade_UIFileNewDialog_h__