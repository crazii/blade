/********************************************************************
	created:	2011/05/26
	filename: 	CommonMenuToolNotify.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include "CommonMenuToolNotify.h"
#include <interface/IEventManager.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/widgets/IUIToolPanel.h>
#include <interface/public/ui/widgets/IUIPropertyGrid.h>
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/IEditorFramework.h>
#include <interface/EditorEvents.h>
#include <BladeEditor_blang.h>


namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	CommonNotify::CommonNotify()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	CommonNotify::~CommonNotify()
	{
		
	}

	/************************************************************************/
	/* IFileUINotify interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void	CommonNotify::onFileSwitch(const IEditorFile* efOld, const IEditorFile* efNew)
	{
		assert( efOld != efNew && (efOld != NULL || efNew != NULL) );
		EditorFileSwitchEvent switchState(efOld, efNew);
		IEventManager::getSingleton().dispatchEvent(switchState);

		if( efNew == NULL )
		{
			std::for_each(mNotifiedTools.begin(),mNotifiedTools.end(),std::bind2nd( std::mem_fun(&IEditorTool::setEnable),false) );
			std::for_each(mNotifiedMenus.begin(),mNotifiedMenus.end(),std::bind2nd( std::mem_fun(&IMenu::setEnabled),false) );
		}

		//update file type related menu/tool
		bool changestatus;
		if( efOld != NULL )
			changestatus = (efNew == NULL) || efOld->getType() != efNew->getType();
		else
			changestatus = true;
		
		if( efNew != NULL && changestatus )
		{
			//BLADE_LW_PROFILING(ENABLE);
			const TString& type = efNew->getType();
			FileTypeMenu::iterator i = mTypedMenu.find(type);
			if( i != mTypedMenu.end() )
			{
				MenuList& list = i->second;
				std::for_each(list.begin(),list.end(),std::bind2nd( std::mem_fun(&IMenu::setEnabled),true) );
			}

			FileTypeTool::iterator it = mTypedTool.find(type);
			if( it != mTypedTool.end() )
			{
				ToolList& list = it->second;
				std::for_each(list.begin(),list.end(),std::bind2nd( std::mem_fun(&IEditorTool::setEnable),true) );
			}

			bool enable = efNew->getEditorScene() != NULL;
			std::for_each(mNotifiedTools.begin(),mNotifiedTools.end(),std::bind2nd( std::mem_fun(&IEditorTool::setEnable), enable) );
			std::for_each(mNotifiedMenus.begin(),mNotifiedMenus.end(),std::bind2nd( std::mem_fun(&IMenu::setEnabled), enable) );
		}

		if( efOld != NULL && changestatus )
		{
			//BLADE_LW_PROFILING(DISABLE);
			const TString& type = efOld->getType();
			FileTypeMenu::iterator i = mTypedMenu.find(type);
			if( i != mTypedMenu.end() )
			{
				MenuList& list = i->second;
				std::for_each(list.begin(),list.end(),std::bind2nd( std::mem_fun(&IMenu::setEnabled),false) );
			}

			FileTypeTool::iterator it = mTypedTool.find(type);
			if( it != mTypedTool.end() )
			{
				ToolList& list = it->second;
				std::for_each(list.begin(),list.end(),std::bind2nd( std::mem_fun(&IEditorTool::setEnable),false) );
			}
		}

		{
			//BLADE_LW_PROFILING(SWITCH);
			for(ToolList::iterator i = mNotifiedTools.begin(); i != mNotifiedTools.end(); ++i)
				(*i)->onFileSwitch(efOld,efNew);

			for( FileTypeTool::iterator n = mTypedTool.begin(); n != mTypedTool.end(); ++n)
			{
				ToolList& toolist = n->second;
				for(ToolList::iterator i = toolist.begin(); i != toolist.end(); ++i)
					(*i)->onFileSwitch(efOld,efNew);	
			}
		}

		//update view related menu/tool
		bool enable = efNew != NULL && efNew->getView() != NULL;
		std::for_each(mViewMenus.begin(), mViewMenus.end(), std::bind2nd( std::mem_fun(&IMenu::setEnabled), enable) );
		std::for_each(mViewMenus.begin(), mViewMenus.end(), std::bind2nd( std::mem_fun(&IMenu::setChecked), false ) );

		//clear old entity selection
		EntitySlectedEvent entitySelState(NULL);
		IEventManager::getSingleton().dispatchEvent(entitySelState);
		//update widgets
		if (efNew != NULL)
		{
			const IEntity* entity = efNew->getSelectedEntity();
			EntitySlectedEvent state(entity);
			IEventManager::getSingleton().dispatchEvent(state);

			IUIMainWindow* ui = IEditorFramework::getSingleton().getEditorUI();
			if (ui != NULL)
			{
				IUIToolPanel* tp = static_cast<IUIToolPanel*>(ui->getLayoutManager()->getWidget(IEditorFramework::EDITOR_WIDGET_TOOL));
				if (tp != NULL)
					tp->switchCategory(efNew->getType());
				else
					tp->switchCategory( tp->getDefaultCategory() );

				//note: property grid is notified by EntitySlectedEvent, @see EntitySlectedEvent
				//IUIPropertyGrid* pg = static_cast<IUIPropertyGrid*>(ui->getWidget(IEditorFramework::EDITOR_WIDGET_PROPERTY));
				//const IEntity* selected = efNew->getSelectedEntity();
				//if (pg != NULL && selected != NULL)
				//	pg->bindEntity(selected);
			}
			else
				assert(false);
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool			CommonNotify::addCommonFileMenu(IMenu* menu)
	{
		assert(menu != NULL);
		menu->setEnabled(false);
		return mNotifiedMenus.insert(menu).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			CommonNotify::addViewRelatedMenu(IMenu* menu)
	{
		assert(menu != NULL);
		menu->setEnabled(false);
		return mViewMenus.insert(menu).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			CommonNotify::addCommonFileTool(IEditorTool* tool)
	{
		assert(tool != NULL);
		tool->setEnable(false);
		return mNotifiedTools.insert(tool).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			CommonNotify::addFileTypeTool(const TString& filetype,IEditorTool* tool)
	{
		assert(tool != NULL && filetype != TString::EMPTY );
		tool->setEnable(false);

		ToolList& toolList = mTypedTool[filetype];
		return toolList.insert(tool).second;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			CommonNotify::addFileTypeMenu(const TString& filetype,IMenu* menu)
	{
		assert(menu != NULL && filetype != TString::EMPTY );
		menu->setEnabled(false);

		MenuList& menuList = mTypedMenu[filetype];
		return menuList.insert(menu).second;
	}

}//namespace Blade
