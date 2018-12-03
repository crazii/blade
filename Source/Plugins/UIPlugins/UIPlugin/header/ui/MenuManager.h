/********************************************************************
	created:	2011/04/11
	filename: 	MenuManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MenuManager_h__
#define __Blade_MenuManager_h__
#include <utility/BladeContainer.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <interface/public/ui/IUIWindow.h>
#include <ui/private/UIMenu.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class RootMenu : public CUIMenu, public StaticAllocatable
	{
	public:
		using StaticAllocatable::operator new;
		using StaticAllocatable::operator delete;
		using StaticAllocatable::operator new[];
		using StaticAllocatable::operator delete[];
	};

	class Menu;

	class MenuManager : public IMenuManager , public Singleton<MenuManager>
	{
	public:
		using Singleton<MenuManager>::getSingleton;
		using Singleton<MenuManager>::getSingletonPtr;

		MenuManager();
		~MenuManager();

		/************************************************************************/
		/* IMenuManager interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual IMenu*	addRootMenu(const TString& name,tchar accessKey = tchar());

		/** @brief  */
		virtual size_t	getRootMenuCount() const;

		/** @brief  */
		virtual IMenu*	getRootMenu(index_t index) const;

		/** @brief  */
		virtual IMenu*	getRootMenu(const TString& name) const;

		/** @brief  */
		virtual IMenu*	findMenu(const TString& name) const;

		/** @brief  */
		virtual IMenu*	createExtraMenu(const TString& name);

		/** @brief  */
		virtual bool	destroyExtraMenu(IMenu* menu);

		/** @brief  */
		virtual IMenu*	getExtraMenu(const TString& name);

		/** @brief  */
		virtual bool	popupMenu(IMenu* menu, int x, int y, IUIWindow* window);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		UINT_PTR		getCommandID(IUICommand* cmd);

		/** @brief  */
		IUICommand*		getCommand(UINT_PTR id) const;

		/** @brief  */
		UINT_PTR		getBaseCommandID() const;

		/** @brief  */
		UINT_PTR		getMaxCommandID()	const;

		/** @brief  */
		LRESULT			handleMenuCommand(WPARAM nPos, LPARAM hMenu);

		/** @brief  */
		bool			updateCommand(UINT_PTR id, CCmdUI* cmdui, index_t index = 0);

		/** @brief  */
		bool			handlePopupMenuCommand(int x, int y, UINT_PTR id, CWnd* owner);

		/** @brief  */
		bool			popupMenu(IMenu* menu, int x, int y, CWnd* cwnd);

		/** @brief  */
		CUIMenu*		getAppMenu();

		/** @brief get menu item ID of the menu */
		UINT_PTR		getActiveMenu(CMenu* menu, tchar accessKey);

		/** @brief  add a parent menu (with ID of 0 ) for pop-up */
		UINT_PTR		addPopupMenu(IMenu* imenu);

		/** @brief  */
		bool			isPopupMenu(UINT_PTR id) const;

		/** @brief  */
		IMenu*			getPopupMenu(UINT_PTR id) const;
		
		/** @brief  */
		UINT_PTR		addConfigID(IConfig* config);
		/** @brief  */
		IConfig*		getConfig(UINT_PTR popupID);

		/** @brief  */
		inline LRESULT	OnMenuChar(UINT nChar, UINT /*nFlags*/, CMenu* pMenu)
		{
			UINT_PTR id = this->getActiveMenu( pMenu, (tchar)nChar);
			if( id != INVALID_INDEX )
				return MAKELRESULT(id, MNC_EXECUTE);
			else
				return MNC_IGNORE;
		}

	protected:
		typedef StaticTStringMap<IMenu*>	MenuRegistry;
		typedef StaticVector<Menu*>		MenuList;
		typedef StaticMap<UINT_PTR,IMenu*> PopupIDMap;
		typedef StaticMap<IMenu*,UINT_PTR> PopupRegistry;

		typedef StaticMap<UINT_PTR,IConfig*> PopupConfigIDMap;
		typedef StaticMap<IConfig*, UINT_PTR> PopupConfigRegistry;

		typedef StaticMap<IUICommand*, UINT_PTR> CommandIDMap;
		typedef StaticMap<UINT_PTR, IUICommand*> IDCommandMap;

		CUIMenu*		mRoot;
		MenuRegistry	mRootMenus;
		MenuList		mMenuList;
		MenuRegistry	mExtraMenus;

		PopupRegistry	mPopRegistry;
		PopupIDMap		mPopIDs;

		CommandIDMap	mCommandIDs;
		IDCommandMap	mIDCommands;

		PopupConfigRegistry	mPopConfigRegistry;
		PopupConfigIDMap mPopConfigIDs;
	};

}//namespace Blade



#endif // __Blade_MenuManager_h__

