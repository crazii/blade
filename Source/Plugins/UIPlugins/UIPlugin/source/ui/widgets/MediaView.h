/********************************************************************
	created:	2013/01/12
	filename: 	MediaView.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MediaView_h__
#define __Blade_MediaView_h__
#include <ui/DockWindow.h>

namespace Blade
{
	class MediaView : public DockWindowBase<IUIWidgetLeaf>, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		using Allocatable::operator new;
		using Allocatable::operator delete;
		using Allocatable::operator new[];
		using Allocatable::operator delete[];

		DECLARE_MESSAGE_MAP()
	public:
		MediaView();
		~MediaView();


		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		/** @brief  */
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		/** @brief  */
		afx_msg void OnSize(UINT nType, int cx, int cy);
		/** @brief  */
		afx_msg void OnDestroy();

	protected:

		/************************************************************************/
		/* internal methods                                                                     */
		/************************************************************************/

		//event handler
		void	onMediaView(const Event& data);
		void	updateMediaView();

	protected:

		CStatic mView;
		index_t	mMediaTypeIndex;
		TString	mMediaFileName;
	};


}//namespace Blade


#endif //  __Blade_MediaView_h__