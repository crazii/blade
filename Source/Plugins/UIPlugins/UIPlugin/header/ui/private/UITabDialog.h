/********************************************************************
	created:	2013/04/28
	filename: 	UITabDialog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UITabDialog_h__
#define __Blade_UITabDialog_h__
#include "afxcmn.h"
#include "resource.h"
#include <ui/private/UIDialog.h>
#include <ui/private/UITabCtrl.h>

class ITabPage
{
public:
	virtual ~ITabPage()	{}

	/** @brief  */
	virtual CWnd*	createWindow(CWnd* parent) = 0;
	/** @brief  */
	virtual CWnd*	getWindow() = 0;
	/** @brief  */
	virtual bool	onOK() = 0;
	/** @brief  */
	virtual bool	onCancel() = 0;
	/** @brief  */
	virtual bool	onApply() = 0;
	/** @brief  */
	virtual void	onWindowResize() {}
};

class CUITabDialog : public CUIDialog
{
	DECLARE_DYNAMIC(CUITabDialog)
public:
	CUITabDialog(CWnd* pParent = NULL); 
	virtual ~CUITabDialog();

	enum { IDD = IDD_TAB_DIALOG };

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	/** @brief  */
	Blade::index_t	addTab(ITabPage* page);
	/** @brief  */
	ITabPage*		getTab(Blade::index_t index);
	/** @brief  */
	void			setCaption(const Blade::TString& caption);

protected:
	/** @brief  */
	void			addTabItem(ITabPage* page);

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/

protected:
	/** @brief  */
	virtual void DoDataExchange(CDataExchange* pDX);
	/** @brief  */
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	/** @brief  */
	afx_msg void OnTcnSelchangingUITabDialogTabCtrl(NMHDR *pNMHDR, LRESULT *pResult);


	DECLARE_MESSAGE_MAP()

	typedef Blade::Vector<ITabPage*>	TabPageList;

	TabPageList		mTabs;
	CUITabCtrl		mTabCtrl;
	Blade::TString	mCaption;
public:
	afx_msg void OnBnClickedApply();
};


#endif//__Blade_UITabDialog_h__
