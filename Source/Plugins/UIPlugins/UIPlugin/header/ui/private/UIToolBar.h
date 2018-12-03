/********************************************************************
	created:	2011/05/11
	filename: 	UIToolBar.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_UIToolBar_h__
#define __Blade_UIToolBar_h__
#include <interface/public/ui/IUICommand.h>

class ConfigControl;
namespace Blade
{
	class Menu;
}//namespace Blade

class CUIToolBar : public CToolBar
{
public:
	static const int DEF_BUTTON_XEDGE = 8;
	static const int DEF_BUTTON_YEDGE = 6;
public:
	struct Button : public Blade::ICommandUI, public Blade::NonAssignable
	{
		const Blade::TString	mName;
		const bool				mCheckable;
		BYTE					mState;
		Button(Blade::TString name, bool checkable) :mName(name), mCheckable(checkable) { mState = 0; }
		virtual ~Button() {}
		inline bool operator<(const Button& rhs) const
		{
			return Blade::FnTStringFastLess::compare(mName, rhs.mName);
		}
		inline void setState(BYTE state)
		{
			mState = state;
			mState &= !mCheckable ? (~TBSTATE_CHECKED) : BYTE(-1);
		}

		/** @brief  */
		inline virtual const Blade::TString&	getName() const { return mName; }
		/** @brief  */
		inline virtual void setEnabled(bool enable)
		{
			mState |= enable ? TBSTATE_ENABLED : 0;
			mState &= !enable ? (~TBSTATE_ENABLED) : BYTE(-1);
		}
		/** @brief  */
		virtual void setChecked(bool checked)
		{
			mState |= (mCheckable && checked) ? TBSTATE_CHECKED : 0;
			mState &= !checked ? (~TBSTATE_CHECKED) : BYTE(-1);
		}
	};

public:
	DECLARE_DYNAMIC(CUIToolBar);
public:
	CUIToolBar(const Blade::TString& name);
	virtual ~CUIToolBar();

	/************************************************************************/
	/* common method                                                                     */
	/************************************************************************/
	/** @brief  */
	void		SetColumnCount(Blade::uint8 nColumn);

	/** @brief  */
	int			initDropDownMenu(UINT buttonID, Blade::Menu* menu, bool text);

	/** @brief return: whether a valid drop down. if not, normal update is needed */
	bool		updateDropDownMenu(int buttonIndex, Blade::index_t instance);

	/** @brief  */
	bool		updateCmdUI(Blade::IUICommand* cmd, Blade::index_t index, UINT_PTR id, CCmdUI* cmdui);

	/** @brief  */
	bool		handleToolCommand(UINT_PTR id);

	/** @brief  */
	bool		addConfigTool(Blade::IConfig* config, size_t width);

	/** @brief  */
	void		updateConfigTools();

	/** @brief  */
	const Blade::TString&	getName() const		{return mName;}

	/** @brief  */
	int			GetColumnCount() const			{return mCurrentColumn;}

	/** @brief  */
	void		setToolTipLink(CToolTipCtrl* ToolTip)	{mTooTipLink = ToolTip;}

	/** @brief  */
	CToolTipCtrl*	getToolTipLink() const				{return mTooTipLink;}

	/** @brief  */
	inline Button* getButton(UINT_PTR id)
	{
		TBBUTTON tbb;
		for (int i = 0; i < this->GetToolBarCtrl().GetButtonCount(); ++i)
		{
			this->GetToolBarCtrl().GetButton(i, &tbb);
			if (tbb.idCommand == (int)id && tbb.dwData != 0)
			{
				Button& button = *(Button*)tbb.dwData;
				button.setState(tbb.fsState);
				return &button;
			}
		}
		return NULL;
	}

protected:

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	inline BOOL IsToolBar() const
	{
		return CToolBar::GetToolBarCtrl().GetButtonCount() > 0;
	}

	inline BOOL IsHorzDocked()
	{
		return BOOL(this->GetBarStyle()&(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM));
	}

	inline BOOL IsVertDocked()
	{
		return BOOL(this->GetBarStyle()&(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT));
	}

	inline BOOL	IsFloating()
	{
		return BOOL(this->GetBarStyle()&CBRS_FLOATING);
	}

	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam);
	afx_msg void	OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void	OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void	OnWindowPosChanging(WINDOWPOS* lpwndpos);

	afx_msg void	OnNcPaint();
	afx_msg void	OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg int		OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void	OnDestroy();
	afx_msg HBRUSH	OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);//control color for options tools (dropdown, etc)
	afx_msg void	OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

public:
	/** @brief  */
	virtual CSize	CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual void	OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	/** @brief  */
	void NcPaintGripper(CDC* pDC, CRect rcClient);
	typedef struct SDropDownInfo
	{
		Blade::Menu*menu;
		int		index;
		int		indexBase;
	}DROP_DOWN_INFO;

	typedef Blade::Map<Blade::index_t, DROP_DOWN_INFO> ButtonTextMap;	//button ID => active text index map
	typedef Blade::Vector<ConfigControl*>	ConfigToolList;

	Blade::TString	mName;
	Blade::TString	mTitle;
	ConfigToolList	mConfigTools;
	ButtonTextMap	mDropDownInfo;
	CToolTipCtrl*	mTooTipLink;
	Blade::int16	mMaxStringWidth;
	Blade::uint8	mColumn;
	Blade::uint8	mCurrentColumn;
	UINT			mDockBarID;
	int				mStringIndexBase;
};//class UIToolBar


#endif // __Blade_UIToolBar_h__