/********************************************************************
	created:	2016/07/30
	filename: 	QtMouseDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtWindowDevice.h>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsSceneEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMouseDevice.h>
#include <QtMessagePump.h>
#include <utility/Functors.h>

namespace Blade
{
	const TString QtMouseDevice::QT_DEVICE_TYPE = BTString("QtMouse");
	POINT2I	QtMouseDevice::msMouseHidePos = POINT2I::ZERO;
	bool	QtMouseDevice::msCursorVisible = true;
	static const int SCREEN_BORDER_RESERVE = 50;

	//////////////////////////////////////////////////////////////////////////
	QtMouseDevice::QtMouseDevice()
		:mWidget(NULL)
		,mMousePos(POINT2I::ZERO)
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	QtMouseDevice::~QtMouseDevice()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	QtMouseDevice::close()
	{
		if(mWidget != NULL)
			mWidget->removeEventFilter(this);
		mWidget = NULL;
		QtMessagePump::getSingleton().removeMessageListener(this);
	}
	//////////////////////////////////////////////////////////////////////////
	bool	QtMouseDevice::reset()
	{
		for(int i = MSB_BEGIN; i < MSB_COUNT; ++i)
			mButtonState[i] = MBS_UP;
		mMouseMovement = POINT3I::ZERO;

		//initial position
		mMousePos = this->getGlobalMousePosition();
		if(mWidget != NULL)
		{
			QPoint pt(mMousePos.x, mMousePos.y);
			pt = mWidget->mapFromGlobal(pt);
			mMousePos.x = pt.x();
			mMousePos.y = pt.y();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtMouseDevice::update()
	{
		if (!msCursorVisible)
		{
			//this->setGlobalMousePosition(msMouseHidePos);
			POINT2I pos = this->getGlobalMousePosition();
			int width = QApplication::desktop()->screenGeometry().width();
			int height = QApplication::desktop()->screenGeometry().height();

			//note: adjust mouse position when comes to screen border
			//this allows generating mouse moving event when pos reaches border
			if (pos.x <= 0)
				pos.x = width - SCREEN_BORDER_RESERVE;
			else if (pos.x >= width - 1)
				pos.x = SCREEN_BORDER_RESERVE;

			if (pos.y <= 0)
				pos.y = height - SCREEN_BORDER_RESERVE;
			else if (pos.y >= height - 1)
				pos.y = SCREEN_BORDER_RESERVE;

			if (pos != mMousePos)
				this->setGlobalMousePosition(pos);
		}
		return true;
	}

	/************************************************************************/
	/* IMouseDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				QtMouseDevice::attach(IWindow* window)
	{
		if(mWidget != NULL)
		{
			assert(false);
			return;
		}

		assert(window != NULL);
		assert(window->getWindowType() == QtWindowDevice::QT_DEVICE_TYPE);
		uintptr_t impl = window->getImpl();
		QWidget* widget = (QWidget*)impl;
		if(widget != NULL)
		{
			mWidget = widget;
			mWidget->installEventFilter(this);
			QGraphicsView* view = qobject_cast<QGraphicsView*>(mWidget);
			if( view != NULL && view->scene() != NULL )
				view->scene()->installEventFilter(this);
			mWidget->setMouseTracking(true);
			this->reset();
			QtMessagePump::getSingleton().addMessageListener(this);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtMouseDevice::notifyWindowSize(size_t width,size_t height)
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	bool				QtMouseDevice::addListener(IListener* listener)
	{
		return listener != NULL && mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				QtMouseDevice::removeListener(IListener* listener)
	{
		return listener != NULL && mListeners.erase(listener) == 1;
	}

	/************************************************************************/
	/* IMouse interface                                                                     */
	/************************************************************************/
	
	/************************************************************************/
	/* Qt events                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				QtMouseDevice::eventFilter(QObject *watched, QEvent *evt)
	{
		//not always a safe cast, but for the following enum, it is safe
		QMouseEvent* msEvt = static_cast<QMouseEvent*>(evt);
		QWheelEvent* wheelEvt = static_cast<QWheelEvent*>(evt);

		QGraphicsSceneMouseEvent* gsmsEvt = static_cast<QGraphicsSceneMouseEvent*>(evt);
		QGraphicsSceneWheelEvent* gswheelEvt = static_cast<QGraphicsSceneWheelEvent*>(evt);

		int button = -1;
		bool down = false;
		bool dbclick = false;
		int x = 0,y = 0;

		switch(evt->type())
		{
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
			this->reset();
			break;
		//case QEvent::FocusIn:
		//case QEvent::FocusOut:
		//	this->reset();	//separate to debug
		//	break;
		case QEvent::MouseButtonDblClick:
			dbclick = true;	//no break
		case QEvent::MouseButtonPress:
			down = true;	//no break
		case QEvent::MouseButtonRelease:
			if(msEvt->button() == Qt::LeftButton)
				button = MSB_LEFT;
			else if(msEvt->button() == Qt::RightButton)
				button = MSB_RIGHT;
			else if(msEvt->button() == Qt::MiddleButton)
				button = MSB_RIGHT;
			x = msEvt->pos().x();
			y = msEvt->pos().y();
			break;
		case QEvent::MouseMove:
		{
			int dx = (msEvt->globalPos().x() - mMousePos.x);
			int dy = (msEvt->globalPos().y() - mMousePos.y);
			if (!msCursorVisible)
			{
				//filter out position jumps
				//@see QtMouseDevice::update
				int sw = QApplication::desktop()->screenGeometry().width();
				int sh = QApplication::desktop()->screenGeometry().height();
				if (mMouseMovement.x >= 0 && dx < -sw * 3 / 4)
					dx = 0;
				if (mMouseMovement.x <= 0 && dx > sw * 3 / 4)
					dx = 0;
				if (mMouseMovement.y >= 0 && dy < -sh * 3 / 4)
					dy = 0;
				if (mMouseMovement.y <= 0 && dy > sh * 3 / 4)
					dy = 0;
			}

			mMouseMovement.x += dx;
			mMouseMovement.y += dy;
			mMousePos.x = msEvt->pos().x();
			mMousePos.y = msEvt->pos().y();
			x = msEvt->pos().x();
			y = msEvt->pos().y();
		}
			break;
		case QEvent::Wheel:
			//TODO:
			mMouseMovement.z += (int)wheelEvt->pixelDelta().rx();
			x = wheelEvt->pos().x();
			y = wheelEvt->pos().y();
			break;
		//////////////////////////////////////////////////////////////////////////
		//QGraphicsScene event
		//////////////////////////////////////////////////////////////////////////
		case QEvent::GraphicsSceneMouseDoubleClick:
			dbclick = true;	//no break
		case QEvent::GraphicsSceneMousePress:
			down = true;	//no break
		case QEvent::GraphicsSceneMouseRelease:
			if(gsmsEvt->button() == Qt::LeftButton)
				button = MSB_LEFT;
			else if(gsmsEvt->button() == Qt::RightButton)
				button = MSB_RIGHT;
			else if(gsmsEvt->button() == Qt::MiddleButton)
				button = MSB_RIGHT;
			x = gsmsEvt->scenePos().x();
			y = gsmsEvt->scenePos().y();
			break;
		case QEvent::GraphicsSceneMouseMove:
		{
			int dx = (gsmsEvt->scenePos().x() - gsmsEvt->lastScenePos().x());
			int dy = (gsmsEvt->scenePos().y() - gsmsEvt->lastScenePos().y());

			if(!msCursorVisible)
			{
				//filter out position jumps
				//@see QtMouseDevice::update
				int sw = QApplication::desktop()->screenGeometry().width();
				int sh = QApplication::desktop()->screenGeometry().height();
				if (mMouseMovement.x >= 0 && dx < -sw * 3 / 4)
					dx = 0;
				if (mMouseMovement.x <= 0 && dx > sw * 3 / 4)
					dx = 0;
				if (mMouseMovement.y >= 0 && dy < -sh * 3 / 4)
					dy = 0;
				if (mMouseMovement.y <= 0 && dy > sh * 3 / 4)
					dy = 0;
			}

			mMouseMovement.x += dx;
			mMouseMovement.y += dy;
			mMousePos.x = gsmsEvt->scenePos().x();
			mMousePos.y = gsmsEvt->scenePos().y();
			x = gsmsEvt->scenePos().x();
			y = gsmsEvt->scenePos().y();
		}
			break;
		case QEvent::GraphicsSceneWheel:
			mMouseMovement.z += gswheelEvt->delta();
			x = gswheelEvt->scenePos().x();
			y = gswheelEvt->scenePos().y();
			break;
		}

		if (button != -1)
		{
			assert(button >= MSB_BEGIN && button < MSB_COUNT);
			if (down)
			{
				bool bKeepData = std::for_each(mListeners.begin(), mListeners.end(),
					Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonDown), MSBUTTON(button))).getResult();

				if (bKeepData)
				{
					mButtonState[button] = MBS_DOWN;

					if(dbclick)
					{
						std::for_each(mListeners.begin(),mListeners.end(), 
							std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonDBClick),MSBUTTON(button)) );
					}
					else
					{
						mDownPos.x = x;
						mDownPos.y = y;
					}
				}
			}
			else
			{
				bool bKeepData = std::for_each(mListeners.begin(), mListeners.end(),
					Fn::BoolBind2nd(Fn::BoolMemFun1(&IMouse::IListener::onMouseButtonUp), MSBUTTON(button))).getResult();

				//click detection
				static const int THRESHOULD = 3;
				if(mButtonState[button] == MBS_DOWN)
				{
					if(std::abs(mDownPos.x - x) < THRESHOULD && std::abs(mMousePos.y - y) < THRESHOULD)
					{
						std::for_each(mListeners.begin(),mListeners.end(), 
							std::bind2nd( std::mem_fun(&IMouse::IListener::onMouseButtonClick),MSBUTTON(button)) );
					}
				}

				if (bKeepData)
					mButtonState[button] = MBS_UP;
			}
		}
		return false;
	}

}//namespace Blade