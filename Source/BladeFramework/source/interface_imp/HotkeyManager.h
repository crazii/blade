/********************************************************************
	created:	2011/04/22
	filename: 	HotkeyManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_HotkeyManager_h__
#define __Blade_HotkeyManager_h__
#include <interface/IHotkeyManager.h>
#include <interface/public/input/IKeyboard.h>
#include <utility/FixedArray.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class HotkeyManager : public IHotkeyManager , public IKeyboard::IListener, public Singleton<HotkeyManager>
	{
	public:
		HotkeyManager();
		~HotkeyManager();

		/************************************************************************/
		/* IHotkeyManager interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual TString		getHotkeyString(HOTKEY hotkey);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		registerHotkey(HOTKEY hotkey, IHotkeyCommand* cmd);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		unregisterHotkey(HOTKEY hotkey);

		/************************************************************************/
		/* IKeyboard::IListener interface                                                                     */
		/************************************************************************/
		/** @brief  */
		/* @note modifierMask: bit0 - ctrl,bit1 - alt, bit2 - shift */
		/* @remarks return value : tell keyboard device whether to record this event into key state  */
		virtual bool	onKeyDown(EKeyCode kc,uint32 modifierMask);

		/** @brief  */
		virtual bool	onKeyUp(EKeyCode kc,uint32 modifierMask);

		/** @brief  */
		virtual void	onChar(char charCode)		{	BLADE_UNREFERENCED(charCode);	}

		/** @brief  */
		virtual void	onWChar(wchar wcharCode)	{	BLADE_UNREFERENCED(wcharCode);	}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		static uint32	getModifierMask(HOTKEY hotkey)
		{
			uint32 modifier = 0;
			if( hotkey.mModifier0 == KC_CTRL || hotkey.mModifier1 == KC_CTRL )
				modifier |= 0x01;
			if( hotkey.mModifier0 == KC_ALT || hotkey.mModifier1 == KC_ALT )
				modifier |= 0x02;
			if( hotkey.mModifier0 == KC_SHIFT || hotkey.mModifier1 == KC_SHIFT )
				modifier |= 0x04;
			return modifier;
		}

	protected:
		typedef FixedArray<IHotkeyCommand*,8>		ModifierCmdList;
		typedef Map<EKeyCode,ModifierCmdList>	HotkeyRegistry;


		HotkeyRegistry	mHotKeys;
	};//class HotkeyManager
	

}//namespace Blade



#endif // __Blade_HotkeyManager_h__