// ColorPicker.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/ColorPicker.h>
#include <ConfigDialog/ConfigControlUtil.h>
#include <iomanip>

using namespace Blade;

#define GetRValueFloat(dwColor)	( (float)GetRValue(dwColor)/float(255.0f) )
#define GetGValueFloat(dwColor)	( (float)GetGValue(dwColor)/float(255.0f) )
#define GetBValueFloat(dwColor)	( (float)GetBValue(dwColor)/float(255.0f) )


// CColorPicker dialog

//////////////////////////////////////////////////////////////////////////
CColorPicker::CColorPicker(CWnd* pParent /*=NULL*/)
	: CCompositedControl(CColorPicker::IDD, pParent)
	, mColor( RGB(255,255,255) )
	, mFormat(CPF_FLOAT)
{

}

//////////////////////////////////////////////////////////////////////////
CColorPicker::~CColorPicker()
{
}

//////////////////////////////////////////////////////////////////////////
BOOL	CColorPicker::Create(const RECT& rect,CWnd* pParent,UINT nID,IConfig* config)
{
	BOOL ret = __super::Create(IDD,pParent);
	if( ret )
	{
		ASSERT(config != NULL );
		this->GetDlgItem(IDC_COLOR_TEXT)->ModifyStyleEx(WS_EX_CLIENTEDGE,0,SWP_DRAWFRAME);
		this->SetWindowPos(NULL,0,0,rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOMOVE);
		this->SetDlgCtrlID((int)nID);
		this->SetWindowText( config->getValue().c_str() );
		this->Init(config, this);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void	CColorPicker::SetOutputFormat(EColorPackFormat format)
{
	mFormat = format;
}

//////////////////////////////////////////////////////////////////////////
void	CColorPicker::updateDataImpl(const Blade::TString& Val)
{
	Variant var = Color::WHITE;
	var.fromTString(Val);
	Color c = (Color)var;
	c.a = 0.0f;
	mColor = c.getABGR();
	this->UpdateData(FALSE);
}

//////////////////////////////////////////////////////////////////////////
bool	CColorPicker::ValidateColorString()
{
	CWnd* colorText = this->GetDlgItem(IDC_COLOR_TEXT);
	if(colorText == NULL )
		return false;

	CString colorString;
	colorText->GetWindowText(colorString);

	bool result = false;
	TString strColor(colorString.GetString());
	Color color;
	if( strColor[0] == TEXT('(') && strColor[ strColor.size() -1 ] == TEXT(')') )
	{
		//set to config
		const TString floatString = strColor.substr_nocopy(1,strColor.size()-2);
		TStringTokenizer tokenizer;
		tokenizer.tokenize(floatString, TEXT(", ") );
		if( tokenizer.size() == 3 || tokenizer.size() == 4 )
		{
			color.a = 1.0;
			for(size_t i = 0; i < tokenizer.size(); ++i)
			{
				const TString& s = tokenizer[i];
				scalar val = TStringHelper::toScalar(s);
				color[i] = val;
			}
			result = true;
		}
	}

	if(result)
	{
		mColor = RGB( color.r*255, color.g*255, color.b*255 );
		CString text;
		this->GetWindowText(text);
		this->ApplyConfig( text.GetString() );
		this->GetDlgItem(IDC_COLOR_BUTTON)->InvalidateRect(NULL);
	}
	else
	{
		this->SetWindowText( mConfig->getValue().c_str() );
		::AfxMessageBox(TEXT("invalid input."), MB_OK);
	}

	return result;
}


//////////////////////////////////////////////////////////////////////////
void	CColorPicker::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	if( pDX->m_bSaveAndValidate == FALSE )
	{
		CWnd* text = this->GetDlgItem(IDC_COLOR_TEXT);
		if( mFormat == CPF_FLOAT )
		{
			TStringStream stream;
			stream.format(TEXT("(%0.3f,%0.3f,%0.3f,%0.3f)"),GetRValueFloat(mColor),GetGValueFloat(mColor),GetBValueFloat(mColor),1.0f );
			text->SetWindowText( stream.str().c_str() );
		}
		else
		{
			TStringStream stream;
			stream.format(TEXT("(%d,%d,%d)"),GetRValue(mColor),GetGValue(mColor),GetBValue(mColor) );
			text->SetWindowText( stream.str().c_str() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
BOOL	CColorPicker::PreTranslateMessage(MSG* pMsg)
{
	if( mConfig != NULL && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus() == GetDlgItem(IDC_COLOR_TEXT) )
	{
		if( this->ValidateColorString() )
		{
			UpdateData(FALSE);
		}
	}
	return __super::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
LRESULT	CColorPicker::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if( message ==  WM_GETTEXTLENGTH)
	{
		CWnd* text = this->GetDlgItem(IDC_COLOR_TEXT);
		return text->GetWindowTextLength();
	}
	else if( message == WM_GETTEXT )
	{
		CString windowtext;
		CWnd* text = this->GetDlgItem(IDC_COLOR_TEXT);
		text->GetWindowText(windowtext);

		WPARAM cchTextMax = wParam;
		LPTSTR lpszText = (LPTSTR)lParam;
		UINT minchars = min((UINT)windowtext.GetLength()+1, (UINT)cchTextMax);
		if(lpszText != NULL)
		{
			UpdateData(TRUE);
			Char_Traits<TCHAR>::copy(lpszText,windowtext.GetString(),minchars);
			//if string is truncated,assure NULL char appended
			lpszText[minchars-1] = TEXT('\0');
			return (LRESULT)minchars;
		}
		return 0;
	}
	else if( message == WM_SETTEXT )
	{
		LPCTSTR lpsz = (LPCTSTR)lParam;

		if( lpsz != NULL )
		{
			TStringStream stream;
			stream.str( TString(lpsz) );
			if( mFormat == CPF_FLOAT )
			{
				Variant var = Color::WHITE;
				TString strVal(lpsz);
				var.fromTString(strVal);
				mColor = ((const Color&)var).getRGBA();
				mColor &= 0x00FFFFFF; //mask the alpha bits
			}
			else
			{
				LONG r = 255,g = 255,b = 255;
				stream >> r >> g >> b;
				mColor = RGB( r,g,b );
			}

			UpdateData(FALSE);
			return TRUE;
		}
		else
			return FALSE;
	}
	return __super::DefWindowProc(message, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////
void	CColorPicker::OnOK()
{

}

//////////////////////////////////////////////////////////////////////////
void	CColorPicker::OnCancel()
{

}

BEGIN_MESSAGE_MAP(CColorPicker, CCompositedControl)
	ON_BN_CLICKED(IDC_COLOR_BUTTON, &CColorPicker::OnBnClickedColorButton)
	ON_EN_KILLFOCUS(IDC_COLOR_TEXT, &CColorPicker::OnEnKillfocusColorPicker)
#if ENABLE_THEME
	ON_WM_PAINT()
#endif
	ON_WM_DRAWITEM()
	ON_WM_WINDOWPOSCHANGING()
	ON_MESSAGE(WM_SETFONT, &CColorPicker::OnSetFont)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// CColorPicker message handlers


//////////////////////////////////////////////////////////////////////////
void	CColorPicker::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	const int cx = lpwndpos->cx;
	const int cy = lpwndpos->cy;

	CRect rectButton;
	CRect rectText;
	CWnd* button = this->GetDlgItem(IDC_COLOR_BUTTON);
	CWnd* text = this->GetDlgItem(IDC_COLOR_TEXT);

	if( button != NULL && text != NULL && !(lpwndpos->flags & SWP_NOSIZE ))
	{
		rectButton.left = cx-cy;
		rectButton.right = cx;

		rectButton.top = 0;
		rectButton.bottom = cy;

		rectButton.top = (cy - rectButton.Height())/2;
		rectButton.bottom = rectButton.top + rectButton.Height();

		rectText.right = cx - cy;
		rectText.left = 0;

		rectText.top = 0;
		rectText.bottom = cy;

		button->MoveWindow(&rectButton);
		text->MoveWindow( &rectText );
	}

	__super::OnWindowPosChanging(lpwndpos);
}

//////////////////////////////////////////////////////////////////////////
void CColorPicker::OnBnClickedColorButton()
{
	CColorDialog colorDlg(mColor,CC_FULLOPEN,this);

	if( colorDlg.DoModal() == IDOK )
	{
		mColor = colorDlg.GetColor();
		this->RedrawWindow();
		UpdateData(FALSE);
		CString text;
		this->GetWindowText(text);
		this->ApplyConfig( text.GetString() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CColorPicker::OnPaint()
{
#if ENABLE_THEME
	CPaintDC dc(this); // device context for painting
	CRect rect;
	this->GetClientRect(&rect);
	dc.FillSolidRect(&rect, THEMECOLOR(TC_CTLBK));
#endif
}

//////////////////////////////////////////////////////////////////////////
void CColorPicker::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	__super::OnDrawItem(nIDCtl, lpDrawItemStruct);
	if( nIDCtl == IDC_COLOR_BUTTON )
	{
		CClientDC	dc(this);
		CRect rect;
		CWnd* preview = this->GetDlgItem(IDC_COLOR_BUTTON);
		preview->GetWindowRect(&rect);
		this->ScreenToClient(&rect);
		dc.Rectangle(&rect);

		rect.left +=1;
		rect.top += 1;
		rect.right -= 1;
		rect.bottom -= 1;
		dc.FillSolidRect(rect,mColor);
	}
}

//////////////////////////////////////////////////////////////////////////
void CColorPicker::OnEnKillfocusColorPicker()
{
	if( this->ValidateColorString() )
		UpdateData(FALSE);
}

//////////////////////////////////////////////////////////////////////////
LRESULT	CColorPicker::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	CFont* font = CFont::FromHandle((HFONT)wParam);
	CWnd* edit = this->GetDlgItem(IDC_COLOR_TEXT);
	if( edit != NULL )
		edit->SetFont(font);
	return this->DefWindowProc(WM_SETFONT, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
void CColorPicker::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);
	CEdit* edit = static_cast<CEdit*>( this->GetDlgItem(IDC_COLOR_TEXT) );
	if( edit != NULL )
	{
		CString s;
		edit->GetWindowText(s);
		edit->SetSel(0, s.GetLength());
		edit->SetFocus();
	}
}