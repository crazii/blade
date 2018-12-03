/********************************************************************
	created:	2014/10/01
	filename: 	Track.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Track.h"
#include <interface/ITimeService.h>

namespace Blade
{
	template class Factory<ITrackManager>;

	//////////////////////////////////////////////////////////////////////////
	TrackManager::TrackManager()
	{
		mTrackList.reserve(8);
	}

	//////////////////////////////////////////////////////////////////////////
	TrackManager::~TrackManager()
	{

	}

	/************************************************************************/
	/* ITrackManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t	TrackManager::getTrackCount() const
	{
		return mTrackList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	ITrack*	TrackManager::getTrack(index_t index) const
	{
		if( index < mTrackList.size() )
			return mTrackList[index];
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t	TrackManager::getTrackIndex(ITrack* track) const
	{
		if( track != NULL )
		{
			TrackList::const_iterator i = std::find(mTrackList.begin(), mTrackList.end(), track);
			if( i != mTrackList.end() )
				return (index_t)(i-mTrackList.begin());
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TrackManager::addTrack(ITrack* track)
	{
		if( track != NULL )
		{
			index_t index = this->getTrackIndex(track);
			if( index == INVALID_INDEX)
			{
				mTrackList.push_back(track);
				return true;
			}
		}
		return false;
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool	TrackManager::removeTrack(index_t index)
	{
		if( index < mTrackList.size() )
		{
			mTrackList.erase( mTrackList.begin() + (indexdiff_t)index );
			return true;
		}
		else
			return false;
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TrackData::TrackData()
		:mTrack(NULL)
		,mTimeLine(0)
		,mRangeStart(0)
		,mRangeEnd(0)
		,mIsPlaying(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TrackData::~TrackData()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void TrackData::setTrack(ITrack* track)
	{
		mTrack = track;
		mTimeLine = 0;
		mRangeStart = 0;
		if( mTrack != NULL && track->getFrameTime() > Math::LOW_EPSILON)
		{
			scalar frameTime = track->getFrameTime();
			mRangeEnd = (int)(track->getDuration() / frameTime);
			mIsPlaying = mRangeEnd != 0;
			mAnimation = track->getCurrentAnimation();
		}
		else
		{
			mAnimation = TString::EMPTY;
			mRangeEnd = 0;
			mIsPlaying = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void TrackData::update()
	{
		if( mTrack == NULL )
			return;

		const tchar* animation = mTrack->getCurrentAnimation();
		scalar frameTime = mTrack->getFrameTime();
		if( mAnimation != animation )
		{
			mAnimation = animation;
			int frameCount = frameTime < Math::HIGH_EPSILON ? 0 : int(mTrack->getDuration() / frameTime);
			mRangeStart = 0;
			mRangeEnd = frameCount;
			mIsPlaying = frameCount != 0;
		}

		if( mIsPlaying )
		{
			scalar time = ITimeService::getSingleton().getTimeSource().getTimeThisLoop();
			if( time < 1.0f )
			{
				mTimeLine += time;
				scalar startTime = mRangeStart*frameTime;
				scalar span = (mRangeEnd+1)*frameTime - startTime;
				mTimeLine = startTime + std::fmod(mTimeLine-startTime, span);
				if( mTimeLine < startTime )
					mTimeLine += startTime;
			}
		}

		mTrack->setPosition(mTimeLine);
	}

	//////////////////////////////////////////////////////////////////////////
	void TrackData::gotoPrevFrame()
	{
		if(mTrack == NULL)
			return;
		mTimeLine -= mTrack->getFrameTime();
		scalar start = this->getStartTime();
		if( mTimeLine < start )
			mTimeLine = start;
	}

	//////////////////////////////////////////////////////////////////////////
	void TrackData::gotoNextFrame()
	{
		if(mTrack == NULL)
			return;
		scalar oldTime = mTimeLine;
		mTimeLine += mTrack->getFrameTime();
		mTimeLine = this->getStartTime() + std::fmod(mTimeLine-this->getStartTime(), this->getEndTime() - this->getStartTime());

		scalar end = this->getEndTime();
		if( mTimeLine > end || mTimeLine < oldTime)
			mTimeLine = mTrack->getDuration();	//align to end
	}

	//////////////////////////////////////////////////////////////////////////
	void TrackData::gotoStartFrame()
	{
		if(mTrack == NULL)
			return;
		mTimeLine = this->getStartTime();
	}

	//////////////////////////////////////////////////////////////////////////
	void TrackData::gotoEndFrame()
	{
		if(mTrack == NULL)
			return;
		if( mRangeEnd == this->getFrameCount() )
			mTimeLine = mTrack->getDuration();	//align to end
		else
			mTimeLine = this->getEndTime();
	}

}//namespace Blade