// FilePicker.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/FilePicker.h>
#include <interface/IPlatformManager.h>

#define TEXT_BUTTON_SPACING (10)

using namespace Blade;

// CFilePicker dialog

//////////////////////////////////////////////////////////////////////////
CFilePicker::CFilePicker(CWnd* pParent /*=NULL*/)
	: CCompositedControl(CFilePicker::IDD, pParent)
	, mPathString(_T(""))
	,mFileExtension( BTString("*") )
{

}

//////////////////////////////////////////////////////////////////////////
CFilePicker::~CFilePicker()
{
}

//////////////////////////////////////////////////////////////////////////
BOOL		CFilePicker::Create(const RECT& rect,CWnd* pParent,UINT nID, Blade::IConfig* config)
{
	ASSERT(config != NULL );
	this->Init(config, this);

	BOOL ret = __super::Create(CFilePicker::IDD, pParent);
	if( ret )
	{
		this->SetDlgCtrlID((int)nID);
		this->MoveWindow(&rect);
		this->SetWindowText( config->getValue().c_str() );

		if( config->getUIHint() == CUIH_FILE )
		{
			TString dir,filename,base,ext;
			TStringHelper::getFilePathInfo( config->getValue(),dir,filename,base,ext);
			this->SetFileExtension( ext );
		}
		else if( config->getUIHint() == CUIH_CUSTOM )
		{
			this->GetDlgItem(IDC_FILE_PATH)->EnableWindow(FALSE);
		}

	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void		CFilePicker::SetFileExtension(const TString& fileExt)
{
	if( !fileExt.empty() )
		mFileExtension = fileExt;
	else
		mFileExtension = BTString("*");
}

//////////////////////////////////////////////////////////////////////////
void		CFilePicker::SetTextWidth(int cx)
{
	CRect rect;
	this->GetWindowRect(&rect);
	CWnd* button = this->GetDlgItem(IDC_BROWSE_BUTTON);

	CRect rectButton;
	button->GetWindowRect(&rectButton);
	int dy = rect.Height();
	int dx = cx + rectButton.Width() + TEXT_BUTTON_SPACING;
	this->SetWindowPos(&wndTop,0,0,dx,dy,SWP_NOMOVE|SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////////
void	CFilePicker::updateDataImpl(const Blade::TString& Val)
{
	mPathString = Val.c_str();
	this->UpdateData(FALSE);
}

//////////////////////////////////////////////////////////////////////////
BOOL	CFilePicker::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if( ret )
	{
		if( this->GetConfig()->getUIHint() == CUIH_CUSTOM )
			this->GetDlgItem(IDC_BROWSE_BUTTON)->SetWindowText( BTLang(BLANG_CONFIG).c_str() );
		else
			this->GetDlgItem(IDC_BROWSE_BUTTON)->SetWindowText( BTLang(BLANG_BROWSE).c_str() );
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void	CFilePicker::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILE_PATH, mPathString);
}

LRESULT CFilePicker::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if( message == WM_GETTEXTLENGTH)
		return mPathString.GetLength();
	else if( message == WM_GETTEXT )
	{
		WPARAM cchTextMax = wParam;
		LPTSTR lpszText = (LPTSTR)lParam;
		UINT minchars = min((UINT)mPathString.GetLength()+1, (UINT)cchTextMax);
		if(lpszText != NULL)
		{
			UpdateData(TRUE);
			Char_Traits<TCHAR>::copy(lpszText,mPathString.GetString(),minchars);
			return (LRESULT)minchars;
		}
		return 0;
	}
	else if( message == WM_SETTEXT )
	{
		LPCTSTR lpsz = (LPCTSTR)lParam;

		if( lpsz != NULL )
		{
			mPathString = lpsz;
			UpdateData(FALSE);
			return TRUE;
		}
		else
			return FALSE;
	}
	return __super::DefWindowProc(message, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
void	CFilePicker::OnOK()
{

}

//////////////////////////////////////////////////////////////////////////
void	CFilePicker::OnCancel()
{

}

//////////////////////////////////////////////////////////////////////////
BOOL	CFilePicker::PreTranslateMessage(MSG* pMsg)
{
	if( mConfig != NULL && pMsg->message == WM_KEYDOWN && GetFocus() == GetDlgItem(IDC_FILE_PATH) )
	{
		UpdateData(TRUE);
		this->ApplyConfig( mPathString.GetString() );
	}
	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CFilePicker, CCompositedControl)
	ON_WM_WINDOWPOSCHANGING()
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CFilePicker::OnBnClickedBrowseButton)
	ON_EN_KILLFOCUS(IDC_FILE_PATH, &CFilePicker::OnEnKillfocusFilePath)
	ON_MESSAGE(WM_SETFONT, &CFilePicker::OnSetFont)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// CFilePicker message handlers

//////////////////////////////////////////////////////////////////////////
void CFilePicker::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	const int cx = lpwndpos->cx;
	const int cy = lpwndpos->cy;

	if (!(lpwndpos->flags&SWP_NOSIZE))
	{
		CRect rectButton;
		CRect rectText;
		CWnd* button = this->GetDlgItem(IDC_BROWSE_BUTTON);
		CWnd* text = this->GetDlgItem(IDC_FILE_PATH);
		button->GetWindowRect(&rectButton);
		text->GetWindowRect(&rectText);
		text->MoveWindow(0, 0, cx - rectButton.Width() - TEXT_BUTTON_SPACING, cy);
		button->MoveWindow(cx - rectButton.Width(), 0, rectButton.Width(), cy);
	}

	__super::OnWindowPosChanging(lpwndpos);
}

void CFilePicker::OnBnClickedBrowseButton()
{
	// TODO: Add your control notification handler code here
	if( mConfig->getUIHint() == CUIH_FILE )
	{
		TString filetype = mFileExtension;


		if( filetype == BTString("*") )
			filetype = BTString("All");

		TString filter = filetype + TEXT(" File (*.") + mFileExtension + TEXT(")|") +
							+ TEXT("*.") + mFileExtension + TEXT("||");

		CFileDialog fileOpen(TRUE,NULL,NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR,filter.c_str(),this);
		tchar filename[MAX_PATH+1] = {0};
		tchar initDir[MAX_PATH+1] = {0};

		{
			TString initialDir;
			const TString& cwd = IPlatformManager::getSingleton().getCurrentWorkingPath();
			const TString& initVal = this->GetConfig()->getValue();
			HFILEDEVICE hFolder = IResourceManager::getSingleton().loadFolder(initVal);
			if(hFolder != NULL)
				initialDir = TStringHelper::getAbsolutePath(cwd, initVal);
			else
			{
				TString dir, file;
				TStringHelper::getFilePathInfo(this->GetConfig()->getValue(), dir, file);
				initialDir = TStringHelper::getAbsolutePath(cwd, dir);
				::memcpy(filename, file.c_str(), std::min<size_t>(file.size()*sizeof(tchar), MAX_PATH*sizeof(tchar)));
			}
			TStringHelper::replace(initialDir, BTString("/"), BTString("\\"));
			::memcpy(initDir, initialDir.c_str(), std::min<size_t>(initialDir.size()*sizeof(tchar), MAX_PATH*sizeof(tchar)));
		}

		fileOpen.GetOFN().nMaxFile = MAX_PATH;
		fileOpen.GetOFN().lpstrFile = filename;
		fileOpen.GetOFN().lpstrInitialDir = initDir;

		const INT_PTR ret= fileOpen.DoModal();
		if( ret == IDOK )
		{
			TString path = TString(filename);
			TStringHelper::toUpperCase(path);
			TString curPath = TStringHelper::getUpperCase( IPlatformManager::getSingleton().getCurrentWorkingPath() );
			path = TStringHelper::getRelativePath( curPath,path );
			mPathString = path.c_str();

			UpdateData(FALSE);
			this->ApplyConfig( mPathString.GetString() );
		}
		else
		{
			//BLADE_EXCEPT(EXC_API_ERROR,BTString("open file failed.") );
		}
	}
	else if( mConfig->getUIHint() == CUIH_PATH )
	{
		BROWSEINFO bi;
		tchar dispName[MAX_PATH+1];
		ZeroMemory(&bi,sizeof(BROWSEINFO));
		bi.hwndOwner = this->GetSafeHwnd();
		bi.pszDisplayName = dispName;
		bi.lpszTitle = TEXT("File Location");
		bi.ulFlags = BIF_RETURNFSANCESTORS;
		LPITEMIDLIST idl = ::SHBrowseForFolder(&bi);
		if(idl == NULL)
		{
			//BLADE_EXCEPT(EXC_API_ERROR,BTString("select directory failed.") );
			return;
		}

		//::SHGetPathFromIDList(idl, mPathString.GetBuffer(MAX_PATH+1));
		//mPathString.ReleaseBuffer();

		::SHGetPathFromIDList(idl, dispName );
		TString path = TString(dispName);
		TStringHelper::toUpperCase(path);
		TString curPath = TStringHelper::getUpperCase( IPlatformManager::getSingleton().getCurrentWorkingPath() );

		path = TStringHelper::getRelativePath(curPath,path);
		mPathString = path.c_str();
		UpdateData(FALSE);
		this->ApplyConfig( mPathString.GetString() );
	}
	else if(mConfig->getUIHint() == CUIH_CUSTOM )
	{
		mConfig->getUIHint().mCustomCall.call();
	}
	else
		assert(false);
}

//////////////////////////////////////////////////////////////////////////
void CFilePicker::OnEnKillfocusFilePath()
{
	UpdateData(TRUE);
	this->ApplyConfig( mPathString.GetString() );
}

//////////////////////////////////////////////////////////////////////////
LRESULT	CFilePicker::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	CFont* font = CFont::FromHandle((HFONT)wParam);
	CWnd* edit = this->GetDlgItem(IDC_FILE_PATH);
	if( edit != NULL )
		edit->SetFont(font);
	return this->DefWindowProc(WM_SETFONT, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
void CFilePicker::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);
	CEdit* edit = static_cast<CEdit*>( this->GetDlgItem(IDC_FILE_PATH) );
	if( edit != NULL )
	{
		edit->SetSel(0, mPathString.GetLength());
		edit->SetFocus();
	}
}