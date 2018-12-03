/********************************************************************
	created:	2015/08/09
	filename: 	TableView.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TableView_h__
#define __Blade_TableView_h__
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <ConfigDialog/ConfigControlUtil.h>
#include <ui/MenuManager.h>
#include <ui/private/UIListCtrl.h>

class ConfigControl;

namespace Blade
{	
	/** @brief  */
	enum ESelectMethod
	{
		SM_SINGLE,	//single select
		SM_ADD,		//append new selection
		SM_RANGE,	//range selection
	};

	class TableView : public CUIListCtrl, public Allocatable
	{
		DECLARE_DYNAMIC(TableView)
		DECLARE_MESSAGE_MAP()
	public:
		using Blade::Allocatable::operator new;
		using Blade::Allocatable::operator delete;
		using Blade::Allocatable::operator new[];
		using Blade::Allocatable::operator delete[];
	public:
		TableView();
		~TableView();

		/************************************************************************/
		/* MFC Related                                                                     */
		/************************************************************************/
		afx_msg	void	OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void	OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg	void	OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void	OnRButtonDblClk(UINT nFlags, CPoint point);
		afx_msg	void	OnMouseMove(UINT nFlags, CPoint point);
		afx_msg	void	OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg	void	OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
		afx_msg void	OnNMDisableDefaultSelectState(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnNMEndScroll(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void	OnPaint();
		afx_msg UINT	OnGetDlgCode()	{return /*DLGC_WANTALLKEYS|*/DLGC_WANTARROWS|DLGC_WANTTAB;}
		afx_msg void	OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
		afx_msg void	OnNcPaint();
		afx_msg LRESULT	OnNcHitTest(CPoint point);
		afx_msg void	OnNcLButtonDown(UINT nHitTest, CPoint point);
		afx_msg void	OnNcLButtonDblClk(UINT nHitTest, CPoint point)		{__super::OnNcLButtonDblClk(nHitTest, point); this->OnNcLButtonDown(nHitTest,point);}
		afx_msg void	OnNcLButtonUp(UINT nHitTest, CPoint point);
		afx_msg void	OnNcRButtonUp(UINT nHitTest, CPoint point);
		virtual BOOL	PreTranslateMessage(MSG* pMsg);

		/** @brief  */
		virtual BOOL	Create(CWnd* pParent,const RECT& rect,UINT nID);
		/** @brief  */
		void			AutoSizeColumn()
		{
			for(size_t i = 0; i < mColumnDesc.size(); ++i)
			{
				this->SetColumnWidth((int)i, LVSCW_AUTOSIZE);
				this->SetColumnWidth((int)i, LVSCW_AUTOSIZE_USEHEADER);
			}
		}
		/** @brief  */
		inline void EnableRedraw(BOOL Enable)
		{
			if( Enable )
			{
				if( ++mReDrawEnableCount == 1 )
				{
					this->AutoSizeColumn();
					this->SetRedraw(Enable);
					this->Invalidate();
					this->UpdateWindow();
					this->updateRowHeader();
				}
			}
			else
			{
				if( --mReDrawEnableCount == 0 )
					this->SetRedraw(Enable);
			}
		}
		/** @brief  */
		inline ConfigControl* RepositionControl()
		{
			const CONFIG_UIHINT& hint = mColumnDesc[mSelCol]->getUIHint();
			assert( hint >= CUIH_START && hint < CUIH_COUNT );
			CRect rect;
			CRect iconRect;
			this->GetSubItemRect((int)mSelRow, (int)mSelCol, LVIR_BOUNDS, rect);
			this->GetSubItemRect((int)mSelRow, (int)mSelCol, LVIR_ICON, iconRect);
			rect.left = iconRect.right;
			//the first item (col 0) is the main item, the rect contains the entire row
			if( mSelCol == 0 && this->getColumnCount() > 1 )
			{
				this->GetSubItemRect((int)mSelRow, (int)mSelCol+1, LVIR_BOUNDS, iconRect);
				rect.right = iconRect.left;
			}
			//why? the CListCtrl on Win7 can't calculate last item right if NC area is preserved
			else if( mSelCol == this->getColumnCount() - 1 && this->hasRowHeader() )
				rect.right -= mRowHeaderWidth;
			CWnd* window = mConfigControls[hint]->GetWindow();
			rect.DeflateRect(2,2,2,2);
			window->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
			window->Invalidate(FALSE);
			return mConfigControls[hint];
		}

		/** @brief  */
		inline void startEditing()
		{
			if(mSelRow >= (size_t)this->GetItemCount() || mSelCol >= this->getColumnCount() )
			{
				assert(false);
				return;
			}
			if( !(mColumnDesc[mSelCol]->getAccess()&CAF_WRITE) || mColumnDesc[mSelCol]->getUIHint() == CUIH_NONE )
				return;
			if(mEditing)
				return;
			mEditing = true;

			//update control pos
			ConfigControl* control = this->RepositionControl();
			//current table item value
			const Variant& text = *this->getCell(mSelRow, mSelCol);

			const CONFIG_UIHINT& hint = mColumnDesc[mSelCol]->getUIHint();
			assert( hint >= CUIH_START && hint < CUIH_COUNT );
			CWnd* window = mConfigControls[hint]->GetWindow();

			const IConfig::Option& options = mColumnDesc[mSelCol]->getOptions();
			IConfig* config = control->GetConfig();

			config->setUIHint( mColumnDesc[mSelCol]->getUIHint() );
			if(options.size() > 0)
				config->setOptions(&options[0], options.size());
			//fill control data from table item
			config->setValue(text.toTString());
			config->backupValue();
			window->ShowWindow(SW_SHOW);

			//TODO: control's binary value type may mismatch column desc's type!
			//there may be assertion failure if using binary value directly
			control->updateData(true);
			window->SetFocus();
		}

		/** @brief  */
		inline void endEditing(bool update)
		{
			if(mSelRow >= (size_t)this->GetItemCount() || mSelCol >= this->getColumnCount() )
				return;
			if( !(mColumnDesc[mSelCol]->getAccess()&CAF_WRITE) || mColumnDesc[mSelCol]->getUIHint() == CUIH_NONE )
				return;
			if(!mEditing)
				return;
			mEditing = false;

			EConfigUIHint hint = mColumnDesc[mSelCol]->getUIHint();
			assert( hint >= CUIH_START && hint < CUIH_COUNT );
			mConfigControls[hint]->GetWindow()->ShowWindow(SW_HIDE);
			this->SetFocus();
			
			if(!update)
				return;

			//save last selected data
			const TString& newData = mConfigControls[hint]->GetConfig()->getValue();
			CString oldData = this->GetItemText((int)mSelRow, (int)mSelCol);
			if( newData != oldData.GetString() )
			{
				Variant v = *this->getCell(mSelRow, mSelCol);
				//try to apply data
				v.fromTString(newData);

				if( mListener != NULL && !mListener->onItemChange(mSelRow, mSelCol, v) )
					return;

				Variant& cell = *this->getCell(mSelRow, mSelCol);
				cell = v;
				//get new data (new data may not match applied data, in case of failure)
				const TString realData = cell.toTString();
				this->SetItemText((int)mSelRow, (int)mSelCol, realData.c_str());
				this->AutoSizeColumn();
			}
		}

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool		initialize(const HCONFIG* columnDesc, size_t columnCount, int style);

		/** @brief  */
		inline size_t	getColumnCount() const
		{
			return mColumnDesc.size();
		}

		/** @brief  */
		inline const HCONFIG&	getColumnDesc(index_t index) const
		{
			return index < mColumnDesc.size() ? mColumnDesc[index] : HCONFIG::EMPTY;
		}

		/** @brief  */
		index_t		findColumn(const TString& header) const;

		/** @brief  */
		bool		addColumn(index_t index, HCONFIG columnDesc);

		/** @brief  */
		bool		refreshColumn(index_t index);

		/** @brief  */
		bool		deleteColumn(index_t index);

		/** @brief  */
		bool		clearColumns();

		/** @brief  */
		size_t		getRowCount() const	{return (size_t)this->GetItemCount();}

		/** @brief  */
		Variant*	getRow(index_t index) const;
		Variant*	getRow(index_t index)
		{
			const TableView* view = this;
			return view->getRow(index);
		}

		/** @brief  */
		bool		addRow(size_t index, const Variant* cols, size_t colCount, IconIndex icon = INVALID_ICON_INDEX);

		/** @brief  */
		bool		deleteRow(index_t index);

		/** @brief  */
		bool		clearRows();

		/** @brief  */
		enum EMouseState
		{
			NONE,
			DOWN,
			UP,
		};
		/** @brief  */
		bool		updateRowHeader(POINT* pt = NULL, EMouseState mouseState = NONE);

		/** @brief  */
		bool		selectCell(index_t row, index_t col, bool edit, ESelectMethod method);

		/** @brief  */
		Variant*	getCell(index_t row, index_t col) const;
		Variant*	getCell(index_t row, index_t col)
		{
			const TableView* view = this;
			return view->getCell(row, col);
		}

		/** @brief  */
		inline index_t getSelRow() const		{return mSelRow;}
		/** @brief  */
		inline index_t	getSelCol() const		{return mSelCol;}

		/** @brief  */
		inline void	resetSelection() {mSelRow = INVALID_INDEX; mSelCol = INVALID_INDEX; mRangeStart = INVALID_INDEX;}

		/** @brief  */
		bool	setListener(ITableWindow::IListener* listener)
		{
			if( mListener == NULL )
				mListener = listener;
			return mListener == listener;
		}

		/** @brief  */
		bool	setController(TableDefController* controller)
		{
			if( mController == NULL )
				mController = controller;
			return mController == controller;
		}

		/** @brief  */
		bool	hasRowNumber() const
		{
			return (mStyle&ITableWindow::TES_SHOW_ROWNO) != 0;
		}

		/** @brief  */
		bool	hasRowEditing() const
		{
			return (mStyle&(ITableWindow::TES_ROWEDIT)) != 0;
		}

		/** @brief  */
		bool	hasRowEditingHelper() const
		{
			return (mStyle&(ITableWindow::TES_ROWEDIT)) != 0 && (mStyle&(ITableWindow::TES_ROWEDIT_HELPER)) != 0;
		}

		/** @brief  */
		bool	hasRowHeader() const
		{
			return this->hasRowNumber() || this->hasRowEditingHelper();
		}

		/** @brief  */
		void	clearRowButtonState()
		{
			if( !this->hasRowEditingHelper() )
				return;
			mTopIndex = -1;
			mDeletingIndex = -1;
			mHitAppend = false;
		}

		/** @brief  */
		int		getStyle() const
		{
			return mStyle;
		}

		/** @brief  */
		void	deleteSelectedRows()
		{
			//we don't know pos is valid after deleting,
			//or all selected flag would be cleared after deleting item
			//so cache the indices first and perform delete
			TempVector<int> list;
			list.reserve( (size_t)this->GetSelectedCount() );
			POSITION pos = this->GetFirstSelectedItemPosition();
			while(pos != NULL)
			{
				int index = this->GetNextSelectedItem(pos);
				list.push_back(index);
			}

			if(list.size() > 0)
				this->endEditing(true);

			while(!list.empty())
			{
				int index = list.back();
				for(size_t i = 1; i < list.size(); ++i)
				{
					if( list[i] > index )
						--list[i];
				}
				this->deleteRow( (index_t)index);
				list.pop_back();
			}
		}

		/** @brief  */
		void	deleteCurrentRow()
		{
			assert(mSelRow != INVALID_INDEX);
			this->endEditing(true);
			this->deleteRow(mSelRow);
		}

		/** @brief  */
		void	insertRow(index_t index)
		{
			assert(index != INVALID_INDEX);
			
			index_t editableIndex = INVALID_INDEX;
			TempVector<Variant> cols( mColumnDesc.size() );
			for(size_t j = 0; j < cols.size(); ++j)
			{
				if( editableIndex == INVALID_INDEX && (mColumnDesc[j]->getAccess()&CAF_WRITE) )
					editableIndex = j;
				cols[j] = mColumnDesc[j]->getBinaryValue();
			}
			if( cols.size() > 0 )
			{
				this->endEditing(true);
				this->addRow(index, &cols[0], cols.size() );
				this->selectCell(index, editableIndex == INVALID_INDEX ? 0 : editableIndex, true, SM_SINGLE);
			}
		}


	protected:
		/** @brief  */
		const HCONFIG&	getHintType(EConfigUIHint hint) const; 

		typedef Vector<HCONFIG>	ColumnList;
		typedef struct SRowItem : public Vector<Variant>, public Allocatable	{}RowItem;
		typedef Vector<RowItem*>	TableContent;

		class RedrawHelper : public NonCopyable, public NonAssignable
		{
		public:
			TableView& TV_THIS;
			RedrawHelper(TableView* _this) :TV_THIS(*_this)	{ TV_THIS.EnableRedraw(FALSE);}
			~RedrawHelper()	{TV_THIS.EnableRedraw(TRUE);}
		};

		ColumnList		mColumnDesc;
		TableContent	mContent;
		ITableWindow::IListener*	mListener;
		TableDefController*			mController;
		const EKeyCode*	mKeyMap;

		static IMenu*	msRowEditMenu;	//shared among all views

		index_t			mSelRow;
		index_t			mSelCol;
		index_t			mRangeStart;
		uint32			mKeyModifier;
		int				mTopIndex;
		int				mStyle;
		int				mRowHeaderWidth;
		int				mReDrawEnableCount;
		int				mDeletingIndex;

		ConfigControl*	mConfigControls[CUIH_COUNT];
		bool			mEditing;
		bool			mHitAppend;

		static HCONFIG	msHintTypes[CUIH_COUNT];
	};//class TableView

}//namespace Blade


#endif // __Blade_TableView_h__