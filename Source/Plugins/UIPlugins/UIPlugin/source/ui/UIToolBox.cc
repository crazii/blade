/********************************************************************
	created:	2011/05/07
	filename: 	UIToolBox.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ui/UIToolBox.h>
#include <ui/IconManager.h>
#include <ui/MenuManager.h>
#include <ui/Menu.h>
#include <ui/private/UIToolTip.h>

#define ENABLE_BUTTON_TEXT		0
#define DEF_ICON_SIZE			(IS_16)

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	UIToolBox::UIToolBox(const TString& name, ICONSIZE is, UINT nID, CFrameWnd* pParent,size_t existingWidth, 
		bool autoDock/* = true*/, bool gripper/* = true*/, CFrameWnd* Owner/* = NULL*/)
		:mToolbar(name)
		,mAutoDock(autoDock)
		,mIconSize(is)
	{
		AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
		assert(pParent != NULL);
		mParent = pParent;
		if( Owner == NULL )
			Owner = mParent;

		mToolbar.setToolTipLink(CUIToolTip::getSingletonPtr());
		gripper = false;

		/* @note CBRS_FLYBY flag will cause tooltips problems */
		/* @note: TBSTYLE_EX_DRAWDDARROWS need to be set before control is visible */
		DWORD style = DWORD( WS_CHILD | (gripper?CBRS_GRIPPER:0) | CBRS_ALIGN_TOP | CBRS_ALIGN_LEFT | CBRS_SIZE_FIXED);
		BOOL result = mToolbar.CreateEx(pParent, TBSTYLE_LIST|TBSTYLE_FLAT/*|CCS_VERT*/|TBSTYLE_TRANSPARENT, style, CRect(0,0,0,0), nID);
		assert(result);
		BLADE_UNREFERENCED(result);
		this->getToolbarCtrl()->SetExtendedStyle( TBSTYLE_EX_DRAWDDARROWS|TBSTYLE_EX_MIXEDBUTTONS );
		this->getToolbarCtrl()->SetImageList( IconManager::getSingleton().getIconList(mIconSize) );
		this->getToolbarCtrl()->SetDisabledImageList( IconManager::getSingleton().getIconList(mIconSize,true) );
		this->getToolbarCtrl()->SetFont( mParent->GetFont() );

		mToolbar.SetOwner(Owner);
		int iconSize = (int)IIconManager::getSingleton().getIconSize(mIconSize);
		mToolbar.SetSizes( CSize(iconSize+CUIToolBar::DEF_BUTTON_XEDGE,iconSize+CUIToolBar::DEF_BUTTON_YEDGE),CSize(iconSize,iconSize) );
		mToolbar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_LEFT);
		mToolbar.ShowWindow(SW_SHOW);

		if( mAutoDock )
		{
#if !BLADE_USE_MFC_FEATURE_PACK
			CControlBar* dockBar = mParent->GetControlBar(AFX_IDW_DOCKBAR_TOP);
			CRect rect;
			dockBar->GetWindowRect(&rect);
			//just make offset
			rect.OffsetRect( (int)existingWidth, 2);
			mParent->RecalcLayout();
			mParent->DockControlBar(&mToolbar, (CDockBar*)dockBar, &rect);
			mParent->RecalcLayout();
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////////
	UIToolBox::~UIToolBox()
	{

	}

	/************************************************************************/
	/* IUIToolBox interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			UIToolBox::addButton(IUICommand* cmd, const TString& name, IconIndex icon, int style/* = TBS_NONE*/)
	{
		if (cmd == NULL || name.empty())
			return false;
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		const TString& text = BTString2Lang(name);
		TempBuffer buffer;
		buffer.reserve((text.size() + 2) * sizeof(tchar));
		tchar* str = (tchar*)buffer.getData();
		::memcpy(str, text.c_str(), buffer.getCapacity() - sizeof(tchar));
		str[text.size() + 1] = tchar('\0');

#if ENABLE_BUTTON_TEXT
		int iString = this->getToolbarCtrl()->AddStrings(str);
#else
		int iString = -1;
#endif
		bool checkable = (style&TBS_CHECK) != 0;
		const CUIToolBar::Button& button = *mButtons.insert(CUIToolBar::Button(name, checkable)).first;
		assert(button.mCheckable == checkable);	//same name but different places?
		int index = (int)this->getButtonCount();
		TBBUTTON tbb;
		tbb.iBitmap = icon;
		tbb.idCommand = (int)MenuManager::getSingleton().getCommandID(cmd);
		tbb.fsState = TBSTATE_ENABLED;
		tbb.fsStyle = BYTE(BTNS_BUTTON | (checkable ? BTNS_CHECK : 0));
		tbb.dwData = (DWORD_PTR)&button;
		tbb.iString = iString;
		this->getToolbarCtrl()->AddButtons(1, &tbb);

		CRect rcButton;
		this->getToolbarCtrl()->GetItemRect(index, &rcButton);

		CToolTipCtrl* tooltip = mToolbar.getToolTipLink();
		if (tooltip != NULL)
		{
			BOOL ret = tooltip->AddTool(this->getToolbarCtrl(), text.c_str(), &rcButton, (UINT_PTR)tbb.idCommand);
			assert(ret);
			BLADE_UNREFERENCED(ret);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UIToolBox::addButton(IMenu* menu, int style/* = TBS_CHECKGROUP*/)
	{
		if(menu == NULL )
			return false;

		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		if (menu->isSubItem())
		{
			if (menu->isCheckable())
				style |= TBS_CHECK;
			else
				style &= ~TBS_CHECK;
			return this->addButton(menu->getBindingCommand(), menu->getName(), menu->getIconIndex(), style);
		}
		else if( menu->isEmbedded() || (menu->isGrouped() && style == TBS_CHECKGROUP) )
		{
			TempBuffer buttonBuffer;
			IOBuffer strBuffer;

			const MENU_DATA* checkedData = static_cast<Menu*>(menu)->getCheckedItemData();

			int existingCount = this->getToolbarCtrl()->GetButtonCount();
			//add separator if last added buttons are groups
			if( existingCount > 0 )
			{
				TBBUTTON tbb;
				this->getToolbarCtrl()->GetButton(existingCount-1,&tbb);
				if( (tbb.fsStyle&BTNS_CHECKGROUP) )
				{
					tbb.fsStyle = BTNS_SEP;
					tbb.iBitmap = 0;
					tbb.idCommand = 0;
					this->getToolbarCtrl()->AddButtons(1, &tbb);
				}
			}
			
			buttonBuffer.reserve( sizeof(TBBUTTON)*menu->getSubMenuCount() );
			LPTBBUTTON tbbs = (LPTBBUTTON)buttonBuffer.getData();

#if ENABLE_BUTTON_TEXT
			int iStringBase = this->getToolbarCtrl()->GetButtonCount();
#endif
			TempVector<IMenu*> validItems;
			validItems.reserve(menu->getSubMenuCount());

			for(size_t i = 0; i < menu->getSubMenuCount(); ++i)
			{
				IMenu* subItem = menu->getSubMenu(i);
				if( !subItem->isSubItem() )
					continue;
				validItems.push_back(subItem);

				const CUIToolBar::Button& button = *mButtons.insert(CUIToolBar::Button(subItem->getName(), true)).first;
				assert(button.mCheckable == true);	//same name but different places?

				strBuffer.writeTStringRAW(BTString2Lang(subItem->getName()));
				tbbs[i].iBitmap = subItem->getIconIndex();
				tbbs[i].idCommand = (int)MenuManager::getSingleton().getCommandID(subItem->getBindingCommand());
				bool checked = checkedData != NULL && (checkedData->mFlags&UIMF_SUBITEM) && checkedData == subItem->getMenuData();
				tbbs[i].fsState = BYTE( (subItem->isEnabled()?TBSTATE_ENABLED:0) | (checked?TBSTATE_CHECKED:0) );
				tbbs[i].fsStyle = BTNS_BUTTON|BTNS_CHECKGROUP;
				//@see DynamicToolBar::updateCmdUI
				tbbs[i].dwData = (DWORD_PTR)&button;
#if ENABLE_BUTTON_TEXT
				tbbs[i].iString = iStringBase+i;
#else
				tbbs[i].iString = -1;
#endif
			}
			tchar end = tchar();
			strBuffer.write(&end);

#if ENABLE_BUTTON_TEXT
			int base = this->getToolbarCtrl()->AddStrings((const tchar*)strBuffer.getData());
			assert(base == iStringBase);
#endif
			this->getToolbarCtrl()->AddButtons( (int)validItems.size(), tbbs);

			CToolTipCtrl* tooltip = mToolbar.getToolTipLink();
			if( tooltip != NULL )
			{
				for(size_t i =0; i < validItems.size(); ++i )
				{
					CRect rect;
					mToolbar.GetItemRect( existingCount + (int)i ,&rect);
					TString tiptext = BTString2Lang(menu->getName())
						+ BTString(" - ") + 
						BTString2Lang(validItems[i]->getName());
					BOOL ret = tooltip->AddTool(this->getToolbarCtrl(),tiptext.c_str(),&rect, (UINT_PTR)tbbs[i].idCommand);
					assert(ret);
					BLADE_UNREFERENCED(ret);
				}
			}
		}
		else
		{
			const TString& text = BTString2Lang( menu->getName() );
			TempBuffer buffer;
			buffer.reserve( (text.size()+2)*sizeof(tchar) );
			tchar* str = (tchar*)buffer.getData();
			::memcpy(str,text.c_str(),buffer.getCapacity()-sizeof(tchar) );
			str[text.size()+1] = tchar('\0');

#if ENABLE_BUTTON_TEXT
			int iString = this->getToolbarCtrl()->AddStrings(str);
#else
			int iString = -1;
#endif
			int index = (int)this->getButtonCount();
			TBBUTTON tbb;
			bool checked = false;
			bool checkable = (style&TBS_CHECK) != 0;
			//const DynamicToolBar::Button& button = *mButtons.insert(DynamicToolBar::Button(subItem->getName(), true)).first;
			//assert(button.mCheckable == true);	//same name but different places?
			tbb.iBitmap = menu->getIconIndex();
			tbb.fsState = BYTE( (menu->isEnabled()?TBSTATE_ENABLED:0) | (checked?TBSTATE_CHECKED:0) );
			tbb.fsStyle = BYTE( BTNS_BUTTON|(checkable?BTNS_CHECK:0) );
			//@see DynamicToolBar::updateCmdUI
			tbb.dwData = 0;// (DWORD_PTR)&button;
			tbb.iString = iString;
			//it has sub items, so use a pop - up command to show it
			tbb.idCommand = (int)MenuManager::getSingleton().addPopupMenu(menu);
			//its weird that use BTNS_WHOLEDROPDOWN will result in width problem
			//but BTNS_DROPDOWN|BTNS_WHOLEDROPDOWN will be OK (BTNS_DROPDOWN need TBSTYLE_EX_DRAWDDARROWS extended style)
			tbb.fsStyle = BTNS_BUTTON|BTNS_WHOLEDROPDOWN;
			if( menu->isGrouped() && !menu->isEmbedded())
			{
				tbb.fsStyle |= BTNS_DROPDOWN;
				bool hasText = (style&TBS_TEXT) != 0;
				int stringID = mToolbar.initDropDownMenu((UINT)tbb.idCommand, static_cast<Menu*>(menu), hasText);

				const MENU_DATA* data = static_cast<Menu*>(menu)->getCheckedItemData();				
				if (data != NULL)
				{
					if(data->mIcon != INVALID_ICON_INDEX)
						tbb.iBitmap = data->mIcon;

					if (hasText)
					{
						tbb.iString = (INT_PTR)(stringID + (int)data->mIndex);
						tbb.fsStyle |= BTNS_SHOWTEXT;
					}
				}
			}
			this->getToolbarCtrl()->AddButtons(1,&tbb);
			CRect rcButton;
			this->getToolbarCtrl()->GetItemRect( index, &rcButton);

			CToolTipCtrl* tooltip = mToolbar.getToolTipLink();
			if(tooltip != NULL )
			{
				BOOL ret = tooltip->AddTool(this->getToolbarCtrl(), text.c_str(), &rcButton, (UINT_PTR)tbb.idCommand);
				assert(ret);
				BLADE_UNREFERENCED(ret);
			}
		}

		if( mAutoDock )
			mParent->RecalcLayout();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UIToolBox::addTool(IConfig* config, int width/* = -1*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		return mToolbar.addConfigTool(config, (size_t)width );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UIToolBox::addToolSet(IConfig* config, IconIndex icon)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		if (config == NULL || config->getSubConfigCount() == 0)
		{
			assert(false);
			return false;
		}

		TBBUTTON tbb;
		tbb.idCommand = (int)MenuManager::getSingleton().addConfigID(config);
		tbb.fsStyle = BTNS_BUTTON | BTNS_WHOLEDROPDOWN;
		tbb.iBitmap = icon;
		tbb.dwData = 0;
		tbb.fsState = TBSTATE_ENABLED;
		tbb.iString = -1;
		bool ret = this->getToolbarCtrl()->AddButtons(1, &tbb) == TRUE;
		if (ret)
		{
			const TString& title = BTString2Lang(config->getName());
			int index = this->getToolbarCtrl()->GetButtonCount() - 1;
			CRect rect;
			mToolbar.GetItemRect(index, &rect);
			CToolTipCtrl* tooltip = mToolbar.getToolTipLink();
			tooltip->AddTool(this->getToolbarCtrl(), title.c_str(), &rect, (UINT_PTR)tbb.idCommand);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void			UIToolBox::updateTools()
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		mToolbar.updateConfigTools();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			UIToolBox::getButtonCount() const
	{
		return (size_t)this->getToolbarCtrl()->GetButtonCount();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	UIToolBox::setIconSize(ICONSIZE is)
	{
		if( is >= IS_COUNT )
			return false;
		if( mIconSize == is )
			return true;

		mIconSize = is;
		this->getToolbarCtrl()->SetImageList( IconManager::getSingleton().getIconList(mIconSize) );
		this->getToolbarCtrl()->SetDisabledImageList( IconManager::getSingleton().getIconList(mIconSize,true) );
		int iconSize = (int)IIconManager::getSingleton().getIconSize(mIconSize);
		mToolbar.SetSizes( CSize(iconSize+CUIToolBar::DEF_BUTTON_XEDGE,iconSize+CUIToolBar::DEF_BUTTON_YEDGE),CSize(iconSize,iconSize) );
		return true;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t		UIToolBox::getWidth() const
	{
		const CToolBarCtrl* ctrl = this->getToolbarCtrl();
		if( ctrl == NULL )
			return 0;
		else
		{
			CSize size;
			if( ctrl->GetMaxSize(&size) )
				return (size_t)size.cx;
			else
				return 0;
		}
	}

}//namespace Blade