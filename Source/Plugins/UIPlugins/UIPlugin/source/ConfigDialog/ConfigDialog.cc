// ConfigDialog.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/ConfigDialog.h>
#include <ConfigDialog/ConfigControlUtil.h>
#include <interface/IConfigManager.h>
#include <interface/IEnvironmentManager.h>
#include <ConstDef.h>
#include <interface/public/IStartupOutput.h>
#include <ui/IconManager.h>

using namespace Blade;
#undef min
#undef max
#define MAX_HEIGHTRATE (0.8f)

static const int PADDING = 2;
static const int HSPACING = 10;
static const int VSPACING = 10;
static const int TAB_HEIGHT = 22;

IMPLEMENT_DYNAMIC(CConfigDialog, CUIDialog)

//////////////////////////////////////////////////////////////////////////
CConfigDialog::CConfigDialog(CWnd* pParent /*=NULL*/)
	:CUIDialog(CConfigDialog::IDD, pParent)
	,mTabSelection(0)
	,mHideStartUpCheck(false)
	,mHideBanner(false)
{
	mShowOnStartUp = TStringHelper::toBool( IConfigManager::getSingleton().getEntry(BTString("ShowDialog"),ConifgDialogSection ) );
}

//////////////////////////////////////////////////////////////////////////
CConfigDialog::~CConfigDialog()
{

}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OPTION_SLELECTION_TAB, mOptionTab);
	DDX_Check(pDX, IDC_CHECK_SHOW_ON_STARTUP, mShowOnStartUp);
}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::EndDialog( int nResult )
{
	BLADE_UNREFERENCED(nResult);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
bool	CConfigDialog::show(const INFO& info, const HCONFIG* configs, size_t count,
	const TString& caption/* = BTString("Blade Config") */)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	this->setCaption(caption);

	mConfigList.clear();

	for(size_t i = 0; i < count; ++i)
		this->addConfig(configs[i]);

	return this->show(info, false);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
bool CConfigDialog::show(const INFO& info, bool hideAutoCheck, bool hideBanner/* = false*/, CWnd* parent/* = NULL*/)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	mInfo = info;
	mHideStartUpCheck = hideAutoCheck;
	mHideBanner = hideBanner;

	if( mInfo.mModal )
		return this->DoModal() == IDOK;
	else
	{
		this->Create(IDD, parent );
		this->ShowWindow(SW_SHOW);
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CConfigDialog::addConfig(IConfig* configItem)
{
	AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	assert(configItem != NULL);

	bool devMode = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvBool::DEVELOPER_MODE);

	CONFIG_ACCESS access = configItem->getAccess();
	if ((access&CAF_READWRITE) == 0 || (!devMode && (access&CAF_DEVMODE)))
		return true;

	for (ConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
	{
		if ((*i) == configItem || (*i)->getName() == configItem->getName())
			return false;
	}

	mConfigList.push_back(configItem);

	if (this->GetSafeHwnd())
		mOptionTab.InsertItem(mOptionTab.GetItemCount(), configItem->getName().c_str());
	return true;
}

//////////////////////////////////////////////////////////////////////////
void	CConfigDialog::setCaption(const Blade::TString& caption)
{
	if( caption != TString::EMPTY )
		mCaption = BTString2Lang( caption );
}

//////////////////////////////////////////////////////////////////////////
void	CConfigDialog::ResetList(int tabIndex)
{
	mOptionList.SetRedraw(FALSE);
	mOptionList.DeleteAllItems();
	IConfig* cfg;
	if (mTabSelection < mOptionTab.GetItemCount() && (index_t)tabIndex < mConfigList.size() )
	{
		ConfigList::iterator i = mConfigList.begin();
		std::advance(i, (indexdiff_t)tabIndex);
		cfg = *i;
	}
	else
		BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("tab index of range.") );

	mOptionList.setSubConfigs(cfg);
	mOptionList.Invalidate(FALSE);
	mOptionList.SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
BOOL CConfigDialog::OnInitDialog()
{
	if( !__super::OnInitDialog() )
		return FALSE;

	mOptionList.setAllVisible( mInfo.mForceShowAll );
	mOptionTab.EnableCustomLook(TRUE, ETC_COLOR|ETC_FLAT);
	mOptionTab.EnableDraw();

	CRect rcClient;
	this->GetClientRect(rcClient);
	CRect rcWindow;
	this->GetWindowRect(&rcWindow);
	mOptionList.Create(this, rcClient, IDC_OPTION_LIST);
	int NCWidth = rcWindow.Width() - rcClient.Width();
	mNCHeight = rcWindow.Height() - rcClient.Height();

	if( mHideStartUpCheck )
	{
		mOptionList.setDefaultExpand(true);
		this->GetDlgItem(IDC_CHECK_SHOW_ON_STARTUP)->ShowWindow(SW_HIDE);
	}

	int bannerHeight;
	if( mHideBanner || Factory<IImageManager>::getSingleton().getNumRegisteredClasses() == 0 )
	{
		mHideBanner = true;
		bannerHeight = 0;
		CWnd* img = this->GetDlgItem(IDC_IMG_HEADER);
		CRect imgRect;
		img->GetWindowRect(&imgRect);
		this->ScreenToClient(&imgRect);
		CRect tabRect;
		mOptionTab.GetWindowRect(&tabRect);
		mOptionTab.MoveWindow(imgRect.left,imgRect.top,tabRect.Width(),tabRect.Height());
		img->ShowWindow(SW_HIDE);

		if( mConfigList.size() == 1 )
			mOptionTab.ShowWindow(SW_HIDE);

		this->SetWindowPos(NULL,0,0,rcWindow.Width(),rcWindow.Height()-imgRect.Height(),SWP_NOMOVE|SWP_NOZORDER);

		int ID[]={IDOK,IDCANCEL};
		for( size_t i = 0; i < 2; ++i)
		{
			CRect buttonRect;
			CWnd* button = this->GetDlgItem(ID[i]);
			button->GetWindowRect(buttonRect);
			this->ScreenToClient(&buttonRect);
			button->SetWindowPos(NULL,buttonRect.left,buttonRect.top-imgRect.Height(),0,0,SWP_NOZORDER|SWP_NOSIZE);
		}
	}
	else
	{
		CStatic* header = (CStatic*)this->GetDlgItem(IDC_IMG_HEADER);
		//TODO: read & write pic config using common config file through config manager
		const TString& image = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvString::STARTUP_IMAGE);
		HSTREAM stream = IResourceManager::getSingleton().loadStream(image);

		if( stream != NULL )
		{
			HIMAGE img = IImageManager::getSingleton().loadDecompressedImage(stream, IP_TEMPORARY, PF_ARGB, 1);
			CRect tabRect;
			mOptionTab.GetWindowRect(&tabRect);
			this->ScreenToClient(&tabRect);
			tabRect.bottom = tabRect.top-2;
			tabRect.top = 0;

			SIZE2I size(rcClient.Width(), tabRect.Height());
			img = IImageManager::getSingleton().scaleImage(img, size, IP_TEMPORARY);
			HBITMAP hBitmap = IconManager::getSingleton().createBitmap(img);

			HBITMAP hOldBitMap = header->SetBitmap(hBitmap);
			if( hOldBitMap != NULL )
				::DeleteObject(hOldBitMap);
		}

		CRect rcImg;
		header->GetWindowRect(&rcImg);
		bannerHeight = rcImg.Height() + VSPACING;
	}

	if( mCaption != TString::EMPTY )
		this->SetWindowText(mCaption.c_str());

	if( mConfigList.size() == 0 )
		return TRUE;

	mOptionList.setDefaultExpand(true);

	//add tabs
	int tabIndex = 0;
	for (ConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
	{
		(*i)->updateData(true, true);
		mOptionTab.InsertItem(tabIndex++, BTString2Lang((*i)->getName()).c_str());
		(*i)->backupValue();
	}

	//find the max list height by try all tabs
	mOptionList.ShowWindow(SW_HIDE);
	size_t listHeight = 0;
	size_t listWidth = 0;
	for(size_t i = 0; i < mConfigList.size(); ++i)
	{
		ResetList((int)i);
		listHeight = std::max(listHeight, mOptionList.getIdealHeight());
		listWidth = std::max(listWidth, mOptionList.getIdealWidth());
	}
	ResetList(0);
	mOptionList.ShowWindow(SW_SHOW);

	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int maxHeight = int(MAX_HEIGHTRATE * (float)screenHeight);
	int maxWidth = int(MAX_HEIGHTRATE * (float)screenWidth);

	int height = std::min(maxHeight, (int)listHeight);
	int width = std::min(maxWidth, (int)listWidth);
	height += PADDING*2 + bannerHeight;
	width += PADDING*2;
	width += NCWidth;

	if( mOptionTab.GetItemCount() > 0 )
		height += TAB_HEIGHT;
	CRect rcButton;
	CWnd* buttonOK = this->GetDlgItem(IDOK);
	buttonOK->GetWindowRect(&rcButton);
	height += rcButton.Height() + VSPACING;
	height += mNCHeight;

	const float offset = 1.0f-0.618f;
	width = mHideBanner ? std::max(width,rcWindow.Width()) : rcWindow.Width();
	this->SetWindowPos(NULL, int(0.5*(screenWidth-width)), int(offset*(screenHeight-height)), width, height, SWP_NOZORDER);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::PostNcDestroy()
{
	mConfigList.clear();
	__super::PostNcDestroy();
	if( !mInfo.mModal )
		BLADE_DELETE this;
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CConfigDialog, CUIDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_OPTION_SLELECTION_TAB, &CConfigDialog::OnTcnSelchangeOptionSeletionTab)
	ON_BN_CLICKED(IDCANCEL, &CConfigDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ON_STARTUP, &CConfigDialog::OnBnClickedCheckShowOnStartup)
	ON_BN_CLICKED(IDOK, &CConfigDialog::OnBnClickedOk)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// CConfigDialog message handlers
//////////////////////////////////////////////////////////////////////////
void CConfigDialog::OnTcnSelchangeOptionSeletionTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	BLADE_UNREFERENCED(pNMHDR);

	// TODO: Add your control notification handler code here
	*pResult = 0;

	int n = mOptionTab.GetCurSel();
	if( n != mTabSelection )
	{
		mTabSelection = n;
		ResetList(n);
	}
}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::OnBnClickedCancel()
{
	CStatic* header = (CStatic*)this->GetDlgItem(IDC_IMG_HEADER);
	HBITMAP hBitmap = header->SetBitmap(NULL);
	if(hBitmap!= NULL)
		::DeleteObject(hBitmap);

	for (ConfigList::iterator i = mConfigList.begin(); i != mConfigList.end(); ++i)
		(*i)->restoreToBackup();

	if( mOptionList.GetSafeHwnd() )
		mOptionList.DeleteAllItems();
	__super::OnCancel();

	if( !mInfo.mModal )
		this->DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::OnBnClickedOk()
{
	
	CStatic* header = (CStatic*)this->GetDlgItem(IDC_IMG_HEADER);
	HBITMAP hBitmap = header->SetBitmap(NULL);
	if (hBitmap != NULL)
		::DeleteObject(hBitmap);

	if( mOptionList.GetSafeHwnd() )
		mOptionList.DeleteAllItems();
	__super::OnOK();

	if( !mInfo.mModal )
	{
		mInfo.mNoneModalDelegate.call( mInfo.mUserData );
		this->DestroyWindow();
	}
}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::OnBnClickedCheckShowOnStartup()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	TString ShowStringBOOL = TStringHelper::fromBool(mShowOnStartUp == TRUE);
	IConfigManager::getSingleton().setEntry(BTString("ShowDialog"),ShowStringBOOL,ConifgDialogSection );
}

//////////////////////////////////////////////////////////////////////////
void CConfigDialog::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	int bannerHeight = 0;
	if( !mHideBanner )
	{
		CRect rect;
		this->GetDlgItem(IDC_IMG_HEADER)->GetWindowRect(&rect);
		bannerHeight += rect.Height();

		const int BUTTON_HEIGHT = 20;
		if( lpwndpos->cy < bannerHeight+mNCHeight + TAB_HEIGHT + VSPACING*2 + BUTTON_HEIGHT)
			lpwndpos->cy = bannerHeight+mNCHeight + TAB_HEIGHT + VSPACING*2 + BUTTON_HEIGHT;

		lpwndpos->cx = rect.Width();
	}

	__super::OnWindowPosChanging(lpwndpos);

	CRect rcClient;
	this->GetClientRect(&rcClient);
	CRect rcTab;
	mOptionTab.GetWindowRect(&rcTab);
	this->ScreenToClient(&rcTab);

	CWnd* buttonOK = this->GetDlgItem(IDOK);
	CWnd* buttonCancel = this->GetDlgItem(IDCANCEL);
	CWnd* checkStartup = this->GetDlgItem(IDC_CHECK_SHOW_ON_STARTUP);

	CWnd* moveWnd[] = {buttonCancel, buttonOK, checkStartup};
	int left = rcClient.right - PADDING;
	int height = 0;
	const size_t count = sizeof(moveWnd)/sizeof(CWnd*);
	for(size_t i = 0; i < count; ++i)
	{
		CRect rect;
		moveWnd[i]->GetWindowRect(&rect);
		this->ClientToScreen(&rect);
		if( i == 2 )	//left align
			left = rcTab.left;
		else			//right align
			left -= rect.Width();
		height = std::max(height, rect.Height());
		moveWnd[i]->SetWindowPos(NULL, left, rcClient.Height() - rect.Height() - PADDING, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_DEFERERASE);
		left -= HSPACING;
	}

	height += bannerHeight;
	mOptionTab.SetWindowPos(&CWnd::wndBottom, 0, 0, rcClient.Width() - rcTab.left*2, rcClient.Height() - height - VSPACING, SWP_NOMOVE|SWP_DEFERERASE|SWP_NOACTIVATE);

	if( mOptionTab.GetItemCount() > 1)
	{
		CRect rect;
		mOptionTab.GetWindowRect(&rect);
		this->ScreenToClient(&rect);
		rect.top += TAB_HEIGHT;
		rect.left += PADDING;
		rect.bottom -= PADDING;
		rect.right -= PADDING;
		mOptionList.MoveWindow(&rect);
	}
	else
	{
		rcClient.DeflateRect(PADDING, PADDING, PADDING, height + VSPACING);
		mOptionList.MoveWindow(&rcClient);
	}
}