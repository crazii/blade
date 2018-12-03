#pragma once
#pragma warning(push)
#pragma warning(disable:4355) //'this' : used in base member initializer list
/************************************************************************/
/*                                                                      */
/************************************************************************/
#define EXP_BUTTON_SIZE		(11)

#include <ui/private/UIDialog.h>

class CExpButton : public CButton , public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];

public:
	inline CExpButton() :mbExpanded(FALSE)	{}
	inline ~CExpButton()					{}
	/** @brief  */
	virtual BOOL	Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	/** @brief  */
	inline void	SetExpanded(BOOL expanded) { mbExpanded = expanded; }
	/** @brief  */
	inline BOOL	GetExpanded() const { return mbExpanded; }

protected:
	DECLARE_MESSAGE_MAP()
	/** @brief  */
	virtual void	DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
	/** @brief  */
	afx_msg BOOL	OnClicked();
	BOOL		mbExpanded;
};

//notification interface
class IControlNotify
{
public:
	virtual void onConfigChanging(Blade::index_t  ctlID) = 0;
	virtual void onConfigChanged(Blade::index_t  ctlID) = 0;
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class ConfigControl
{
public:
	inline ConfigControl()
		:mConfig(NULL),mWnd(NULL),mNotify(NULL),mEventNotify(false)	{}
	inline virtual ~ConfigControl()									{}

	/** @brief  */
	inline void			Init(Blade::IConfig* config,CWnd* wnd)			{mConfig = config;mWnd = wnd;}
	/** @brief  */
	inline void			SetNotify(IControlNotify* notify)				{mNotify = notify;}

	/** @brief  */
	inline void			ApplyConfig(const Blade::TString& val)
	{
		if( mConfig != NULL )
		{
			if( mNotify != NULL )
				mNotify->onConfigChanging( (Blade::index_t)::GetDlgCtrlID(mWnd->GetSafeHwnd()) );
			mConfig->setValue(val);
			if( mNotify != NULL )
				mNotify->onConfigChanged( (Blade::index_t)::GetDlgCtrlID(mWnd->GetSafeHwnd()) );
		}
	}
	inline void			ApplyConfig(LPCTSTR val)
	{
		return this->ApplyConfig( Blade::TString(val) );
	}

	/** @brief  */
	inline Blade::IConfig* GetConfig() const	{return mConfig;}
	/** @brief  */
	inline CWnd*		GetWindow() const		{return mWnd;}

	/** @brief tooltip helper */
	virtual void		AddToolTip(CToolTipCtrl* tooltip, const Blade::TString& desc) = 0;

	/** @brief  */
	bool		updateData(bool force = false)
	{
		if( mConfig != NULL && mConfig->updateData(force) || force)
		{
			this->updateDataImpl( mConfig->getValue() );
			return true;
		}
		else
			return false;
	}

	/** @brief  */
	void		setEventNotify(bool notify)
	{
		mEventNotify = notify;
	}

protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val) = 0;

	Blade::IConfig* mConfig;
	CWnd*			mWnd;
	IControlNotify* mNotify;
	bool			mEventNotify;	//event type: only has effect on controls with edit
									//1.don't notify on lost focus, 2.always notify on key ENTER
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class CheckControl : public CButton, public ConfigControl, public Blade::Allocatable
{
	DECLARE_MESSAGE_MAP()
protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	using Blade::Allocatable::operator new[];
	using Blade::Allocatable::operator delete[];
public:
	inline CheckControl()						{}
	inline ~CheckControl()						{}
	/** @brief tooltip helper */
	virtual void	AddToolTip(CToolTipCtrl* tooltip, const Blade::TString& desc)
	{
		tooltip->AddTool(this, desc.c_str());
	}
	/** @brief  */
	virtual BOOL	Create(const RECT& rect, CWnd* pParentWnd, UINT nID,Blade::IConfig* config);
	/** @brief  */
	virtual LRESULT	DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class DropDownControl : public CComboBox, public ConfigControl, public Blade::Allocatable
{
	DECLARE_MESSAGE_MAP()
protected:
	/** @brief  */
	virtual void	updateDataImpl(const Blade::TString& Val);
public:
	using Allocatable::operator new;
	using Allocatable::operator delete;
	using Allocatable::operator new[];
	using Allocatable::operator delete[];
public:
	inline DropDownControl()						{}
	inline ~DropDownControl()						{}

	/** @brief tooltip helper */
	virtual void		AddToolTip(CToolTipCtrl* tooltip, const Blade::TString& desc)
	{
		tooltip->AddTool(this, desc.c_str());
	}

	virtual BOOL	Create(const RECT& rect, CWnd* pParentWnd, UINT nID,Blade::IConfig* config);
	virtual void	MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	afx_msg void	OnPaint();
	afx_msg LRESULT	OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL	OnSelectChange();
	afx_msg LRESULT	OnMsgMouseWheel(WPARAM wParam, LPARAM lParam);

protected:
	int		mItemHeight;
	CFont	mFont;
	CFont	mRealFont;
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
class CCompositedControl : public CUIDialog, public ConfigControl
{
public:
	CCompositedControl(UINT nIDTemplate, CWnd* pParentWnd = NULL) :CUIDialog(nIDTemplate, pParentWnd),mTooltip(NULL)	{}
	virtual ~CCompositedControl() {}

	/** @brief tooltip helper */
	virtual void		AddToolTip(CToolTipCtrl* tooltip, const Blade::TString& desc)
	{
		mTooltip = tooltip;

		CWnd* child = this->CUIDialog::GetWindow(GW_CHILD);
		while(child != NULL)
		{
			tooltip->AddTool(child, desc.c_str());
			child = child->GetWindow(GW_HWNDNEXT);
		}
		tooltip->AddTool(this, desc.c_str() );
	}

	/** @brief  */
	virtual BOOL		PreTranslateMessage(MSG* pMsg)
	{
		//disable this' ESC so that it will pass to the parent
		if( pMsg->message == WM_KEYDOWN && 
			(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_TAB || pMsg->wParam == VK_RETURN ) )
			return FALSE;
		//disable child's & this' mouse wheel and redirect it to parent
		if(pMsg->message == WM_MOUSEWHEEL)
		{
			if( this->GetParent() != NULL )
				pMsg->hwnd = this->GetParent()->m_hWnd;
			return FALSE;
		}


		if ( mTooltip != NULL &&
			pMsg->message >= WM_MOUSEFIRST &&
			pMsg->message <= WM_MOUSELAST)
		{
			mTooltip->RelayEvent(pMsg);
		}
		return __super::PreTranslateMessage(pMsg);
	}

	/** @brief  */
	virtual BOOL OnInitDialog()
	{
		BOOL ret = __super::OnInitDialog();
		if( ret )
			this->ModifyStyle(0, WS_TABSTOP);
		return ret;
	}
protected:
	CToolTipCtrl* mTooltip;
};//CCompositedControl

#pragma warning(pop)