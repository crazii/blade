/********************************************************************
	created:	2011/04/11
	filename: 	Menu.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Menu_h__
#define __Blade_Menu_h__
#include <utility/BladeContainer.h>
#include <interface/public/ui/IMenu.h>

class CUIMenu;

namespace Blade
{
	class Menu : public IMenu, public IHotkeyCommand, public Allocatable
	{
	public:
		static const UINT_PTR INVALID_ID = UINT_PTR(-1);
	public:
		typedef List<Menu*>		MenuList;
	public:
		Menu(const TString& name, const MENU_DATA& data, IUICommand* cmd, Menu* parent, CUIMenu* parentCMenu, bool topLevel = false);
		~Menu();

		/************************************************************************/
		/* ICommandUI interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getName() const	{return mName;}

		/** @brief  */
		virtual void		setChecked(bool checked);

		/** @brief  */
		virtual void		setEnabled(bool enable);

		/** @brief  */
		virtual ICommandUI*	getParentCUI() const {return mParent;}

		/************************************************************************/
		/* IMenu interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const MENU_DATA*getMenuData() const	{return &mData;}

		/** @brief  */
		virtual IMenu*		getParent() const { return mParent; }

		/** @brief  */
		virtual IUICommand*	getBindingCommand() const	{return mCommand;}

		/** @brief  */
		virtual size_t		getSubMenuCount() const		{return mSubmenus.size();}

		/** @brief  */
		virtual IMenu*		getSubMenu(index_t index) const;

		/** @brief  */
		virtual IMenu*		getSubMenu(const TString& name) const;

		/** @brief  */
		/* @note: if the a subitem added into this menu, its original command and hot key become invalid */
		virtual IMenu*		addSubMenu(const TString& name, MENU_DATA& data, IUICommand* command = NULL);

		/** @brief  */
		virtual bool		removeSubMenu(index_t index);

		/** @brief  */
		virtual bool		removeAllMenus();

		/************************************************************************/
		/* IHotkeyCommand interface                                                                     */
		/************************************************************************/
		virtual void		execute()
		{
			if (mCommand != NULL)
				mCommand->execute(this);
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline const MENU_DATA*	getCheckedItemData()
		{
			IMenu* subMenu = this->getCheckedSubMenu();
			return subMenu != NULL ? subMenu->getMenuData() : NULL;
		}

		/** @brief  */
		bool				getItems(TStringParam& outList);

		/** @brief  */
		UINT_PTR			getMenuID()	const	{return mID;}

		/** @brief  */
		void				setBindingCommand(IUICommand* cmd)	{ mCommand = cmd; }

		/** @brief  */
		CUIMenu*			getCMenu() const	{return mCMenu;}

		/** @brief  */
		Menu*				findMenu(UINT_PTR id) const;

		/** @brief  */
		static TString&		generateMenuText(const MenuList& siblings, const TString& name, TString& text, tchar& accKey, HOTKEY hotkey);

		/** @brief  */
		static TString&		generateMenuText(TString& text, tchar accessKey, HOTKEY hotkey);

	protected:

		TString		mName;
		MENU_DATA	mData;
		IUICommand*	mCommand;
		Menu*		mParent;

		MenuList	mSubmenus;
		
		CUIMenu*	mCMenu;
		CUIMenu*	mCMenuParent;
		UINT_PTR	mID;
	};//class Menu
	

}//namespace Blade



#endif // __Blade_Menu_h__

