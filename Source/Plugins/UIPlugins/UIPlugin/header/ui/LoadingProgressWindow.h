/********************************************************************
	created:	2013/03/05
	filename: 	LoadingProgressWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LoadingProgressWindow_h__
#define __Blade_LoadingProgressWindow_h__
#include <interface/public/ISerializable.h>
#include <interface/public/window/IWindowMessagePump.h>

class CLoadingProgressWindow : public CWnd, public Blade::TempAllocatable
{
public:
	using Blade::TempAllocatable::operator new;
	using Blade::TempAllocatable::operator delete;
public:
	CLoadingProgressWindow()
	{
	}

	/** @brief  */
	virtual BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect,
		CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL)
	{
		BOOL ret = __super::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
		if( ret )
		{
			CRect crect;
			this->GetClientRect(&crect);
			mProgressBar.Create(WS_CHILD|WS_VISIBLE, crect, this, UINT(-1));
			mProgressBar.SetRange(0,100);
			mProgressBar.SetStep(1);
		}
		return ret;
	}

	/** @brief  */
	void		SetProgress(Blade::scalar percent)
	{
		if( mProgressBar.GetSafeHwnd() == NULL )
			return;

		mProgressBar.SetPos( int(percent*100) );
	}

protected:
	CProgressCtrl mProgressBar;
};

namespace Blade
{
	class LoadingProgress : public IProgressCallback, public StaticAllocatable
	{
	public:
		/** @brief  */
		LoadingProgress()
			:mWindow(NULL)
		{

		}

		/** @brief  */
		virtual void onNotify(Blade::scalar percent)
		{
			if (mWindow != NULL)
			{
				//process message to prevent no response
				IWindowMessagePump::getSingleton().processMessage();

				AFX_MANAGE_STATE(::AfxGetStaticModuleState());
				mWindow->SetProgress(percent);
				mWindow->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_UPDATENOW);

				if (percent >= 1.0f)
				{
					mWindow->GetParent()->EnableWindow(TRUE);
					mWindow->ShowWindow(SW_HIDE);
					BLADE_DELETE mWindow;
					mWindow = NULL;
				}
			}
		}

		/** @brief  */
		void	reset(CWnd* parent, const TString& title)
		{
			parent->EnableWindow(FALSE);

			CRect rect;
			parent->GetClientRect(&rect);
			CPoint center = rect.CenterPoint();
			int width = rect.Width() / 3;
			int height = 25;
			CSize halfSize(width / 2, height / 2);
			rect.SetRect(center - halfSize, center + halfSize);

			mWindow = BLADE_NEW CLoadingProgressWindow();
			mWindow->Create(NULL, TEXT("loading"), WS_OVERLAPPED | WS_CHILD | WS_CAPTION | WS_BORDER, rect, parent, UINT(-1));
			mWindow->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			mWindow->SetWindowText(title.c_str());
			mWindow->ShowWindow(SW_SHOW);
			//mWindow->RedrawWindow();
		}

	protected:
		CLoadingProgressWindow*	mWindow;
	};
	
}//namespace Blade


#endif //  __Blade_LoadingProgressWindow_h__