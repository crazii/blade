/********************************************************************
	created:	2016/07/30
	filename: 	QtKeyboardDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtWindowDevice.h>
#include <QtKeyboardDevice.h>
#include <QtKeyMap.h>

namespace Blade
{
	const TString QtKeyboardDevice::QT_DEVICE_TYPE = BTString("QtKeyboard");

	//////////////////////////////////////////////////////////////////////////
	QtKeyboardDevice::QtKeyboardDevice()
		:mWidget(NULL)
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	QtKeyboardDevice::~QtKeyboardDevice()
	{

	}

	/************************************************************************/
	/* IDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	QtKeyboardDevice::close()
	{
		if(mWidget != NULL)
			mWidget->removeEventFilter(this);
		mWidget = NULL;
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtKeyboardDevice::reset()
	{
		::memset(mKeyState,KS_UP,sizeof(mKeyState));
		::memset(mPrevKeyState,KS_UP,sizeof(mPrevKeyState));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtKeyboardDevice::update()
	{
		return true;
	}

	/************************************************************************/
	/* IKeyboardDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				QtKeyboardDevice::attach(IWindow* window)
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
			this->reset();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool				QtKeyboardDevice::addListener(IListener* listener)
	{
		return listener != NULL && mListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				QtKeyboardDevice::removeListener(IListener* listener)
	{
		return listener != NULL && mListeners.erase(listener) == 1;
	}

	/************************************************************************/
	/* IKeyboard interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	uint32				QtKeyboardDevice::getKeyModifier() const
	{
		uint32 modifierMask = 0;
		if( this->mKeyState[ KC_CTRL ] == KS_DOWN )
			modifierMask |= KM_CTRL;
		if( this->mKeyState[ KC_ALT ] == KS_DOWN )
			modifierMask |= KM_ALT;
		if( this->mKeyState[ KC_SHIFT ] == KS_DOWN )
			modifierMask |= KM_SHIFT;

		return modifierMask;
	}
	
	/************************************************************************/
	/* Qt events                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool QtKeyboardDevice::eventFilter(QObject *watched, QEvent *evt)
	{
		QKeyEvent* keyEvt = static_cast<QKeyEvent*>(evt);
		int keycode = 0;

		switch (evt->type())
		{
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
		case QEvent::FocusIn:
		case QEvent::FocusOut:
			this->reset();
			break;
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
			keycode = keyEvt->key();
			if(!(keycode & 0xFF00))	//ignore localized keys (JPN,EOUR keys)
			{
				EKeyCode kc = QtMapKey(keyEvt->key(), keyEvt->modifiers());
				EKeyState ks = evt->type() == QEvent::KeyPress ? KS_DOWN : KS_UP;

				bool record = true;
				uint32 modifierMask = this->getKeyModifier();
				for(ListenerList::iterator i = mListeners.begin(); i != mListeners.end(); ++i)
				{
					IKeyboard::IListener* listener = *i;
					if(ks == KS_UP)
					{
						if( !listener->onKeyUp(kc, modifierMask) )
							record = false;
					}
					else
					{
						if( !listener->onKeyDown(kc, modifierMask) )
							record = false;
					}
				}

				if(record)
				{
					mPrevKeyState[kc] = mKeyState[kc];
					mKeyState[kc] = ks;
				}
				return !record;
			}
			break;
		}
		return false;
	}

}//namespace Blade