/********************************************************************
	created:	2016/08/01
	filename: 	QtTableWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtPluginPCH.h>
#include <uiwindows/QtTableWindow.h>
#include <QtIconManager.h>

namespace Blade
{
	class QtTableDelegate : public QItemDelegate, public Allocatable
	{
	public:
		QtTableDelegate(QtTableWindow* table, TableDefController* controller, ITableWindow::IListener* listener) 
			: mHelper(table)
			, mTable(table)
			, mController(controller)
			, mListener(listener)
		{
			mUpdate = true;
			mHelper.initialize();
		}

		/** @brief  */
		virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const
		{
			int row = index.row();
			int col = index.column();
			assert( col < (int)mTable->mColumnDesc.size());
			IConfig* cfg = mTable->mColumnDesc[col];
			cfg->setBinaryValue(mTable->getCell(row, col));
			QWidget* editor = mHelper.createEditor(cfg, parent);
			editor->setAutoFillBackground(true);
			//editor->setGeometry(option.rect.adjusted(0, 0, 0, -1));
			return editor;
		}

		/** @brief  */
		virtual void destroyEditor(QWidget *editor, const QModelIndex &index) const
		{
			if(mUpdate)
			{
				int row = index.row();
				int col = index.column();
				if (col >= 0 && row >= 0)
				{
					assert(col < (int)mTable->mColumnDesc.size());
					IConfig* cfg = mTable->mColumnDesc[col];
					Variant v = cfg->getBinaryValue();
					//save value won't be treated as changed
					if (mTable->getCell(row, col) != v)
					{
						if (mListener != NULL && !mListener->onItemChange(row, col, v))
							return;
						mTable->getCell(row, col) = v;
					}
				}
			}
			editor->deleteLater();
		}

		/** @brief override default behavior (QItemDelegate switches between cells, but we let controller decide) */
		virtual bool eventFilter(QObject *object, QEvent *evt)
		{
			if (evt->type() == QEvent::KeyPress)
			{
				QKeyEvent* keyEvt = static_cast<QKeyEvent*>(evt);
				if (keyEvt->key() == Qt::Key_Tab || keyEvt->key() == Qt::Key_Backtab)
				{
					EKeyCode kc = QtPlugin::getSingleton().getKeyCode(keyEvt->key(), keyEvt->modifiers());
					uint32 mask = IUIService::getSingleton().getGlobalKeyboard()->getKeyModifier();
					return mController->onTableKeyDown(kc, mask);
				}
			}
			return false;
		}

		/** @brief  */
		inline void setUpdate(bool update) {mUpdate = update;}

		/** @brief  */
		inline void setListener(ITableWindow::IListener* listener) { mListener = listener; }

	protected:
		mutable QtPropertyHelper	mHelper;
		QtTableWindow*				mTable;
		TableDefController*			mController;
		ITableWindow::IListener*	mListener;
		bool						mUpdate;	//commit editing data
	};

	//////////////////////////////////////////////////////////////////////////
	QtTableWindow::QtTableWindow()
		:mStyle(TES_NONE)
		,mListener(NULL)
	{
		mTable.viewport()->installEventFilter(this);
		mTable.installEventFilter(this);
		mTable.horizontalHeader()->setSectionsMovable(false);
		this->setCentralWidget(&mTable);
		this->setAttribute(Qt::WA_DeleteOnClose, true);
		this->setContextMenuPolicy(Qt::NoContextMenu);
		mDelegate = BLADE_NEW QtTableDelegate(this, &mDefController, mListener);
		mTable.setItemDelegate(mDelegate);
		mFindRow = mFindCol = INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	QtTableWindow::~QtTableWindow()
	{
		this->clearContents();
		BLADE_DELETE mDelegate;
	}

	/************************************************************************/
	/* IUIWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		QtTableWindow::initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption/* = TString::EMPTY*/)
	{
		QWidget* parent = (QWidget*)parentWindowImpl;
		this->setParent(parent);
		QIcon* qicon = QtIconManager::getSingleton().getIcon(icon);
		if(qicon != NULL)
			this->setWindowIcon(*qicon);
		return this->init((uintptr_t)static_cast<QWidget*>(this), icon, caption, false, false);
	}

	/************************************************************************/
	/* ITableWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::initialize(const HCONFIG* columnDescList, size_t count, int style/* = 0*/, IListener* listener/* = NULL*/, IController* controller/* = NULL*/)
	{
		if(columnDescList == NULL || count == 0)
		{
			assert(false);
			return false;
		}

		mStyle = style;
		mListener = listener;
		mDelegate->setListener(listener);
		mDefController.initController(this, controller);

		mTable.setShowGrid( (mStyle&TES_GRIDVIEW) != 0);
		mTable.setSelectionMode((mStyle&TES_MULTI_SELECTION) ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
		mTable.setSelectionBehavior((mStyle&TES_FULL_ROW_SELECTION) ? QAbstractItemView::SelectRows : QAbstractItemView::SelectItems);
		mTable.verticalHeader()->setVisible( (mStyle&TES_SHOW_ROWNO) != 0 );

		QStringList headers;
		headers.reserve(count);
		mColumnDesc.resize(count);

		RedrawHelper helper(mTable);

		mFindRow = mFindCol = INVALID_INDEX;
		for(size_t i = 0; i < count; ++i)
		{
			const HCONFIG& config = *(columnDescList + i);
			const TString& columnText = BTString2Lang(config->getName());
			mTable.insertColumn((int)i);
			mColumnDesc[i] = config;
			headers.append( TString2QString(columnText) );
		}
		mTable.setHorizontalHeaderLabels(headers);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::addColumn(index_t index, HCONFIG columnDesc)
	{
		if( columnDesc == NULL || index > mColumnDesc.size() )
		{
			assert(false);
			return false;
		}
		assert( mColumnDesc.size() == mTable.columnCount() );

		RedrawHelper helper(mTable);

		const TString& columnText = BTString2Lang(columnDesc->getName());
		mTable.insertColumn((int)index);
		//mTable.setHorizontalHeaderLabels();

		Variant v = columnDesc->getBinaryValue();
		v.fromTString( columnDesc->getDefaultValue() );
		for(size_t i = 0; i < mContent.size(); ++i)
		{
			RowItem* rowItem = mContent[i];
			assert( rowItem->size() == mColumnDesc.size() );
			rowItem->insert(rowItem->begin()+(indexdiff_t)index, v);
		}
		mColumnDesc.insert(mColumnDesc.begin() + (indexdiff_t)index, columnDesc);
		this->refreshColumn(index);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::refreshColumn(index_t index)
	{
		assert(mTable.columnCount() == (int)mColumnDesc.size());
		if(index >= mColumnDesc.size())
		{
			assert(false);
			return false;
		}
		const TString& text = mColumnDesc[index]->getValue();

		for(int i = 0; i < mTable.rowCount(); ++i)
		{
			if( mTable.item(i, index) == NULL )
				mTable.setItem(i, index, QT_NEW QTableWidgetItem());
			QTableWidgetItem* qitem = mTable.item(i, index);
			qitem->setText( TString2QString(text) );
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::addRow(size_t index, const Variant* cols, size_t colCount, IconIndex icon/* = INVALID_ICON_INDEX*/)
	{
		assert(mTable.rowCount() == mContent.size());

		if( index > mContent.size() || colCount != mColumnDesc.size() || mColumnDesc.size() == 0 || cols == NULL)
		{
			assert(false);
			return false;
		}
		RowItem* rowItem = BLADE_NEW RowItem();
		rowItem->resize( mColumnDesc.size() );

		mTable.insertRow(index);
		mContent.insert(mContent.begin() + (indexdiff_t)index, rowItem);
		for(size_t i = 0; i < colCount; ++i)
		{
			IConfig* column = mColumnDesc[i];
			const Variant& typeCheck = column->getBinaryValue();
			if( cols[i].getType() != typeCheck.getType() )
			{
				assert(false && "column type mismatch.");
				return false;
			}
			(*rowItem)[i].reset(cols[i]);

			if( mTable.item(index, i) == NULL )
				mTable.setItem(index, i, QT_NEW QTableWidgetItem());

			QTableWidgetItem* qitem = mTable.item(index,i);
			qitem->setText( TString2QString(cols[i].toTString()) );

			if( i == 0 && icon != INVALID_ICON_INDEX)
			{
				QIcon* qicon = QtIconManager::getSingleton().getIcon(icon);
				if(qicon != NULL)
					qitem->setIcon(*qicon);
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	QtTableWindow::getSelectedCells(CELLPOS* items, size_t inputCount/* = 1*/) const
	{
		QList<QTableWidgetItem*> selection = mTable.selectedItems();
		size_t count = std::min<size_t>(selection.count(), inputCount);
		for(size_t i = 0; i < count; ++i)
		{
			QTableWidgetItem* qitem = selection[i];
			items[i].row = qitem->row();
			items[i].col = qitem->column();
		}
		return count;
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::findCell(FnCellCompare cmpFunc, CELLPOS& outPos, index_t columnIndex/* = INVALID_INDEX*/) const
	{
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
				startRow = mTable.currentRow();
				startCol = mTable.currentColumn() + 1;
			}
			else
			{
				startRow = mFindRow;
				startCol = mFindCol + 1;
			}

			if( startCol >= mTable.columnCount() )
			{
				startCol = 0;
				++startRow;
			}
			if( startRow >= (index_t)mTable.rowCount() )
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
					assert( col < mColumnDesc.size() && mColumnDesc[col] != NULL );

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
	bool	QtTableWindow::editCurrentCell(bool edit, bool editupdate)
	{
		int row = mTable.currentRow();
		int col = mTable.currentColumn();
		if (col < 0 && row < 0)
			return false;

		//commit data by deselect item
		mDelegate->setUpdate(editupdate);
		//mTable.editItem(NULL);
		mTable.setCurrentItem(NULL);

		QTableWidgetItem* qitem = mTable.item(row, col);
		mTable.setCurrentItem(qitem);

		//restore item
		if(edit)
		{
			assert(col < (int)mColumnDesc.size());
			if( !(mColumnDesc[col]->getAccess()&CAF_WRITE) || mColumnDesc[col]->getUIHint() == CUIH_NONE )
				return false;
			mTable.editItem(qitem);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::hitCell(const POINT2I& pos, CELLPOS& outCell) const
	{
		QTableWidgetItem* qitem = mTable.itemAt(pos.x, pos.y);
		if(qitem == NULL)
			return false;
		outCell.row = qitem->row();
		outCell.col = qitem->column();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::navigateTo(const CELLPOS& pos, bool edit, bool deselect)
	{
		QTableWidgetItem* qitem = mTable.item(pos.row, pos.col);
		if(qitem == NULL)
			return false;

		deselect = deselect || !(mStyle&TES_MULTI_SELECTION);

		mTable.setCurrentItem(qitem, deselect ? QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current : QItemSelectionModel::SelectCurrent);

		this->editCurrentCell(edit, true);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	QtTableWindow::navigateTo(ITEMNAV nav, bool edit, bool deselect)
	{
		QTableWidgetItem* qitem = mTable.currentItem();
		this->editCurrentCell(false, false);

		if(nav == TN_NONE || qitem == NULL)
		{
			mTable.clearSelection();
			mTable.setCurrentCell(0,0);
			return true;
		}

		int curRow = qitem->row();
		int curCol = qitem->column();

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
			mTable.setCurrentCell(0,0);

		mDefController.onSelectItem(this, nav, &pos.row, &pos.col);

		deselect = deselect || !(mStyle&TES_MULTI_SELECTION);

		qitem = mTable.item(pos.row, pos.col);
		mTable.setCurrentItem(qitem, deselect ? QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current : QItemSelectionModel::SelectCurrent);
		mTable.scrollToItem(qitem);

		this->editCurrentCell(edit, true);
		return true;
	}

}//namespace Blade