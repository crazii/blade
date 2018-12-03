/********************************************************************
	created:	2016/7/26
	filename: 	QtWidgetBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_QtWidgetBase_h__
#define __Blade_QtWidgetBase_h__
#include <interface/public/ui/IUIWidget.h>
#include <QtIconManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	//resolve conflict
	//void QWidget::setVisible(bool)
	//bool IUIWidget::setVisible(bool)
	template<typename T>
	class QtWidgetApater : public T
	{
		virtual QWidget* getWidget() = 0;
	public:
		virtual bool setVisible(bool visible)
		{
			if(visible)
				this->getWidget()->setVisible(visible);
			else
				this->getWidget()->hide();
			return true;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	class QtIconDockStyle: public QProxyStyle, public Allocatable
	{
	public:
		QtIconDockStyle(QStyle* style = Q_NULLPTR)
			: QProxyStyle(style)
		{}

		virtual ~QtIconDockStyle()
		{}

		virtual void drawControl(ControlElement element, const QStyleOption* option,
			QPainter* painter, const QWidget* widget = Q_NULLPTR) const
		{
			if(element == QStyle::CE_DockWidgetTitle)
			{
				int margin = this->baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);
				int width = option->rect.height() - margin * 2;

				QPoint pos(margin + option->rect.left(), margin + option->rect.top());
				painter->drawPixmap(pos, widget->windowIcon().pixmap(width, width));
				//dirty modification.TODO: make a copy
				const_cast<QStyleOption*>(option)->rect = option->rect.adjusted(width+margin*2, 0, 0, 0);
			}
			this->baseStyle()->drawControl(element, option, painter, widget);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	class QtWidgetBase : public QDockWidget, public QtWidgetApater<T>
	{
	public:
		QtWidgetBase(const TString& type)
		{
			mData.mType = type;
			mStyle = NULL;
			this->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
			this->setFeatures(DockWidgetClosable|DockWidgetMovable|DockWidgetFloatable);
		}

		~QtWidgetBase()
		{
			BLADE_DELETE mStyle;
		}

		/************************************************************************/
		/* IUIWidget interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWidgetType() const { return mData.mType; }

		/** @brief  */
		virtual bool		initWidget(const WIDGET_DATA& data)
		{
			assert(mData.mType == data.mType);
			mData = data;
			//this->QDockWidget::setVisible((mData.mFlags&WMF_VISIBLE) != 0);
			this->QDockWidget::setWindowTitle(TString2QString(data.mCaption));

			QIcon* icon = QtIconManager::getSingleton().getIcon(mData.mIcon);
			if(icon != NULL)
			{
				this->setWindowIcon(*icon);
				this->setStyle((mStyle=BLADE_NEW QtIconDockStyle(NULL)));
			}
			return true;
		}

		/** @brief  */
		virtual bool		setParent(IUIWidget* /*parent*/)
		{
			//bool ret = mParent == NULL || parent == NULL;
			//if (ret)
			//	mParent = parent;
			return false;
		}

		/** @brief  */
		virtual IUIWidget*	getParent() const { return NULL; }

		/** @brief  */
		virtual const WIDGET_DATA& getWidgetData() const { return mData; }

		/** @brief  */
		virtual void		setCaption(const TString& caption)
		{
			T::setCaption(caption);
			this->QDockWidget::setWindowTitle(TString2QString(caption));
		}

		/** @brief  */
		virtual void		setPosition(POINT2I pos)
		{
			//moveEvent will update mData.mPos
			QDockWidget::move(pos.x, pos.y);
		}

		/** @brief  */
		virtual void		setSize(SIZE2I size)
		{
			//moveEvent will update mData.mPos
			QDockWidget::resize(size.x, size.y);
		}

	protected:
		/** @brief  */
		virtual QWidget* getWidget() { return this; }
		/** @brief  */
		virtual WIDGET_DATA& getWidgetDataW() {return mData;}

		/** @brief  */
		virtual void resizeEvent(QResizeEvent* evt)
		{
			QDockWidget::resizeEvent(evt);
			mData.mSize = SIZE2I(evt->size().width(), evt->size().height());
		}

		/** @brief  */
		virtual void moveEvent(QMoveEvent* evt)
		{
			QDockWidget::moveEvent(evt);
			mData.mPos = POINT2I(evt->pos().x(), evt->pos().y());
		}

		/** @brief  */
		virtual void showEvent(QShowEvent* evt)
		{
			QDockWidget::showEvent(evt);
			mData.mFlags |= WMF_VISIBLE;
		}

		/** @brief  */
		virtual void hideEvent(QHideEvent* evt)
		{
			QDockWidget::hideEvent(evt);
			mData.mFlags &= ~WMF_VISIBLE;
		}

		WIDGET_DATA	mData;
		QtIconDockStyle* mStyle;
	};
	
}//namespace Blade


#endif//__Blade_QtWidgetBase_h__