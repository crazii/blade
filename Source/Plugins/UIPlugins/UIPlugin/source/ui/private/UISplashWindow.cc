/********************************************************************
	created:	2011/09/11
	filename: 	UISplashWindow.cpp
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ui/IconManager.h>
#include <ui//private/UISplashWindow.h>

using namespace Blade;

static const int TEXT_LEFT_PADDING = 5;
static const int TEXT_HEIGHT = 20;

static const int INFO_LEFT = 245;
static const int INFO_TOP = 135;
static const int INFO_WIDTH = 320;
static const int INFO_HEIGHT = 135;

//////////////////////////////////////////////////////////////////////////
CUISplashWindow::CUISplashWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CUISplashWindow::IDD, pParent)
{

}

//////////////////////////////////////////////////////////////////////////
CUISplashWindow::~CUISplashWindow()
{
	HBITMAP hBitmap = mSplashImage.SetBitmap(NULL);
	if (hBitmap != NULL)
		::DeleteObject(hBitmap);
}

/************************************************************************/
/* custom methods                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
void	CUISplashWindow::setInitDetails(const TString& text)
{
	mInitDetail.SetWindowText( text.c_str() );
	CRect rect;
	mInitDetail.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	this->RedrawWindow(&rect);
}

//////////////////////////////////////////////////////////////////////////
void	CUISplashWindow::setStartupInformation(const Blade::TString& text)
{
	mStartupInfo.SetWindowText( text.c_str() );
	CRect rect;
	mInitDetail.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	this->RedrawWindow(&rect);
}

//////////////////////////////////////////////////////////////////////////
void	CUISplashWindow::setSplashImage(const Blade::TString& image)
{
	HSTREAM stream = IResourceManager::getSingleton().loadStream(image);
	HIMAGE img = IImageManager::getSingleton().loadDecompressedImage(stream, IP_TEMPORARY, PF_ARGB, 1);

	size_t width = img->getWidth();
	size_t height = img->getHeight();

	//resize window
	CRect rect;
	this->GetWindowRect(&rect);
	CRect dtRect;
	CWnd::GetDesktopWindow()->GetWindowRect(&dtRect);
	this->MoveWindow((dtRect.Width()- (int)width)/2,(dtRect.Height()- (int)height)/2, (int)width, (int)height,FALSE);

	//resize image control
	mSplashImage.MoveWindow(0,0, (int)width, (int)height,FALSE);
	HBITMAP hBitmap = IconManager::getSingleton().createBitmap(img);
	HBITMAP hOldBitMap = mSplashImage.SetBitmap(hBitmap);
	if( hOldBitMap != NULL )
		::DeleteObject(hOldBitMap);

	//re-position text control
	mInitDetail.MoveWindow(TEXT_LEFT_PADDING, (int)height-TEXT_HEIGHT, (int)width,TEXT_HEIGHT,FALSE);
	//the dimensions are measured from Photoshop..
	mStartupInfo.MoveWindow(INFO_LEFT,INFO_TOP, min(INFO_WIDTH,(int)width-TEXT_LEFT_PADDING*2) ,INFO_HEIGHT,FALSE);
}

/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/
BEGIN_MESSAGE_MAP(CUISplashWindow, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CUISplashWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPLASH_IMAGE, mSplashImage);
	DDX_Control(pDX, IDC_INIT_DETAIL, mInitDetail);
	DDX_Control(pDX, IDC_STARTUP_INFO, mStartupInfo);
}

//////////////////////////////////////////////////////////////////////////
BOOL CUISplashWindow::OnInitDialog()
{
	BOOL result = __super::OnInitDialog();

	if( !result )
		return result;

	mInitDetail.SetWindowText(TEXT("initializing..."));
	return result;
}

//////////////////////////////////////////////////////////////////////////
HBRUSH	CUISplashWindow::OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(RGB(0x0,0x0,0x0));
	pDC->SetBkMode(TRANSPARENT);
	return HBRUSH(::GetStockObject(NULL_BRUSH));
}
