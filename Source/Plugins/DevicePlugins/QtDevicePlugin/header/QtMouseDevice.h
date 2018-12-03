/********************************************************************
	created:	2016/07/30
	filename: 	QtMouseDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMouseDevice_h__
#define __Blade_QtMouseDevice_h__
#include <BladeQtDevice.h>
#include <QtWidgets/QApplication>
#include <interface/public/window/IWindowMessagePump.h>
#include <interface/public/input/IMouseDevice.h>

namespace Blade
{

	class QtMouseDevice : public IMouseDevice, public IWindowMessagePump::IMessageListener, public QObject, public Allocatable
	{
	public:
		static const TString QT_DEVICE_TYPE;
	public:
		QtMouseDevice();
		~QtMouseDevice();

		/************************************************************************/
		/* IDevice interface                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open()	{return mWidget != NULL;}

		/** @brief check if it is open */
		virtual bool	isOpen() const {return mWidget != NULL;}

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update();

		/************************************************************************/
		/* IMouseDevice interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual void				attach(IWindow* window);

		/**
		@describe
		@param
		@return
		*/
		virtual void				notifyWindowSize(size_t width,size_t height);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				addListener(IListener* listener);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				removeListener(IListener* listener);

		/************************************************************************/
		/* IMouse interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		virtual MBSTATE			getButtonState(MSBUTTON button) const
		{
			assert(button >= MSB_BEGIN && button < MSB_COUNT);
			return mButtonState[button];
		}

		/**
		@describe get mouse position, in screen coordinates (0~screen_width,0~screen_height)
		@param 
		@return 
		*/
		virtual POINT2I			getGlobalMousePosition() const
		{
			QPoint pt = QCursor::pos();
			return POINT2I(pt.x(), pt.y());
		}

		/**
		@describe get mouse position, in window coordinates (0~window_width,0~window_height)
		@param 
		@return 
		*/
		virtual POINT2I			getMousePosition() const
		{
			return mMousePos;
		}

		/**
		@describe get normalized mouse position (0~1,0~1), in window coordinates
		@param 
		@return 
		*/
		virtual POINT2			getNormalizedMousePosition() const
		{
			if(mWidget == NULL)
			{
				assert(false);
				return POINT2::ZERO;
			}
			scalar x = (scalar)mMousePos.x / (scalar)mWidget->size().width();
			scalar y = (scalar)mMousePos.y / (scalar)mWidget->size().height();
			return POINT2(x, y);
		}

		/**
		@describe set mouse position in screen coordinates
		@param 
		@return 
		*/
		virtual void			setGlobalMousePosition(POINT2I pos)
		{
			QPoint pt(pos.x, pos.y);
			QCursor::setPos(pt);
		}

		/**
		@describe return false if already hide
		@param 
		@return 
		*/
		virtual bool			hide()
		{
			if(msCursorVisible)
			{
				msCursorVisible = false;
				msMouseHidePos = this->getGlobalMousePosition();
				((QApplication*)QApplication::instance())->setOverrideCursor(QCursor(Qt::BlankCursor));
			}
			return true;
		}

		/**
		@describe return false if already show
		@param 
		@return 
		*/
		virtual bool			show()
		{
			if(!msCursorVisible)
			{
				msCursorVisible = true;
				this->setGlobalMousePosition(msMouseHidePos);
				((QApplication*)QApplication::instance())->restoreOverrideCursor();
			}
			return true;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual POINT3I			getMouseMovement() const
		{
			return mMouseMovement;
		}

		/**
		@describe 
		@param 
		@return 
		@note: z movement is not normalized
		*/
		virtual POINT3			getNormalizedMovement() const
		{
			if(mWidget == NULL)
			{
				assert(false);
				return POINT3::ZERO;
			}

			scalar x = (scalar)mMouseMovement.x / (scalar)mWidget->size().width();
			scalar y = (scalar)mMouseMovement.y / (scalar)mWidget->size().height();
			scalar z = mMouseMovement.z;
			return POINT3(x, y, z);
		}

		/************************************************************************/
		/* IWindowMessagePump::IMessageListener                                                                     */
		/************************************************************************/
		/** @brief  return true to continue message processing,
		false to skip this message */
		virtual bool preProcessMessage(void* /*msg*/) {return true;}
		/** @brief called after processing each message */
		virtual void postProcessMessage(void* /*msg*/) {};
		/** @brief called on each loop before processing all messages */
		virtual void preProcessMessages() { mMouseMovement = POINT3I::ZERO;};
		/** @brief called on each loop after processing all messages */
		virtual void postProcessMessages() {}

	protected:
		/************************************************************************/
		/* Qt events                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt);

		typedef Set<IListener*>		ListenerList;

		QWidget*	mWidget;	//target widget

		POINT2I		mMousePos;
		POINT2I		mDownPos;	//click detecting
		MBSTATE		mButtonState[MSB_COUNT];

		POINT3I		mMouseMovement;
		ListenerList mListeners;

		static POINT2I	msMouseHidePos;
		static bool	msCursorVisible;
	};

	
}//namespace Blade


#endif // __Blade_QtMouseDevice_h__