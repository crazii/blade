/********************************************************************
	created:	2011/04/22
	filename: 	HotkeyManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "HotkeyManager.h"
#include <interface/public/ui/IUIService.h>

namespace Blade
{
	template class Factory<IHotkeyManager>;

	//////////////////////////////////////////////////////////////////////////
	HotkeyManager::HotkeyManager()
	{
		IKeyboard* keyboard = IUIService::getSingleton().getGlobalKeyboard();
		if (keyboard == NULL || Factory<IUIService>::getSingleton().getNumRegisteredClasses() == 0)
			BLADE_LOG(Warning, BTString("Hot key manager cannot work."));
		else
			IUIService::getSingleton().addKeyboardListener(this, keyboard);
	}

	//////////////////////////////////////////////////////////////////////////
	HotkeyManager::~HotkeyManager()
	{
		//UIService already un-installed
		//IUIService::getSingleton().removeKeyboardListener(this);
	}

	/************************************************************************/
	/* IHotkeyManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TString		HotkeyManager::getHotkeyString(HOTKEY hotkey)
	{
		if( hotkey.mNormalKey == KC_UNDEFINED )
			return TString::EMPTY;

		assert( hotkey.mModifier0 != KC_UNDEFINED );

		IUIService& UISvc = IUIService::getSingleton();

		if( hotkey.mModifier1 == KC_UNDEFINED )
			return UISvc.getKeyString( (EKeyCode)hotkey.mModifier0) + BTString("+") + UISvc.getKeyString( (EKeyCode)hotkey.mNormalKey);
		else
			return UISvc.getKeyString((EKeyCode)hotkey.mModifier0) + BTString("+") 
				+ UISvc.getKeyString( (EKeyCode)hotkey.mModifier1) + BTString("+") 
				+ UISvc.getKeyString( (EKeyCode)hotkey.mNormalKey);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		HotkeyManager::registerHotkey(HOTKEY hotkey, IHotkeyCommand* cmd)
	{
		if( !hotkey.isValid() || cmd == NULL )
			return false;

		std::pair<HotkeyRegistry::iterator,bool> ret = mHotKeys.insert( std::make_pair( (EKeyCode)hotkey.mNormalKey,ModifierCmdList() ) );
		if( ret.second )
		{
			ModifierCmdList& list = ret.first->second;
			for( size_t i = 0; i < list.size(); ++i )
			{
				list[i] = NULL;
			}
			//std::memset( &list[0], NULL, ModifierCmdList::SI_TOTALBYTES );
		}

		uint32 modifier = HotkeyManager::getModifierMask(hotkey);

		ModifierCmdList& list = ret.first->second;
		assert(modifier < list.size() );
		if( list[modifier] != NULL )
			return false;

		list[modifier] = cmd;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		HotkeyManager::unregisterHotkey(HOTKEY hotkey)
	{
		HotkeyRegistry::iterator i = mHotKeys.find( (EKeyCode)hotkey.mNormalKey );
		if( i == mHotKeys.end() )
			return false;

		uint32 modifier = HotkeyManager::getModifierMask(hotkey);
		ModifierCmdList& list = i->second;
		assert(modifier < list.size());

		if( list[modifier] == NULL )
			return false;
		list[modifier] = NULL;
		return true;
	}

	/************************************************************************/
	/* IKeyboard::IListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	HotkeyManager::onKeyDown(EKeyCode kc,uint32 modifierMask)
	{
		HotkeyRegistry::iterator i = mHotKeys.find( kc );
		if( i == mHotKeys.end() )
			return true;

		ModifierCmdList& list = i->second;
		assert( modifierMask < list.size() );
		IHotkeyCommand* cmd = list[modifierMask];
		if(cmd == NULL)
			return true;
		cmd->execute();

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	HotkeyManager::onKeyUp(EKeyCode /*kc*/,uint32 /*modifierMask*/)
	{
		return true;
	}

}//namespace Blade
