/********************************************************************
	created:	2016/7/21
	filename: 	UISlider.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <stdafx.h>
#include <ui/private/UISlider.h>

BEGIN_MESSAGE_MAP(CUISlider, CSliderCtrl)
#if ENABLE_THEME
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
#endif
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CUISlider::CUISlider()
{

}

//////////////////////////////////////////////////////////////////////////
CUISlider::~CUISlider()
{

}

//////////////////////////////////////////////////////////////////////////
void CUISlider::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
#if ENABLE_THEME
	// for additional info, read beginning MSDN "Customizing a Control's Appearance Using Custom Draw" at
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/custdraw/custdraw.asp
	// slider controls (which are actually called "trackbar" controls) are specifically discussed at
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/trackbar/notifications/nm_customdraw_trackbar.asp

	const NMCUSTOMDRAW& nmcd = *(LPNMCUSTOMDRAW)pNMHDR;

	UINT drawStage = nmcd.dwDrawStage;
	DWORD_PTR itemSpec = nmcd.dwItemSpec;

	switch (drawStage)
	{
		// #define CDDS_PREPAINT           0x00000001
		// #define CDDS_POSTPAINT          0x00000002
		// #define CDDS_PREERASE           0x00000003
		// #define CDDS_POSTERASE          0x00000004
		// #define CDDS_ITEM               0x00010000
		// #define CDDS_ITEMPREPAINT       (CDDS_ITEM | CDDS_PREPAINT)
		// #define CDDS_ITEMPOSTPAINT      (CDDS_ITEM | CDDS_POSTPAINT)
		// #define CDDS_ITEMPREERASE       (CDDS_ITEM | CDDS_PREERASE)
		// #define CDDS_ITEMPOSTERASE      (CDDS_ITEM | CDDS_POSTERASE)

		// #define TBCD_TICS    0x0001
		// #define TBCD_THUMB   0x0002
		// #define TBCD_CHANNEL 0x0003

		//	#define CDIS_SELECTED       0x0001
		//	#define CDIS_GRAYED         0x0002
		//	#define CDIS_DISABLED       0x0004
		//	#define CDIS_CHECKED        0x0008
		//	#define CDIS_FOCUS          0x0010
		//	#define CDIS_DEFAULT        0x0020
		//	#define CDIS_HOT            0x0040
		//	#define CDIS_MARKED         0x0080
		//	#define CDIS_INDETERMINATE  0x0100

	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_PREERASE:		// Before the erase cycle begins
	case CDDS_POSTERASE:	// After the erase cycle is complete
	case CDDS_ITEMPREERASE:	// Before an item is erased
	case CDDS_ITEMPOSTERASE:	// After an item has been erased
		*pResult = CDRF_DODEFAULT;
		break;
	case CDDS_ITEMPREPAINT:	// Before an item is drawn
		switch (itemSpec)
		{
		case TBCD_CHANNEL:
			*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
			break;

		case TBCD_TICS:
			*pResult = CDRF_DODEFAULT;
			break;

		case TBCD_THUMB:
		{
			CDC* pDC = CDC::FromHandle(nmcd.hdc);
			int iSaveDC = pDC->SaveDC();

			CRect rcBox(nmcd.rc);
			pDC->FillSolidRect(&rcBox, THEMECOLOR(TC_BTN));

			CDC maskMemDC;
			maskMemDC.CreateCompatibleDC(pDC);
			CBitmap maskBitmap;
			maskBitmap.CreateCompatibleBitmap(pDC, rcBox.Width(), rcBox.Height());
			maskMemDC.SelectObject(&maskBitmap);
			CRect rcMask(0, 0, rcBox.Width(), rcBox.Height());
			UINT dfcs = (UINT)DFCS_BUTTONPUSH;
			if (nmcd.uItemState & (CDIS_DISABLED | CDIS_GRAYED))
				dfcs |= DFCS_INACTIVE;
			else if ((nmcd.uItemState & (CDIS_FOCUS | CDIS_HOT)))
				dfcs |= DFCS_HOT;

			maskMemDC.DrawFrameControl(&rcMask, DFC_BUTTON, dfcs);

			pDC->BitBlt(rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(), &maskMemDC, 0, 0, SRCINVERT);
			pDC->BitBlt(rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(), &maskMemDC, 0, 0, SRCAND);
			pDC->BitBlt(rcBox.left, rcBox.top, rcBox.Width(), rcBox.Height(), &maskMemDC, 0, 0, SRCINVERT);

			pDC->RestoreDC(iSaveDC);
		}
		*pResult = CDRF_SKIPDEFAULT;
		break;

		default:
			ASSERT(FALSE);	// all of a slider's items have been listed, so we shouldn't get here
		};

		break;

	case CDDS_ITEMPOSTPAINT:	// After an item has been drawn

		switch (itemSpec)
		{
		case TBCD_CHANNEL:
		{
			CDC* pDC = CDC::FromHandle(nmcd.hdc);
			pDC->FillSolidRect(&nmcd.rc, THEMECOLOR(TC_EMPTYBK) );
		}
		*pResult = CDRF_SKIPDEFAULT;
		break;

		case TBCD_TICS:
			*pResult = CDRF_DODEFAULT;
			break;

		case TBCD_THUMB:
			*pResult = CDRF_DODEFAULT;
			break;
		default:
			ASSERT(FALSE);
		};
		break;

	case CDDS_POSTPAINT:
	*pResult = CDRF_SKIPDEFAULT;
	//*pResult = CDRF_DODEFAULT;
	break;

	default:
		ASSERT(FALSE);
	};
#else
	BLADE_UNREFERENCED(pNMHDR);
	BLADE_UNREFERENCED(pResult);
#endif
}