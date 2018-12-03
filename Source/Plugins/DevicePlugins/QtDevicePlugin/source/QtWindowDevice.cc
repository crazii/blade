/********************************************************************
	created:	2016/07/30
	filename: 	QtWindowDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtWindowDevice.h>
#include <interface/public/window/Helpers.h>
#include <utility/StringHelper.h>

namespace Blade
{
	const TString QtWindowDevice::QT_DEVICE_TYPE = BTString("QtWindow");

	//////////////////////////////////////////////////////////////////////////
	QtWindowDevice::QtWindowDevice()
		:mWidget(NULL)
		,mParent(NULL)
		,mColorDepth(32)
		,mExternal(false)
		,mInitialVisible(false)
		,mFullScreen(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	QtWindowDevice::~QtWindowDevice()
	{

	}

	/************************************************************************/
	/* IDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	QtWindowDevice::open()
	{
		if(mWidget == NULL)
		{
			mWidget = new QWidget(mParent);
			if (mInitialVisible)
			{
				if (mFullScreen)
					mWidget->showFullScreen();
				else
					mWidget->showNormal();
			}
			mWidget->installEventFilter(this);
			mWidget->setAttribute(Qt::WA_DeleteOnClose, true);
			mWidget->setWindowTitle(TString2QString(mCaption));
			if(mName.empty())
				mName = TStringHelper::fromPointer(mWidget);
			QObject::connect(mWidget, &QWidget::windowTitleChanged, this, &QtWindowDevice::onTitleChange);
			this->dispatchWindowEvent( FnOnWinCreate(this) );
			mExternal = false;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtWindowDevice::isOpen() const
	{
		return mWidget != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtWindowDevice::close()
	{
		if( mWidget != NULL )
			mWidget->close();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtWindowDevice::reset()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtWindowDevice::update()
	{
		return true;
	}

	/************************************************************************/
	/* IWindowDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void QtWindowDevice::attachWindow(uintptr_t internalImpl)
	{
		if(mWidget != NULL)
		{
			assert(false);
			return;
		}

		QWidget* widget = (QWidget*)internalImpl;
		if(widget == NULL)
			return;

		mWidget = widget;
		//must be deletable
		assert(mWidget->testAttribute(Qt::WA_DeleteOnClose));
		mWidget->installEventFilter(this);
		mInitialVisible = widget->isVisible();
		mFullScreen = widget->isFullScreen();
		mCaption = QString2TString(widget->windowTitle());
		mName = TStringHelper::fromPointer(mWidget);
		QObject::connect(mWidget, &QWidget::windowTitleChanged, this, &QtWindowDevice::onTitleChange);
		this->dispatchWindowEvent( FnOnWinCreate(this) );
		mExternal = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void QtWindowDevice::destroyWindow()
	{
		if( mExternal )
			this->dispatchWindowEvent(FnOnWinDestroy(this));
		else if(mWidget != NULL)
			delete mWidget;

		mWidget = NULL;
		mName = TString::EMPTY;
	}

	/************************************************************************/
	/* IWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const  TString&	QtWindowDevice::getUniqueName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	bool QtWindowDevice::addEventHandler(IWindowEventHandler* handler)
	{
		return mHandlers.insert(handler).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool QtWindowDevice::removeEventHandler(IWindowEventHandler* handler)
	{
		return mHandlers.erase(handler) == 1;
	}

	/************************************************************************/
	/* Qt events                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool QtWindowDevice::eventFilter(QObject *watched, QEvent *evt)
	{
		switch (evt->type())
		{
		case QEvent::Show:
			this->dispatchWindowEvent( FnOnWinShow(this) );
			break;
		case QEvent::Hide:
			this->dispatchWindowEvent( FnOnWinHide(this) );
			break;
		case QEvent::Resize:
			{
				QResizeEvent* resize = static_cast<QResizeEvent*>(evt);
				bool allow = this->dispatchWindowEvent( FnOnWinResizing(this, resize->size().width(), resize->size().height()) ).mAllowResize;
				if (!allow)
				{
					evt->ignore();
					return true;
				}
				this->dispatchWindowEvent( FnOnWinResized(this) );
			}
			break;
		case QEvent::WindowActivate:
			this->dispatchWindowEvent( FnOnWinActivate(this) );
			break;
		case QEvent::WindowDeactivate:
			this->dispatchWindowEvent( FnOnWinDeactivate(this) );
			break;
		case QEvent::Close:
			if (this->dispatchWindowEvent(FnOnWinClose(this)).getResult())
			{
				//ensure again
				assert(mWidget->testAttribute(Qt::WA_DeleteOnClose));
				QWidget* widget = mWidget;
				mWidget = NULL;
				//note: we need dispatch destroy event immediately
				delete widget;
			}
			evt->ignore();
			return true;
		case QEvent::Destroy:
			this->dispatchWindowEvent( FnOnWinDestroy(this) );
			mWidget = NULL;
			mExternal = false;
			break;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtWindowDevice::onTitleChange(const QString &title)
	{
		mCaption = QString2TString(title);
		this->dispatchWindowEvent( FnOnWinCaptionChange(this) );
	}
	
}//namespace Blade