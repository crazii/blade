/********************************************************************
	created:	2011/04/11
	filename: 	MenuManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ui/MenuManager.h>
#include <ui/Menu.h>
#include <ui/private/UIToolBar.h>

namespace Blade
{
	static const size_t MAX_MENU_COUNT = 4096;

	//reference: http://msdn.microsoft.com/en-us/library/t2zechd4(v=vs.71).aspx
	static const UINT_PTR	CMD_ID_BEGIN = 0xA000;	//Command ID , MFC need it ranges 0x8000-0xDFFF
	static UINT_PTR			CMD_ID_CURRENT = CMD_ID_BEGIN;
	static const UINT_PTR	CMD_ID_END = 0xCFFF;

	//pop-up menu ID
	static const UINT_PTR	MENU_POPUP_BEGIN = 0x9000;
	static UINT_PTR			MENU_POPUP_CURRENT = MENU_POPUP_BEGIN;
	static const UINT_PTR	MENU_POPUP_END = CMD_ID_BEGIN - 1;

	//////////////////////////////////////////////////////////////////////////
	MenuManager::MenuManager()
		:mRoot(NULL)
	{
		mMenuList.reserve(MAX_MENU_COUNT);
	}

	//////////////////////////////////////////////////////////////////////////
	MenuManager::~MenuManager()
	{
		for( size_t i = 0; i < mMenuList.size(); ++i)
		{
			IMenu* menu = mMenuList[i];
			BLADE_DELETE menu;
		}
		BLADE_DELETE mRoot;
		mRoot = NULL;

		//delete extra menus
		for(MenuRegistry::iterator i = mExtraMenus.begin(); i != mExtraMenus.end(); ++i)
		{
			IMenu* menu = i->second;
			BLADE_DELETE menu;
		}
	}

	/************************************************************************/
	/* IMenuManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IMenu*	MenuManager::addRootMenu(const TString& name,tchar accessKey/* = tchar()*/)
	{
		std::pair<MenuRegistry::iterator,bool> ret = mRootMenus.insert( MenuRegistry::value_type(name,(IMenu*)NULL) );
		if( ret.second == false )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("menu with the same name \"")+ name + BTString("\" has already exist") );

		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		CUIMenu* root = this->getAppMenu();

		MENU_DATA data;
		data.mAccKey = accessKey;
		data.mFlags = UIMF_ENABLE;
		data.mHotKey = KC_UNDEFINED;
		data.mIcon = INVALID_ICON_INDEX;
		data.mText = BTString2Lang(name);
		data.mIndex = (index_t)root->GetMenuItemCount();
		Menu::MenuList list(mMenuList.begin(), mMenuList.end());
		Menu::generateMenuText(list, name, data.mText, accessKey, KC_UNDEFINED);
		Menu* menu = BLADE_NEW Menu(name, data, NULL, NULL, root, true);
		ret.first->second = menu;

		mMenuList.push_back(menu);
		return ret.first->second;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		MenuManager::getRootMenuCount() const
	{
		return mMenuList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	MenuManager::getRootMenu(index_t index) const
	{
		if( index >= mMenuList.size() )
			return NULL;
		else
			return mMenuList[index];
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	MenuManager::getRootMenu(const TString& name) const
	{
		MenuRegistry::const_iterator i = mRootMenus.find(name);
		if( i == mRootMenus.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	MenuManager::findMenu(const TString& name) const
	{
		if(name == TString::EMPTY )
			return NULL;

		IMenu* menu;

		if( (menu = this->getRootMenu(name)) != NULL )
			return menu;
		else
		{
			for( size_t i = 0; i < mMenuList.size(); ++i)
			{
				menu = mMenuList[i];
				IMenu* submenu = menu->findMenu(name);
				if( submenu != NULL )
					return submenu;
			}
			return NULL;
		}
		
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	MenuManager::createExtraMenu(const TString& name)
	{
		std::pair<MenuRegistry::iterator,bool> result = mExtraMenus.insert( std::make_pair(name,(IMenu*)NULL) );
		if( !result.second )
			return NULL;
		MenuRegistry::iterator i = result.first;

		MENU_DATA data;
		data.mAccKey = Menu::AUTO_KEY;
		data.mFlags = UIMF_ENABLE | UIMF_EXTRA;
		data.mHotKey = KC_UNDEFINED;
		data.mIcon = INVALID_ICON_INDEX;
		Menu* menu = BLADE_NEW Menu(name, data, NULL, NULL, NULL, false);
		i->second = menu;
		return menu;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MenuManager::destroyExtraMenu(IMenu* menu)
	{
		if( menu == NULL )
			return false;

		MenuRegistry::iterator result = mExtraMenus.find( menu->getName() );
		if(result != mExtraMenus.end() )
		{
			BLADE_DELETE result->second;
			mExtraMenus.erase(result);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	MenuManager::getExtraMenu(const TString& name)
	{
		MenuRegistry::iterator i = mExtraMenus.find(name);
		if( i == mExtraMenus.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	MenuManager::popupMenu(IMenu* menu, int x, int y, IUIWindow* window)
	{
		if(menu == NULL || window == NULL)
			return false;

		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		HWND hWnd = (HWND)(void*)window->getWindow()->getNativeHandle();
		CWnd* parent = CWnd::FromHandle(hWnd);
		return this->popupMenu(menu, x, y, parent);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	UINT_PTR		MenuManager::getCommandID(IUICommand* cmd)
	{
		if (cmd == NULL)
			return 0;

		std::pair<CommandIDMap::iterator, bool> ret = mCommandIDs.insert(std::make_pair(cmd, (UINT_PTR)0));
		if (!ret.second)
			ret.first->second;

		ret.first->second = CMD_ID_CURRENT++;

		mIDCommands[ret.first->second] = cmd;
		return ret.first->second;
	}

	//////////////////////////////////////////////////////////////////////////
	IUICommand*		MenuManager::getCommand(UINT_PTR id) const
	{
		IDCommandMap::const_iterator i = mIDCommands.find(id);
		if (i != mIDCommands.end())
			return i->second;

		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	UINT_PTR		MenuManager::getBaseCommandID() const
	{
		return MENU_POPUP_BEGIN;
	}

	//////////////////////////////////////////////////////////////////////////
	UINT_PTR		MenuManager::getMaxCommandID()	const
	{
		return CMD_ID_CURRENT;
	}

	//////////////////////////////////////////////////////////////////////////
	LRESULT			MenuManager::handleMenuCommand(WPARAM nPos, LPARAM hMenu)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		assert(::IsMenu((HMENU)hMenu));

		CMenu* pMenu = CMenu::FromHandle((HMENU)hMenu);
		CUIMenu* uiMenu = static_cast<CUIMenu*>(pMenu);
		IMenu* menu = uiMenu->getItem((UINT)nPos);

		if (menu->getBindingCommand() != NULL)
			menu->getBindingCommand()->execute(menu);

		CMenu::DeleteTempMap();
		return 0;

	}

	//////////////////////////////////////////////////////////////////////////
	bool			MenuManager::updateCommand(UINT_PTR id, CCmdUI* cmdui, index_t index/* = 0*/)
	{
		if (id < MENU_POPUP_BEGIN || id >= CMD_ID_CURRENT)
			return false;

		if (cmdui == NULL)
		{
			assert(false);
			return false;
		}
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		IDCommandMap::const_iterator iter = mIDCommands.find(id);
		if (iter == mIDCommands.end())
		{
			if (cmdui->m_pOther != NULL)
			{
				//currently tool bar only
				CUIToolBar* tb = DYNAMIC_DOWNCAST(CUIToolBar, cmdui->m_pOther);
				if (tb != NULL)
					return tb->updateDropDownMenu((int)cmdui->m_nIndex, index);
			}
			return false;
		}

		IUICommand* cmd = iter->second;
#if BLADE_USE_MFC_FEATURE_PACK
		CUIPopupMenuBar* mb = DYNAMIC_DOWNCAST(CUIPopupMenuBar, cmdui->m_pOther);
		CUIPopMenu* pm = mb != NULL ? DYNAMIC_DOWNCAST(CUIPopMenu, mb->GetParent()) : NULL;
		if (cmdui->m_pMenu != NULL || cmdui->m_pSubMenu != NULL || pm != NULL )
#else
		if (cmdui->m_pMenu != NULL || cmdui->m_pSubMenu != NULL)
#endif
		{
			CUIMenu* uiMenu = static_cast<CUIMenu*>(cmdui->m_pMenu);
			CUIMenu* subuiMenu = static_cast<CUIMenu*>(cmdui->m_pSubMenu);

#if BLADE_USE_MFC_FEATURE_PACK
			if(uiMenu == NULL && subuiMenu == NULL)
			{
				if(pm != NULL && pm->getMenu() != NULL)
					uiMenu = static_cast<CUIMenu*>(CMenu::FromHandle(pm->getMenu()));
			}
#endif

			if (uiMenu == NULL && subuiMenu == NULL)
				return false;

			IMenu* target = NULL;

			IMenu* menu = uiMenu != NULL ? uiMenu->getMenu() : NULL;
			IMenu* subMenu = subuiMenu != NULL ? subuiMenu->getMenu() : NULL;

			if (menu != NULL && menu->getBindingCommand() == cmd)
				target = menu;
			else if (subMenu != NULL && subMenu->getBindingCommand() == cmd)
				target = subMenu;

			if (target == NULL)
			{
				if (menu != NULL &&  menu->getSubMenu(cmdui->m_nIndex) != NULL && menu->getSubMenu(cmdui->m_nIndex)->getBindingCommand() == cmd)
					target = menu->getSubMenu(cmdui->m_nIndex);
				else if(subMenu != NULL && subMenu->getSubMenu(cmdui->m_nIndex) != NULL && subMenu->getSubMenu(cmdui->m_nIndex)->getBindingCommand() == cmd)
					target = subMenu/*->getSubMenu(cmdui->m_nIndex)*/;
			}

			if (target != NULL )
			{
				cmd->update(target, index);
				cmdui->Enable(target->isEnabled());
				cmdui->SetCheck(target->isChecked() ? TRUE : FALSE);
				return true;
			}
		}
		else if(cmdui->m_pOther != NULL)
		{
			//currently tool bar only
			CUIToolBar* tb = DYNAMIC_DOWNCAST(CUIToolBar, cmdui->m_pOther);
			if (tb != NULL)
				return tb->updateCmdUI(cmd, index, id, cmdui);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MenuManager::handlePopupMenuCommand(int x, int y, UINT_PTR id, CWnd* owner)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		//handle pop-up menus
		if( id >= MENU_POPUP_BEGIN && id <= MENU_POPUP_CURRENT )
		{
			PopupIDMap::const_iterator i = mPopIDs.find(id);
			if( i != mPopIDs.end() )
			{
				Menu* menu = static_cast<Menu*>( i->second );
				assert(!menu->isSubItem() && !menu->isEmbedded());
				
				return this->popupMenu(menu, x, y, owner);
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MenuManager::popupMenu(IMenu* menu, int x, int y, CWnd* cwnd)
	{
		if (menu == NULL || cwnd == NULL || menu->isEmbedded())
		{
			assert(false);
			return false;
		}
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		CPoint pt(x, y);
		cwnd->ClientToScreen(&pt);

		Menu* _menu = static_cast<Menu*>(menu);
		INT id = _menu->getCMenu()->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x + 1, pt.y + 1, cwnd);
		if (id != 0)
		{
			Menu* m = _menu->findMenu((UINT_PTR)id);
			if (m != NULL && m->getBindingCommand() != NULL)
				m->getBindingCommand()->execute(m);
		}
		CWnd::DeleteTempMap();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	CUIMenu*			MenuManager::getAppMenu()
	{
		if( mRoot == NULL )
		{
			mRoot = BLADE_NEW RootMenu();
			mRoot->CreateMenu();

			//note: OnMenuCommand need MNS_NOTIFYBYPOS, to handle command by menu
			MENUINFO info;
			info.cbSize = sizeof(info);
			info.fMask = MIM_STYLE;
			VERIFY(mRoot->GetMenuInfo(&info));
			info.dwStyle |= MNS_NOTIFYBYPOS | MIM_APPLYTOSUBMENUS;
#if ENABLE_THEME
			info.fMask |= MIM_BACKGROUND;
			info.hbrBack = THEMEBRUSH(TC_WINBK);
#endif
			VERIFY(mRoot->SetMenuInfo(&info));	
		}
		return mRoot;
	}

	//////////////////////////////////////////////////////////////////////////
	UINT_PTR		MenuManager::getActiveMenu(CMenu* _menu, tchar accessKey)
	{
		//accessKey is a wide ASCII
		accessKey = (tchar)std::toupper(accessKey);
		CUIMenu* menu = static_cast<CUIMenu*>(_menu);

		for(int i = 0; i < menu->GetMenuItemCount(); ++i)
		{
			IMenu* subMenu = menu->getItem((UINT)i);
			if( subMenu != NULL )
			{
				const MENU_DATA* data = subMenu->getMenuData();
				assert( isupper(data->mAccKey) );
				if( data->mAccKey == accessKey )
					return (UINT_PTR)i;
			}
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	UINT_PTR		MenuManager::addPopupMenu(IMenu* imenu)
	{
		if( imenu == NULL || imenu->isCheckable() || !imenu->isGrouped() && imenu->getSubMenuCount() <= 0 || (imenu->isGrouped() && imenu->isEmbedded()) )
		{
			assert(false);
			return NULL;
		}

		if( imenu->isGrouped() )
		{
			if( imenu->getSubMenuCount() <= 0 )
			{
				assert(false);
				return NULL;
			}
		}
		else
		{
			Menu* menu = static_cast<Menu*>(imenu);
			if( menu->getMenuID() != Menu::INVALID_ID )
			{
				assert(false);
				return NULL;
			}
		}

		std::pair<PopupRegistry::iterator,bool> ret = mPopRegistry.insert( std::make_pair(imenu, MENU_POPUP_CURRENT) );
		if( !ret.second )
			return ret.first->second;

		bool success = mPopIDs.insert( std::make_pair(MENU_POPUP_CURRENT, imenu) ).second;
		assert(success);
		BLADE_UNREFERENCED(success);

		UINT_PTR retID = MENU_POPUP_CURRENT;
		++( MENU_POPUP_CURRENT );
		return retID;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MenuManager::isPopupMenu(UINT_PTR id) const
	{
		return id >= MENU_POPUP_BEGIN && id < MENU_POPUP_CURRENT;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*			MenuManager::getPopupMenu(UINT_PTR id) const
	{
		PopupIDMap::const_iterator i = mPopIDs.find(id);
		return i != mPopIDs.end() ? i->second : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	UINT_PTR		MenuManager::addConfigID(IConfig* config)
	{
		if( config == NULL /*|| config->getSubConfigCount() == 0*/ )
		{
			assert(false);
			return 0;
		}

		std::pair<PopupConfigRegistry::iterator,bool> ret = mPopConfigRegistry.insert( std::make_pair(config, MENU_POPUP_CURRENT) );
		if( !ret.second )
			return ret.first->second;
		bool success = mPopConfigIDs.insert( std::make_pair(MENU_POPUP_CURRENT, config) ).second;
		assert(success);
		BLADE_UNREFERENCED(success);

		UINT_PTR retID = MENU_POPUP_CURRENT;
		++( MENU_POPUP_CURRENT );
		return retID;
	}
	
	//////////////////////////////////////////////////////////////////////////
	IConfig*		MenuManager::getConfig(UINT_PTR popupID)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		//handle pop-up menus
		if( popupID >= MENU_POPUP_BEGIN && popupID <= MENU_POPUP_CURRENT )
		{
			PopupConfigIDMap::const_iterator i = mPopConfigIDs.find(popupID);
			if( i != mPopConfigIDs.end() )
				return i->second;
		}
		return NULL;
	}

}//namespace Blade
