/********************************************************************
	created:	2016/7/27
	filename: 	QtTrackView.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtTrackView_h__
#define __Blade_QtTrackView_h__
#include <interface/public/ui/widgets/IUITrackView.h>
#include <interface/IFramework.h>
#include <widgets/QtWidgetBase.h>

namespace Blade
{
	class QtTrackView : public QtWidgetBase<IUITrackView>, public IFramework::IMainLoopUpdater, public Allocatable
	{
		enum ETrackButton
		{
			TB_BEGIN,
			TB_PREV,
			TB_PLAYPAUSE,
			TB_NEXT,
			TB_END,

			TB_COUNT,
		};

	public:
		static const TString WIDGET_TYPE;
	public:
		QtTrackView();
		~QtTrackView();

		/** @brief  */
		virtual bool	setTrack(ITrack* track);

		/** @brief  */
		virtual ITrack*	getTrack() const;

	protected:
		/** @brief  */
		virtual void update()
		{
			mTrackData.update();
			this->updateUI(false);
		}
		/** @brief  */
		void	updateUI(bool forceUpdate);

		/************************************************************************/
		/* Qt Events                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void resizeEvent(QResizeEvent* evt);

		/** @brief */
		virtual void mouseDoubleClickEvent(QMouseEvent *evt);

		/** @brief SLOT */
		void onBegin()
		{
			if (mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK))
			{
				mTrackData.play(false);
				mTrackData.setTimeLine(mTrackData.getStartTime());
			}
		}
		/** @brief SLOT */
		void onPrev()
		{
			if (mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK))
			{
				mTrackData.gotoPrevFrame();
				mTrackData.play(false);
			}
		}
		/** @brief SLOT */
		void onPlayPause()
		{
			if (mTrackData.getTrack() != NULL)
			{
				QPushButton& button = mButtons[TB_PLAYPAUSE];
				if (mTrackData.isPlaying())
				{
					mTrackData.play(false);
					button.setChecked(false);
					if (mPlayIcon != NULL)
						button.setIcon(*mPlayIcon);
				}
				else
				{
					mTrackData.play(true);
					button.setChecked(true);
					if (mPauseIcon != NULL)
						button.setIcon(*mPauseIcon);
				}
			}
		}
		/** @brief SLOT */
		void onNext()
		{
			if (mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK))
			{
				mTrackData.play(false);
				mTrackData.gotoNextFrame();
			}
		}
		/** @brief SLOT */
		void onEnd()
		{
			if (mTrackData.getTrack() != NULL && (mTrackData.getTrack()->getFeatures()&ITrack::TF_SEEK))
			{
				mTrackData.play(false);
				mTrackData.gotoEndFrame();
			}
		}
		/** @brief  */
		void onSlider(int pos)
		{
			int min = mSlider.minimum();
			int max = mSlider.maximum();

			mTrackData.play(false);
			//force align end to duration
			scalar timeLine = pos == max ? mTrackData.getTrack()->getDuration() : (scalar)pos * mTrackData.getTrack()->getFrameTime();
			mTrackData.setTimeLine(timeLine);
			this->updateUI(true);
		}

		QWidget mContent;
		Timer	mTimer;
		QIcon*	mPlayIcon;
		QIcon*	mPauseIcon;
		TrackData	mTrackData;
		QPushButton mButtons[TB_COUNT];
		QLabel		mLabel;
		QSlider		mSlider;
	};
	
}//namespace Blade

#endif//__Blade_QtTrackView_h__