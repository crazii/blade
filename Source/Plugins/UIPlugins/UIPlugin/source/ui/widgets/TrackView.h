/********************************************************************
	created:	2014/09/30
	filename: 	TrackView.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TrackView_h__
#define __Blade_TrackView_h__
#include <interface/public/ui/widgets/IUITrackView.h>
#include <ui/DockWindow.h>
#include "TrackViewUI.h"

namespace Blade
{
	class TrackView : public DockWindowBase<IUITrackView>, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		using Allocatable::operator new;
		using Allocatable::operator delete;
		using Allocatable::operator new[];
		using Allocatable::operator delete[];
	public:
		TrackView();
		~TrackView();

		/************************************************************************/
		/* IUITrackView interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool	setTrack(ITrack* track) { return mUI.setTrack(track); }

		/** @brief  */
		virtual ITrack*	getTrack() const { return mUI.getTrack(); }

		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
	protected:
		/** @brief  */
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		/** @brief  */
		afx_msg void OnSize(UINT nType, int cx, int cy);
		DECLARE_MESSAGE_MAP()

	public:
		/** @brief  */
		inline void		setToolTip(CToolTipCtrl* tooltip) { mUI.setToolTip(tooltip); }

	protected:
		CTrackViewUI	mUI;
	};


}//namespace Blade

#endif // __Blade_TrackView_h__