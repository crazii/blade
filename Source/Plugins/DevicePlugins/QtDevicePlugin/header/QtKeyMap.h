/********************************************************************
	created:	2016/07/30
	filename: 	QtKeyMap.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtKeyMap_h__
#define __Blade_QtKeyMap_h__
#include <interface/public/input/IKeyboard.h>

namespace Blade
{

	static const EKeyCode FnKeyMap[256] = 
	{
		//00-0b
		KC_ESCAPE,				//Qt::Key_Escape
		KC_TAB,					//Qt::Key_Tab
		KC_UNDEFINED,			//Qt::Key_Backtab
		KC_BACKSPACE,			//Qt::Key_Backspace
		KC_ENTER,				//Qt::Key_Return
		KC_ENTER,				//Qt::Key_Enter
		KC_INSERT,				//Qt::Key_Insert
		KC_DELETE,				//Qt::Key_Delete
		KC_PAUSEBREAK,			//Qt::Key_Pause
		KC_PRTSCN,				//Qt::Key_Print
		KC_PRTSCN,				//Qt::Key_SysReq
		KC_ENTER,				//Qt::Key_Clear (numpad enter)
		//0c-0f
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		//10-17
		KC_HOME,				//Qt::Key_Home
		KC_END,					//Qt::Key_End
		KC_LEFT,				//Qt::Key_Left
		KC_UP,					//Qt::Key_Up
		KC_RIGHT,				//Qt::Key_Right
		KC_DOWN,				//Qt::Key_Down
		KC_PAGEUP,				//Qt::Key_PageUp
		KC_PAGEDOWN,			//Qt::Key_PageDown
		//18-1f
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		//20-26
		KC_SHIFT,				//Qt::Key_Shift
		KC_CTRL,				//Qt::Key_Control
		KC_UNDEFINED,			//Qt::Key_Meta (Win key on windows, Command key on Mac)
		KC_ALT,					//Qt::Key_Alt ( skip Qt::Key_AltGr )
		KC_CAPSLOCK,			//Qt::Key_CapsLock
		KC_NUM_LOCK,			//Qt::Key_NumLock
		KC_SCROLL_LOCK,			//Qt::Key_ScrollLock
		//27-2f
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		//30-3b
		KC_F1,					//Qt::Key_F1
		KC_F2,					//Qt::Key_F2
		KC_F3,					//Qt::Key_F3
		KC_F4,					//Qt::Key_F4
		KC_F5,					//Qt::Key_F5
		KC_F6,					//Qt::Key_F6
		KC_F7,					//Qt::Key_F7
		KC_F8,					//Qt::Key_F8
		KC_F9,					//Qt::Key_F9
		KC_F10,					//Qt::Key_F10
		KC_F11,					//Qt::Key_F11
		KC_F12,					//Qt::Key_F12
		//3c-52 Qt::Key_F13-Qt::Key_F35
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,			//Qt::Key_Super_L
		KC_UNDEFINED,			//Qt::Key_Super_R
		KC_LMENU,				//Qt::Key_Menu
		KC_UNDEFINED,			//Qt::Key_Hyper_L
		KC_UNDEFINED,			//Qt::Key_Hyper_R
		KC_UNDEFINED,			//Qt::Key_Help
		KC_UNDEFINED,			//Qt::Key_Direction_L
		KC_UNDEFINED,			//Qt::Key_Direction_R
	};

	static const EKeyCode KeyMap[256] = 
	{
		//00-1f
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,KC_UNDEFINED,
		//20-7e
		KC_SPACE,				//Qt::Key_Space
		KC_1,					//Qt::Key_Exclam	(!: shift + 1)
		KC_APOSTROPHE,			//Qt::Key_QuoteDbl	(": shift + ')
		KC_3,					//Qt::Key_NumberSign(#: shift + 3)
		KC_4,					//Qt::Key_Dollar	($: shift + 4)
		KC_5,					//Qt::Key_Percent	(%: shift + 5)
		KC_7,					//Qt::Key_Ampersand (&: shift + 7)
		KC_APOSTROPHE,			//Qt::Key_Apostrophe
		KC_9,					//Qt::Key_ParenLeft ((:	shift + 9)
		KC_0,					//Qt::Key_ParenRight():	shift + 10)
		KC_NUM_MULT,			//Qt::Key_Asterisk	(*: shift + 8)
		KC_NUM_ADD,				//Qt::Key_Plus		(+: numpad + or shift + =)
		KC_COMMA,				//Qt::Key_Comma
		KC_MINUS,				//Qt::Key_Minus
		KC_PERIOD,				//Qt::Key_Period
		KC_SLASH,				//Qt::Key_Slash
		KC_0,					//Qt::Key_0		//need Qt::KeypadModifier to determine numpad
		KC_1,					//Qt::Key_1
		KC_2,					//Qt::Key_2
		KC_3,					//Qt::Key_3
		KC_4,					//Qt::Key_4
		KC_5,					//Qt::Key_5
		KC_6,					//Qt::Key_6
		KC_7,					//Qt::Key_7
		KC_8,					//Qt::Key_8
		KC_9,					//Qt::Key_9
		KC_SEMICOLON,			//Qt::Key_Colon		: shift + ;
		KC_SEMICOLON,			//Qt::Key_Semicolon
		KC_COMMA,				//Qt::Key_Less		< shift + ,
		KC_EQUAL,				//Qt::Key_Equal
		KC_PERIOD,				//Qt::Key_Greater	> shift + .
		KC_SLASH,				//Qt::Key_Question	? shift + /
		KC_2,					//Qt::Key_At		@ sfhit + 2
		KC_A,					//Qt::Key_A
		KC_B,					//Qt::Key_B
		KC_C,					//Qt::Key_C
		KC_D,					//Qt::Key_D
		KC_E,					//Qt::Key_E
		KC_F,					//Qt::Key_F
		KC_G,					//Qt::Key_G
		KC_H,					//Qt::Key_H
		KC_I,					//Qt::Key_I
		KC_J,					//Qt::Key_J
		KC_K,					//Qt::Key_K
		KC_L,					//Qt::Key_L
		KC_M,					//Qt::Key_M
		KC_N,					//Qt::Key_N
		KC_O,					//Qt::Key_O
		KC_P,					//Qt::Key_P
		KC_Q,					//Qt::Key_Q
		KC_R,					//Qt::Key_R
		KC_S,					//Qt::Key_S
		KC_T,					//Qt::Key_T
		KC_U,					//Qt::Key_U
		KC_V,					//Qt::Key_V
		KC_W,					//Qt::Key_W
		KC_X,					//Qt::Key_X
		KC_Y,					//Qt::Key_Y
		KC_Z,					//Qt::Key_Z
		KC_LBRACKET,			//Qt::Key_BracketLeft
		KC_BACKSLASH,			//Qt::Key_Backslash
		KC_RBRACKET,			//Qt::Key_BracketRight
		KC_6,					//AsciiCircum			^ shift + 6
		KC_MINUS,				//Qt::Key_Underscore	_ shift + -
		KC_ACCENT,				//Qt::Key_QuoteLeft		`
		KC_LBRACKET,			//Qt::Key_BraceLeft		{ shift + [
		KC_BACKSLASH,			//Qt::Key_Bar			| shift + '\'
		KC_RBRACKET,			//Qt::Key_BraceRight	} shift + ]
		KC_ACCENT,				//Qt::Key_AsciiTilde	~ sfhit + `
		KC_UNDEFINED,
	};

	/** @brief do key map with num pad detection */
	static inline EKeyCode QtMapKey(int qtKey, Qt::KeyboardModifiers qtmodifier)
	{
		bool functionKey = (qtKey & 0x01000000) != 0;
		qtKey &= 0xFF;
		EKeyCode kc = functionKey ? FnKeyMap[qtKey] : KeyMap[qtKey];
		bool numpad = (qtmodifier&Qt::KeypadModifier);

		switch(kc)
		{
		case KC_NUM_DIVIDE: case KC_SLASH:
			kc = numpad ? KC_NUM_DIVIDE : KC_SLASH;
			break;
		case KC_NUM_MULT:
			kc = numpad ? KC_NUM_MULT : KC_8;
			break;
		case KC_NUM_ADD : case KC_EQUAL:
			kc = numpad ? KC_NUM_ADD : KC_EQUAL;
			break;
		case KC_MINUS : case KC_NUM_SUBTRACT:
			kc = numpad ? KC_NUM_SUBTRACT : KC_MINUS;
			break;
		case KC_NUM_ENTER: case KC_ENTER:
			kc = numpad ? KC_NUM_ENTER : KC_ENTER;
			break;
		case KC_NUM_DOT: case KC_PERIOD:
			kc = numpad ? KC_NUM_DOT : KC_PERIOD;
			break;
		case KC_0: case KC_NUM0:
			kc = numpad ? KC_NUM0 : KC_0;
			break;
		case KC_1: case KC_NUM1:
			kc = numpad ? KC_NUM1 : KC_1;
			break;
		case KC_2: case KC_NUM2:
			kc = numpad ? KC_NUM2 : KC_2;
			break;
		case KC_3: case KC_NUM3:
			kc = numpad ? KC_NUM3 : KC_3;
			break;
		case KC_4: case KC_NUM4:
			kc = numpad ? KC_NUM4 : KC_4;
			break;
		case KC_5: case KC_NUM5:
			kc = numpad ? KC_NUM5 : KC_5;
			break;
		case KC_6: case KC_NUM6:
			kc = numpad ? KC_NUM6 : KC_6;
			break;
		case KC_7: case KC_NUM7:
			kc = numpad ? KC_NUM7 : KC_7;
			break;
		case KC_8: case KC_NUM8:
			kc = numpad ? KC_NUM8 : KC_8;
			break;
		case KC_9: case KC_NUM9:
			kc = numpad ? KC_NUM9 : KC_9;
			break;
		default:
			break;
		}
		return kc;
	}
	
}//namespace Blade


#endif // __Blade_QtKeyMap_h__