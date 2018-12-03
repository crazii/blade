/********************************************************************
	created:	2016/7/26
	filename: 	QtTrackView.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <widgets/QtTrackView.h>

namespace Blade
{
	const TString QtTrackView::WIDGET_TYPE = BTString(BLANG_TRACKVIEW);
	static const int LABEL_WIDTH = 80;

	//////////////////////////////////////////////////////////////////////////
	QtTrackView::QtTrackView()
		:QtWidgetBase<IUITrackView>(WIDGET_TYPE)
	{
		this->setWidget(&mContent);

		struct
		{
			TString icon;
			TString tip;
		}buttonList[] =
		{
			BTString("start.png"),		BTLang(BLANG_TRACK_BEGIN),
			BTString("previous.png"),	BTLang(BLANG_TRACK_PREV),
			BTString("play.png"),		BTLang(BLANG_TRACK_PLAY),
			BTString("next.png"),		BTLang(BLANG_TRACK_NEXT),
			BTString("end.png"),		BTLang(BLANG_TRACK_END),
		};

		for (int i = TB_BEGIN; i < TB_COUNT; ++i)
		{
			mButtons[i].setParent(&mContent);
			mButtons[i].setToolTip(TString2QString(buttonList[i].tip));

			IconIndex icon = IIconManager::getSingleton().loadSingleIconImage(buttonList[i].icon);
			QIcon* qicon = QtIconManager::getSingleton().getIcon(icon);
			if (qicon != NULL)
				mButtons[i].setIcon(*qicon);

			mButtons[i].setEnabled(false);
			if (i == TB_PLAYPAUSE)
			{
				mButtons[i].setCheckable(true);
				mPlayIcon = qicon;
				IconIndex icon = IIconManager::getSingleton().loadSingleIconImage(BTString("pause.png"));
				qicon = QtIconManager::getSingleton().getIcon(icon);
				mPauseIcon = qicon;
			}
		}

		mSlider.setParent(&mContent);
		mSlider.setEnabled(false);
		mSlider.setOrientation(Qt::Horizontal);

		mLabel.setParent(&mContent);
		mLabel.setEnabled(false);
		mLabel.setAlignment(Qt::AlignCenter);
		mLabel.setToolTip(TString2QString(BTLang("double click here to set custom palying range")));

		IFramework::getSingleton().addUpdater(this);

		QObject::connect(&mButtons[TB_BEGIN], &QPushButton::clicked, this, &QtTrackView::onBegin);
		QObject::connect(&mButtons[TB_PREV], &QPushButton::clicked, this, &QtTrackView::onPrev);
		QObject::connect(&mButtons[TB_PLAYPAUSE], &QPushButton::clicked, this, &QtTrackView::onPlayPause);
		QObject::connect(&mButtons[TB_NEXT], &QPushButton::clicked, this, &QtTrackView::onNext);
		QObject::connect(&mButtons[TB_END], &QPushButton::clicked, this, &QtTrackView::onEnd);
		QObject::connect(&mSlider, &QSlider::valueChanged, this, &QtTrackView::onSlider);

		//TODO move to slider to second line if width too small?
		mContent.setMinimumHeight( mButtons[0].sizeHint().height()/* + mSlider.sizeHint().height()*/);
		mContent.setMinimumWidth(mButtons[0].sizeHint().width()*TB_COUNT + LABEL_WIDTH);
	}

	//////////////////////////////////////////////////////////////////////////
	QtTrackView::~QtTrackView()
	{
		IFramework::getSingleton().removeUpdater(this);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTrackView::setTrack(ITrack* track)
	{
		if (mTrackData.getTrack() != track)
		{
			mTrackData.setTrack(track);
			if (track != NULL)
			{
				Mask mask = (uint32)track->getFeatures();
				if (mask.checkBits(ITrack::TF_SEEK))
					mSlider.setEnabled(true);
				for (int i = TB_BEGIN; i < TB_COUNT; ++i)
					mButtons[i].setEnabled(true);
				mLabel.setEnabled(true);

				this->updateUI(true);
			}
			else
			{
				//mSlider.setSliderPosition(0);
				bool prevb = mSlider.blockSignals(true);
				mSlider.setValue(0);
				mSlider.setEnabled(false);
				mSlider.blockSignals(prevb);

				for (int i = TB_BEGIN; i < TB_COUNT; ++i)
					mButtons[i].setEnabled(false);

				mLabel.setText(tr("0.0/1.0\n0/1"));

				mLabel.setEnabled(false);
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	ITrack*	QtTrackView::getTrack() const
	{
		return mTrackData.getTrack();
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtTrackView::updateUI(bool forceUpdate)
	{
		if (mTrackData.getTrack() == NULL || mTrackData.getTrack()->getFrameTime() < Math::HIGH_EPSILON || mTrackData.getTrack()->getDuration() < Math::HIGH_EPSILON)
			return;

		mTimer.update();
		if (mTimer.getSeconds() > mTrackData.getTrack()->getFrameTime())
			mTimer.reset();
		else if (!forceUpdate)
			return;

		scalar frameTime = mTrackData.getTrack()->getFrameTime();
		if (!this->QDockWidget::isVisible())
			return;

		scalar position = mTrackData.getTimeLine();
		int sliderPos = (int)(position / frameTime);
		int oldPos = mSlider.sliderPosition();
		//const tchar* animation = mTrack->getCurrentAnimation();
		if (oldPos != sliderPos || forceUpdate)
		{
			//mSlider.setSliderPosition(sliderPos);
			bool prevb = mSlider.blockSignals(true);
			mSlider.setValue(sliderPos);
			mSlider.blockSignals(prevb);
			TStringStream stream;

			scalar duration = mTrackData.getTrack()->getDuration();
			//animation changed, update slider range.
			mSlider.setMinimum(0);
			mSlider.setMaximum(mTrackData.getFrameCount());	//frame count
			mSlider.setPageStep(std::max(1, (int)mTrackData.getFrameCount()/8));
			//selected playing range
			//mSlider.setSelection(mTrackData.getStartFrame(), mTrackData.getEndFrame());

			stream.format(TEXT("%.3f/%.3f\n%d/%d"), position, duration, sliderPos, mTrackData.getFrameCount());
			TString s = stream.str();
			mLabel.setText(TString2QString(s));
		}

		QPushButton& button = mButtons[TB_PLAYPAUSE];
		if (mTrackData.isPlaying() && !button.isChecked())
		{
			button.setChecked(true);
			if(mPauseIcon != NULL)
				button.setIcon(*mPauseIcon);
		}
		else if (!mTrackData.isPlaying() && button.isChecked())
		{
			button.setChecked(false);
			if(mPlayIcon != NULL)
				button.setIcon(*mPlayIcon);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void QtTrackView::resizeEvent(QResizeEvent* evt)
	{
		QtWidgetBase<IUITrackView>::resizeEvent(evt);

		QSize sz = mContent.size();

		int xoff = 0;
		const int SPACING = 2;

		for(int i = TB_BEGIN; i < TB_COUNT; ++i)
		{
			mButtons[i].move(xoff, 0);
			xoff += mButtons[i].size().width() + SPACING;
		}

		mLabel.move(xoff, 0);
		mLabel.resize(LABEL_WIDTH, mButtons[0].height());

		xoff += mLabel.size().width() + SPACING;

		mSlider.move(xoff, 0);
		mSlider.resize(sz.width()-xoff, mButtons[0].size().height());
	}

	//////////////////////////////////////////////////////////////////////////
	void QtTrackView::mouseDoubleClickEvent(QMouseEvent *evt)
	{
		QPoint pt = mLabel.mapFromGlobal(evt->globalPos());
		if( mLabel.isEnabled() && mLabel.rect().contains(pt))
		{
			QDialog dialog;
			QVBoxLayout* layout = QT_NEW QVBoxLayout();
			dialog.setLayout(layout);
			
			QHBoxLayout* startLayout = QT_NEW QHBoxLayout();
			QLabel* lablelStart = QT_NEW QLabel();
			lablelStart->setText( TString2QString(BTLang("Start Frame")) );

			QLineEdit* editStart = QT_NEW QLineEdit();
			editStart->setText(TString2QString(TStringHelper::fromUInt(mTrackData.getStartFrame())));

			startLayout->addWidget(lablelStart);
			startLayout->addWidget(editStart);
			layout->addLayout(startLayout);

			QHBoxLayout* endLayout = QT_NEW QHBoxLayout();
			QLabel* lablelEnd = QT_NEW QLabel();
			lablelEnd->setText( TString2QString(BTLang("End Frame")) );
			lablelEnd->setMinimumWidth( lablelStart->minimumSizeHint().width() );

			QLineEdit* editEnd = QT_NEW QLineEdit();
			editEnd->setText(TString2QString(TStringHelper::fromUInt(mTrackData.getEndFrame())));

			endLayout->addWidget(lablelEnd);
			endLayout->addWidget(editEnd);
			layout->addLayout(endLayout);

			QHBoxLayout* buttonLayout = QT_NEW QHBoxLayout();

			QPushButton* cancel = QT_NEW QPushButton();
			cancel->setText(TString2QString(BTLang(BLANG_CANCEL)));
			QObject::connect(cancel, SIGNAL(clicked()), &dialog, SLOT(reject()));
			buttonLayout->addWidget(cancel);

			QPushButton* ok = QT_NEW QPushButton();
			ok->setText(TString2QString(BTLang(BLANG_OK)));
			QObject::connect(ok, SIGNAL(clicked()), &dialog, SLOT(accept()));
			buttonLayout->addWidget(ok);

			layout->addLayout(buttonLayout);
			while( dialog.exec() )
			{
				TString start = QString2TString(editStart->text());
				TString end = QString2TString(editEnd->text());
				int s = TStringHelper::toInt(start);
				int e = TStringHelper::toInt(end);
				if( s > e || s < mTrackData.getStartFrame() || e > mTrackData.getEndFrame() )
				{
					TStringStream ss;
					ss.format(BTLang("Invalid range. valid ranges are [%d - %d].").c_str(), mTrackData.getStartFrame(), mTrackData.getEndFrame());
					QMessageBox message(QMessageBox::Critical, TString2QString(BTLang(BLANG_ERROR)), TString2QString(ss.str()), QMessageBox::Ok);
					message.exec();
				}
				else
				{
					mTrackData.setStartFrame( s );
					mTrackData.setEndFrame( e );
					break;
				}
			}
		}
		else
			QtWidgetBase<IUITrackView>::mouseDoubleClickEvent(evt);
	}
	
}//namespace Blade