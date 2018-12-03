/********************************************************************
	created:	2010/04/29
	filename: 	UISystem.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UISystem_h__
#define __Blade_UISystem_h__
#include <interface/public/window/IWindowEventHandler.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <interface/public/input/IMouseDevice.h>
#include <interface/public/input/IKeyboardDevice.h>
#include <interface/public/input/ITouchDevice.h>

#include <interface/IUISystem.h>
#include <interface/IConfig.h>
#include <utility/BladeContainer.h>
#include <utility/FixedArray.h>

namespace Blade
{
	class UISystem : public IUISystem , public IWindowEventHandler ,public Singleton<UISystem>
	{
	public:
		static const TString UI_SYSTEM_NAME;
	public:
		UISystem();
		~UISystem();

		/************************************************************************/
		/* ISubsystem interface                                                                     */
		/************************************************************************/

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getName();


		/*
		@describe this would be called when added to Framework
		@param 
		@return 
		*/
		virtual void		install();

		/*
		@describe
		@param
		@return
		*/
		virtual void		uninstall();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		initialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		update();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		shutdown();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		createScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		destroyScene(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IScene*		getScene(const TString& name) const;

		/************************************************************************/
		/* IUIService Interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual IKeyboard*	getGlobalKeyboard() const {return IWindowMessagePump::getSingleton().getGlobalKeyboard();}
		/*
		@describe
		@param
		@return
		*/
		virtual IMouse*		createInputMouse(IWindow* window);

		/*
		@describe
		@param
		@return
		*/
		virtual IKeyboard*	createInputKeyboard(IWindow* window);

		/*
		@describe
		@param
		@return
		*/
		virtual IMouse*		getInputMouse(IWindow* window) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IKeyboard*	getInputKeyboard(IWindow* window) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getKeyString(EKeyCode code) const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual EKeyCode		getKeyFromString(const TString& keyString) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addKeyboardListener(IKeyboard::IListener* listener,IKeyboard* pKeyboard = NULL);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeKeyboardListener(IKeyboard::IListener* listener,IKeyboard* pKeyboard = NULL);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addMouseListener(IMouse::IListener* listener,IMouse* mouse);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			removeMouseListener(IMouse::IListener* listener,IMouse* mouse = NULL);

		/**
		@describe 
		@param
		@return
		*/
		virtual void			setInputTypeOverride(const TString* keyboardType, const TString* mouseType);

		/************************************************************************/
		/* IWindowEventHandler interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return
		@remark pwin 's width and pwin 's height is 0 indicates it is minimized
		*/
		virtual void		onWindowResized(IWindow* pwin);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		onWindowDestroy(IWindow* pwin);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		onWindowActivate(IWindow* pwin);

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void				genterateOptions();

		typedef		Map<IWindow*,HMOUSEDEVICE>	InputMouseMap;
		typedef		Map<IWindow*,HKBDDEVICE>	InputKeyboardMap;
		typedef		FixedArray<TString,KC_CODECOUNT>	KeyDescMap;
		typedef		Map<TString,EKeyCode>				DescKeyMap;

		typedef		Set<IKeyboardDevice*>		KeyBoardList;
		typedef		Map<IKeyboard::IListener*,	KeyBoardList>	KeyListenerMap;
		typedef		Set<IKeyboard::IListener*>	KeyListenerList;

		typedef		Set<IMouseDevice*>			MouseList;
		typedef		Map<IMouse::IListener*,		MouseList>	MouseListenerMap;
		
		InputMouseMap		mMouseDevices;
		InputKeyboardMap	mKeyboardDevices;
		KeyDescMap			mKeyCodeDesc;
		DescKeyMap			mDesc2KeyCode;
		KeyListenerMap		mKeyboardListenerMap;
		KeyListenerList		mGlobalListener;
		MouseListenerMap	mMouseListenerMap;
		HCONFIG				mUIOption;
		TString				mKeyboardType;
		TString				mMouseType;
	};//class UISystem
	
}//namespace Blade


#endif //__Blade_UISystem_h__