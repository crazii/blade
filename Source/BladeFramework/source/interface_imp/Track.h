/********************************************************************
	created:	2014/10/01
	filename: 	Track.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Track_h__
#define __Blade_Track_h__
#include <interface/public/ITrack.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class TrackManager : public Singleton<TrackManager>
	{
	public:
		TrackManager();
		virtual ~TrackManager();

		/************************************************************************/
		/* ITrackManager interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual size_t	getTrackCount() const;

		/** @brief get bound track */
		virtual ITrack*	getTrack(index_t index) const;

		/** @brief  */
		virtual index_t	getTrackIndex(ITrack* track) const;

		/** @brief bind track to view */
		virtual bool	addTrack(ITrack* track);

		/** @brief  */
		virtual bool	removeTrack(index_t index);

	protected:
		typedef Vector<ITrack*>	TrackList;
		TrackList	mTrackList;
	};
	
}//namespace Blade


#endif // __Blade_Track_h__