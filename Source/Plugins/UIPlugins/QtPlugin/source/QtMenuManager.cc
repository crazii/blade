/********************************************************************
	created:	2016/07/23
	filename: 	QtMenuManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <QtMenuManager.h>
#include <QtMenu.h>

namespace Blade
{
	class QtMenuBar : public QMenuBar, public Allocatable
	{
	public:
		~QtMenuBar()
		{
			//dirty hack
			QtMenuManager::getSingleton().mRoot = NULL;
		}
	};
	class QtExtraMenu : public QMenu , public Allocatable {};

	//////////////////////////////////////////////////////////////////////////
	QtMenuManager::QtMenuManager()
	{
		mRoot = BLADE_NEW QtMenuBar();
		mMenuList.reserve(512);
	}

	//////////////////////////////////////////////////////////////////////////
	QtMenuManager::~QtMenuManager()
	{
		this->shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	QtMenuManager::addRootMenu(const TString& name,tchar accessKey/* = tchar()*/)
	{
		MENU_DATA data;
		data.mAccKey = accessKey;
		data.mFlags = UIMF_ENABLE;
		data.mIcon = INVALID_ICON_INDEX;
		data.mIndex = mRoot->actions().count();
		data.mText = BTString2Lang(name);
		//if (accessKey != TEXT('\0'))
		//	data.mHotKey = HOTKEY(IUIService::getSingleton().getKeyFromChar(accessKey), KC_ALT);

		QtMenu::MenuList list(mMenuList.begin(), mMenuList.end());
		QtMenu::generateMenuText(list, data.mText, data.mText, data.mAccKey);
		QString str = TString2QString(data.mText);

		QtMenu* menu = BLADE_NEW QtMenu(name, data, NULL, NULL);
		menu->mQMenu = mRoot->addMenu(TString2QString(data.mText));
		menu->mQMenu->setUserData(0, BLADE_NEW QMenuData(menu));
		mRoot->actions().back()->setShortcut(0);

		mMenuList.push_back(menu);
		return menu;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	QtMenuManager::getRootMenuCount() const
	{
		return mRoot->actions().count();
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	QtMenuManager::getRootMenu(index_t index) const
	{
		return index < mMenuList.size() ? mMenuList[index] : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	QtMenuManager::getRootMenu(const TString& name) const
	{
		for(size_t i = 0; i < mMenuList.size(); ++i)
		{
			if(mMenuList[i]->getName() == name)
				return mMenuList[i];
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	QtMenuManager::findMenu(const TString& name) const
	{
		for(size_t i = 0; i < mMenuList.size(); ++i)
		{
			if(mMenuList[i]->getName() == name)
				return mMenuList[i];

			IMenu* target = mMenuList[i]->findMenu(name);
			if(target != NULL)
				return target;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	QtMenuManager::createExtraMenu(const TString& name)
	{
		MENU_DATA data;
		data.mIndex = 0;
		data.mFlags = UIMF_ENABLE | UIMF_EXTRA;
		QtMenu* menu = BLADE_NEW QtMenu(name, data, NULL, NULL);
		menu->mQMenu = BLADE_NEW QtExtraMenu();
		mExtraMenuList.push_back(menu);
		return menu;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtMenuManager::destroyExtraMenu(IMenu* menu)
	{
		ExtraMenuList::iterator iter = std::find(mExtraMenuList.begin(), mExtraMenuList.end(), menu);
		if(iter == mExtraMenuList.end())
			return false;
		BLADE_DELETE *iter;
		mExtraMenuList.erase(iter);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IMenu*	QtMenuManager::getExtraMenu(const TString& name)
	{
		for(ExtraMenuList::iterator iter = mExtraMenuList.begin(); iter != mExtraMenuList.end(); ++iter)
		{
			if((*iter)->getName() == name)
				return *iter;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtMenuManager::popupMenu(IMenu* menu, int x, int y, IUIWindow* window)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	QtMenuManager::shutdown()
	{
		//note: now QMainWindow doesn't delete menubar on closing, in case exiting event fired from menu and causing crash
		//but we need to delete QMenuBar before QApplication exist, or Qt dll unloading,
		//so manually call shutdown at QtPlugin::shutdown()
		BLADE_DELETE mRoot;
		assert(mRoot == NULL);

		for(size_t i = 0; i < mMenuList.size(); ++i)
			BLADE_DELETE mMenuList[i];

		for(ExtraMenuList::iterator i = mExtraMenuList.begin(); i != mExtraMenuList.end(); ++i)
			BLADE_DELETE *i;

		mMenuList.clear();
		mExtraMenuList.clear();
	}

}//namespace Blade