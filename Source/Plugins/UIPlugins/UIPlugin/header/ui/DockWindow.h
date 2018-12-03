/********************************************************************
	created:	2011/05/06
	filename: 	DockWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DockWindow_h__
#define __Blade_DockWindow_h__
#include <interface/public/ui/IUIWidget.h>

namespace Blade
{
	//note: DockWindow uses template inheriting from abstract class
	//in order to "safe" cast between different template instantiate, (i.e. DockWindow<IUIWidget>* to DockWindow<IUITrackView>*)
	//DockWindow cannot have any pure function calls, otherwise it will cause a 
	//virtual function mismatch
	//so trait the interface of dock window's own, to another definition, and
	//prepend it to template interface
	class DockWindowImpl
	{
	public:
		/** @brief  */
		virtual CWnd*			getCWnd() const = 0;

		/** @brief  */
		virtual bool			createWindow() = 0;
	};

	template<typename T>
	class DockWindow : public DockWindowImpl, public T
	{
	public:
		DockWindow(const TString& type)
			:mType(type)
		{
		}

		virtual ~DockWindow() {}

		/************************************************************************/
		/* IUIWidget interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWidgetType() const { return mType; }

		/** @brief  */
		virtual bool		initWidget(const WIDGET_DATA& data)
		{
			if (mType != data.mType)
			{
				assert(false);
				return false;
			}
			mData = data;
			mShouldBeVisible = (mData.mFlags&WMF_VISIBLE) != 0;
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			return this->createWindow();
		}

		/** @brief  */
		virtual void		setPosition(POINT2I pos) { this->getCWnd()->SetWindowPos(NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER); }

		/** @brief  */
		virtual void		setSize(SIZE2I size) { this->getCWnd()->SetWindowPos(NULL, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER); }

		/** @brief  */
		virtual bool		setVisible(bool visible)
		{
			mShouldBeVisible = visible;	//record user setting

			if (this->isVisible() != visible)
			{
				T::setVisible(visible);
				CWnd* wnd = this->getCWnd();
				wnd->ShowWindow(visible ? SW_SHOW : SW_HIDE);
				return true;
			}
			return false;
		}

		/** @brief  */
		virtual bool		setParent(IUIWidget* /*parent*/) { assert(false); return false; }

		/** @brief  */
		virtual IUIWidget*	getParent() const { return NULL; }

		/** @brief  */
		virtual const WIDGET_DATA& getWidgetData() const { return mData; }

		/** @brief  */
		virtual WIDGET_DATA& getWidgetDataW() { return mData; }

		/** @brief  */
		inline bool			shouldBeVisible() const { return mShouldBeVisible; }

	protected:
		TString		mType;
		WIDGET_DATA	mData;
		bool		mShouldBeVisible;	//to keep user setting. dock window may hide due to docking tab groups. 
	};

	template<typename T>
	class DockWindowBase : public CStatic, public DockWindow<T>
	{
	public:
		DockWindowBase(const TString& type)
			:DockWindow(type)
			, mActivedChild(NULL)
		{
		}

		virtual ~DockWindowBase() {}

	protected:

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual CWnd*	getCWnd() const { return (CStatic*)this; }

		/** @brief  */
		virtual bool	createWindow()
		{
			RECT rect = { 0, 0, 0, 0 };
			return this->Create(mData.mCaption.c_str(), WS_VISIBLE | WS_CHILD, rect, ::AfxGetMainWnd(), UINT(-1)) == TRUE;
		}

		/************************************************************************/
		/* internal                                                                     */
		/************************************************************************/
		/** @brief  */
		void		setActiveChild(CWnd* child)
		{
			if (m_hWnd != NULL && this->IsChild(child))
				mActivedChild = child;
		}

		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		/** @brief  */
		afx_msg void OnSetFocus(CWnd* pOldWnd)
		{
			if (mActivedChild != NULL)
				mActivedChild->SetFocus();

			__super::OnSetFocus(pOldWnd);
		}

		/** @brief  */
		afx_msg LRESULT OnGetText(WPARAM wParam, LPARAM lParam)
		{
			WPARAM cchTextMax = wParam;
			LPTSTR lpszText = (LPTSTR)lParam;
			UINT minchars = min((UINT)mData.mCaption.size() + 1, (UINT)cchTextMax);
			if (lpszText != NULL)
			{
				Char_Traits<TCHAR>::copy(lpszText, mData.mCaption.c_str(), minchars);
				//if string is truncated,assure NULL char appended
				lpszText[minchars - 1] = TEXT('\0');
				return (LRESULT)minchars;
			}
			else
				return 0;
		}

		/** @brief  */
		afx_msg LRESULT OnGetTextLength(WPARAM wParam, LPARAM lParam)
		{
			BLADE_UNREFERENCED(wParam);
			BLADE_UNREFERENCED(lParam);
			return (LRESULT)mData.mCaption.size();
		}

		/** @brief  */
		afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos)
		{
			CStatic::OnWindowPosChanged(lpwndpos);

			if (!(lpwndpos->flags&SWP_NOSIZE))
			{
				mData.mSize.x = lpwndpos->cx;
				mData.mSize.y = lpwndpos->cy;
			}

			if (!(lpwndpos->flags&SWP_NOMOVE))
			{
				mData.mPos.x = lpwndpos->x;
				mData.mPos.y = lpwndpos->y;
			}
		}

		/** @brief  */
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus)
		{
			CStatic::OnShowWindow(bShow, nStatus);
			mData.mFlags |= bShow ? WMF_VISIBLE : 0;
			mData.mFlags &= bShow ? uint32(-1) : (~WMF_VISIBLE);
		}
#if ENABLE_THEME
		/** @brief  */
		afx_msg BOOL OnEraseBkgnd(CDC* pDC)
		{
			CRect rect;
			this->GetClientRect(&rect);
			pDC->FillSolidRect(&rect, THEMECOLOR(TC_WINBK));
			return TRUE;
		}

		/** @brief  */
		afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
		{
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor( THEMECOLOR(TC_TEXT) );
			return THEMEBRUSH(TC_CTLBK);
		}

		/** @brief  */
		afx_msg void OnPaint()
		{
			CRect rect;
			this->GetClientRect(&rect);
			CPaintDC dc(this);
			dc.FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));
		}
#endif

		DECLARE_MESSAGE_MAP()
		CWnd*	mActivedChild;
		CBrush	mBrush;
	};

	BEGIN_TEMPLATE_MESSAGE_MAP(DockWindowBase, T, CStatic)
		ON_WM_SETFOCUS()
		ON_MESSAGE(WM_GETTEXT, OnGetText)
		ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
		ON_WM_WINDOWPOSCHANGED()
		ON_WM_SHOWWINDOW()
#if ENABLE_THEME
		ON_WM_ERASEBKGND()
		ON_WM_CTLCOLOR()
		ON_WM_PAINT()
#endif
	END_MESSAGE_MAP()
	
}//namespace Blade


#endif // __Blade_DockWindow_h__