/********************************************************************
	created:	2011/05/09
	filename: 	UIDialog.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UIDialog_h__
#define __Blade_UIDialog_h__

class CUIDialog : public CDialog
{
public:
	CUIDialog(UINT idd,CWnd* pParent = NULL);	// standard constructor

protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	/** @brief  */
	afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};


#endif // __Blade_UIDialog_h__