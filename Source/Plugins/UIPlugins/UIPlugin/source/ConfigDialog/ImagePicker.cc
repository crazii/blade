// FilePicker.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/ImagePicker.h>
#include <interface/IPlatformManager.h>
#include <interface/IEventManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/public/ui/IUIMediaLibrary.h>
#include <ui/IconManager.h>

#define TEXT_BUTTON_SPACING (10)

using namespace Blade;

// CImagePicker dialog

//////////////////////////////////////////////////////////////////////////
CImagePicker::CImagePicker(CWnd* /*pParent*/ /*=NULL*/)
:mIniting(false)
{
}

//////////////////////////////////////////////////////////////////////////
CImagePicker::~CImagePicker()
{
}

/************************************************************************/
/* IMediaLibrary::ISelectionReceiver interface                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
void	CImagePicker::setSelectedTarget(index_t mediaIndex, const IMediaLibrary::MediaFile& media)
{
	const HIMAGE& img = media.mPreview;

	HBITMAP hBitmap = IconManager::getSingleton().createBitmap(img);
	mBitmap.Attach(hBitmap);

	mConfig->setValue( media.mSchemePath );

	if( !mIniting )
	{
		MediaViewEvent state( media.mSchemePath, mediaIndex);
		IEventManager::getSingleton().dispatchEvent(state);
	}
}

/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
BOOL		CImagePicker::Create(const RECT& rect,CWnd* pParent,UINT nID,Blade::IConfig* config)
{
	BOOL ret = this->CButton::Create( config != NULL? config->getValue().c_str() : TEXT(""),
		BS_CHECKBOX | BS_OWNERDRAW | WS_TABSTOP | BS_LEFTTEXT,rect, pParent, nID);
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
	
	const TString& type = config->getUIHint().getUserData();
	if( !type.empty() )
	{
		mIniting = true;
		IMediaLibrary::getInterface().getMediaFileInfo(type, config->getValue(), *this);
		mIniting = false;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void	CImagePicker::updateDataImpl(const Blade::TString& Val)
{
	const TString& type = mConfig->getUIHint().getUserData();
	IMediaLibrary::getInterface().getMediaFileInfo(type, Val, *this);
}

BEGIN_MESSAGE_MAP(CImagePicker, CButton)
	ON_WM_SHOWWINDOW()
	ON_CONTROL_REFLECT_EX(BN_CLICKED,OnClicked)
	ON_CONTROL_REFLECT_EX(BN_DBLCLK,OnClicked)

END_MESSAGE_MAP()

// CImagePicker message handlers
//////////////////////////////////////////////////////////////////////////
void	CImagePicker::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if( bShow && mConfig != NULL )
	{
		MediaViewEvent state( mConfig->getValue(), mConfig->getUIHint().getUserData() );
		IEventManager::getSingleton().dispatchEvent(state);
	}
	__super::OnShowWindow(bShow, nStatus);
}

//////////////////////////////////////////////////////////////////////////
BOOL CImagePicker::OnClicked()
{
	if( (mConfig->getAccess()&CAF_WRITE) != 0)
	{
		const TString& type = mConfig->getUIHint().getUserData();
		IUIMediaLibrary::getInterface().showMediaLibrary(type, mConfig->getValue(), *this );
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void	CImagePicker::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	dc->SetBkMode(TRANSPARENT);

	CRect rc(lpDrawItemStruct->rcItem);
	dc->FillSolidRect(&rc, THEMECOLOR(TC_CTLBK));

	TString path = mConfig->getValue();

	COLORREF oldTextColor = dc->SetTextColor(THEMECOLOR(TC_TEXT));
	CFont* oldFont = dc->SelectObject(this->GetFont());
	dc->DrawText(path.c_str(), (int)path.size(), &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	dc->SetTextColor(oldTextColor);
	dc->SelectObject(oldFont);

	rc.left = rc.right - rc.Height();
	dc->DrawState(rc.TopLeft(), rc.Size(), &mBitmap, DST_BITMAP);
}