// SliderInput.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/SliderInput.h>

using namespace Blade;

// CSliderInput dialog
#undef min
#undef max

//////////////////////////////////////////////////////////////////////////
CSliderInput::CSliderInput(CWnd* pParent /*=NULL*/)
	: CCompositedControl(CSliderInput::IDD, pParent)
	, mInputString(_T(""))
	,mRangeMin(0.0f)
	,mRangeMax(1.0f)
	,mSliderPos(0)
{

}

//////////////////////////////////////////////////////////////////////////
CSliderInput::~CSliderInput()
{
}

//////////////////////////////////////////////////////////////////////////
BOOL	CSliderInput::Create(const RECT& rect, CWnd* pParent, UINT nID, Blade::IConfig* config, bool edge/* = true*/)
{
	BOOL ret = __super::Create(IDD, pParent);
	this->Init(config, this);
	if( ret )
	{
		ASSERT(config != NULL );
		if( edge )
		{
			//this->GetDlgItem(IDC_SLIDER_EDIT)->ModifyStyleEx(0,WS_EX_CLIENTEDGE,WS_BORDER);
			this->GetDlgItem(IDC_SLIDER_EDIT)->ModifyStyle(0, WS_BORDER, SWP_DRAWFRAME);
		}
		else
			this->GetDlgItem(IDC_SLIDER_EDIT)->ModifyStyle(WS_BORDER, 0, SWP_DRAWFRAME);
			//this->GetDlgItem(IDC_SLIDER_EDIT)->ModifyStyleEx(WS_EX_CLIENTEDGE,0,SWP_DRAWFRAME);

		mInputString = config->getValue().c_str();
		UpdateData(FALSE);

		if( config->getUIHint() != CUIH_INPUT_RANGE )
		{
			ASSERT( config->getUIHint() == CUIH_INPUT );
			mInputSlider.ShowWindow(SW_HIDE);
			mRangeMin = 0;
			mRangeMax = 0;
			mRangeStep = 0;
		}
		else
		{
			ASSERT( config->getUIHint() == CUIH_INPUT_RANGE );
			ASSERT( config->getOptions().size() >= 3 );

			mRangeMin = TStringHelper::toScalar( config->getOptions()[0] );
			mRangeMax = TStringHelper::toScalar( config->getOptions()[1] );
			mRangeStep = TStringHelper::toScalar( config->getOptions()[2] );

			mInputSlider.ShowWindow(SW_SHOW);
			float val_diff = mRangeMax - mRangeMin;
			int iCount = int( uint(val_diff/mRangeStep) );
			if( val_diff < 0 || iCount > 0x0FFFFFFF || iCount < 0 )
			{
				BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid range for maxval/minval.") );
			}
			mInputSlider.SetRange( 0, iCount );

			float val = TStringHelper::toScalar( config->getValue() );
			mSliderPos = int((val-mRangeMin)/mRangeStep);
			mInputSlider.SetPos(mSliderPos);
		}
		this->MoveWindow(&rect);
		this->SetDlgCtrlID((int)nID);
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
void	CSliderInput::updateDataImpl(const Blade::TString& Val)
{
	mInputString = Val.c_str();

	float val = TStringHelper::toScalar( Val );
	mSliderPos = int((val-mRangeMin)/mRangeStep);
	mInputSlider.SetPos( mSliderPos );
	this->UpdateData(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CSliderInput::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INPUT_SLIDER, mInputSlider);
	DDX_Text(pDX,IDC_SLIDER_EDIT,mInputString);
}

//////////////////////////////////////////////////////////////////////////
BOOL	CSliderInput::PreTranslateMessage(MSG* pMsg)
{
	if( mConfig != NULL && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && CWnd::GetFocus() == GetDlgItem(IDC_SLIDER_EDIT) )
	{
		if( mConfig->getUIHint() == CUIH_INPUT_RANGE )
		{
			UpdateData(TRUE);
			TString str( mInputString.GetString() );
			float val = TStringHelper::toScalar( str );
			if( val < mRangeMin)
			{
				val = mRangeMin;
				mInputString = TStringHelper::fromScalar(val).c_str();
			}
			else if( val > mRangeMax )
			{
				val = mRangeMax;
				mInputString = TStringHelper::fromScalar(val).c_str();
			}
			else
			{
				val = mRangeMin + ((int)((val-mRangeMin)/mRangeStep))*mRangeStep;
				mInputString = TStringHelper::fromScalar(val).c_str();
			}

			mSliderPos = (int)((val - mRangeMin)/mRangeStep);
			mInputSlider.SetPos(mSliderPos);
			UpdateData(FALSE);
		}
		else
		{
			UpdateData(TRUE);
			IConfig* config = this->GetConfig();
			if( config->getValue() == mInputString.GetString() && mEventNotify )
			{
				const HCDHANDLER& handler = config->getDataHandler();
				const Variant& val = config->getBinaryValue();
				if( handler != NULL && handler->getTarget() != NULL )
					handler->setData(config->getName(), val.getBinaryData(), val.getSize(), config->getAccess());
			}
			//set to config
			this->ApplyConfig( mInputString.GetString() );
			//read new( may diff from value that is just set)
			mInputString = mConfig->getValue().c_str();
			UpdateData(FALSE);
		}
	}
	return __super::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
void	CSliderInput::OnOK()
{

}

//////////////////////////////////////////////////////////////////////////
void	CSliderInput::OnCancel()
{

}

BEGIN_MESSAGE_MAP(CSliderInput, CCompositedControl)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_SLIDER_EDIT, &CSliderInput::OnEnKillfocusSliderEdit)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_SETFONT, &CSliderInput::OnSetFont)
END_MESSAGE_MAP()


// CSliderInput message handlers
void	CSliderInput::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if( !(lpwndpos->flags & SWP_NOSIZE ) && mInputSlider.GetSafeHwnd() != NULL )
	{
		CWnd* edit = this->GetDlgItem(IDC_SLIDER_EDIT);

		CRect editRect;
		{
			CDC*   pDC = this->GetDC();
			TEXTMETRIC tm;
			CFont*   pFont = this->GetFont();
			// Select the listbox font, save the old font
			CFont* pOldFont = pDC->SelectObject(pFont);
			pDC->GetTextMetrics(&tm);
			
			CSize size;

			float maxLengthVal = mRangeMax + mRangeStep;
			TString lengthVal = TStringHelper::fromScalar(maxLengthVal);
			size = pDC->GetTextExtent(lengthVal.c_str(), (int)lengthVal.size());
			size.cx += tm.tmAveCharWidth;
			size.cy = lpwndpos->cy/* - ::GetSystemMetrics(SM_CYEDGE)*2*/;
			editRect.left = -1;
			editRect.top  = 0;
			editRect.right = editRect.left + size.cx + ::GetSystemMetrics(SM_CXEDGE)*2;
			editRect.bottom = lpwndpos->cy;
			pDC->SelectObject(pOldFont);
			this->ReleaseDC(pDC);
		}

		if(mConfig != NULL && mConfig->getUIHint() == CUIH_INPUT_RANGE)
		{
			edit->MoveWindow(&editRect);
			CRect rect;
			rect.left = editRect.right;
			rect.top = editRect.top;
			rect.right = lpwndpos->cx - ::GetSystemMetrics(SM_CXEDGE);
			rect.bottom = editRect.bottom;
			mInputSlider.MoveWindow(&rect);
			mInputSlider.ShowWindow(SW_SHOW);
		}
		else
		{
			editRect.right = editRect.left + lpwndpos->cx - 2;
			edit->MoveWindow(&editRect);
			mInputSlider.ShowWindow(SW_HIDE);
		}
	}

	__super::OnWindowPosChanging(lpwndpos);
}

//////////////////////////////////////////////////////////////////////////
void CSliderInput::OnHScroll(UINT /*nSBCode*/, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData(TRUE);
	assert(reinterpret_cast<CSliderCtrl*>(pScrollBar) == &mInputSlider); BLADE_UNREFERENCED(pScrollBar);

	int newPos = (int)nPos;// mInputSlider.GetPos();
	if( newPos != mSliderPos )
	{
		mSliderPos = newPos;
		float val = mRangeMin + (float)newPos*(float)mRangeStep;
		mInputString = TStringHelper::fromScalar( val ).c_str();
		UpdateData(FALSE);
		this->ApplyConfig( mInputString.GetString() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSliderInput::OnEnKillfocusSliderEdit()
{
	//get data from dialog
	UpdateData(TRUE);
	if( mEventNotify )
		return;

	if( mConfig->getUIHint() == CUIH_INPUT_RANGE )
	{
		TString str( mInputString.GetString() );
		float val = TStringHelper::toScalar( str );
		if( val < mRangeMin)
		{
			val = mRangeMin;
			mInputString = TStringHelper::fromScalar(val).c_str();
		}
		else if( val > mRangeMax )
		{
			val = mRangeMax;
			mInputString = TStringHelper::fromScalar(val).c_str();
		}
		else
		{
			val = mRangeMin + ((int)((val-mRangeMin)/mRangeStep))*mRangeStep;
			mInputString = TStringHelper::fromScalar(val).c_str();
		}
		
		mSliderPos = (int)((val - mRangeMin)/mRangeStep);
		mInputSlider.SetPos(mSliderPos);
	}
	this->ApplyConfig( mInputString.GetString() );
	mInputString = mConfig->getValue().c_str();
	UpdateData(FALSE);
}


//////////////////////////////////////////////////////////////////////////
void	CSliderInput::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	this->GetClientRect(&rect);
	dc.FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));
}

//////////////////////////////////////////////////////////////////////////
void	CSliderInput::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if( mConfig->getUIHint() == CUIH_INPUT_RANGE )
		mInputSlider.SetFocus();
	else
	{
		CEdit* edit = static_cast<CEdit*>( this->GetDlgItem(IDC_SLIDER_EDIT) );
		edit->SetSel(0, mInputString.GetLength() );
		edit->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
LRESULT	CSliderInput::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	CFont* font = CFont::FromHandle((HFONT)wParam);
	CWnd* edit = this->GetDlgItem(IDC_SLIDER_EDIT);
	if( edit != NULL )
		edit->SetFont(font);
	return this->DefWindowProc(WM_SETFONT, wParam, lParam);
}