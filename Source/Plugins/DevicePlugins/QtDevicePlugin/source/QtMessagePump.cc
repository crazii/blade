/********************************************************************
	created:	2016/07/30
	filename: 	QtMessagePump.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <utility/Profiling.h>
#include <QtWidgets/QApplication>
#include <QtMessagePump.h>
#include <QtKeyboardDevice.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QtMessagePump::QtMessagePump()
	{
		QApplication::instance()->installEventFilter(this);
	}

	//////////////////////////////////////////////////////////////////////////
	QtMessagePump::~QtMessagePump()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IKeyboardDevice* QtMessagePump::getGlobalKeyboard() const
	{
		return QtGlobalKeyboardDevice::getSingletonPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtMessagePump::processMessage()
	{
		std::for_each(mMessageListeners.begin(),mMessageListeners.end(), std::mem_fun(&IMessageListener::preProcessMessages) );

		{
			//BLADE_LW_PROFILING(UPDATE);
			QApplication::instance()->processEvents(QEventLoop::AllEvents, 2);
		}

		std::for_each(mMessageListeners.begin(),mMessageListeners.end(), std::mem_fun(&IMessageListener::postProcessMessages) );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtMessagePump::addMessageListener(IMessageListener* listener)
	{
		return listener != NULL && mMessageListeners.insert(listener).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtMessagePump::removeMessageListener(IMessageListener* listener)
	{
		return listener != NULL && mMessageListeners.erase(listener) == 1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool QtMessagePump::eventFilter(QObject *watched, QEvent *evt)
	{
		//we don't use Qt shortcut, use IHotKeyManager instead
		//@see QtMenu::addSubMenu, which doesn't add shortcut but only changes text
		if (evt->type() == QEvent::Shortcut)
		{
			QShortcutEvent* sc = static_cast<QShortcutEvent*>(evt);
			const QKeySequence& seq = sc->key();
			int c = seq.count();
			//we want ALT+'KEY' for drop down menus in menu bar
			//ALT+'KEY' the key down of 'KEY' cannot be triggered from Qt if bound to an menu, and register it to IHotkeyManager won't trigger
			//so have to use Qt.
			if(c > 1 || (seq[0]&Qt::MODIFIER_MASK) != Qt::ALT)
				return true;
		}

		return QtGlobalKeyboardDevice::getSingleton().eventFilter(watched, evt);
	}

}//namespace Blade