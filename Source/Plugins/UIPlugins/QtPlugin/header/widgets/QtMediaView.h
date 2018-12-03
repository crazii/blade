/********************************************************************
	created:	2016/7/27
	filename: 	QtMediaView.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMediaView_h__
#define __Blade_QtMediaView_h__
#include <interface/public/ui/IUIWidget.h>
#include <widgets/QtWidgetBase.h>

namespace Blade
{
	class QtMediaView : public QtWidgetBase<IUIWidgetLeaf>, public Allocatable
	{
	public:
		static const TString WIDGET_TYPE;
	public:
		QtMediaView();
		~QtMediaView();

	protected:
		/** @brief  */
		void	onMediaView(const Event& data);
		/** @brief  */
		void	updateMediaView();

		/** @brief  */
		virtual void resizeEvent(QResizeEvent* evt)
		{
			if(!mImage.isNull())
			{
				int h = mContent.size().width() * ((fp64)mImage.height() / (fp64)mImage.width());
				int w = mContent.size().height() * ((fp64)mImage.width() / (fp64)mImage.height());
				mContent.setPixmap(mImage.scaled(w, h, mAspect));
			}
			QDockWidget::resizeEvent(evt);
		}

		QLabel	mContent;
		index_t	mMediaTypeIndex;
		TString	mMediaFileName;
		QPixmap mImage;
		Qt::AspectRatioMode mAspect;
	};
	
}//namespace Blade

#endif//__Blade_QtMediaView_h__