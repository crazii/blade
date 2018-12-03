/********************************************************************
	created:	2016/07/30
	filename: 	QtKeyboardDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtKeyboardDevice_h__
#define __Blade_QtKeyboardDevice_h__
#include <QWidget.h>
#include <utility/BladeContainer.h>
#include <interface/public/input/IKeyboardDevice.h>

namespace Blade
{
	class QtKeyboardDevice : public IKeyboardDevice, public QObject, public Allocatable
	{
	public:
		static const TString QT_DEVICE_TYPE;
	public:
		QtKeyboardDevice();
		~QtKeyboardDevice();

		/************************************************************************/
		/* IDevice interface                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open() {return mWidget != NULL;}

		/** @brief check if it is open */
		virtual bool	isOpen() const {return mWidget != NULL;}

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset();

		/** @brief update data(I/O) */
		virtual bool	update();

		/************************************************************************/
		/* IKeyboardDevice interface                                                                     */
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
		virtual bool				addListener(IListener* listener);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				removeListener(IListener* listener);

		/************************************************************************/
		/* IKeyboard interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual EKeyState			getKeyState(EKeyCode code) const
		{
			assert(code >= KC_UNDEFINED && code < KC_CODECOUNT);
			return (EKeyState)mKeyState[code];
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool				isKeyPressed(EKeyCode code) const
		{
			assert(code >= KC_UNDEFINED && code < KC_CODECOUNT);
			return mKeyState[code] == KS_UP && mPrevKeyState[code] == KS_DOWN;
		}

		/**
		@describe 
		@param 
		@return EKeyModifier
		*/
		virtual uint32				getKeyModifier() const;

	protected:
		/************************************************************************/
		/* Qt events                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt);

		typedef Set<IListener*>		ListenerList;

		QWidget*	mWidget;
		ListenerList	mListeners;
		char		mKeyState[256];
		char		mPrevKeyState[256];
	};

	class QtGlobalKeyboardDevice : public QtKeyboardDevice, public Singleton<QtGlobalKeyboardDevice>
	{
	public:
		using Singleton<QtGlobalKeyboardDevice>::operator new;
		using Singleton<QtGlobalKeyboardDevice>::operator delete;
		using QtKeyboardDevice::eventFilter;
	};
	
}//namespace Blade


#endif // __Blade_QtKeyboardDevice_h__