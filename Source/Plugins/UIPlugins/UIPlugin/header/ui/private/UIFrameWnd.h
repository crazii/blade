/********************************************************************
	created:	2015/08/12
	filename: 	UIFrameWnd.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UIFrameWnd_h__
#define __Blade_UIFrameWnd_h__

class CUIFrameWnd : public MFCFrameWindow
{
	DECLARE_MESSAGE_MAP()
public:
	CUIFrameWnd()	{}
	~CUIFrameWnd()	{}

	/** @brief  */
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	/** @brief  */
	afx_msg LRESULT	OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	/** @brief  */
	afx_msg LRESULT	OnMenuCommand(WPARAM nPos, LPARAM hMenu);
	/** @brief  */
	afx_msg void	OnToolbarDropDown(NMHDR* pNMHDR, LRESULT* pResult);
#if ENABLE_THEME
	/** @brief  */
	afx_msg BOOL	OnEraseBkgnd(CDC* pDC);
	/** @brief  */
	afx_msg void	OnPaint();
	/** @brief  */
	afx_msg void	OnNcPaint();
#endif

	/** @brief  */
	virtual BOOL OnCmdMsg(UINT nID,int nCode,void* pExtra,AFX_CMDHANDLERINFO* pHandlerInfo);

	/** @brief  */
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	/** @brief  */
	virtual Blade::index_t getIndex() const { return 0; }
	
	/** @brief  */
	void EnableDocking(DWORD dwDockStyle);
};


#endif // __Blade_UIFrameWnd_h__