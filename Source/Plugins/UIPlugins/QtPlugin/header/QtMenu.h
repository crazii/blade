/********************************************************************
	created:	2016/07/23
	filename: 	QtMenu.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtMenu_h__
#define __Blade_QtMenu_h__
#include <interface/public/ui/IMenu.h>

namespace Blade
{
	class QtMenu : public IMenu, public IHotkeyCommand, public Allocatable
	{
	protected:
		typedef List<QtMenu*> MenuList;
	public:
		QtMenu(const TString& name, const MENU_DATA& data, IUICommand* cmd, QtMenu* parent);
		~QtMenu();

		/************************************************************************/
		/* IUICommand interface                                                                     */
		/************************************************************************/
		virtual void setEnabled(bool enable)
		{
			if(this->isSeparator())
				return;

			if(this->isGrouped() && this->isEmbedded())
			{
				for(size_t i = 0; i < this->getSubMenuCount(); ++i)
					mParent->mQMenu->actions().at(mData.mIndex + i)->setDisabled(!enable);
			}
			else if(mParent != NULL)
				mParent->mQMenu->actions().at(mData.mIndex)->setDisabled(!enable);
			else
				this->enableTopLevel(enable);

			if (enable)
				mData.mFlags |= UIMF_ENABLE;
			else
				mData.mFlags &= ~UIMF_ENABLE;
		}

		/** @brief  */
		virtual void setChecked(bool checked)
		{
			if(mQMenu != NULL || !this->isSubItem())
				return;
			assert(mParent != NULL);

			QList<QAction*> act = mParent->mQMenu->actions();
			if(mParent->isGrouped() && checked)
			{
				for(int i = 0; i < act.count(); ++i)
					act.at(i)->setChecked(false);
			}

			act.at(mData.mIndex)->setChecked(checked);
			if (checked)
				mData.mFlags |= UIMF_CHECKED;
			else
				mData.mFlags &= ~UIMF_CHECKED;
		}

		/** @brief  */
		virtual ICommandUI*	getParentCUI() const {return mParent;}

		/************************************************************************/
		/* IMenu interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual const TString&	getName() const {return mName;}
		/** @brief  get pointer to one data */
		virtual const MENU_DATA*getMenuData() const	{return &mData;}

		/** @brief */
		virtual IMenu*		getParent() const {return mParent;}

		/** @brief  */
		virtual IUICommand*	getBindingCommand() const {return mCommand;}

		/** @brief  */
		virtual size_t		getSubMenuCount() const {return mMenus.size();}

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
		/** @brief  */
		virtual void		execute();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void			updateCmdUI(index_t instance);
		/** @brief  */
		void			enableTopLevel(bool enable);
		/** @brief  */
		inline QMenu*	getQMenu() const	{return mQMenu;}

		/** @brief  */
		static TString&		generateMenuText(const MenuList& siblings, const TString& name, TString& text, tchar& accKey);

		/** @brief  */
		static TString&		generateMenuText(TString& text, tchar accessKey);

	protected:

		TString		mName;
		QtMenu*		mParent;
		IUICommand*	mCommand;
		QMenu*		mQMenu;
		MENU_DATA	mData;
		MenuList	mMenus;
		friend class QtMenuManager;
	};

	class QMenuData : public QObjectUserData, public Allocatable
	{
	public:
		QMenuData(QtMenu* m) :qtmenu(m)	{}
		virtual ~QMenuData() {}
		QtMenu* qtmenu;
	};
	
}//namespace Blade


#endif // __Blade_QtMenu_h__