/********************************************************************
	created:	2013/01/12
	filename: 	MediaView.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "MediaView.h"

#include <interface/public/ui/UIEvents.h>
#include <interface/IEventManager.h>
#include <ui/IconManager.h>

#undef min
#undef max
namespace Blade
{	
	BEGIN_MESSAGE_MAP(MediaView, DockWindowBase)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_SIZE()
	END_MESSAGE_MAP()

	const TString MediaView::WIDGET_TYPE = BTString("MediaView");

	//////////////////////////////////////////////////////////////////////////
	MediaView::MediaView()
		:DockWindowBase(WIDGET_TYPE)
	{
		mMediaTypeIndex = INVALID_INDEX;
		IEventManager::getSingleton().addEventHandler(MediaViewEvent::NAME, EventDelegate(this, &MediaView::onMediaView));
	}

	//////////////////////////////////////////////////////////////////////////
	MediaView::~MediaView()
	{
		IEventManager::getSingleton().removeEventHandlers(MediaViewEvent::NAME, this);
	}


	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	int MediaView::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int result = __super::OnCreate(lpCreateStruct);

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | SS_BITMAP/*|SS_CENTERIMAGE*/;
		mView.Create(TEXT(""), dwStyle, CRect(0, 0, 0, 0), this);
		mView.ModifyStyleEx(0, WS_EX_TRANSPARENT);

		CRect rect;
		this->GetClientRect(&rect);
		mView.MoveWindow(&rect);
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	void MediaView::OnSize(UINT nType, int cx, int cy)
	{
		__super::OnSize(nType, cx, cy);
		if (mView.GetSafeHwnd() == NULL)
			return;
		CRect rect;
		this->GetClientRect(&rect);
		int size = std::min(rect.Width(), rect.Height());
		mView.MoveWindow(0,0, size, size);
	}

	//////////////////////////////////////////////////////////////////////////
	void MediaView::OnDestroy()
	{
		HBITMAP hOldBitmap = mView.SetBitmap(NULL);
		if (hOldBitmap != NULL)
			::DeleteObject(hOldBitmap);

		__super::OnDestroy();
	}

	/************************************************************************/
	/* internal methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	MediaView::onMediaView(const Event& data)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		if (this->GetSafeHwnd() == NULL || mView.GetSafeHwnd() == NULL)
			return;

		const MediaViewEvent& mv = static_cast<const MediaViewEvent&>(data);
		mMediaTypeIndex = mv.mMediaType;
		mMediaFileName = mv.mMediaFileRef;

		if (this->IsWindowVisible())
			this->updateMediaView();
	}

	void	MediaView::updateMediaView()
	{
		if (mMediaTypeIndex == INVALID_INDEX || mMediaFileName == TString::EMPTY)
			return;

		const IMediaLibrary::MEDIA_TYPE& type = IMediaLibrary::getSingleton().getMediaTypeByIndex(mMediaTypeIndex);
		const IMediaLibrary::MediaFile* mediaFile = IMediaLibrary::getSingleton().getMediaFile(mMediaTypeIndex, mMediaFileName);

		//TODO: add media view control with media control bar for MediaLibraryUI and MediaView
		if (type.mPreviewer == NULL || mediaFile == NULL)
			return;

		HIMAGE hView = type.mPreviewer->generatePreview(mediaFile->mSchemePath, /*size*/ (size_t)-1, IP_TEMPORARY);
		HBITMAP hBitmap = IconManager::getSingleton().createBitmap(hView);
		HBITMAP hOldBitmap = mView.SetBitmap(NULL);
		if (hOldBitmap != NULL)
			::DeleteObject(hOldBitmap);

		mView.SetRedraw(FALSE);
		mView.SetBitmap(hBitmap);

		//resize after set bitmap or bitmap will not scale
		CRect rect;
		this->GetWindowRect(&rect);
		int size = std::min(rect.Width(), rect.Height());
		//size = (int)Math::GetLowerPowerOf2((size_t)size);

		mView.SetRedraw(TRUE);
		mView.MoveWindow(0, 0, (int)size, (int)size);

	}

}//namespace Blade