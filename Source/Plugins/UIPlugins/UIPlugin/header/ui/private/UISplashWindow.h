/********************************************************************
	created:	2011/09/11
	filename: 	UISplashWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UISplashWindow_h__
#define __Blade_UISplashWindow_h__
#include <resource.h>
#include "afxwin.h"

class CUISplashWindow : public CDialog , public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;

public:
	CUISplashWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUISplashWindow();

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	/** @brief  */
	void	setInitDetails(const Blade::TString& text);

	/** @brief  */
	void	setStartupInformation(const Blade::TString& text);

	/** @brief  */
	void	setSplashImage(const Blade::TString& image);

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/

	// Dialog Data
	enum { IDD = IDD_SPLASH };

protected:
	/** @brief  */
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	/** @brief  */
	virtual BOOL OnInitDialog();

	/** @brief  */
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

protected:
	DECLARE_MESSAGE_MAP()

	CStatic mSplashImage;
	CStatic mInitDetail;
	CStatic mStartupInfo;
};


#endif // __Blade_UISplashWindow_h__



