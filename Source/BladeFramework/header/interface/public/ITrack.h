/********************************************************************
	created:	2014/09/30
	filename: 	ITrack.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ITrack_h__
#define __Blade_ITrack_h__
#include <BladeTypes.h>
#include <BladeFramework.h>

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ITrack
	{
	public:
		typedef enum ETrackFeature
		{
			TF_SEEK		= 0x00000001,
			TF_SETLENGTH= 0x00000002,
			TF_KEYFRAME	= 0x00000004|TF_SEEK,
			TF_ADDKEY	= 0x00000008|TF_KEYFRAME,
			TF_REMOVEKEY= 0x000000010|TF_KEYFRAME,
		}FEATURE_MASK;
	public:
		/** @brief  */
		virtual scalar	getDuration() const = 0;
		/** @brief  */
		virtual scalar	getFrameTime() const = 0;
		/** @brief get current play pos */
		virtual scalar	getPosition() const = 0;
		/** @brief FEATURE_MASK  */
		virtual int		getFeatures() const = 0;

		/** @brief get current animation name, if have any */
		virtual const tchar* getCurrentAnimation() const	{return NULL;}

		/** @brief TF_SEEK */
		virtual bool	setPosition(scalar pos)		{BLADE_UNREFERENCED(pos); return false;}

		/** @brief TF_SETLENGTH */
		virtual bool	setDuration(scalar length)	{BLADE_UNREFERENCED(length); return false;}

		/** @brief TF_KEYFRAME */
		virtual size_t	getKeyFrameCount() const			{return 0;}
		virtual scalar	getKeyFrame(index_t index) const	{BLADE_UNREFERENCED(index);return 0;}

		/** @brief TF_ADDKEY */
		virtual index_t	addKeyFrame(scalar pos)		{BLADE_UNREFERENCED(pos); return INVALID_INDEX;}

		/** @brief TF_REMOVEKEY */
		virtual bool	removeKeyFrame(index_t index){BLADE_UNREFERENCED(index); return false;}

	};//class ITrack


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class ITrackManager : public InterfaceSingleton<ITrackManager>
	{
	public:
		virtual ~ITrackManager() {}

		/** @brief  */
		virtual size_t	getTrackCount() const = 0;

		/** @brief get bound track */
		virtual ITrack*	getTrack(index_t index) const = 0;

		/** @brief  */
		virtual index_t	getTrackIndex(ITrack* track) const = 0;

		/** @brief bind track to view */
		virtual bool	addTrack(ITrack* track) = 0;

		/** @brief  */
		virtual bool	removeTrack(index_t index) = 0;
		inline bool	removeTrack(ITrack* track)
		{
			return this->removeTrack(this->getTrackIndex(track));
		}
	};

	extern template class BLADE_FRAMEWORK_API Factory<ITrackManager>;


	class BLADE_FRAMEWORK_API TrackData : public Allocatable
	{
	public:
		TrackData();
		~TrackData();

		/** @brief  */
		inline ITrack*	getTrack() const	{return mTrack;}

		/** @brief  */
		inline void play(bool playing)		{mIsPlaying = playing;}

		/** @brief  */
		inline bool isPlaying() const		{return mIsPlaying;}

		/** @brief  */
		inline void setTimeLine(scalar timeLine)	{mTimeLine = timeLine;}

		/** @brief  */
		inline scalar getTimeLine() const	{return mTimeLine;}

		/** @brief  */
		inline int	getFrameCount() const	{return mTrack != NULL && mRangeEnd != 0 ? (int)(mTrack->getDuration()/mTrack->getFrameTime()) : 0;}

		/** @brief  */
		inline int getStartFrame() const {return mRangeStart;}

		/** @brief  */
		inline int getEndFrame() const {return mRangeEnd;}

		/** @brief  */
		inline void setStartFrame(int start)	{mRangeStart = start; mTimeLine = this->getStartTime();}

		/** @brief  */
		inline void setEndFrame(int end)		{mRangeEnd = end; mTimeLine = this->getStartTime();}

		/** @brief  */
		inline scalar getStartTime() const		{return mTrack != NULL ? mRangeStart*mTrack->getFrameTime() : 0;}

		/** @brief  */
		inline scalar getEndTime() const		{return mTrack != NULL ? mRangeEnd*mTrack->getFrameTime() : 0;}

		/** @brief  */
		void setTrack(ITrack* track);

		/** @brief  */
		void update();

		/** @brief  */
		void gotoPrevFrame();

		/** @brief  */
		void gotoNextFrame();

		/** @brief  */
		void gotoStartFrame();

		/** @brief  */
		void gotoEndFrame();

	protected:
		ITrack*	mTrack;
		TString	mAnimation;
		scalar	mTimeLine;
		int		mRangeStart;
		int		mRangeEnd;
		bool	mIsPlaying;
	};
	
}//namespace Blade


#endif // __Blade_ITrackView_h__