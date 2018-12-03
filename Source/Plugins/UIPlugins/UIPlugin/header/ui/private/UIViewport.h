/********************************************************************
	created:	2013/04/23
	filename: 	CUIViewport.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_CUIViewport_h__
#define __Blade_CUIViewport_h__
#include <interface/public/ui/IViewport.h>
#include <ui/private/UIFrameWnd.h>

class ViewportContent;

class CUIViewport : public CUIFrameWnd, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	CUIViewport();
	~CUIViewport();
	
	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	/** @brief  */
	virtual BOOL Create(CWnd* parent);
	/** @brief  */
	virtual BOOL OnCmdMsg(UINT nID,int nCode,void* pExtra,AFX_CMDHANDLERINFO* pHandlerInfo);
	/** @brief  */
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	/** @brief  */
	afx_msg int	 OnCreate(LPCREATESTRUCT lpCreateStruct);
	/** @brief  */
	afx_msg void OnNcPaint();
	/** @brief  */
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	/** @brief  */
	afx_msg	void OnShowWindow(BOOL bShow, UINT nStatus);


	/************************************************************************/
	/* EditorFrameWnd                                                                     */
	/************************************************************************/
	/** @brief  */
	virtual Blade::index_t getIndex() const { return mViewport != NULL ? mViewport->getIndex() : 0; }

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	/** @brief  */
	void	setViewport(Blade::IViewport* viewport);
	/** @brief  */
	void	updateLayout();
	/** @brief  */
	void	activate();
	/** @brief  */
	void	draw();
	/** @brief  */
	HWND	getContentHWND() const;
	/** @brief  */
	bool	setMaximize(bool max);
	/** @brief  */
	bool	isMaximized() const	{return mViewport != NULL ? mViewport->isMaximized() : false;}
	/** @brief  */
	Blade::IViewport* getViewport() {return mViewport;}

protected:
	/** @brief  */
	void	calculcateRect(CRect& rect);

	/** @brief  */
	virtual void RecalcLayout(BOOL bNotify = TRUE);

	DECLARE_MESSAGE_MAP()

	Blade::IViewport*	mViewport;
	ViewportContent*	mContent;
};//class CUIViewport
	

#endif//__Blade_CUIViewport_h__