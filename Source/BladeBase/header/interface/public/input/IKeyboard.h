/********************************************************************
	created:	2010/04/30
	filename: 	IKeyboard.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IKeyboard_h__
#define __Blade_IKeyboard_h__
#include <BladeTypes.h>

namespace Blade
{
	enum EKeyCode
	{
		KC_UNDEFINED = 0,

		KC_ESCAPE,
		KC_F1,
		KC_F2,
		KC_F3,
		KC_F4,
		KC_F5,
		KC_F6,
		KC_F7,
		KC_F8,
		KC_F9,
		KC_F10,
		KC_F11,
		KC_F12,

		KC_PRTSCN,		//print screen
		KC_SCROLL_LOCK,
		KC_PAUSEBREAK,

		KC_ACCENT,	//Grave accent: `
		KC_1,
		KC_2,
		KC_3,
		KC_4,
		KC_5,
		KC_6,
		KC_7,
		KC_8,
		KC_9,
		KC_0,
		KC_MINUS,
		KC_EQUAL,
		KC_BACKSPACE,
		KC_TAB,
		KC_Q,
		KC_W,
		KC_E,
		KC_R,
		KC_T,
		KC_Y,
		KC_U,
		KC_I,
		KC_O,
		KC_P,
		KC_LBRACKET,	//[
		KC_RBRACKET,	//]
		KC_BACKSLASH,	//"\"
		KC_CAPSLOCK,
		KC_A,
		KC_S,
		KC_D,
		KC_F,
		KC_G,
		KC_H,
		KC_J,
		KC_K,
		KC_L,
		KC_SEMICOLON,	//;
		KC_APOSTROPHE,	//'
		KC_ENTER,
		KC_LSHIFT,
		KC_Z,
		KC_X,
		KC_C,
		KC_V,
		KC_B,
		KC_N,
		KC_M,
		KC_COMMA,		//,
		KC_PERIOD,		//.
		KC_SLASH,		//"/"
		KC_RSHIFT,
		
		KC_LCTRL,		//left ctrl
		KC_LMENU,
		KC_LALT,		//left alt
		KC_SPACE,
		KC_RALT,
		KC_RMENU,
		KC_RCTRL,

		KC_SHIFT,
		KC_ALT,
		KC_CTRL,


		KC_HOME,
		KC_END,
		KC_PAGEUP,
		KC_PAGEDOWN,
		KC_INSERT,
		KC_DELETE,

		KC_UP,
		KC_DOWN,
		KC_LEFT,
		KC_RIGHT,

		KC_NUM0,
		KC_NUM_DOT,
		KC_NUM_ENTER,
		KC_NUM1,
		KC_NUM2,
		KC_NUM3,
		KC_NUM4,
		KC_NUM5,
		KC_NUM6,
		KC_NUM7,
		KC_NUM8,
		KC_NUM9,
		KC_NUM_ADD,			//numpad +
		KC_NUM_SUBTRACT,	//numpad -
		KC_NUM_DIVIDE,		//numpad /
		KC_NUM_MULT,		//numpad *
		KC_NUM_LOCK,


		KC_CODECOUNT,
		KC_FORCEWORD = 0xFFFFFFFF,
	};

	enum EKeyState
	{
		KS_UP,
		KS_DOWN,
	};

	enum EKeyModifier
	{
		KM_CTRL	= 0x01,
		KM_ALT	= 0x02,
		KM_SHIFT= 0x04,
	};


	//////////////////////////////////////////////////////////////////////////
	class IKeyboard
	{
	public:
		class IListener
		{
		public:

			/** @brief  */
			/* @note modifierMask: bit0 - ctrl,bit1 - alt, bit2 - shift */
			/* @see enum EKeyModifier */
			/* @remarks return value : tell keyboard device whether to record this event into key state  */
			virtual bool	onKeyDown(EKeyCode kc,uint32 modifierMask) = 0;

			/** @brief  */
			virtual bool	onKeyUp(EKeyCode kc,uint32 modifierMask) = 0;

			/** @brief  */
			virtual void	onChar(char charCode) = 0;

			/** @brief  */
			virtual void	onWChar(wchar wcharCode) = 0;
		};

	public:
		virtual ~IKeyboard()	{}

		/**
		@describe
		@param
		@return
		*/
		virtual EKeyState			getKeyState(EKeyCode code) const = 0;

		/** @brief  */
		inline bool		isKeyDown(EKeyCode code) const
		{
			return this->getKeyState(code) == KS_DOWN;
		}

		/** @brief  */
		inline bool		isKeyUp(EKeyCode code) const
		{
			return this->getKeyState(code) == KS_UP;
		}

		/**
		@describe key pressed. key down is a state, press here is a short event after key down & up
		@param
		@return
		*/
		virtual bool				isKeyPressed(EKeyCode code) const = 0;

		/**
		@describe 
		@param 
		@return EKeyModifier
		*/
		virtual uint32				getKeyModifier() const = 0;

	};//class IKeyboard
	
}//namespace Blade


#endif //__Blade_IKeyboard_h__