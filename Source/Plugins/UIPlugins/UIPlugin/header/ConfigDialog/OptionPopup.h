/********************************************************************
	created:	2013/11/14
	filename: 	OptionPopup.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_OptionPopup_h__
#define __Blade_OptionPopup_h__
#include <ConfigDialog/OptionList.h>

class COptionPopup : public CWnd, public Blade::Allocatable
{
public:
	/** @brief show popup */
	static bool	showOptionPopup(int x, int y, CWnd* parent, Blade::IConfig* config);

protected:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	COptionPopup(CWnd* parent, Blade::IConfig* config);
	~COptionPopup();

	/** @brief x,y in screen coordinates */
	void	popup(int x, int y);

	DECLARE_MESSAGE_MAP()

	/** @brief  */
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	/** @brief  */
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnPaint();

protected:
	CWnd*		mParent;
	COptionList	mList;
	BOOL		mParentEnable;
};


#endif //  __Blade_OptionPopup_h__