/********************************************************************
	created:	2011/04/11
	filename: 	Menu.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ConfigDialog/ConfigControlUtil.h>
#include <ui/Menu.h>
#include <ui/IconManager.h>
#include <ui/MenuManager.h>
#include <ui/private/UIMenu.h>


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Menu::Menu(const TString& name, const MENU_DATA& data, IUICommand* cmd, Menu* parent, CUIMenu* parentCMenu, bool topLevel/* = false*/)
		:mName(name)
		,mData(data)
		,mCommand(cmd)
		,mParent(parent)
		,mCMenu(NULL)
		,mCMenuParent(parentCMenu)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		UINT Flags = MF_BYPOSITION;

#if !BLADE_USE_MFC_FEATURE_PACK
		Flags |= MF_OWNERDRAW;
		ULONG_PTR menuData = (ULONG_PTR)static_cast<IMenu*>(this);
#else
		ULONG_PTR menuData = (ULONG_PTR)mData.mText.c_str();
#endif

		if ((mData.mFlags&UIMF_SEPARATOR))
		{
			mData.mFlags |= UIMF_EMBEDDED;
			mCMenuParent->AppendMenu(Flags | MF_SEPARATOR, 0, LPCTSTR(NULL));
			return;
		}
		
		if (this->isEnabled())
			Flags |= MF_ENABLED;
		else
			Flags |= MF_DISABLED | MF_GRAYED;

		if (this->isChecked())
			Flags |= MF_CHECKED;
		else
			Flags |= MF_UNCHECKED;

		mID = INVALID_ID;
		if (mCommand != NULL)
			mID = MenuManager::getSingleton().getCommandID(mCommand);

		if (!this->isEmbedded())
		{
			CUIMenu* uiMenu = BLADE_NEW CUIMenu();
			uiMenu->CreatePopupMenu();
			uiMenu->setMenu(this);
			mCMenu = uiMenu;

			if (topLevel)
			{
				assert(parentCMenu != NULL);
				parentCMenu->AppendMenu(MF_POPUP | Flags, (UINT_PTR)mCMenu->m_hMenu, mData.mText.c_str());
			}
			else if (parentCMenu != NULL)
				parentCMenu->AppendMenu(MF_POPUP | Flags, (UINT_PTR)mCMenu->m_hMenu, (LPCTSTR)menuData);
		}
		else if(this->isSubItem())
		{
			assert(parentCMenu != NULL);
			CUIMenu* uiMenuParent = static_cast<CUIMenu*>(mCMenuParent);

			//embedded subitem
			if (mParent != NULL && mParent->isEmbedded())
			{
				mCMenuParent->InsertMenu((UINT)(mParent->mData.mIndex + mData.mIndex), Flags, mID, (LPCTSTR)menuData);
				uiMenuParent->setItem((UINT)(mParent->mData.mIndex + mData.mIndex), this);
			}
			else
			{
				mCMenuParent->AppendMenu(MF_STRING | Flags, mID, (LPCTSTR)menuData);
				uiMenuParent->setItem((UINT)mData.mIndex, this);
			}

			if(mData.mHotKey.isValid())
				IHotkeyManager::getSingleton().registerHotkey(mData.mHotKey, this);
		}
		else
		{
			//embedded menu / group menu
			//nothing to do
			assert(mCMenuParent != NULL);
		}

		if(parentCMenu != NULL)
			parentCMenu->setItem((UINT)data.mIndex, this);

#if ENABLE_THEME
		if(mCMenu != NULL )
		{
			MENUINFO info;
			info.cbSize = sizeof(MENUINFO);
			info.fMask = MIM_BACKGROUND | MIM_STYLE;
			mCMenu->GetMenuInfo(&info);

			info.hbrBack = THEMEBRUSH(TC_MENUBK);
			info.dwStyle |= MIM_APPLYTOSUBMENUS;
			mCMenu->SetMenuInfo(&info);
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	Menu::~Menu()
	{
		for(MenuList::iterator i = mSubmenus.begin(); i != mSubmenus.end(); ++i)
		{
			IMenu* menu = *i;
			BLADE_DELETE menu;
		}
		BLADE_DELETE mCMenu;

		if (this->isSubItem() && mData.mHotKey.isValid())
			IHotkeyManager::getSingleton().unregisterHotkey(mData.mHotKey);
	}

	/************************************************************************/
	/* ICommandUI interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		Menu::setChecked(bool checked)
	{
		if (this->isCheckable())
		{
			if (this->isChecked() == checked)
				return;

			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			/* @note use BYCOMMAND because the menu bar will be insert menu item, \n
			i.e. when MDI child window maximized */
			UINT Flags = MF_BYPOSITION;

			if (checked)
			{
				mData.mFlags |= UIMF_CHECKED;
				Flags |= MF_CHECKED;
			}
			else
			{
				Flags |= MF_UNCHECKED;
				mData.mFlags &= ~UIMF_CHECKED;
			}

			index_t index = mParent != NULL && mParent->isEmbedded() ? mParent->mData.mIndex + mData.mIndex : mData.mIndex;
			if (checked && this->isSubItem() && this->isGrouped())
			{
				for (size_t i = 0; i < mParent->getSubMenuCount(); ++i)
				{
					IMenu* sibling = mParent->getSubMenu(i);
					if(sibling->isSeparator() || sibling == this)
						continue;
					sibling->setChecked(false);
				}
			}
			mCMenuParent->CheckMenuItem((UINT)index, Flags);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void		Menu::setEnabled(bool enable)
	{
		if (this->isEnabled() == enable)
			return;

		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		UINT flags = MF_BYPOSITION;

		if (enable)
		{
			mData.mFlags |= UIMF_ENABLE;
			flags |= MF_ENABLED;
		}
		else
		{
			mData.mFlags &= ~UIMF_ENABLE;
			flags |= MF_DISABLED | MF_GRAYED;
		}

		//gray text refresh
		if (::AfxGetMainWnd() != NULL && ::AfxGetMainWnd()->GetMenu() == mCMenuParent)
		{
			/* @note find the right index because the menu bar will be inserted with new menu item, \n
			i.e. when MDI child window maximized */
			int index = -1;
			for (int i = 0; i < mCMenuParent->GetMenuItemCount(); ++i)
			{
				if (mCMenuParent->GetSubMenu(i) == mCMenu)
				{
					index = i;
					break;
				}
			}
			assert(index != -1);
			mCMenuParent->EnableMenuItem((UINT)index, flags);
			::AfxGetMainWnd()->DrawMenuBar();
		}
		else
			mCMenuParent->EnableMenuItem((UINT)mData.mIndex, flags);
	}

	/************************************************************************/
	/* IMenu interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IMenu*		Menu::getSubMenu(index_t index) const
	{
		if( index >= mSubmenus.size() )
			return NULL;
		else
		{
			MenuList::const_iterator i = mSubmenus.begin();
			std::advance(i, index);
			return *i;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*		Menu::getSubMenu(const TString& name) const
	{
		for(MenuList::const_iterator i = mSubmenus.begin(); i != mSubmenus.end(); ++i)
		{
			if ((*i)->getName() == name)
				return *i;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*		Menu::addSubMenu(const TString& name, MENU_DATA& data, IUICommand* command/* = NULL*/)
	{
		//already a sub menu
		if (mCMenu == NULL)
		{
			assert(false);
			return NULL;
		}

		IMenu* existing = this->getSubMenu(name);
		if(existing != NULL && !(data.mFlags&UIMF_SEPARATOR) )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("menu with the same name \"")+ name + BTString("\" has already exist") );

		data.mAccKey = (tchar)std::toupper(data.mAccKey);

		if (data.mText.empty())
		{
			data.mText = BTString2Lang(name);

			if((data.mFlags&UIMF_3DOT))
					data.mText += BTString("...");

			Menu::generateMenuText(this->isEmbedded() ? mParent->mSubmenus : mSubmenus, name, data.mText, data.mAccKey, KC_UNDEFINED);
		}

		//update index
		data.mIndex = mSubmenus.size();

		if (data.mFlags&UIMF_SUBITEM)
			data.mFlags |= UIMF_EMBEDDED;

		if (this->isEmbedded() || this->isGrouped())
		{
			assert(data.mFlags&UIMF_SUBITEM);	//cannot embed into an embedded menu again
			if( this->isGrouped() )
				data.mFlags |= UIMF_GROUP;
		}

		Menu* menu;
		if (this->isEmbedded())//add to parent directly
		{
			data.mFlags |= UIMF_EMBEDDED;
			menu = BLADE_NEW Menu(name, data, command, this, mCMenuParent);
		}
		else
			menu = BLADE_NEW Menu(name, data, command, this, mCMenu);
		mSubmenus.push_back( menu );
		return menu;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Menu::removeSubMenu(index_t index)
	{
		if (this->isEmbedded() && (this->isSubItem() || this->isSeparator()))
		{
			assert(false);
			return false;
		}

		if (index >= mSubmenus.size())
		{
			assert(false);
			return false;
		}
		CMenu* cmenu = this->isEmbedded() ? mCMenuParent : mCMenu;
		index_t base = this->isEmbedded() ? mData.mIndex : 0;

		cmenu->RemoveMenu((UINT)(base + index), MF_BYPOSITION);

		MenuList::const_iterator i = mSubmenus.begin();
		std::advance(i, index);
		IMenu* menu = *i;
		mSubmenus.erase(i);
		BLADE_DELETE menu;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Menu::removeAllMenus()
	{
		if (this->isEmbedded() && (this->isSubItem() || this->isSeparator()))
		{
			assert(false);
			return false;
		}
		CMenu* cmenu = this->isEmbedded() ? mCMenuParent : mCMenu;
		index_t index = this->isEmbedded() ? mData.mIndex : 0;

		index_t i = 0;
		for(MenuList::iterator iter = mSubmenus.begin(); iter != mSubmenus.end(); ++iter)
		{
			cmenu->RemoveMenu(UINT(index + i), MF_BYPOSITION);
			BLADE_DELETE *iter;
			++i;
		}
		mSubmenus.clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				Menu::getItems(TStringParam& outList)
	{
		for(MenuList::iterator iter = mSubmenus.begin(); iter != mSubmenus.end(); ++iter)
		{
			outList.push_back((*iter)->getName());
		}
		return outList.size() > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	Menu*				Menu::findMenu(UINT_PTR id) const
	{
		for(MenuList::const_iterator iter = mSubmenus.begin(); iter != mSubmenus.end(); ++iter)
		{
			Menu* menu = *iter;
			if (menu->isEmbedded() || menu->isSubItem())
			{
				if (menu->getMenuID() == id)
					return menu;
			}

			menu = menu->findMenu(id);
			if (menu != NULL)
				return menu;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	TString&	Menu::generateMenuText(const MenuList& siblings, const TString& name, TString& text, tchar& accessKey, HOTKEY hotkey)
	{
		if( accessKey != AUTO_KEY )
			return Menu::generateMenuText(text, accessKey, hotkey);

		//auto generate key
		const MenuList* menuList = &siblings;
		for(index_t i = 0; i < name.size(); ++i)
		{
			tchar key = (tchar)toupper( name[i] );
			bool reduplicated = menuList != NULL;
			for(MenuList::const_iterator j = menuList->begin(); !reduplicated && j != menuList->end(); ++j)
			{
				if((*j)->isSubItem() )
					reduplicated = key == (*j)->getAccessKey();
				else
				{
					size_t n = (*j)->getSubMenuCount();
					for(size_t k = 0; k < n && !reduplicated; ++k)
					{
						if( key == (*j)->getSubMenu(k)->getAccessKey() )
							reduplicated = true;
					}
				}
			}
			if( !reduplicated )
			{
				accessKey = key;
				break;
			}
		}

		//all keys are taken, not suitable key found
		if( accessKey == AUTO_KEY )
			accessKey = tchar();

		return Menu::generateMenuText(text, accessKey, hotkey);
	}

	//////////////////////////////////////////////////////////////////////////
	TString&		Menu::generateMenuText(TString& text, tchar accessKey, HOTKEY hotkey)
	{
		if( accessKey != tchar() )
		{
			index_t index = text.find_first_of(accessKey, 0);
			if( index != TString::npos )
				text = text.substr(0,index) + TEXT('&') + text.substr(index);
			else
			{
				tchar accessKeyString[2] = {0};
				accessKeyString[0] = accessKey;
				text += BTString("(&") + accessKeyString + TEXT(')');
			}
		}

		if( hotkey.isValid() )
			text += TEXT('\t') + IHotkeyManager::getSingleton().getHotkeyString(hotkey);
		return text;
	}

}//namespace Blade
