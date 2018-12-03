/********************************************************************
	created:	2015/09/16
	filename: 	TableCommand.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TableCommand_h__
#define __Blade_TableCommand_h__
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include "TableWindow.h"
#include "TableView.h"
#include <interface/public/ui/IUICommand.h>
#include <ui/MainWindow.h>

namespace Blade
{
	//delete current
	class TableViewCmd : public IUICommand
	{
	public:
		/** @brief  */
		virtual void  executeImpl(ITableWindow* window) = 0;

		/** @brief  */
		virtual void execute(ICommandUI* /*cui*/)
		{
			if (MainWindow::getSingleton().getLayoutManager() != NULL)
			{
				IUIWindow* window = MainWindow::getSingleton().getLayoutManager()->getActiveWindow();
				if (window->getWindowType() != ITableWindow::TABLE_WINDOW_TYPE)
					return;
				this->executeImpl(static_cast<ITableWindow*>(window));
			}
			else
			{
				CWnd* activeWindow = CWnd::GetActiveWindow();
				TableWindow* tableWindow = NULL;
				TableView* tableView = NULL;
				tableWindow = DYNAMIC_DOWNCAST(TableWindow, activeWindow);
				if (tableWindow == NULL && (tableView = DYNAMIC_DOWNCAST(TableView, activeWindow)) != NULL)
					tableWindow = DYNAMIC_DOWNCAST(TableWindow, tableView->GetParent());
				if(tableWindow != NULL)
					this->executeImpl(tableWindow);
			}
		}

		/** @brief  */
		virtual bool update(ICommandUI* cui, index_t )
		{
			if (MainWindow::getSingleton().getLayoutManager() != NULL)
			{
				IUIWindow* window = MainWindow::getSingleton().getLayoutManager()->getActiveWindow();
				if (window->getWindowType() != ITableWindow::TABLE_WINDOW_TYPE)
					return false;
				cui->setEnabled(static_cast<ITableWindow*>(window)->getSelectedCount() > 0);
				return true;
			}
			else
			{
				CWnd* activeWindow = CWnd::GetActiveWindow();
				TableWindow* tableWindow = NULL;
				TableView* tableView = NULL;
				tableWindow = DYNAMIC_DOWNCAST(TableWindow, activeWindow);
				if (tableWindow == NULL && (tableView = DYNAMIC_DOWNCAST(TableView, activeWindow)) != NULL)
					tableWindow = DYNAMIC_DOWNCAST(TableWindow, tableView->GetParent());
				if (tableWindow != NULL)
				{
					cui->setEnabled(tableWindow->getSelectedCount() > 0);
					return true;
				}
			}
			return false;
		}
	};//class TableViewCmd

	class TableViewDeleteCmd : public TableViewCmd, public Singleton<TableViewDeleteCmd>
	{
	public:
		/** @brief  */
		virtual void  executeImpl(ITableWindow* window);
	};//class TableViewDeleteCmd

	class TableViewRangeDeleteCmd : public TableViewCmd, public Singleton<TableViewRangeDeleteCmd>
	{
	public:
		/** @brief  */
		virtual void  executeImpl(ITableWindow* window);
	};//class TableViewRangeDeleteCmd

	class TableViewInsertCmd : public TableViewCmd, public Singleton<TableViewInsertCmd>
	{
		/** @brief  */
		virtual void  executeImpl(ITableWindow* window);
	};

}//namespace Blade


#endif // __Blade_TableCommand_h__