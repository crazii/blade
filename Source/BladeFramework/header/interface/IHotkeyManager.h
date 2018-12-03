/********************************************************************
	created:	2010/05/23
	filename: 	IHotkeyManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IHotkeyManager_h__
#define __Blade_IHotkeyManager_h__
#include <BladeFramework.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/input/IKeyboard.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	struct BLADE_FRAMEWORK_API SHotKey
	{
		uint16	mNormalKey;
		uint8	mModifier0;
		uint8	mModifier1;

		SHotKey(EKeyCode key = KC_UNDEFINED) : mNormalKey((uint16)key), mModifier0((uint8)KC_CTRL), mModifier1((uint8)KC_UNDEFINED)
		{}

		SHotKey(EKeyCode key, EKeyCode m0) : mNormalKey((uint16)key), mModifier0((uint8)m0), mModifier1((uint8)KC_UNDEFINED)
		{
			assert(mModifier0 == KC_CTRL || mModifier0 == KC_ALT || mModifier0 == KC_SHIFT);
		}

		SHotKey(EKeyCode key, EKeyCode m0, EKeyCode m1) : mNormalKey((uint16)key), mModifier0((uint8)m0), mModifier1((uint8)m1)
		{
			assert(mModifier0 == KC_CTRL || mModifier0 == KC_ALT || mModifier0 == KC_SHIFT);
			assert(mModifier1 == KC_CTRL || mModifier1 == KC_ALT || mModifier1 == KC_SHIFT);
		}

		inline bool isValid() const
		{
			return mNormalKey != KC_UNDEFINED;
		}

	};
	typedef SHotKey HOTKEY;

	class BLADE_FRAMEWORK_API IHotkeyCommand
	{
	public:
		virtual ~IHotkeyCommand() {}

		/** @brief  */
		virtual void execute() = 0;
	};

	class IHotkeyManager : public InterfaceSingleton<IHotkeyManager>
	{
	public:
		virtual ~IHotkeyManager()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual TString		getHotkeyString(HOTKEY hotkey) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		registerHotkey(HOTKEY hotkey, IHotkeyCommand* cmd) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		unregisterHotkey(HOTKEY hotkey) = 0;

	};//class IHotkeyManager

	extern template class BLADE_FRAMEWORK_API Factory<IHotkeyManager>;
	
}//namespace Blade


#endif //__Blade_IHotkeyManager_h__