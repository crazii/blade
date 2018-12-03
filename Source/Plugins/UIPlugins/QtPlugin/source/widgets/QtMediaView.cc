/********************************************************************
	created:	2016/7/26
	filename: 	QtMediaView.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <widgets/QtMediaView.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IEventManager.h>


namespace Blade
{
	const TString QtMediaView::WIDGET_TYPE = BTString(BLANG_MEDIAVIEW);

	//////////////////////////////////////////////////////////////////////////
	QtMediaView::QtMediaView()
		:QtWidgetBase<IUIWidgetLeaf>(WIDGET_TYPE)
	{
		this->setWidget(&mContent);

		IEventManager::getSingleton().addEventHandler(MediaViewEvent::NAME, EventDelegate(this, &QtMediaView::onMediaView));
		mContent.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		mContent.setScaledContents(false);
		mAspect = Qt::KeepAspectRatio;
	}

	//////////////////////////////////////////////////////////////////////////
	QtMediaView::~QtMediaView()
	{
		IEventManager::getSingleton().removeEventHandlers(MediaViewEvent::NAME, this);
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtMediaView::onMediaView(const Event& data)
	{
		const MediaViewEvent& mv = static_cast<const MediaViewEvent&>(data);
		if(!mImage.isNull() && mMediaTypeIndex == mv.mMediaType && mMediaFileName == mv.mMediaFileRef)
			return;
		mMediaTypeIndex = mv.mMediaType;
		mMediaFileName = mv.mMediaFileRef;

		if (this->QDockWidget::isVisible())
			this->updateMediaView();
	}

	void	QtMediaView::updateMediaView()
	{
		mImage = QPixmap();
		mContent.setPixmap(mImage);

		if (mMediaTypeIndex == INVALID_INDEX || mMediaFileName == TString::EMPTY)
			return;

		const IMediaLibrary::MEDIA_TYPE& type = IMediaLibrary::getSingleton().getMediaTypeByIndex(mMediaTypeIndex);
		const IMediaLibrary::MediaFile* mediaFile = IMediaLibrary::getSingleton().getMediaFile(mMediaTypeIndex, mMediaFileName);

		if (type.mPreviewer == NULL || mediaFile == NULL)
			return;

		HIMAGE hView = mediaFile->mPreview;
		//HIMAGE hView = type.mPreviewer->generatePreview(mediaFile->mSchemePath, -1, IP_TEMPORARY);
		Handle<QImage> qimg = toQImage(hView);

		if( qimg != NULL )
		{
			bool ret = mImage.convertFromImage(*qimg);
			if(ret)
			{
				int w = mContent.size().width();
				int h = mContent.size().height();
				h = std::min(w,h) * ((fp64)mImage.height() / (fp64)mImage.width());
				w = std::min(w,h) * ((fp64)mImage.width() / (fp64)mImage.height());
				mContent.setPixmap(mImage.scaled(w, h, mAspect));
			}
		}
	}
	
}//namespace Blade