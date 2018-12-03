/********************************************************************
	created:	2014/09/30
	filename: 	TrackView.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "TrackView.h"

#pragma warning(disable:4355)	//this used in base member initializer list

namespace Blade
{

	BEGIN_MESSAGE_MAP(TrackView, DockWindowBase<Blade::IUITrackView>)
		//{{AFX_MSG_MAP(LogWindow)
		ON_WM_CREATE()
		ON_WM_SIZE()
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	const TString TrackView::WIDGET_TYPE = BTString("TrackView");

	//////////////////////////////////////////////////////////////////////////
	TrackView::TrackView()
		:DockWindowBase(WIDGET_TYPE)
		, mUI(this)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TrackView::~TrackView()
	{

	}

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	int TrackView::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int result = __super::OnCreate(lpCreateStruct);

		mUI.Create(CTrackViewUI::IDD, this);

		return result;

	}

	//////////////////////////////////////////////////////////////////////////
	void TrackView::OnSize(UINT nType, int cx, int cy)
	{
		__super::OnSize(nType, cx, cy);

		if (mUI.GetSafeHwnd() != NULL)
		{
			CRect clientRect;
			this->GetClientRect(&clientRect);
			mUI.MoveWindow(&clientRect);
		}
	}
	
}//namespace Blade