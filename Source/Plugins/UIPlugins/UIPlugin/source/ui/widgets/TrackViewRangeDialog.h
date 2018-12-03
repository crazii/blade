/********************************************************************
	created:	2015/11/25
	filename: 	TrackViewRangeDialog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TrackViewRangeDialog_h__
#define __Blade_TrackViewRangeDialog_h__
#include <resource.h>
#include <ui/private/UIDialog.h>

namespace Blade
{
	class CTrackViewRangeDialog : public CUIDialog
	{
	public:
		CTrackViewRangeDialog(CWnd* pParent = NULL);   // standard constructor
		CTrackViewRangeDialog(int32 minFrame, int32 maxFrame, int32 startFrame, int32 endFrame, CWnd* pParent = NULL);   // standard constructor
		virtual ~CTrackViewRangeDialog();

		// Dialog Data
		enum { IDD = IDD_TRACKVIEW_RANGE };

		/** @brief  */
		inline int32 getRangeStart() const		{return mStart;}

		/** @brief  */
		inline int32 getRangeEnd() const		{return mEnd;}

	protected:
		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnInitDialog();
		DECLARE_MESSAGE_MAP()

		int32 mMin;
		int32 mMax;
		int32 mStart;
		int32 mEnd;
	};

	
}//namespace Blade


#endif // __Blade_TrackViewRangeDialog_h__