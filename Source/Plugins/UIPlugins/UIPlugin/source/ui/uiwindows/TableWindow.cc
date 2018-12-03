/********************************************************************
	created:	2015/08/09
	filename: 	TableWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "resource.h"
#include "TableWindow.h"
#include <interface/public/window/IWindowService.h>

namespace Blade
{
	const TCHAR*	TableWindow::mTableWindowClassName = NULL;

	IMPLEMENT_DYNAMIC(TableWindow, CUIFrameWnd);

	BEGIN_MESSAGE_MAP(TableWindow, CUIFrameWnd)
		ON_WM_WINDOWPOSCHANGED()
		ON_WM_SHOWWINDOW()
		ON_WM_SETFOCUS()
	END_MESSAGE_MAP()
	
	//////////////////////////////////////////////////////////////////////////
	TableWindow::TableWindow()
	{
		mFindRow = INVALID_INDEX;
		mFindCol = INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	TableWindow::~TableWindow()
	{
		for(size_t i = 0; i < mToolBars.size(); ++i)
			BLADE_DELETE mToolBars[i];
	}

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	void TableWindow::OnWindowPosChanged(WINDOWPOS* lpwndpos)
	{
		__super::OnWindowPosChanged(lpwndpos);
		this->updateLayout();
	}

	//////////////////////////////////////////////////////////////////////////
	void TableWindow::OnShowWindow(BOOL bShow, UINT nStatus)
	{
		if(bShow && mView.GetSafeHwnd() != NULL)
			mView.SetFocus();
		return __super::OnShowWindow(bShow, nStatus);
	}

	//////////////////////////////////////////////////////////////////////////
	void TableWindow::OnSetFocus(CWnd* /*pOldWnd*/)
	{
		if( mView.GetSafeHwnd() != NULL)
			mView.SetFocus();
	}

	/************************************************************************/
	/* IUIWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				TableWindow::initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption/* = TString::EMPTY*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		HWND hWnd = (HWND)(void*)parentWindowImpl;
		if( hWnd == NULL )
		{
			assert(false);
			return false;
		}

		if(mTableWindowClassName == NULL )
		{
			mTableWindowClassName =
				::AfxRegisterWndClass(0,
				LoadCursor(NULL, IDC_ARROW),
				(HBRUSH) (COLOR_WINDOW+1),
				NULL);
		}

		CWnd* parent = CWnd::FromHandle(hWnd);
		BOOL ret =  CFrameWnd::CreateEx(0, mTableWindowClassName, TEXT(""), WS_VISIBLE|WS_CHILD, CFrameWnd::rectDefault, parent, IDC_TABLE_WINDOW);

		if(ret)
		{
			this->init((uintptr_t)m_hWnd, icon, caption, false, false);
			this->EnableDocking(CBRS_ALIGN_TOP);
		}

		return ret != FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	Blade::IUIToolBox*	TableWindow::createToolBox(const Blade::TString& name, ICONSIZE is)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		size_t width = 0;
		for(size_t i = 0; i < mToolBars.size(); ++i)
			width += mToolBars[i]->getWidth();

		UIToolBox* box = BLADE_NEW UIToolBox(name, is, AFX_IDW_TOOLBAR, this, width, true, false);
		mToolBars.push_back(box);
		return box;
	}

	//////////////////////////////////////////////////////////////////////////
	Blade::IUIToolBox*	TableWindow::getToolBox(const TString& name) const
	{
		for(size_t i = 0; i < mToolBars.size(); ++i)
		{
			if( mToolBars[i]->getName() == name )
				return mToolBars[i];
		}
		return NULL;
	}

	/************************************************************************/
	/* ITableWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	TableWindow::initialize(const HCONFIG* columnDescList, size_t count, int style/* = 0*/, IListener* listener/* = NULL*/, IController* controller/* = NULL*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		if( this->GetSafeHwnd() == NULL )
		{
			assert(false);
			return false;
		}

		bool ret = mView.GetSafeHwnd() != NULL;
		if( !ret )
			ret = mView.Create(this, CFrameWnd::rectDefault, IDC_TABLE_VIEW) != FALSE;

		if(ret)
		{
			mController.initController(this, controller);
			ret = mView.initialize(columnDescList, count, style);
			ret = ret && mView.setListener(listener) && mView.setController(&mController);
			this->updateLayout();
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	TableWindow::getSelectedCount() const
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		//multi selection
		size_t count = 0;
		POSITION pos = mView.GetFirstSelectedItemPosition();
		while(pos != NULL)
		{
			++count;
			mView.GetNextSelectedItem(pos);
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	TableWindow::getSelectedCells(CELLPOS* items, size_t inputCount/* = 1*/) const
	{
		if( inputCount == 0 )
		{
			assert(false);
			return false;
		}
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		index_t row = mView.getSelRow();
		index_t col = mView.getSelCol();
		if( row >= this->getRowCount() || col >= this->getColumnCount() )
			return 0;

		//then 1 item selected, we use SelRow & SelCol
		//otherwise the client code may take the CListCtrl's selection directly
		//i.e. on first mouse click, the CListCtrl's default behavior will set a selection
		//and client will know it already selected, and goto editing mode.
		//but we need enter editing mode on second time's click
		if( mView.GetSelectedCount() <= 1)
		{
			items[0].row = row;
			items[0].col = col;
			return 1;
		}

		size_t count = 0;
		POSITION pos = mView.GetFirstSelectedItemPosition();
		while(pos != NULL && count < inputCount)
		{
			index_t index = (index_t)mView.GetNextSelectedItem(pos);
			items[count].row = index;
			items[count].col = col;
			++count;
		}
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TableWindow::findCell(FnCellCompare cmpFunc, CELLPOS& outPos, index_t columnIndex/* = INVALID_INDEX*/) const
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		index_t startCol = 0;
		index_t endCol = this->getColumnCount();
		index_t startRow = 0;
		index_t endRow = this->getRowCount();

		FindContext context = {this, 0, 0};

		if(columnIndex != INVALID_INDEX)
		{
			startCol = columnIndex;
			endCol = startCol + 1;
		}
		else
		{
			context.row = mFindRow;
			context.col = mFindCol;

			if( mFindRow >= endRow || mFindCol >= endCol ||
				!cmpFunc(this->getCell(mFindRow, mFindCol), context) )
			{
				startRow = mView.getSelRow();
				startCol = mView.getSelCol() + 1;
			}
			else
			{
				startRow = mFindRow;
				startCol = mFindCol + 1;
			}

			if( startCol >= mView.getColumnCount() )
			{
				startCol = 0;
				++startRow;
			}
			if( startRow >= (index_t)mView.GetItemCount() )
				startRow = 0;
		}

		index_t foundRow = INVALID_INDEX;
		index_t foundCol = INVALID_INDEX;
		int count = 0;
		do
		{
			for(size_t row = startRow; row < endRow; ++row)
			{
				for(size_t col = startCol; col < endCol; ++col)
				{
					assert( mView.getColumnDesc(col) != NULL );

					const Variant& v = this->getCell(row, col);
					context.row = row;
					context.col = col;
					if( cmpFunc(v, context) )
					{
						//locate the item
						foundRow =  row;
						foundCol = col;
						break;
					}
				}
				if( foundRow != INVALID_INDEX )
					break;

				//start form first col on next row
				if(columnIndex != INVALID_INDEX)
				{
					startCol = 0;
					endCol = this->getColumnCount();
				}
			}

			if( startRow == 0 || foundRow != INVALID_INDEX )
				break;
			//wrap around from the beginning
			endRow = startRow-1;
			startRow = 0;

		}while(++count < 2);

		mFindRow = foundRow;
		mFindCol = foundCol;
		outPos.row = foundRow;
		outPos.col = foundCol;
		return foundRow != INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TableWindow::navigateTo(const CELLPOS& pos, bool edit, bool deselect)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		size_t rowCount = this->getRowCount();
		size_t colCount = this->getColumnCount();
		if( pos.row >= rowCount || pos.col >= colCount )
			return false;
		if( this->GetSafeHwnd() == NULL || mView.GetSafeHwnd() == NULL )
			return false;
		return mView.selectCell(pos.row, pos.col, edit, deselect ? SM_SINGLE : SM_ADD);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TableWindow::navigateTo(ITEMNAV nav, bool edit, bool deselect)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		if( nav == TN_NONE )
		{
			mView.EnsureVisible(0, FALSE);
			mView.resetSelection();
			return true;
		}

		size_t rowCount = this->getRowCount();
		size_t colCount = this->getColumnCount();

		if(nav < TN_LEFT || nav > TN_END || rowCount == 0 || colCount == 0)
		{
			assert(nav >= TN_LEFT && nav <= TN_END);
			return false;
		}

		CELLPOS pos;
		bool prevSel = this->getSelectedCells(&pos) != 0;
		if( !prevSel )
			mView.selectCell(0, 0, false, SM_SINGLE);

		mController.onSelectItem(this, nav, &pos.row, &pos.col);
		return mView.selectCell(pos.row, pos.col, edit, deselect ? SM_SINGLE : SM_ADD);
	}
	
}//namespace Blade

