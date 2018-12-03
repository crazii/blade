/********************************************************************
	created:	2016/08/01
	filename: 	QtTableWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtTableWindow_h__
#define __Blade_QtTableWindow_h__
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <Qtwindow.h>
#include <QtPlugin.h>

namespace Blade
{
	class QtTableDelegate;

	class QtTableWindow : public ITableWindow, public QtWindow, public Allocatable
	{
	public:
		QtTableWindow();
		~QtTableWindow();

		/************************************************************************/
		/* IUIWindow interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool		initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption = TString::EMPTY);

		/** @brief  */
		virtual IUIToolBox*	createToolBox(const TString& name, ICONSIZE is)
		{
			QtToolbox* toolBox = QtWindow::createQtToolBox(name, is, Qt::TopToolBarArea, false);
			return toolBox;
		}

		/** @brief  */
		virtual IUIToolBox*	getToolBox(const TString& name) const {return QtWindow::getQtToolBox(name);}

		/************************************************************************/
		/* ITableWindow interface                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param
		@return
		*/
		virtual bool	initialize(const HCONFIG* columnDescList, size_t count, int style = 0, IListener* listener = NULL, IController* controller = NULL);

		/**
		@describe 
		@param
		@return
		*/
		virtual IController*	getDefaultController() const {return &mDefController;}

		/**
		@describe 
		@param
		@return
		*/
		virtual int		getStyle() const {return mStyle;}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	activate() {mTable.activateWindow(); return true;}
		
		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getColumnCount() const {return (size_t)mTable.columnCount();}

		/**
		@describe 
		@param
		@return
		*/
		virtual const HCONFIG&	getColumnDesc(index_t index) const
		{
			return index < mColumnDesc.size() ? mColumnDesc[index] : HCONFIG::EMPTY;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual index_t	findColumn(const TString& columnName) const
		{
			FnHConfigFindByName finder(columnName);
			ColumnList::const_iterator i = std::find_if(mColumnDesc.begin(), mColumnDesc.end(), finder);
			if( i != mColumnDesc.end() )
				return index_t(i - mColumnDesc.begin());
			else
				return INVALID_INDEX;
		}

		/**
		@describe add one column, all item for this column is initialized with config's default value.
		@param
		@return
		*/
		virtual bool	addColumn(index_t index, HCONFIG columnDesc);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	refreshColumn(index_t index);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	deleteColumn(index_t index)
		{
			assert((int)mColumnDesc.size() == mTable.columnCount());
			if((int)index < mTable.columnCount())
			{
				mTable.removeColumn(index);
				mColumnDesc.erase( mColumnDesc.begin() + (indexdiff_t)index);
				for(size_t i = 0; i < mContent.size(); ++i)
					mContent[i]->erase( mContent[i]->begin() + (indexdiff_t)index);
				return true;
			}
			return false;
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	clearColumns() {mTable.clear(); mColumnDesc.clear(); mTable.setRowCount(0); mTable.setColumnCount(0); this->clearContents(); return true;}

		/**
		@describe row count
		@param
		@return
		*/
		virtual size_t	getRowCount() const {return (size_t)mTable.rowCount();}

		/**
		@describe 
		@param
		@return
		*/
		virtual Variant*	getRow(index_t index)
		{
			return this->getRowImpl(index);
		}

		virtual const Variant*	getRow(index_t index) const
		{
			return this->getRowImpl(index);
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	beginAddRow(size_t totalRows = 0) { mTable.setUpdatesEnabled(false); return true;}

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	endAddRow() { mTable.setUpdatesEnabled(true); mTable.resizeColumnsToContents(); return true; }

		/**
		@describe insert one item. colCount must equal to current column count of the window.
		@param
		@return
		*/
		virtual bool	addRow(size_t index, const Variant* cols, size_t colCount, IconIndex icon = INVALID_ICON_INDEX);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	deleteRow(index_t index)
		{
			assert( mTable.rowCount() == (int)mContent.size() );
			if((int)index < mTable.rowCount())
			{
				mTable.removeRow((int)index);
				RowItem* item = mContent[index];
				BLADE_DELETE item;
				mContent.erase( mContent.begin() + (indexdiff_t)index);
				return true;
			}
			return false;
		}

		/**
		@describe delete all rows
		@param
		@return
		*/
		virtual bool	clearRows() { mTable.clearContents(); mTable.setRowCount(0); this->clearContents(); return true;}

		/**
		@describe get the visible row count according to current window size
		@param
		@return
		*/
		virtual size_t	getVisibleRowCount() const
		{
			return mTable.rowAt(mTable.height()) - mTable.rowAt(0);
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual Variant& getCell(index_t row, index_t col)
		{
			return this->getCellImpl(row, col);
		}
		virtual const Variant& getCell(index_t row, index_t col) const
		{
			return this->getCellImpl(row, col);
		}

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getSelectedCount() const {return mTable.selectedItems().count();}

		/**
		@describe 
		@param
		@return
		*/
		virtual size_t	getSelectedCells(CELLPOS* items, size_t inputCount = 1) const;

		/**
		@describe 
		@param columnIndex: specify the column in which to find. INVALID_INDEX means find in all columns
		@return
		*/
		virtual bool	findCell(FnCellCompare cmpFunc, CELLPOS& outPos, index_t columnIndex = INVALID_INDEX) const;

		/**
		@describe 
		@param [in] edit whether to edit current cell
		@param [in] if edit is false and current cell is being edited, whether to apply editing change.\n
		if edit is true, this parameter is ignored.
		@return
		*/
		virtual bool	editCurrentCell(bool edit, bool editupdate);

		/**
		@describe hit test
		@param [in] pos: position in window coordinate
		@return
		*/
		virtual bool	hitCell(const POINT2I& pos, CELLPOS& outCell) const;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	navigateTo(const CELLPOS& pos, bool edit, bool deselect);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	navigateTo(ITEMNAV nav, bool edit, bool deselect);

	protected:
		/** @brief  */
		inline Variant*	getRowImpl(index_t index) const
		{
			assert(mTable.rowCount() == (int)mContent.size());
			if(index < mContent.size() && mContent[index]->size() > 0)
				return &(*mContent[index])[0];
			return NULL;
		}
		/** @brief  */
		inline Variant&	getCellImpl(index_t row, index_t col) const
		{
			assert(mTable.rowCount() == (int)mContent.size() );
			assert(mTable.columnCount() == (int)mColumnDesc.size() );
			if(col < mColumnDesc.size() && row < mContent.size() )
			{
				RowItem* item = mContent[row];
				assert(item != NULL && item->size() == mColumnDesc.size());
				return (*item)[col];
			}
			BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("row or column index out of range"));
		}
		/** @brief  */
		inline void clearContents()
		{
			for(size_t i = 0; i < mContent.size(); ++i)
				BLADE_DELETE mContent[i];
			mContent.clear();
		}

		/************************************************************************/
		/* Qt events                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool eventFilter(QObject *watched, QEvent *evt)
		{
			QMouseEvent* msEvt = static_cast<QMouseEvent*>(evt);
			int b = -1;

			switch(evt->type())
			{
				case QEvent::MouseButtonPress:
				case QEvent::MouseButtonDblClick:
					if(msEvt->button() == Qt::LeftButton)
						b = MSB_LEFT;
					else if(msEvt->button() == Qt::RightButton)
						b = MSB_RIGHT;
					else if(msEvt->button() == Qt::MiddleButton)
						b = MSB_MIDDLE;
					if(b != -1)
					{
						if(evt->type() == QEvent::MouseButtonPress)
							return mDefController.onTableMouseButtonDown((MSBUTTON)b, POINT2I(msEvt->pos().x(), msEvt->pos().y()));
						else
							return mDefController.onTableMouseButtonDBClick((MSBUTTON)b, POINT2I(msEvt->pos().x(), msEvt->pos().y()));
					}
					break;
				case QEvent::KeyPress:
					{
						QKeyEvent* keyEvt = static_cast<QKeyEvent*>(evt);
						EKeyCode kc = QtPlugin::getSingleton().getKeyCode(keyEvt->key(), keyEvt->modifiers());
						uint32 mask = IUIService::getSingleton().getGlobalKeyboard()->getKeyModifier();
						return mDefController.onTableKeyDown(kc, mask);
					}
			}
			return false;
		}

		struct RedrawHelper
		{
			QTableWidget& table;
			RedrawHelper(QTableWidget& t) :table(t) { table.setUpdatesEnabled(false); }
			~RedrawHelper()	{table.setUpdatesEnabled(true);}
		};

		typedef Vector<HCONFIG>	ColumnList;
		struct RowItem : public Vector<Variant>, public Allocatable	{};
		typedef Vector<RowItem*>	TableContent;

		QTableWidget	mTable;
		ColumnList		mColumnDesc;
		TableContent	mContent;
		mutable TableDefController	mDefController;
		IListener*		mListener;
		int				mStyle;//EStyle
		QtTableDelegate*mDelegate;
		mutable index_t	mFindRow;	//last finding pos, for continue finding mode 
		mutable index_t	mFindCol;
		friend class QtTableDelegate;
	};

	
}//namespace Blade


#endif // __Blade_QtTableWindow_h__