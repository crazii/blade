/********************************************************************
	created:	2016/07/23
	filename: 	QtMenuManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMenuManager_h__
#define __Blade_QtMenuManager_h__
#include <interface/public/ui/IMenuManager.h>
#include <QtMenu.h>

namespace Blade
{
	class QtMenuManager : public IMenuManager, public Singleton<QtMenuManager>
	{
	public:
		using Singleton<QtMenuManager>::getSingleton;
	public:
		QtMenuManager();
		~QtMenuManager();

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

		/** @brief popup a menu for a window in window coordinates  */
		virtual bool	popupMenu(IMenu* menu, int x, int y, IUIWindow* window);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		inline QMenuBar* getAppMenuBar()
		{
			return mRoot;
		}

		/** @brief  */
		void shutdown();

	protected:
		typedef StaticVector<QtMenu*>		MenuList;
		typedef StaticList<QtMenu*>			ExtraMenuList;
		QMenuBar*		mRoot;
		MenuList		mMenuList;
		ExtraMenuList	mExtraMenuList;

		friend class QtMenuBar;
	};
	
}//namespace Blade


#endif // __Blade_QtMenuManager_h__