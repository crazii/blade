/********************************************************************
	created:	2015/09/16
	filename: 	TableCommand.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "TableCommand.h"
#include "TableWindow.h"
#include "TableView.h"

namespace Blade
{
	
	//////////////////////////////////////////////////////////////////////////
	void  TableViewDeleteCmd::executeImpl(ITableWindow* window)
	{
		TableWindow* win = static_cast<TableWindow*>(window);
		win->getTableView().deleteCurrentRow();
	}

	//////////////////////////////////////////////////////////////////////////
	void  TableViewRangeDeleteCmd::executeImpl(ITableWindow* window)
	{
		TableWindow* win = static_cast<TableWindow*>(window);
		win->getTableView().deleteSelectedRows();
	}

	//////////////////////////////////////////////////////////////////////////
	void  TableViewInsertCmd::executeImpl(ITableWindow* window)
	{
		TableWindow* win = static_cast<TableWindow*>(window);
		win->getTableView().insertRow( win->getTableView().getSelRow()+1 );
	}

}//namespace Blade
