/********************************************************************
	created:	2015/08/09
	filename: 	TableWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TableWindow_h__
#define __Blade_TableWindow_h__
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <ui/UIToolBox.h>
#include <ui/private/UIFrameWnd.h>
#include "TableView.h"

namespace Blade
{
	class TableWindow : public CUIFrameWnd, public ITableWindow,  public Allocatable
	{
		DECLARE_DYNAMIC(TableWindow)
		DECLARE_MESSAGE_MAP()
	public:
		using Blade::Allocatable::operator new;
		using Blade::Allocatable::operator delete;
		using Blade::Allocatable::operator new[];
		using Blade::Allocatable::operator delete[];
	public:
		TableWindow();
		~TableWindow();

		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		/** @brief  */
		afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
		/** @brief  */
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		/** @brief  */
		afx_msg void OnSetFocus(CWnd* pOldWnd);

		/************************************************************************/
		/* IUIWindow interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool				initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption = TString::EMPTY);

		/** @brief  */
		virtual Blade::IUIToolBox*	createToolBox(const Blade::TString& name, ICONSIZE is);

		/** @brief  */
		virtual Blade::IUIToolBox*	getToolBox(const TString& name) const;

		/************************************************************************/
		/* ITableWindow interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param
		@return
		*/
		virtual bool	initialize(const HCONFIG* columnDescList, size_t count, int style = TES_NONE, IListener* listener = NULL, IController* controller = NULL);

		/*
		@describe 
		@param
		@return
		*/
		virtual IController*	getDefaultController() const		{return &mController;}

		/*
		@describe 
		@param
		@return
		*/
		virtual int		getStyle() const							{return mView.getStyle();}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	activate()									{if(mView.GetSafeHwnd() == NULL) mView.SetFocus(); return mView.GetSafeHwnd() != NULL;}

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t	getColumnCount() const						{return mView.getColumnCount();}

		/*
		@describe 
		@param
		@return
		*/
		virtual const HCONFIG&	getColumnDesc(index_t index) const	{return mView.getColumnDesc(index);}

		/*
		@describe 
		@param
		@return
		*/
		virtual index_t	findColumn(const TString& columnName) const	{return mView.findColumn(columnName);}

		/*
		@describe add one column, all item for this column is initialized with config's default value.
		@param
		@return
		*/
		virtual bool	addColumn(index_t index, HCONFIG columnDesc)	{return mView.addColumn(index, columnDesc); }

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	refreshColumn(index_t index)					{return mView.refreshColumn(index);}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	deleteColumn(index_t index)						{return mView.deleteColumn(index);}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	clearColumns()									{return mView.clearColumns();}

		/*
		@describe row count
		@param
		@return
		*/
		virtual size_t	getRowCount() const								{return mView.getRowCount();}

		/*
		@describe 
		@param
		@return
		*/
		virtual Variant*	getRow(index_t index)						{return mView.getRow(index);}
		virtual const Variant*	getRow(index_t index) const				{return mView.getRow(index);}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	beginAddRow(size_t totalRows/* = 0*/)
		{
			if (mView.GetSafeHwnd() != NULL)
			{
				mView.EnableRedraw(FALSE);
				if (totalRows != 0)
					mView.SetItemCount((int)totalRows);
			}
			return mView.GetSafeHwnd() != NULL;
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	endAddRow()
		{
			if( mView.GetSafeHwnd() != NULL )
				mView.EnableRedraw(TRUE);
			return mView.GetSafeHwnd() != NULL;
		}

		/*
		@describe insert one row. colCount must equal to current column count of the window.
		@param
		@return
		*/
		virtual bool	addRow(size_t index, const Variant* cols, size_t colCount, IconIndex icon = INVALID_ICON_INDEX)
		{
			return mView.addRow(index, cols, colCount, icon);
		}

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	deleteRow(index_t index)						{return mView.deleteRow(index);}

		/*
		@describe delete all rows
		@param
		@return
		*/
		virtual bool	clearRows()										{return mView.clearRows();}

		
		/*
		@describe get the visible row count according to current window size
		@param
		@return
		*/
		virtual size_t	getVisibleRowCount() const						{return (size_t)mView.GetCountPerPage();}

		/*
		@describe 
		@param
		@return
		*/
		virtual Variant& getCell(index_t row, index_t col)				{return *mView.getCell(row, col);}
		virtual const Variant& getCell(index_t row, index_t col) const	{return *mView.getCell(row, col);}

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t	getSelectedCount() const;

		/*
		@describe 
		@param
		@return
		*/
		virtual size_t	getSelectedCells(CELLPOS* items, size_t inputCount = 1) const;

		/*
		@describe 
		@param columnIndex: specify the column in which to find. INVALID_INDEX means find in all colmuns
		@return
		*/
		virtual bool	findCell(FnCellCompare cmpFunc, CELLPOS& outPos, index_t columnIndex = INVALID_INDEX) const;

		/*
		@describe 
		@param [in] edit whether to edit current item
		@param [in] if edit is false and current item is being edited, whether to apply editing change.\n
		if edit is true, this parameter is ignored.
		@return
		*/
		virtual bool	editCurrentCell(bool edit, bool editupdate)
		{
			if( mView.GetSafeHwnd() == NULL)
				return false;
			index_t row = mView.getSelRow();
			index_t col = mView.getSelCol();
			if( row >= this->getRowCount() || col >= this->getColumnCount() )
				return false;
			if(edit)
				return mView.selectCell(row, col, edit, SM_SINGLE);
			else
				mView.endEditing(editupdate);
			return true;
		}

		/*
		@describe hit test
		@param [in] pos: position in window coordinate
		@return
		*/
		virtual bool	hitCell(const POINT2I& pos, CELLPOS& outCell) const
		{
			if( mView.GetSafeHwnd() == NULL || this->GetSafeHwnd() == NULL )
				return false;

			//the position mostly came from controller mouse event call back,
			//which use this window's coordinates, we need convert it to table view's coordinates
			POINT pt = {pos.x, pos.y};
			this->ClientToScreen(&pt);
			mView.ScreenToClient(&pt);

			LVHITTESTINFO info = {{pt.x, pt.y}, 0, -1, -1};
			mView.SubItemHitTest(&info);
			//if( info.flags & (LVHT_ONITEM) )
			index_t row = (index_t)info.iItem;
			index_t col = (index_t)info.iSubItem;
			if( row < this->getRowCount() && col < this->getColumnCount() )
			{
				outCell.row = row;
				outCell.col = col;
				return true;
			}
			return false;
		}

		
		/*
		@describe 
		@param
		@return
		*/
		virtual bool	navigateTo(const CELLPOS& pos, bool edit, bool deselect);

		/*
		@describe 
		@param
		@return
		*/
		virtual bool	navigateTo(ITEMNAV nav, bool edit, bool deselect);

		/** @brief internally used */
		TableView&		getTableView() const	{return mView;}

	protected:

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		void updateLayout()
		{
			if( mView.GetSafeHwnd() != NULL )
			{
				this->RecalcLayout();
				CRect rect;
				this->GetClientRect(&rect);
				CRect toolbarRect;
				this->GetControlBar(AFX_IDW_DOCKBAR_TOP)->GetWindowRect(&toolbarRect);
				this->ScreenToClient(&toolbarRect);
				rect.top = toolbarRect.bottom;
				mView.MoveWindow(&rect);
				mView.AutoSizeColumn();
			}
		}

		static const TCHAR*	mTableWindowClassName;
		typedef Vector<UIToolBox*>	ToolbarList;
		typedef Set<int>			RowSelection;
		
		mutable TableView	mView;
		ToolbarList			mToolBars;
		mutable index_t		mFindRow;	//starting row for find
		mutable index_t		mFindCol;	//starting row for find
		mutable TableDefController	mController;
	};// class TableWindow

}//namespace Blade



#endif // __Blade_TableWindow_h__