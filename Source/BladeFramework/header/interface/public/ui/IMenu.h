/********************************************************************
	created:	2010/05/23
	filename: 	IMenu.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IMenu_h__
#define __Blade_IMenu_h__
#include <BladeFramework.h>
#include <utility/StringList.h>
#include <interface/IHotkeyManager.h>
#include <interface/IConfig.h>
#include "UITypes.h"
#include "IUICommand.h"

namespace Blade
{
	enum EUIMenuFlags
	{
		UIMF_NONE		= 0,
		UIMF_ENABLE		= 0x0001,
		UIMF_CHECKABLE	= 0x0002,
		UIMF_CHECKED	= 0x0004,
		UIMF_EMBEDDED	= 0x0008,	//menu is embedded. any children are attached to its parent. UIMF_SUBITEM, UIMF_SEPARATOR are always embedded
		UIMF_3DOT		= 0x0010,	//menu text has "..." suffix

		UIMF_TYPEMASK	= 0x0F00,
		UIMF_SUBITEM	= 0x0100,	//menu is an leaf sub item
		UIMF_GROUP		= 0x0200,	//menu is a group menu with radio items
		UIMF_SEPARATOR  = 0x0400,
		UIMF_EXTRA		= 0x0800,	//extra created menu
	};

	//////////////////////////////////////////////////////////////////////////
	typedef struct SMenuData
	{
		TString		mText;		//localized text with access key, hot key text, usually should left empty to get auto generated
		index_t		mIndex;		//index in parent if a sub menu
		HOTKEY		mHotKey;	//accelerator/hot key (shortcut)
		tchar		mAccKey;	//access key / mnemonic 
		IconIndex	mIcon;
		uint		mFlags;		//EUIMenuFlags
	}MENU_DATA;

	class IMenu : public ICommandUI
	{
	public:
		///auto-generating access key
		static const tchar AUTO_KEY = tchar(-1);

	public:
		virtual ~IMenu()	{}

		/** @brief  get pointer to one data */
		virtual const MENU_DATA*getMenuData() const = 0;

		/** @brief */
		virtual IMenu*		getParent() const = 0;

		/** @brief  */
		virtual IUICommand*	getBindingCommand() const = 0;

		/** @brief  */
		virtual size_t		getSubMenuCount() const = 0;

		/** @brief  */
		virtual IMenu*		getSubMenu(index_t index) const = 0;

		/** @brief  */
		virtual IMenu*		getSubMenu(const TString& name) const = 0;

		/** @brief  */
		/* @note: if the a subitem added into this menu, its original command and hot key become invalid */
		virtual IMenu*		addSubMenu(const TString& name, MENU_DATA& data, IUICommand* command = NULL) = 0;

		/** @brief  */
		virtual bool		removeSubMenu(index_t index) = 0;

		/** @brief  */
		virtual bool		removeAllMenus() = 0;

		/** @brief find menu in all descendants */
		inline IMenu*		findMenu(const TString& name) const
		{
			IMenu* menu = this->getSubMenu(name);
			if( menu != NULL )
				return menu;
			else
			{
				for(size_t i = 0; i < this->getSubMenuCount(); ++i)
				{
					menu = this->getSubMenu(i)->findMenu(name);
					if( menu != NULL )
						return menu;
				}
			}
			return NULL;
		}

		/** @brief  */
		inline IMenu*		getCheckedSubMenu()
		{
			size_t count = this->getSubMenuCount();
			for (size_t i = 0; i < count; ++i)
			{
				IMenu* sub = this->getSubMenu(i);
				if (sub->isChecked())
					return sub;
			}
			return NULL;
		}

		/** @brief  */
		inline HOTKEY		getHotKey() const { return this->getMenuData()->mHotKey; }

		/** @brief  */
		inline tchar		getAccessKey() const { return this->getMenuData()->mAccKey; }

		/** @brief  */
		inline IconIndex	getIconIndex() const { return this->getMenuData()->mIcon; }

		/** @brief  */
		inline bool			isEnabled() const { return (this->getMenuData()->mFlags&UIMF_ENABLE); }

		/** @brief  */
		inline bool			isCheckable() const	{ return (this->getMenuData()->mFlags&UIMF_CHECKABLE) != 0; }

		/** @brief  */
		inline bool			isChecked() const { return (this->getMenuData()->mFlags&UIMF_CHECKABLE) && (this->getMenuData()->mFlags&UIMF_CHECKED); }

		/** @brief  */
		inline bool			isCheckable(bool& checked) const
		{
			uint flags = this->getMenuData()->mFlags;
			bool ret = (flags&UIMF_CHECKABLE) != 0;
			checked = ret && (flags&UIMF_CHECKED) != 0;
			return ret;
		}

		/** @brief  */
		inline bool			isEmbedded() const { return (this->getMenuData()->mFlags&UIMF_EMBEDDED) != 0; }

		/** @brief  */
		inline bool			isGrouped() const	{ return (this->getMenuData()->mFlags&UIMF_GROUP) != 0; }

		/** @brief  */
		inline bool			isSubItem() const { return (this->getMenuData()->mFlags&UIMF_SUBITEM) != 0; }

		/** @brief  */
		inline bool			isSeparator() const { return (this->getMenuData()->mFlags&UIMF_SEPARATOR) != 0; }

		/** @brief  */
		inline IMenu*		addSubMenu(const TString& name, IconIndex icon = INVALID_ICON_INDEX, tchar accKey = AUTO_KEY, uint flags = UIMF_ENABLE)
		{
			MENU_DATA data;
			data.mIcon = icon;
			data.mFlags = flags;
			data.mAccKey = accKey;
			data.mHotKey = KC_UNDEFINED;
			return this->addSubMenu(name, data, NULL);
		}

		/** @brief  */
		inline IMenu*		addSubMenu(const TString& name, IUICommand* command)
		{
			MENU_DATA data;
			data.mIcon = INVALID_ICON_INDEX;
			data.mFlags = UIMF_ENABLE;
			data.mAccKey = AUTO_KEY;
			data.mHotKey = KC_UNDEFINED;
			return this->addSubMenu(name, data, command);
		}

		/** @brief  */
		inline IMenu*		addItem(const TString& name, IUICommand* command,
			IconIndex iconIndex = INVALID_ICON_INDEX, bool append3Dot = false, tchar accKey = AUTO_KEY, HOTKEY hotkey = KC_UNDEFINED)
		{
			MENU_DATA data;
			data.mIcon = iconIndex;
			data.mFlags = UIMF_SUBITEM | (append3Dot ? UIMF_3DOT : 0u) | UIMF_ENABLE;
			data.mAccKey = accKey;
			data.mHotKey = hotkey;
			return this->addSubMenu(name, data, command);
		}

		/** @brief  */
		inline IMenu*		addCheckItem(const TString& name, IUICommand* command, bool checked,
			IconIndex iconIndex = INVALID_ICON_INDEX, tchar accKey = AUTO_KEY, HOTKEY hotkey = KC_UNDEFINED)
		{
			MENU_DATA data;
			data.mIcon = iconIndex;
			data.mFlags = UIMF_SUBITEM | UIMF_CHECKABLE | UIMF_ENABLE | (checked ? UIMF_CHECKED : 0u);
			data.mAccKey = accKey;
			data.mHotKey = hotkey;
			return this->addSubMenu(name, data, command);
		}

		/** @brief  */
		inline IMenu*		addSubMenu(IConfig* config, IUICommand* subCmd, IconIndex icon = INVALID_ICON_INDEX, tchar accKey = AUTO_KEY,
			const IconIndex* iconIndex = NULL, const HOTKEY* hotKeys = NULL)
		{
			//config must not be a group config
			//and must be a list index list
			if (config == NULL || (config->getUIHint() != CUIH_INDEX && config->getUIHint() != CUIH_LIST) || config->getSubConfigCount() > 0)
				return NULL;

			config->updateData(false, true);

			const TString& name = config->getName();
			
			IMenu* menu = this->addSubMenu(name, icon, accKey, UIMF_ENABLE|UIMF_GROUP);

			const TStringList& options = config->getOptions();

			index_t cfgIndex = INVALID_INDEX;
			for (cfgIndex = 0; cfgIndex < options.size(); ++cfgIndex)
			{
				if (config->getValue() == options[cfgIndex])
					break;
			}
			assert(cfgIndex != INVALID_INDEX);
			
			for (size_t i = 0; i < config->getOptions().size(); ++i)
			{
				IconIndex subIcon = iconIndex != NULL ? iconIndex[i] : INVALID_ICON_INDEX;
				HOTKEY subHotkey = hotKeys != NULL ? hotKeys[i] : KC_UNDEFINED;
				menu->addCheckItem(options[i], subCmd, cfgIndex == i, subIcon, AUTO_KEY, subHotkey);
			}
			return menu;
		}


		/** @brief  */
		inline bool			addSeparator()
		{
			if ((this->getMenuData()->mFlags&UIMF_TYPEMASK) == UIMF_EMBEDDED)
				return false;
			MENU_DATA data;
			data.mFlags = UIMF_SEPARATOR;
			this->addSubMenu(TString::EMPTY, data);
			return true;
		}

	};//class IMenu

	
}//namespace Blade


#endif //__Blade_IMenu_h__