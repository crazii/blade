/********************************************************************
	created:	2011/05/09
	filename: 	UIDialog.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ui/private/UIDialog.h>
#include "resource.h"

using namespace Blade;

#define ENABLE_DLG_THEME 0

#if ENABLE_DLG_THEME
#	pragma comment(lib,"uxtheme.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CUIDialog, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CUIDialog dialog
CUIDialog::CUIDialog(UINT idd,CWnd* pParent /*=NULL*/)
: CDialog(idd,pParent)
{
}

//////////////////////////////////////////////////////////////////////////
BOOL CUIDialog::OnInitDialog()
{
	BOOL result = __super::OnInitDialog();

	//--------------------------
	// Optional: for Themes XP dialogs: enable Themes dialog texture
	if( result )
	{
		//this->EnableThemeDialogTexture();

		CString caption;
		this->GetWindowText(caption);
		if( !caption.IsEmpty() )
		{
			const TString& langCap = BTString2Lang(TString(caption.GetString()));
			if( langCap != NULL )
				this->SetWindowText( langCap.c_str() );
		}

		CWnd* child = this->GetWindow(GW_CHILD);
		while( child != NULL )
		{
			child->GetWindowText(caption);
			TString langCaption = TString(caption.GetString());
			if( !langCaption.empty() )
			{
				langCaption = BTString2Lang( langCaption );
				child->SetWindowText( langCaption .c_str() );
			}
			child = child->GetNextWindow(GW_HWNDNEXT);
		}
		CWnd::DeleteTempMap();
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
class CHackWnd : public CWnd
{
	public: using CWnd::OnCtlColor;
};

HBRUSH  CUIDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
#if ENABLE_THEME
	BLADE_UNREFERENCED(pWnd);
	CWnd* parent = this->GetParent();
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(THEMECOLOR(TC_TEXT));

	if (nCtlColor == CTLCOLOR_EDIT)
	{
		return THEMEBRUSH(TC_TEXTBK);
	}

	if(parent != NULL && (this->GetStyle()&WS_CHILD) )
		return THEMEBRUSH(TC_CTLBK);
	else
		return THEMEBRUSH(TC_WINBK);
#else
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
#endif
}