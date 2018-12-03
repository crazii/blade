// TextSlider.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/TextSlider.h>

using namespace Blade;

// CTextSlider dialog

CTextSlider::CTextSlider()
	: CCompositedControl(CTextSlider::IDD)
	, mInfoText(_T(""))
	,mCurIndex(0)
{
}

CTextSlider::~CTextSlider()
{
}

//////////////////////////////////////////////////////////////////////////
BOOL	CTextSlider::Create(DWORD /*style*/,const RECT& prect,CWnd* parent,UINT nID,Blade::IConfig* config)
{
	BOOL result = __super::Create(IDD,parent);
	if( result )
	{
		ASSERT(config != NULL );
		this->MoveWindow(&prect);
		this->SetDlgCtrlID((int)nID);
		OptionValueList TempOptions;
		//find default index
		int DefaultValue;
		ConfigControlUtil::ParseConfigOption(config,TempOptions,DefaultValue);
		this->InitValues(TempOptions, (UINT)DefaultValue);
		this->Init(config, this);
	}
	return result;
}


//////////////////////////////////////////////////////////////////////////
void	CTextSlider::InitValues(const OptionValueList& strings,UINT curvalIndex/* = 0*/)
{
	mTStringValues.clear();
	mTStringValues.reserve(strings.size());

	for(size_t i = 0; i < strings.size(); ++i)
		mTStringValues.push_back(  strings[i] );

	mCurIndex = curvalIndex;

	assert( curvalIndex < strings.size() );
	mInfoText = BTString2Lang(mTStringValues[(size_t)mCurIndex]).c_str();
	mSliderCtrl.SetRange(0, (int)strings.size()-1,TRUE);
	mSliderCtrl.SetPos((int)curvalIndex);
	UpdateData(FALSE);

	//re-calculate size according to values just set
	CRect rect;
	this->GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	this->SetWindowPos(NULL,0,0,rect.Width(),rect.Height(), SWP_NOMOVE | SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////////
void	CTextSlider::updateDataImpl(const Blade::TString& Val)
{
	mInfoText = BTString2Lang(Val).c_str();
	TextList::iterator i = std::find( mTStringValues.begin(), mTStringValues.end(), Val);
	assert( i != mTStringValues.end() );
	mSliderCtrl.SetPos( int(i-mTStringValues.begin()) );

	this->UpdateData(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CTextSlider::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_VAR, mSliderCtrl);
	DDX_Text(pDX, IDC_SLIDER_INFO, mInfoText);
}


BEGIN_MESSAGE_MAP(CTextSlider, CCompositedControl)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_SETFONT, &CTextSlider::OnSetFont)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CTextSlider::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar)
{
	CSliderCtrl* slider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	assert( slider->GetSafeHwnd() == mSliderCtrl.GetSafeHwnd() );
	BLADE_UNREFERENCED(slider);

	if( mCurIndex != (UINT)mSliderCtrl.GetPos() )
	{
		mCurIndex = (UINT)mSliderCtrl.GetPos();

		if( mTStringValues.size() > 0 )
		{
			assert( mCurIndex < (int)mTStringValues.size() );
			mInfoText = BTString2Lang(mTStringValues[mCurIndex]).c_str();
			UpdateData(FALSE);
			//send a notificationO
			this->ApplyConfig( mTStringValues[mCurIndex] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTextSlider::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
}


//////////////////////////////////////////////////////////////////////////
void CTextSlider::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
}


//////////////////////////////////////////////////////////////////////////
void	CTextSlider::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if( (lpwndpos->flags & SWP_NOSIZE ) || mSliderCtrl.GetSafeHwnd() == NULL )
	{
		__super::OnWindowPosChanging(lpwndpos);
		return;
	}

	int cx = lpwndpos->cx;
	int cy = lpwndpos->cy;

	// TODO: Add your message handler code here
	CDC*   pDC = this->GetDC();
	TEXTMETRIC tm;
	CFont*   pFont = this->GetFont();
	CSize  size(0,0);

	// Select the listbox font, save the old font
	CFont* pOldFont = pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tm);

	if( mTStringValues.size() > 0 )
	{
		for( size_t i = 0; i < (int)mTStringValues.size(); ++i)
		{
			CSize sz = pDC->GetTextExtent(mTStringValues[i].c_str(), (int)mTStringValues[i].size() );
			if( sz.cx > size.cx )
				size.cx = sz.cx;
		}
	}
	else
		size = pDC->GetTextExtent(TEXT(" "),1);

	pDC->SelectObject(pOldFont);
	this->ReleaseDC(pDC);

	size.cx += tm.tmAveCharWidth;
	size.cy = lpwndpos->cy;

	CRect rect;
	rect.top = (cy - size.cy)/2;
	rect.left = 0;
	rect.right = rect.left + size.cx + ::GetSystemMetrics(SM_CXEDGE)*2;
	rect.bottom = rect.top + size.cy;
	this->GetDlgItem(IDC_SLIDER_INFO)->MoveWindow(&rect);

	CRect rectSlider;
	mSliderCtrl.GetWindowRect(&rectSlider);

	rect.left = rect.right;
	rect.right = cx;
	mSliderCtrl.MoveWindow(&rect);

	// TODO: Add your message handler code here
	__super::OnWindowPosChanging(lpwndpos);

}


//////////////////////////////////////////////////////////////////////////
void	CTextSlider::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);

	// TODO: Add your message handler code here
	if( mSliderCtrl.GetSafeHwnd() )
		mSliderCtrl.SetFocus();
}

//////////////////////////////////////////////////////////////////////////
LRESULT	CTextSlider::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	CFont* font = CFont::FromHandle((HFONT)wParam);
	CWnd* text = this->GetDlgItem(IDC_SLIDER_INFO);
	if( text != NULL )
		text->SetFont(font);
	return this->DefWindowProc(WM_SETFONT, wParam, lParam);
}