#pragma once
#include "ConfigControlUtil.h"
#include <ui/private/UIListCtrl.h>

// COptionList
class COptionList : public CUIListCtrl, public IControlNotify, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];

public:
	COptionList();

	virtual ~COptionList();

	/** @brief  */
	virtual BOOL	Create(CWnd* pParent,const RECT& rect,UINT nID,DWORD dwExWinStyle = 0,DWORD dwExLVStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	/*
	@describe not all controls in rows could be see by default,one one row that's selected
	@param
	@return
	*/
	void			setAllVisible(bool visible)				{mForceShowAll = visible;}

	/*
	@describe expand all trees.
	by default don't expand sub items on init, unless user click it.
	@param 
	@return 
	*/
	void			setDefaultExpand(bool expand)			{mExpandAll = expand;}

	/*
	@describe 
	@param 
	@return 
	*/
	size_t			getIdealHeight() const	{return mConfigList.size() * this->getItemHeight() + (size_t)::GetSystemMetrics(SM_CYBORDER)*2;}

	/*
	@describe 
	@param
	@return
	*/
	size_t			getIdealWidth();

	/*
	@describe refresh changed data only
	@param 
	@return 
	*/
	void			refresh();

	/*
	@describe 
	@param 
	@return 
	*/
	void			clear();

	/*
	@describe		add sub configs of root, ignoring root itself
	@param 
	@return 
	*/
	void			setSubConfigs(Blade::IConfig* root);

protected:
	/** @brief  */
	bool		addConfig(Blade::IConfig* config, Blade::CONFIG_ACCESS parentAccess, bool devMode);
	/** @brief  */
	Blade::uint	addListItem(Blade::IConfig* config);
	/** @brief bExpandSub indicates this item is a sub item expanded */
	Blade::uint	insertListItem(Blade::index_t row,Blade::IConfig* config, bool bExpandSub = false);
	/** @brief  */
	void		insertColumn(Blade::index_t nCol, LPCTSTR lpszColumnHeading, float width, int nSubItem= -1, int nFormat = LVCFMT_LEFT);
	/** @brief  */
	void		refreshControl();
	/** @brief  */
	/* @note: bCascade is internally used only (recursive method)  */
	void		expandMultiRows(Blade::index_t index,BOOL bExpand, BOOL bCascade = FALSE);
	/** @brief  */
	bool		isInCenterLine(const CPoint& pt);
	/** @brief  */
	size_t		getItemHeight() const;
	/** @brief  */
	Blade::index_t	buttonIDToIndex(Blade::index_t expandButtonID);
	/** @brief  */
	void		updateColumnWidth(int center = 0);


	/** @brief IControlNotify interface */
	virtual void onConfigChanging(Blade::index_t ctlID);
	virtual void onConfigChanged(Blade::index_t ctlID);

	struct SControlTypeInfo
	{
		Blade::uint16	depth;
		Blade::uint16	controlType;
		ListSubControlType	getType() const	{return ListSubControlType(controlType);}
		Blade::uint16		getDepth() const {return depth;}
	};

	typedef Blade::Vector<SControlTypeInfo>			TypeList;
	typedef Blade::Vector<ListSubControlAccess>		AccessList;
	typedef Blade::Vector<ConfigControl*>			ControlList;
	typedef Blade::Vector<CExpButton*>				ExpButtonList;
	typedef Blade::Vector<Blade::IConfig*>			ConfigList;

	TypeList		mListTypeInfo;
	AccessList		mListAccess;
	ControlList		mListControls;
	ExpButtonList	mListExpandButton;
	ConfigList		mConfigList;
	Blade::index_t	mRefreshExpandIndex;
	UINT_PTR		mTimerID;

	int				mPaintPhase;

	float			mColumnRate;	//first column width rate
	bool			mForceShowAll;
	bool			mExpandAll;
	bool			mDraging;

	Blade::HCONFIGUIDATA	mUIExpanded;
	Blade::HCONFIGUIDATA	mUINotExpanded;
protected:
	DECLARE_MESSAGE_MAP()

protected:
	//
	virtual LRESULT DefWindowProc(UINT message,WPARAM wParam,LPARAM lParam);
	virtual void	DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void	MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void	OnPaint();
	afx_msg void	OnNcDestroy();
	afx_msg void	OnTimer(UINT_PTR nIDEvent);
	afx_msg void	OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg	void	OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg BOOL	OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnCaptureChanged(CWnd* pWnd);
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void	OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


