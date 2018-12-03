/********************************************************************
	created:	2016/07/30
	filename: 	QtWindowDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtWindowDevice_h__
#define __Blade_QtWindowDevice_h__
#include <BladeQtDevice.h>
#include <QWidget.h>
#include <utility/BladeContainer.h>
#include <interface/public/window/IWindowDevice.h>

namespace Blade
{
	class QtWindowDevice : public IWindowDevice, public QObject, public Allocatable
	{
	public:
		static const TString QT_DEVICE_TYPE;
	public:
		QtWindowDevice();
		virtual ~QtWindowDevice();
		/************************************************************************/
		/* IDevice interface                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update();

		/************************************************************************/
		/* IWindowDevice interface                                                                     */
		/************************************************************************/
		/** @brief specify a external window to the device */
		/* @remark in this situation,when calling destroyWindow, it doesn't destroy the internal window and just detach it */
		virtual void attachWindow(uintptr_t internalImpl);

		/** @brief set the window's specific color depth */
		virtual void setColorDepth(int depth) {mColorDepth = depth;}

		/** @brief  */
		virtual void destroyWindow();

		/************************************************************************/
		/* IWindow interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getWindowType() const
		{
			return QT_DEVICE_TYPE;
		}

		/** @brief  */
		virtual const  TString&	getUniqueName() const;

		/** @brief set the window size */
		virtual void setSize(size_t width,size_t height)
		{
			if (mWidget != NULL)
			{
				mWidget->resize((int)width, (int)height);
				if (!mWidget->isVisible())
				{
					//force hidden window emit Resize event
					mWidget->setAttribute(Qt::WA_DontShowOnScreen, true);
					mWidget->show();
					mWidget->hide();
					mWidget->setAttribute(Qt::WA_DontShowOnScreen, false);
				}
			}
		}

		/** @brief get the window size */
		virtual size_t getWidth() const
		{
			return (mWidget != NULL) ? (int)mWidget->size().width() : 0;
		}

		/** @brief get the window size */
		virtual size_t getHeight() const
		{
			return (mWidget != NULL) ? (int)mWidget->size().height() : 0;
		}

		/** @brief set the window caption */
		virtual void setCaption(const TString& caption)
		{
			mCaption = caption;
			if(mWidget != NULL)
				mWidget->setWindowTitle( TString2QString(caption) );
		}

		/** @brief get the window caption */
		virtual const TString& getCaption() const
		{
			return mCaption;
		}

		/** @brief set the window visibility */
		virtual void setVisibility(bool visibile)
		{
			if (mWidget != NULL)
				mWidget->setVisible(visibile);
		}

		/** @brief get the window visibility */
		virtual bool isVisible() const
		{
			return mWidget != NULL ? !mWidget->visibleRegion().isEmpty() : false;
		}

		/** @brief set the window's full screen mode */
		virtual void setFullScreen(bool fullscreen)
		{
			if(mWidget != NULL && mFullScreen != fullscreen)
			{
				if(fullscreen)
					mWidget->showFullScreen();
				else
					mWidget->showNormal();

				mFullScreen = mWidget->isFullScreen();
			}
		}

		/** @brief get whether the window is in full screen mode */
		virtual bool getFullScreen() const
		{
			return mFullScreen;
		}

		/** @brief get the window's specific color depth */
		virtual int  getColorDepth() const
		{
			return mColorDepth;
		}

		/** @brief get the native platform/API related window pointer(i.e. HWND for win32) */
		virtual uintptr_t	getNativeHandle() const
		{
			return (uintptr_t)( mWidget != NULL ? mWidget->winId() : NULL );
		}

		/** @brief get the implementation defined pointer */
		virtual uintptr_t	getImpl() const {return (uintptr_t)mWidget;}

		/** @brief set the parent of the window by internal implementation
		note: internalImpl may NOT be a native handle */
		virtual void setParent(uintptr_t internalImpl)
		{
			QWidget* widget = (QWidget*)internalImpl;
			mParent = widget;
			if(mWidget != NULL )
			{
				
				mWidget->setParent(widget);
			}
		}

		/** @brief get the internal implementation of parent window */
		virtual uintptr_t getParent() const
		{
			return (uintptr_t)mParent;
		}

		/** @brief close the window */
		virtual bool closeWindow()
		{
			this->close();
			return true;
		}

		/** @brief  */
		virtual bool addEventHandler(IWindowEventHandler* handler);

		/** @brief  */
		virtual bool removeEventHandler(IWindowEventHandler* handler); 

	protected:
		/************************************************************************/
		/* Qt events                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt);

		/** @brief SLOT */
		void	onTitleChange(const QString &title);

		/** @brief  */
		template< typename operation>
		operation		dispatchWindowEvent(operation op)
		{
			//note: because a handler may remove itself on notifying iteration 
			//so the iterator may become invalid

			//copy to a tmp buffer: use local buffer instead of this to avoid re-enter
			//if a message called in a message, HandlersBuffer of this will be cleared!
			TempVector<IWindowEventHandler*> hanlders;
			hanlders.reserve(mHandlers.size());
			hanlders.insert(hanlders.end(), mHandlers.begin(),mHandlers.end());

			for(size_t i = 0; i < hanlders.size(); ++i)
			{
				IWindowEventHandler* handler = hanlders[i];
				//check if handler is deleted already
				if( mHandlers.find(handler) != mHandlers.end() )
					op(handler);
			}
			return op;
		}

		typedef Set<IWindowEventHandler*>	WinHandlerList;

		WinHandlerList	mHandlers;

		TString		mCaption;
		TString		mName;
		QWidget*	mWidget;
		QWidget*	mParent;
		int			mColorDepth;
		bool		mExternal;
		bool		mInitialVisible;
		bool		mFullScreen;
	};

	
}//namespace Blade


#endif // __Blade_QtWindowDevice_h__