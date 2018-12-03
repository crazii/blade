#pragma once
#include <afxcmn.h>
#include <ui/private/UIDialog.h>
#include <ui/private/UITabCtrl.h>
#include "resource.h"
#include "OptionList.h"

const Blade::TString ConifgDialogSection = BTString("ConfigDialog");

class CConfigDialog : public CUIDialog, public Blade::IConfigDialog ,public Blade::Allocatable
{
	DECLARE_DYNAMIC(CConfigDialog)
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	CConfigDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigDialog();


	/************************************************************************/
	/* blade IConfigDialog interface                                                                      */
	/************************************************************************/
	/*
	@describe 
	@param 
	@return 
	*/
	virtual bool	show(const INFO& info, const Blade::HCONFIG* configs, size_t count,
		const Blade::TString& caption = BTString("Blade Config") );

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	/*
	@describe 
	@param [in] showAutoCheck: show the check box in the bottom for dialog auto-startup showing
	@param [in] banner: show top banner pic
	@return 
	*/
	bool			show(const INFO& info, bool hideAutoCheck, bool hideBanner = false, CWnd* parent = NULL);

	/** @brief  */
	bool			addConfig(Blade::IConfig* configItem);

	/** @brief  */
	void			setCaption(const Blade::TString& caption);


// Dialog Data
	enum { IDD = IDD_CONFIG_DIALOG };

protected:
	/** @brief  */
	void			ResetList(int tabIndex);
	/** @brief  */
	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	/** @brief  */
	virtual void	EndDialog( int nResult );				//endup finish

private:
	typedef Blade::List<Blade::IConfig*>	ConfigList;

	ConfigList	mConfigList;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTcnSelchangeOptionSeletionTab(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
protected:
	int				mTabSelection;
	COptionList		mOptionList;
	CUITabCtrl		mOptionTab;

	INFO			mInfo;
	Blade::TString	mCaption;

	int				mNCHeight;
	BOOL			mShowOnStartUp;
	bool			mHideStartUpCheck;
	bool			mHideBanner;

protected:
	CWnd*			mParent;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCheckShowOnStartup();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
};
