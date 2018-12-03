/********************************************************************
	created:	2015/11/25
	filename: 	TrackViewRangeDialog.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <stdafx.h>
#include "TrackViewRangeDialog.h"

namespace Blade
{
	BEGIN_MESSAGE_MAP(CTrackViewRangeDialog, CUIDialog)
	END_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	CTrackViewRangeDialog::CTrackViewRangeDialog(CWnd* pParent/* = NULL*/)
		:CUIDialog(IDD, pParent)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	CTrackViewRangeDialog::CTrackViewRangeDialog(int32 minFrame, int32 maxFrame, int32 startFrame, int32 endFrame, CWnd* pParent/* = NULL*/)
		:CUIDialog(IDD, pParent)
		,mMin(minFrame)
		,mMax(maxFrame)
		,mStart(startFrame)
		,mEnd(endFrame)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	CTrackViewRangeDialog::~CTrackViewRangeDialog()
	{

	}

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void CTrackViewRangeDialog::DoDataExchange(CDataExchange* pDX)
	{
		__super::DoDataExchange(pDX);
		DDX_Text(pDX, IDC_TRACKVIEW_RANGE_START, mStart);
		DDX_Text(pDX, IDC_TRACKVIEW_RANGE_END, mEnd);
		DDV_MinMaxInt(pDX, mStart, mMin, mMax);
		DDV_MinMaxInt(pDX, mEnd, mMin, mMax);
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL CTrackViewRangeDialog::OnInitDialog()
	{
		return __super::OnInitDialog();
	}

	
}//namespace Blade