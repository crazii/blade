/********************************************************************
	created:	2014/10/01
	filename: 	IUITrackView.h
	author:		Crazii
	
	purpose:	track view (UI) abstraction
				currently it only support single view of track.
*********************************************************************/
#ifndef __Blade_IUITrackView_h__
#define __Blade_IUITrackView_h__
#include <interface/public/ITrack.h>
#include <interface/public/ui/IUIWidget.h>

namespace Blade
{
	class IUITrackView : public IUIWidgetLeaf
	{
	public:
		//TODO: multiple track support

		/** @brief  */
		virtual bool	setTrack(ITrack* track) = 0;

		/** @brief  */
		virtual ITrack*	getTrack() const = 0;
	};
	
}//namespace Blade


#endif // __Blade_IUITrackView_h__