/********************************************************************
	created:	2010/06/05
	filename: 	OptionControls.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ConfigDialog/OptionControls.h>
#include <ConfigDialog/ConfigControlUtil.h>
#include <ui/private/UIToolBar.h>

using namespace Blade;

/************************************************************************/
/*                                                                      */
/************************************************************************/
BEGIN_MESSAGE_MAP(CExpButton, CButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED,OnClicked)
	ON_CONTROL_REFLECT_EX(BN_DBLCLK,OnClicked)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
BOOL	CExpButton::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return this->CButton::Create(TEXT(""),BS_OWNERDRAW | WS_VISIBLE, rect, pParentWnd,nID);
}

//////////////////////////////////////////////////////////////////////////
void	CExpButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC;
	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	//expanded
	bool expand = this->GetExpanded() == TRUE;
	
	CRect rc(lpDrawItemStruct->rcItem);
	rc.right = rc.bottom = min(rc.right, rc.bottom);

	pDC->FrameRect(&lpDrawItemStruct->rcItem, &THEMEBRUSH(TC_TEXT));

	CPen pen(PS_SOLID, 1, THEMECOLOR(TC_TEXT));
	CPen* op = pDC->SelectObject(&pen);

	pDC->MoveTo(2, rc.Height()/2);
	pDC->LineTo(rc.Width()-2, rc.Height()/2);

	if(!expand)
	{
		pDC->MoveTo(rc.Width()/2, 2);
		pDC->LineTo(rc.Width()/2, rc.Height()-2);
	}
	pDC->SelectObject(op);
}

//////////////////////////////////////////////////////////////////////////
BOOL CExpButton::OnClicked()
{
	this->SetExpanded( !this->GetExpanded() );
	this->Invalidate();
	return FALSE;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
BEGIN_MESSAGE_MAP(CheckControl, CButton)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void	CheckControl::updateDataImpl(const TString& Val)
{
	bool bVal = TStringHelper::toBool(Val);
	if( bVal)
		this->SetCheck( BST_CHECKED );
	else
		this->SetCheck( BST_UNCHECKED );
}

//////////////////////////////////////////////////////////////////////////
BOOL	CheckControl::Create(const RECT& rect, CWnd* pParentWnd, UINT nID,Blade::IConfig* config)
{
	//note: BS_AUTOCHECKBOX cannot combine with BS_OWNERDRAW
	BOOL ret = this->CButton::Create( config != NULL? config->getValue().c_str() : TEXT(""),
		BS_AUTOCHECKBOX | WS_TABSTOP | BS_LEFTTEXT, rect, pParentWnd, nID);
	if( ret )
	{
		this->ModifyStyleEx(0, WS_EX_TRANSPARENT);
		ASSERT(config != NULL );
		CString text;
		this->GetWindowText(text);

		if(  Blade::TStringHelper::toBool( Blade::TString(text.GetString()) ) )
			this->SetCheck(TRUE);
		else
			this->SetCheck(FALSE);

		this->Init(config, this);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
LRESULT CheckControl::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CButton::DefWindowProc(message,wParam,lParam);
	if( message == BM_SETCHECK )
	{
		if( wParam == BST_CHECKED )
		{
			this->SetWindowText( BTLang(BLANG_TRUE).c_str() );
			this->ApplyConfig( TEXT("TRUE") );
		}
		else
		{
			this->SetWindowText( BTLang(BLANG_FALSE).c_str() );
			this->ApplyConfig( TEXT("FALSE") );
		}
	}
	return ret;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
#define DROPDOWN_OWNERDRAW 0

BEGIN_MESSAGE_MAP(DropDownControl, CComboBox)
	ON_CONTROL_REFLECT_EX(CBN_SELCHANGE,&DropDownControl::OnSelectChange)
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETFONT, &DropDownControl::OnSetFont)
	ON_MESSAGE(WM_MOUSEWHEEL,&DropDownControl::OnMsgMouseWheel)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void	DropDownControl::updateDataImpl(const TString& /*Val*/)
{
	OptionValueList TempOptions;
	//find default index
	int DefaultValue;
	ConfigControlUtil::ParseConfigOption(mConfig, TempOptions, DefaultValue);
	int index = DefaultValue;
	assert(index != CB_ERR );
	this->SetCurSel( index );
}

//////////////////////////////////////////////////////////////////////////
BOOL DropDownControl::Create(const RECT& rect, CWnd* pParentWnd, UINT nID,Blade::IConfig* config)
{
	ASSERT(config != NULL );
	if(config == NULL)
		return FALSE;

	mItemHeight = rect.bottom - rect.top - ::GetSystemMetrics(SM_CYBORDER)*2;
	DWORD style = CBS_DROPDOWNLIST /*| CBS_NOINTEGRALHEIGHT*/ /*| WS_VISIBLE*/ | WS_CHILD | WS_TABSTOP;
#if DROPDOWN_OWNERDRAW
	style |= CBS_OWNERDRAWFIXED | CBS_HASSTRINGS;
#endif
	BOOL ret = this->CComboBox::Create(style, rect, pParentWnd, nID);
	if( !ret )
		return ret;
	this->ModifyStyleEx(0,WS_EX_TRANSPARENT);
	assert( config->getUIHint() == CUIH_LIST || config->getUIHint() == CUIH_INDEX );
	OptionValueList TempOptions;
	//find default index
	int DefaultValue;
	ConfigControlUtil::ParseConfigOption(config,TempOptions,DefaultValue);

	size_t n;
	for(n = 0;n < TempOptions.size();++n )
	{
		TString str = (TString)TempOptions[n];
		if( str != TString::EMPTY )
			str = BTString2Lang(TempOptions[n]);
		this->AddString( str.c_str() );
	}
	this->SetCurSel(DefaultValue);
	this->SetItemHeight(-1, (UINT)mItemHeight);
			
	//from MSDN
	// Find the longest string in the combo box.
	CString  str;
	CSize  sz;
	int		dx = 0;
	int		dy = 0;
	TEXTMETRIC tm;
	CDC*   pDC = this->GetDC();
	CFont*   pFont = this->GetFont();

	// Select the listbox font, save the old font
	CFont* pOldFont = pDC->SelectObject(pFont);
	// Get the text metrics for avg char width
	pDC->GetTextMetrics(&tm);

	for (int i = 0;i < this->GetCount();i++)
	{
		this->GetLBText(i, str);
		sz = pDC->GetTextExtent(str);
		// Add the avg width to prevent clipping
		sz.cx += tm.tmAveCharWidth;
		dy += tm.tmHeight+ sz.cy;
		if (sz.cx > dx)
			dx = sz.cx;
	}
	// Select the old font back into the DC
	pDC->SelectObject(pOldFont);
	this->ReleaseDC(pDC);

	// Adjust the width for the vertical scroll bar and the left and right border.
	dx += ::GetSystemMetrics(SM_CXVSCROLL) + 2*::GetSystemMetrics(SM_CXEDGE);
	dy += 2*::GetSystemMetrics(SM_CYEDGE);

	CRect crect(rect);
	crect.bottom += dy;
	//this->SetWindowPos(NULL,0,0,crect.Width(),crect.Height(),SWP_NOZORDER|SWP_NOMOVE);

	// Set the width of the list box so that every item is completely visible.
	this->SetDroppedWidth((UINT)dx);
	this->Init(config, this);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void	DropDownControl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
#if DROPDOWN_OWNERDRAW
	lpMeasureItemStruct->itemHeight = (UINT)mItemHeight;
#else
	BLADE_UNREFERENCED(lpMeasureItemStruct);
#endif
}

//////////////////////////////////////////////////////////////////////////
void	DropDownControl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
#if DROPDOWN_OWNERDRAW
	ASSERT(lpDrawItemStruct->CtlType == ODT_COMBOBOX);
	if( lpDrawItemStruct->itemID == CB_ERR || lpDrawItemStruct->itemID == CB_ERRSPACE )
		return;
	if( lpDrawItemStruct->itemState&ODS_COMBOBOXEDIT)
		return;

	CRect rect(lpDrawItemStruct->rcItem);

	//reset height
	//lpDrawItemStruct->rcItem.bottom = lpDrawItemStruct->rcItem.top + mItemHeight;

	CString text;
	this->GetLBText((int)lpDrawItemStruct->itemID, text);
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	// Save these value to restore them when done drawing.
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	// If this item is selected, set the background color 
	// and the text color to appropriate values. Erase
	// the rect by filling it with the background color.
	if ( (lpDrawItemStruct->itemState & (ODS_SELECTED)) )
	{
		dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		dc.FillSolidRect(&rect, ::GetSysColor(COLOR_HIGHLIGHT));
	}
	else
		dc.FillSolidRect(&rect, crOldBkColor);

	// Draw the text.
	CFont* oldFont = dc.SelectObject(&mRealFont);
	dc.DrawText(text, text.GetLength(), &rect, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	dc.SelectObject(oldFont);

	// Reset the background color and the text color back to their
	// original values.
	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);

	dc.Detach();
#else
	BLADE_UNREFERENCED(lpDrawItemStruct);
#endif
}

//////////////////////////////////////////////////////////////////////////
void DropDownControl::OnPaint()
{
	CPaintDC dc(this);

	COMBOBOXINFO info;
	info.cbSize = sizeof(COMBOBOXINFO);
	this->GetComboBoxInfo(&info);

	UINT state = DFCS_SCROLLCOMBOBOX;
	if( info.stateButton == STATE_SYSTEM_PRESSED )
		state |= DFCS_PUSHED;

	CRect rc;
	this->GetClientRect(&rc);

	//dc.SetBkMode(TRANSPARENT);
	dc.SetBkColor(THEMECOLOR(TC_CTLBK));

	int nCurSel = this->GetCurSel();
	if( nCurSel != -1 )
	{
		//dc.FillSolidRect(&rc, THEMECOLOR(TC_CTLBK));

		CString text;
		this->GetLBText(nCurSel,text);

		CFont* oldFont = dc.SelectObject(&mRealFont);
		COLORREF oldTextColor = dc.SetTextColor(THEMECOLOR(TC_TEXT));
		dc.DrawText(text.GetString(),text.GetLength(),&rc,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
		dc.SelectObject(oldFont);
		dc.SetTextColor(oldTextColor);
	}

	CRect rcButton(info.rcButton);
	//do default drawing for dropdown arrow
	this->InvalidateRect(&rcButton);
	Default();
}

//////////////////////////////////////////////////////////////////////////
LRESULT	DropDownControl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	CFont* font = CFont::FromHandle((HFONT)wParam);

	//create a font by current height
	//so that the combobox measure item's in the specified height
	//and it will change the window (appearance) height of the combobox
	LOGFONT lf;
	if( font == NULL || !font->GetLogFont(&lf) )
		return this->DefWindowProc(WM_SETFONT, wParam, lParam);
	mRealFont.CreateFontIndirect(&lf);
	
	CRect rect;
	this->GetWindowRect(&rect);
	scalar rate = (scalar)mItemHeight*(scalar)lf.lfHeight/(scalar)rect.Height();
	lf.lfHeight = (LONG)(rate);

	mFont.DeleteObject();
	mFont.CreateFontIndirect(&lf);

	CFont::DeleteTempMap();

	LRESULT ret = this->DefWindowProc(WM_SETFONT, (WPARAM)(HFONT)mFont, lParam);

	//hack: change internal list box's font using real font
	COMBOBOXINFO info;
	info.cbSize = sizeof(COMBOBOXINFO);
	this->GetComboBoxInfo(&info);
	::SendMessage(info.hwndList, WM_SETFONT, (WPARAM)(HFONT)mRealFont, lParam);
	return ret;
}


//////////////////////////////////////////////////////////////////////////
BOOL	DropDownControl::OnSelectChange()
{
	OptionValueList TempOptions;
	//find default index
	int currentVal = -1;
	ConfigControlUtil::ParseConfigOption(mConfig,TempOptions, currentVal);
	if( currentVal != this->GetCurSel() && (size_t)this->GetCurSel() < TempOptions.size()/* && mConfig->getSubConfigs() != NULL*/)
	{
		if( mConfig->getUIHint() == CUIH_LIST )
		{
			const TString& val = TempOptions[ (index_t)this->GetCurSel() ];
			this->ApplyConfig(val);
		}
		else if( mConfig->getUIHint() == CUIH_INDEX )
		{
			TString val = TStringHelper::fromUInt( (unsigned int)this->GetCurSel() );
			this->ApplyConfig(val);
		}
		else
			assert(false);
		return FALSE;
	}
	else
		return FALSE;	//parent don't process
}

//////////////////////////////////////////////////////////////////////////
LRESULT	DropDownControl::OnMsgMouseWheel(WPARAM wParam, LPARAM lParam)
{
	//disable mouse wheel for dropdown list
	if( this->GetParent() != NULL )
		this->GetParent()->SendMessage(WM_MOUSEWHEEL, wParam, lParam );
	return TRUE;
}