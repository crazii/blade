/********************************************************************
	created:	2011/05/08
	filename: 	UIMenu.h
	author:		Crazii
	purpose:	MFC CMenu derived class
*********************************************************************/
#ifndef __Blade_UIMenu_h__
#define __Blade_UIMenu_h__
#include <interface/public/ui/IMenu.h>

/************************************************************************/
/*                                                                      */
/************************************************************************/
class CUIMenu : public CMenu ,public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
public:
	virtual ~CUIMenu()	{}
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

	/** @brief  */
	inline void setItem(UINT index, Blade::IMenu* menu)
	{
		MENUITEMINFO info;
		info.cbSize = sizeof(info);
		info.fMask = MIIM_DATA;
		info.dwItemData = (ULONG_PTR)menu;
		this->SetMenuItemInfo(index, &info, TRUE);
	}

	/** @brief  */
	inline Blade::IMenu* getItem(UINT index)
	{
		MENUITEMINFO info;
		info.cbSize = sizeof(info);
		info.fMask = MIIM_DATA;
		info.dwItemData = NULL;
		this->GetMenuItemInfo(index, &info, TRUE);
		return (Blade::IMenu*)info.dwItemData;
	}

	/** @brief  */
	inline void setMenu(Blade::IMenu* menu)
	{
		MENUINFO info;
		info.cbSize = sizeof(info);
		info.fMask = MIM_MENUDATA;
		info.dwMenuData = (ULONG_PTR)menu;
		this->SetMenuInfo(&info);
	}

	/** @brief  */
	inline Blade::IMenu* getMenu()
	{
		MENUINFO info;
		info.cbSize = sizeof(info);
		info.fMask = MIM_MENUDATA;
		info.dwMenuData = NULL;
		this->GetMenuInfo(&info);
		return (Blade::IMenu*)info.dwMenuData;
	}
};//CUIMenu


/************************************************************************/
/*                                                                      */
/************************************************************************/
#if BLADE_USE_MFC_FEATURE_PACK
//////////////////////////////////////////////////////////////////////////
class CUIPopMenu;
class CUIPopupMenuBar : public CMFCPopupMenuBar, public Blade::Allocatable
{
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;

	/** @brief  */
	virtual BOOL OnSendCommand(const CMFCToolBarButton* pButton);
	/** @brief  */
	virtual int InsertButton(const CMFCToolBarButton& button, INT_PTR iInsertAt = -1);
	///** @brief  */
	//void LateUpdateCmdUI(CUIPopMenu* pParentMenu);
	/** @brief  */
	inline CFrameWnd* getCmdTarget() {return (CFrameWnd*)this->GetCommandTarget();}
};

//////////////////////////////////////////////////////////////////////////
class CUIPopMenu : public CMFCPopupMenu, public Blade::Allocatable
{
	DECLARE_DYNAMIC(CUIPopMenu);
public:
	using Blade::Allocatable::operator new;
	using Blade::Allocatable::operator delete;
	/** @brief  */
	virtual CMFCPopupMenuBar* GetMenuBar() { return &mMenuBar; }
	/** @brief  */
	inline HMENU getMenu()	{return mMenu;}

protected:
	CUIPopupMenuBar mMenuBar;
	HMENU	mMenu;
	friend class CUIPopupMenuBar;
	friend class CUIMenuButton;
};

class CUIMenuBar : public CMFCMenuBar
{
	DECLARE_DYNAMIC(CUIMenuBar);
public:
	CUIMenuBar();

	/** @brief menus are dynamically created, no need to build items for saving */
	virtual BOOL BuildOrigItems(UINT /*uiMenuResID*/) { return TRUE; }
};
#endif


#endif // __Blade_UIMenu_h__