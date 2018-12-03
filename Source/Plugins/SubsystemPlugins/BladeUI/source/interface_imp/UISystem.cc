/********************************************************************
	created:	2010/04/29
	filename: 	UISystem.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "UISystem.h"
#include <interface/IConfigManager.h>
#include <interface/public/window/IWindowService.h>
#include <ConfigTypes.h>
#include <BladeUI_blang.h>

namespace Blade
{
	namespace UIOptionString
	{
		static const TString OPTION_NAME = BXLang(BLANG_INPUT_SETTING);
		static const TString KEYBOARD_TYPE = BXLang(BLANG_KEYBOARD_API);
		static const TString MOUSE_TYPE = BXLang(BLANG_MOUSE_API);
		static const TString TOUCH_TYPE = BXLang(BLANG_TOUCH_API);
	
	}//namespace UIOptionString
	
	const TString UISystem::UI_SYSTEM_NAME = BTString("BladeUISystem");

	//////////////////////////////////////////////////////////////////////////
	UISystem::UISystem()
	{

		mKeyCodeDesc[KC_UNDEFINED] = BTString("UnDefined");

		mKeyCodeDesc[KC_ESCAPE] = BTString("Esc");
		mKeyCodeDesc[KC_F1] = BTString("F1");
		mKeyCodeDesc[KC_F2] = BTString("F2");
		mKeyCodeDesc[KC_F3] = BTString("F3");
		mKeyCodeDesc[KC_F4] = BTString("F4");
		mKeyCodeDesc[KC_F5] = BTString("F5");
		mKeyCodeDesc[KC_F6] = BTString("F6");
		mKeyCodeDesc[KC_F7] = BTString("F7");
		mKeyCodeDesc[KC_F8] = BTString("F8");
		mKeyCodeDesc[KC_F9] = BTString("F9");
		mKeyCodeDesc[KC_F10] = BTString("F10");
		mKeyCodeDesc[KC_F11] = BTString("F11");
		mKeyCodeDesc[KC_F12] = BTString("F12");

		mKeyCodeDesc[KC_PRTSCN] = BTString("PrntSrc");		//print screen
		mKeyCodeDesc[KC_SCROLL_LOCK] = BTString("SrcollLock");
		mKeyCodeDesc[KC_PAUSEBREAK] = BTString("PauseBreak");

		mKeyCodeDesc[KC_ACCENT] = BTString("`");
		mKeyCodeDesc[KC_1] = BTString("1");
		mKeyCodeDesc[KC_2] = BTString("2");
		mKeyCodeDesc[KC_3] = BTString("3");
		mKeyCodeDesc[KC_4] = BTString("4");
		mKeyCodeDesc[KC_5] = BTString("5");
		mKeyCodeDesc[KC_6] = BTString("6");
		mKeyCodeDesc[KC_7] = BTString("7");
		mKeyCodeDesc[KC_8] = BTString("8");
		mKeyCodeDesc[KC_9] = BTString("9");
		mKeyCodeDesc[KC_0] = BTString("0");
		mKeyCodeDesc[KC_MINUS] = BTString("-");
		mKeyCodeDesc[KC_EQUAL] = BTString("=");
		mKeyCodeDesc[KC_BACKSPACE] = BTString("BackSpacce");
		mKeyCodeDesc[KC_TAB] = BTString("Tab");
		mKeyCodeDesc[KC_Q] = BTString("Q");
		mKeyCodeDesc[KC_W] = BTString("W");
		mKeyCodeDesc[KC_E] = BTString("E");
		mKeyCodeDesc[KC_R] = BTString("R");
		mKeyCodeDesc[KC_T] = BTString("T");
		mKeyCodeDesc[KC_Y] = BTString("Y");
		mKeyCodeDesc[KC_U] = BTString("U");
		mKeyCodeDesc[KC_I] = BTString("I");
		mKeyCodeDesc[KC_O] = BTString("O");
		mKeyCodeDesc[KC_P] = BTString("P");
		mKeyCodeDesc[KC_LBRACKET] = BTString("[");	//[
		mKeyCodeDesc[KC_RBRACKET] = BTString("]");	//]
		mKeyCodeDesc[KC_BACKSLASH] = BTString("\\");	//"\"
		mKeyCodeDesc[KC_CAPSLOCK] = BTString("CapsLock");
		mKeyCodeDesc[KC_A] = BTString("A");
		mKeyCodeDesc[KC_S] = BTString("S");
		mKeyCodeDesc[KC_D] = BTString("D");
		mKeyCodeDesc[KC_F] = BTString("F");
		mKeyCodeDesc[KC_G] = BTString("G");
		mKeyCodeDesc[KC_H] = BTString("H");
		mKeyCodeDesc[KC_J] = BTString("J");
		mKeyCodeDesc[KC_K] = BTString("K");
		mKeyCodeDesc[KC_L] = BTString("L");
		mKeyCodeDesc[KC_SEMICOLON] = BTString(";");	//;
		mKeyCodeDesc[KC_APOSTROPHE] = BTString("'");	//'
		mKeyCodeDesc[KC_ENTER] = BTString("ENTER");
		mKeyCodeDesc[KC_LSHIFT] = BTString("LShift");
		mKeyCodeDesc[KC_Z] = BTString("Z");
		mKeyCodeDesc[KC_X] = BTString("X");
		mKeyCodeDesc[KC_C] = BTString("C");
		mKeyCodeDesc[KC_V] = BTString("V");
		mKeyCodeDesc[KC_B] = BTString("B");
		mKeyCodeDesc[KC_N] = BTString("N");
		mKeyCodeDesc[KC_M] = BTString("M");
		mKeyCodeDesc[KC_COMMA] = BTString(",");		//] = BTString("");
		mKeyCodeDesc[KC_PERIOD] = BTString(".");	//.
		mKeyCodeDesc[KC_SLASH] = BTString("/");		//"/"
		mKeyCodeDesc[KC_RSHIFT] = BTString("RShift");

		mKeyCodeDesc[KC_LCTRL] = BTString("LCtrl");		//left ctrl
		mKeyCodeDesc[KC_LMENU] = BTString("LMenu");
		mKeyCodeDesc[KC_LALT] = BTString("LAlt");		//left alt
		mKeyCodeDesc[KC_SPACE] = BTString("Space");
		mKeyCodeDesc[KC_RALT] = BTString("RAlt");
		mKeyCodeDesc[KC_RMENU] = BTString("RMenu");
		mKeyCodeDesc[KC_RCTRL] = BTString("RCtrl");

		mKeyCodeDesc[KC_SHIFT] = BTString("Shift");
		mKeyCodeDesc[KC_ALT] = BTString("Alt");
		mKeyCodeDesc[KC_CTRL] = BTString("Ctrl");


		mKeyCodeDesc[KC_HOME] = BTString("Home");
		mKeyCodeDesc[KC_END] = BTString("End");
		mKeyCodeDesc[KC_PAGEUP] = BTString("PageUp");
		mKeyCodeDesc[KC_PAGEDOWN] = BTString("PageDown");
		mKeyCodeDesc[KC_INSERT] = BTString("Insert");
		mKeyCodeDesc[KC_DELETE] = BTString("Delete");

		mKeyCodeDesc[KC_UP] = BTString("Up");
		mKeyCodeDesc[KC_DOWN] = BTString("Down");
		mKeyCodeDesc[KC_LEFT] = BTString("Left");
		mKeyCodeDesc[KC_RIGHT] = BTString("Right");

		mKeyCodeDesc[KC_NUM0] = BTString("NumPad 0");
		mKeyCodeDesc[KC_NUM_DOT] = BTString("NumPad .");
		mKeyCodeDesc[KC_NUM_ENTER] = BTString("NumPad Enter");
		mKeyCodeDesc[KC_NUM1] = BTString("NumPad 1");
		mKeyCodeDesc[KC_NUM2] = BTString("NumPad 2");
		mKeyCodeDesc[KC_NUM3] = BTString("NumPad 3");
		mKeyCodeDesc[KC_NUM4] = BTString("NumPad 4");
		mKeyCodeDesc[KC_NUM5] = BTString("NumPad 5");
		mKeyCodeDesc[KC_NUM6] = BTString("NumPad 6");
		mKeyCodeDesc[KC_NUM7] = BTString("NumPad 7");
		mKeyCodeDesc[KC_NUM8] = BTString("NumPad 8");
		mKeyCodeDesc[KC_NUM9] = BTString("NumPad 9");
		mKeyCodeDesc[KC_NUM_ADD] = BTString("NumPad +");		//numpad +
		mKeyCodeDesc[KC_NUM_SUBTRACT] = BTString("NumPad -");	//numpad -
		mKeyCodeDesc[KC_NUM_DIVIDE] = BTString("NumPad /");		//numpad /
		mKeyCodeDesc[KC_NUM_MULT] = BTString("NumPad *");		//numpad *
		mKeyCodeDesc[KC_NUM_LOCK] = BTString("NumLock");

		for (index_t i = 0; i < (index_t)KC_CODECOUNT; ++i)
			mDesc2KeyCode[mKeyCodeDesc[i]] = (EKeyCode)i;
	}

	//////////////////////////////////////////////////////////////////////////
	UISystem::~UISystem()
	{
	}

	/************************************************************************/
	/* ISubsystem interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const TString&	UISystem::getName()
	{
		return UI_SYSTEM_NAME;
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::install()
	{
		IWindowService::getSingleton().registerEventHandler(this);
		this->genterateOptions();
		RegisterSingleton(UISystem, IUIService);
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::uninstall()
	{
		IConfigManager::getSingletonPtr()->removeConfig(mUIOption);
		UnregisterSingleton(UISystem, IUIService);
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::initialize()
	{
		const HCONFIG& keyCfg = mUIOption->getSubConfigByName(UIOptionString::KEYBOARD_TYPE);
		if(keyCfg != NULL)
			mKeyboardType = keyCfg->getValue();
		const HCONFIG& mouseCfg = mUIOption->getSubConfigByName(UIOptionString::MOUSE_TYPE);
		if(mouseCfg != NULL)
			mMouseType = mouseCfg->getValue();
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::update()
	{
		for( InputKeyboardMap::iterator i = mKeyboardDevices.begin(); i != mKeyboardDevices.end(); ++i )
		{
			IKeyboardDevice* key = i->second;

			//try open the device if it is closed
			if( !key->isOpen() )
				key->open();
			else
				key->update();
		}

		for( InputMouseMap::iterator i = mMouseDevices.begin(); i != mMouseDevices.end(); ++i )
		{
			IMouseDevice* mouse = i->second;

			//try open the device if it is closed
			if( !mouse->isOpen() )
				mouse->open();
			else
				mouse->update();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::shutdown()
	{
		IWindowService::getSingleton().removeEventHandler(this);

		//we need close all devices,\n
		//otherwise the devices will be closed on UISystem's destructor
		//but at that time some data already becomes invalid (i.e. Win32MouseDevice::msDeviceMap)
		mMouseDevices.clear();
		mKeyboardDevices.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		UISystem::createScene(const TString& name)
	{
		BLADE_UNREFERENCED(name);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::destroyScene(const TString& name)
	{
		BLADE_UNREFERENCED(name);
	}

	//////////////////////////////////////////////////////////////////////////
	IScene*		UISystem::getScene(const TString& name) const
	{
		BLADE_UNREFERENCED(name);
		return NULL;
	}

	/************************************************************************/
	/* IUIService Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IMouse*		UISystem::createInputMouse(IWindow* window)
	{
		HMOUSEDEVICE& mouse = mMouseDevices[window];
		if( mouse != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("input mouse already created for window:")+window->getCaption() );

		mouse.bind( BLADE_FACTORY_CREATE(IMouseDevice, mMouseType) );
		mouse->attach(window);
		mouse->open();

		return mouse;
	}


	//////////////////////////////////////////////////////////////////////////
	IKeyboard*	UISystem::createInputKeyboard(IWindow* window)
	{
		HKBDDEVICE& key = mKeyboardDevices[window];
		if( key != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("input keyboard already created for window:")+window->getCaption() );

		key.bind( BLADE_FACTORY_CREATE(IKeyboardDevice, mKeyboardType) );
		key->attach(window);
		key->open();

		//attach to global listeners
		for(KeyListenerList::iterator i = mGlobalListener.begin(); i != mGlobalListener.end(); ++i )
		{
			IKeyboard::IListener* listener = *i;
			key->addListener(listener);
		}

		return key;
	}


	//////////////////////////////////////////////////////////////////////////
	IMouse*		UISystem::getInputMouse(IWindow* window) const
	{
		InputMouseMap::const_iterator i = mMouseDevices.find(window);
		if( i != mMouseDevices.end() )
			return i->second;
		else
			return NULL;
	}


	//////////////////////////////////////////////////////////////////////////
	IKeyboard*	UISystem::getInputKeyboard(IWindow* window) const
	{
		InputKeyboardMap::const_iterator i = mKeyboardDevices.find(window);
		if( i != mKeyboardDevices.end() )
			return i->second;
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	UISystem::getKeyString(EKeyCode code) const
	{
		if( code >= KC_UNDEFINED && code < KC_CODECOUNT )
			return mKeyCodeDesc[code];
		else
			return mKeyCodeDesc[KC_UNDEFINED];
	}

	//////////////////////////////////////////////////////////////////////////
	EKeyCode		UISystem::getKeyFromString(const TString& keyString) const
	{
		DescKeyMap::const_iterator i = mDesc2KeyCode.find(keyString);
		return i != mDesc2KeyCode.end() ? i->second : KC_UNDEFINED;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UISystem::addKeyboardListener(IKeyboard::IListener* listener,IKeyboard* pKeyboard/* = NULL*/)
	{
		if( listener == NULL )
			return false;

		if( pKeyboard == NULL )
		{
			bool ret = mGlobalListener.insert(listener).second;
			if( ret )
			{
				for( InputKeyboardMap::iterator i = mKeyboardDevices.begin(); i != mKeyboardDevices.end(); ++i )
				{
					IKeyboardDevice* kbDev = i->second;
					kbDev->addListener(listener);
				}
			}
			return ret;
		}
		else
		{
			IKeyboardDevice* dev = static_cast<IKeyboardDevice*>(pKeyboard);

			KeyBoardList& listenedKbs = mKeyboardListenerMap[listener];
			bool ret = listenedKbs.insert(dev).second;
			if( ret )
				dev->addListener(listener);

			return ret;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	bool			UISystem::removeKeyboardListener(IKeyboard::IListener* listener,IKeyboard* pKeyboard/* = NULL*/)
	{
		if( listener == NULL )
			return false;

		KeyListenerList::iterator i = mGlobalListener.find( listener );
		if( i != mGlobalListener.end() )
		{
			mGlobalListener.erase(i);
			for( InputKeyboardMap::iterator iter = mKeyboardDevices.begin(); iter != mKeyboardDevices.end(); ++iter )
			{
				IKeyboardDevice* kbDev = iter->second;
				kbDev->removeListener(listener);
			}

			return true;
		}

		KeyListenerMap::iterator n = mKeyboardListenerMap.find(listener);
		if( n != mKeyboardListenerMap.end() )
		{
			KeyBoardList& listenedKbs = n->second;

			if( pKeyboard == NULL )
			{
				for(KeyBoardList::iterator l = listenedKbs.begin(); l != listenedKbs.end(); ++l )
				{
					IKeyboardDevice* dev = *l;
					dev->removeListener(listener);
				}

				mKeyboardListenerMap.erase(n);
				return true;
			}
			else
			{
				IKeyboardDevice* kbdDev = static_cast<IKeyboardDevice*>(pKeyboard);
				bool ret = listenedKbs.erase(kbdDev) == 1;
				if( ret )
				{
					kbdDev->removeListener(listener);

					if( listenedKbs.size() == 0 )
						mKeyboardListenerMap.erase(n);
				}
				return ret;
			}
		}

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UISystem::addMouseListener(IMouse::IListener* listener,IMouse* mouse)
	{
		if(mouse == NULL || listener == NULL )
			return false;

		IMouseDevice* mouseDev = static_cast<IMouseDevice*>(mouse);
		MouseList& list = mMouseListenerMap[listener];
		bool ret = list.insert(mouseDev).second;
		if( ret )
			mouseDev->addListener(listener);

		return ret;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			UISystem::removeMouseListener(IMouse::IListener* listener,IMouse* mouse/* = NULL*/)
	{
		if( listener == NULL/* || mouse == NULL */)
			return false;

		MouseListenerMap::iterator i = mMouseListenerMap.find(listener);
		if( i == mMouseListenerMap.end() )
			return false;

		MouseList& list = i->second;
		if( mouse == NULL )
		{
			for(MouseList::iterator n = list.begin(); n != list.end(); ++n )
			{
				IMouseDevice* mouseDev = *n;
				mouseDev->removeListener(listener);
			}
			mMouseListenerMap.erase(i);
			return true;
		}
		else
		{
			IMouseDevice* mouseDev = static_cast<IMouseDevice*>(mouse);
			bool ret = list.erase(mouseDev) == 1;
			if( ret )
			{
				mouseDev->removeListener(listener);
				if( list.size() == 0 )
					mMouseListenerMap.erase(i);
			}
			return ret;
		}

	}

	//////////////////////////////////////////////////////////////////////////
	void			UISystem::setInputTypeOverride(const TString* keyboardType, const TString* mouseType)
	{
		if(keyboardType != NULL && !keyboardType->empty())
		{
			if(!KeyboardDeviceFactory::getSingleton().isClassRegistered(*keyboardType))
				assert(false);
			else
				mKeyboardType = *keyboardType;
		}

		if(mouseType != NULL && !mouseType->empty())
		{
			if(!MouseDeviceFactory::getSingleton().isClassRegistered(*mouseType))
				assert(false);
			else
				mMouseType = *mouseType;
		}
	}

	/************************************************************************/
	/* IWindowEventHandler interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				UISystem::onWindowResized(IWindow* pwin)
	{
		InputMouseMap::iterator i = mMouseDevices.find(pwin);
		if( i != mMouseDevices.end() )
		{
			IMouseDevice* mouse = i->second;
			mouse->notifyWindowSize(pwin->getWidth(),pwin->getHeight());
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				UISystem::onWindowDestroy(IWindow* pwin)
	{
		{
			InputMouseMap::iterator i = mMouseDevices.find(pwin);
			if( i != mMouseDevices.end() )
			{
				const HMOUSEDEVICE& hMouse = i->second;
				for( MouseListenerMap::iterator n = mMouseListenerMap.begin(); n != mMouseListenerMap.end(); ++n)
				{
					MouseList& list = n->second;
					list.erase(hMouse);
				}
				mMouseDevices.erase(i);
			}
		}

		{
			InputKeyboardMap::iterator i = mKeyboardDevices.find(pwin);
			if( i != mKeyboardDevices.end() )
			{
				const HKBDDEVICE& hKey = i->second;
				for(KeyListenerMap::iterator n = mKeyboardListenerMap.begin(); n != mKeyboardListenerMap.end(); ++n)
				{
					KeyBoardList& list = n->second;
					list.erase(hKey);
				}

				mKeyboardDevices.erase(pwin);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		UISystem::onWindowActivate(IWindow* /*pwin*/)
	{

	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				UISystem::genterateOptions()
	{
		if( mUIOption != NULL )
			return;

		ConfigGroup* pUIOption = BLADE_NEW ConfigGroup(UIOptionString::OPTION_NAME, CAF_NORMAL_DEVMODE);

		//main API
		TStringParam list;
		for( size_t i = 0; i < MouseDeviceFactory::getSingleton().getNumRegisteredClasses(); ++i )
		{
			const TString& name = MouseDeviceFactory::getSingleton().getRegisteredClass(i);
			list.push_back(name);
		}

		ConfigGroup* group = NULL;
		if( list.size() > 0 )
		{
			group = BLADE_NEW ConfigGroup(UIOptionString::MOUSE_TYPE, list );
			pUIOption->addSubConfigPtr( group );
		}

		list.clear();
		for( size_t i = 0; i < KeyboardDeviceFactory::getSingleton().getNumRegisteredClasses(); ++i )
		{
			const TString& name = KeyboardDeviceFactory::getSingleton().getRegisteredClass(i);
				list.push_back(name);
		}
		if( list.size() > 0 )
		{
			group = BLADE_NEW ConfigGroup(UIOptionString::KEYBOARD_TYPE, list);
			pUIOption->addSubConfigPtr( group );
		}

		list.clear();
		for( size_t i = 0; i < TouchDeviceFactory::getSingleton().getNumRegisteredClasses(); ++i )
		{
			const TString& name = TouchDeviceFactory::getSingleton().getRegisteredClass(i);
			list.push_back(name);
		}
		if( list.size() > 0 )
		{
			group = BLADE_NEW ConfigGroup(UIOptionString::TOUCH_TYPE, list);
			pUIOption->addSubConfigPtr( group );
		}

		mUIOption.bind(pUIOption);
		IConfigManager::getSingleton().addConfig(mUIOption);
	}


}//namespace Blade