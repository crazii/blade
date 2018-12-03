/********************************************************************
	created:	2016/07/23
	filename: 	QtMenu.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtMenu.h>
#include <QtMenuManager.h>
#include <QtIconManager.h>

namespace Blade
{
	class QtMenuFunctor
	{
	public:
		QtMenu* mMenu;
		QtMenuFunctor(QtMenu* m) :mMenu(m)	{}

		/** @brief  */
		void operator()()
		{
			mMenu->execute();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	QtMenu::QtMenu(const TString& name, const MENU_DATA& data, IUICommand* cmd, QtMenu* parent)
		:mName(name)
		,mParent(parent)
		,mCommand(cmd)
		,mQMenu(NULL)
		,mData(data)
	{
		if ((this->isSubItem()/* || (mParent == NULL && !(mData.mFlags&UIMF_EXTRA))*/) && mData.mHotKey.isValid())
			IHotkeyManager::getSingleton().registerHotkey(mData.mHotKey, this);
	}

	//////////////////////////////////////////////////////////////////////////
	QtMenu::~QtMenu()
	{
		if((mData.mFlags&UIMF_EXTRA))
			BLADE_DELETE mQMenu;

		for(MenuList::iterator i = mMenus.begin(); i != mMenus.end(); ++i)
			BLADE_DELETE *i;

		if ((this->isSubItem()/* || (mParent == NULL && !(mData.mFlags&UIMF_EXTRA))*/) && mData.mHotKey.isValid())
			IHotkeyManager::getSingleton().unregisterHotkey(mData.mHotKey);
	}

	/************************************************************************/
	/* IMenu interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IMenu*		QtMenu::getSubMenu(index_t index) const
	{
		if( index >= mMenus.size() )
			return NULL;
		else
		{
			MenuList::const_iterator i = mMenus.begin();
			std::advance(i, index);
			return *i;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*		QtMenu::getSubMenu(const TString& name) const
	{
		for(MenuList::const_iterator i = mMenus.begin(); i != mMenus.end(); ++i)
		{
			if ((*i)->getName() == name)
				return *i;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*		QtMenu::addSubMenu(const TString& name, MENU_DATA& data, IUICommand* command/* = NULL*/)
	{
		if(mQMenu == NULL)	//action item
		{
			assert(false);
			return NULL;
		}
		QIcon* icon = QtIconManager::getSingleton().getIcon(data.mIcon);
		data.mIndex = mQMenu->actions().size();

		const TString& localeName = BTString2Lang(name);
		TString text = localeName;
		QtMenu::generateMenuText(mMenus, localeName, text, data.mAccKey);

		QtMenu* subMenu = BLADE_NEW QtMenu(name, data, command, this);
		if(subMenu->isSeparator())
			mQMenu->addSeparator();
		else if(subMenu->isSubItem())
		{
			TString hotkeyString;
			if(data.mHotKey.isValid())
				hotkeyString = IHotkeyManager::getSingleton().getHotkeyString(data.mHotKey);

			//don't add shortcut:
			QAction* act;
			if(icon != NULL)
				act = mQMenu->addAction(*icon, QObject::tr(""), QtMenuFunctor(subMenu));
			else
				act = mQMenu->addAction(QObject::tr(""), QtMenuFunctor(subMenu));

			//append shortcut text only:
			if(!hotkeyString.empty())
				text += BTString("\t") + hotkeyString;
			act->setText(TString2QString(text));
			act->setCheckable(subMenu->isCheckable());
		}
		else
		{
			if(subMenu->isEmbedded())
			{
				//TODO:
			}
			else
			{
				QMenu* qmenu = NULL;
				if(icon != NULL)
					qmenu = mQMenu->addMenu(*icon, TString2QString(text));
				else
					qmenu = mQMenu->addMenu(TString2QString(text));
				subMenu->mQMenu = qmenu;
				qmenu->setUserData(0, BLADE_NEW QMenuData(subMenu));
			}
		}
		mMenus.push_back(subMenu);
		return subMenu;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtMenu::removeSubMenu(index_t index)
	{
		if (this->isEmbedded() && (this->isSubItem() || this->isSeparator()))
		{
			assert(false);
			return false;
		}

		if (index >= mMenus.size())
		{
			assert(false);
			return false;
		}
		QMenu* qmenu = this->isEmbedded() ? mParent->mQMenu : mQMenu;
		index_t base = this->isEmbedded() ? mData.mIndex : 0;

		QList<QAction*>& list = qmenu->actions();
		qmenu->removeAction(list.at(base+index));

		MenuList::const_iterator i = mMenus.begin();
		std::advance(i, index);
		IMenu* menu = *i;
		mMenus.erase(i);
		BLADE_DELETE menu;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		QtMenu::removeAllMenus()
	{
		if (this->isEmbedded() && (this->isSubItem() || this->isSeparator()))
		{
			assert(false);
			return false;
		}
		QMenu* qmenu = this->isEmbedded() ? mParent->mQMenu : mQMenu;
		index_t index = this->isEmbedded() ? mData.mIndex : 0;

		index_t i = 0;
		QList<QAction*> list = qmenu->actions();
		for(MenuList::iterator iter = mMenus.begin(); iter != mMenus.end(); ++iter)
		{
			qmenu->removeAction(list.at(index+i));
			BLADE_DELETE *iter;
			++i;
		}
		mMenus.clear();
		return true;
	}

	/************************************************************************/
	/* IHotkeyCommand interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		QtMenu::execute()
	{
		if (mParent == NULL)
			QtMenuManager::getSingleton().getAppMenuBar()->actions()[mData.mIndex]->trigger();
		else if (mCommand != NULL)
			mCommand->execute(this);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			QtMenu::updateCmdUI(index_t instance)
	{
		if(mCommand != NULL)
			mCommand->update(this, instance);

		if(this->isEnabled())
		{
			for(MenuList::iterator iter = mMenus.begin(); iter != mMenus.end(); ++iter)
			{
				QtMenu* subMenu = *iter;
				subMenu->updateCmdUI(instance);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			QtMenu::enableTopLevel(bool enable)
	{
		QMenuBar* mb = QtMenuManager::getSingleton().getAppMenuBar();
		mb->actions().at(mData.mIndex)->setEnabled(enable);
	}
	
	//////////////////////////////////////////////////////////////////////////
	TString&	QtMenu::generateMenuText(const MenuList& siblings, const TString& name, TString& text, tchar& accessKey)
	{
		if( accessKey != AUTO_KEY )
			return QtMenu::generateMenuText(text, accessKey);

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

		return QtMenu::generateMenuText(text, accessKey);
	}

	//////////////////////////////////////////////////////////////////////////
	TString&		QtMenu::generateMenuText(TString& text, tchar accessKey)
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
		return text;
	}

}//namespace Blade