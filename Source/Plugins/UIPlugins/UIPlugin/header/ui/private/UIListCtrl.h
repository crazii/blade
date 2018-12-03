/********************************************************************
	created:	2011/05/25
	filename: 	UIListCtrl.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CUIListCtrl_h__
#define __Blade_CUIListCtrl_h__
class CUIListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CUIListCtrl);
public:
	CUIListCtrl();
	~CUIListCtrl();

protected:

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	/** @brief  */
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	/** @brief  */
	virtual void PreSubclassWindow();
	/** @brief  */
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	/** @brief  */
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	/** @brief  */
	afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	/** @brief  */
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	/** @brief  */
	afx_msg	void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);

	/** @brief  */
	afx_msg void OnNcPaint();

	DECLARE_MESSAGE_MAP();

	/** @brief  */
	inline void EnableOwnerDraw(BOOL enable)
	{
		this->ModifyStyle(enable ? 0u : LVS_OWNERDRAWFIXED, enable ? LVS_OWNERDRAWFIXED : 0u);
	}
};



#endif // __Blade_CUIListCtrl_h__