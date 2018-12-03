/********************************************************************
	created:	2015/08/09
	filename: 	TableView.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <interface/IPlatformManager.h>
#include <ConfigTypes.h>
#include <ui/IconManager.h>
#include <ui/Menu.h>
#include "TableView.h"
#include "TableCommand.h"

namespace Blade
{
	IMPLEMENT_DYNAMIC(TableView, CUIListCtrl)
	BEGIN_MESSAGE_MAP(TableView, CUIListCtrl)
		ON_NOTIFY_REFLECT(LVN_ITEMCHANGING, &TableView::OnNMDisableDefaultSelectState)
		ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &TableView::OnItemChanged)
		ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &TableView::OnNMCustomDraw)
		ON_NOTIFY_REFLECT(LVN_ENDSCROLL, &TableView::OnNMEndScroll)
		ON_WM_CREATE()
		ON_WM_PAINT()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_RBUTTONDOWN()
		ON_WM_RBUTTONDBLCLK()
		ON_WM_MOUSEMOVE()
		ON_WM_KEYDOWN()
		ON_WM_KEYUP()
		ON_WM_GETDLGCODE()
		ON_WM_NCCALCSIZE()
		ON_WM_NCPAINT()
		ON_WM_NCHITTEST()
		ON_WM_NCLBUTTONDOWN()
		ON_WM_NCLBUTTONUP()
		ON_WM_NCLBUTTONDBLCLK()
		ON_WM_NCRBUTTONUP()
	END_MESSAGE_MAP()

	static const int ROW_HEADER_WIDTH = 50;
	IMenu*	TableView::msRowEditMenu = NULL;
	HCONFIG	TableView::msHintTypes[CUIH_COUNT];	//dummy config for each hint type

	//////////////////////////////////////////////////////////////////////////
	TableView::TableView()
	{
		mListener = NULL;
		mController = NULL;
		mKeyMap = NULL;

		if( msRowEditMenu == NULL )
		{
			msRowEditMenu = IMenuManager::getSingleton().createExtraMenu(BTString("TableView::RowContextMenu"));
			msRowEditMenu->addItem(BXLang("Delete Selected Row"), TableViewRangeDeleteCmd::getSingletonPtr());
			msRowEditMenu->addItem(BXLang("Delete Current Row"), TableViewDeleteCmd::getSingletonPtr());
			msRowEditMenu->addItem(BXLang("Insert Row After"), TableViewInsertCmd::getSingletonPtr());
		}

		mSelRow = mSelCol = mRangeStart = INVALID_INDEX;
		mKeyModifier = 0;
		mTopIndex = -1;
		mStyle = 0;
		mRowHeaderWidth = ROW_HEADER_WIDTH;
		mReDrawEnableCount = 1;
		mDeletingIndex = -1;

		for(int i = CUIH_START; i < CUIH_COUNT; ++i)
			mConfigControls[i] = NULL;
		
		mEditing = false;
		mHitAppend = false;
		
		//using a back door.. this is OK because 
		//Win32 key map can only be accessed by Win32 module
		IPlugin* plugin = IPlatformManager::getSingleton().getPlugin(BTString("BladeDevicePlugin"));
		if( plugin != NULL )
		{
			typedef const EKeyCode* (*GET_KEYMAP)(void);
			GET_KEYMAP func = (GET_KEYMAP)plugin->getAddOnObject(BTString("GET_KEYMAP"));
			mKeyMap = func();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	TableView::~TableView()
	{
		this->clearRows();
		this->clearColumns();

		for(int i = CUIH_START; i < CUIH_COUNT; ++i)
			BLADE_DELETE mConfigControls[i];
	}

	/************************************************************************/
	/* MFC Related                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnLButtonDown(UINT nFlags, CPoint point)
	{
		__super::OnLButtonDown(nFlags, point);
		if( mController != NULL )
		{
			this->ClientToScreen(&point);
			this->GetParent()->ScreenToClient(&point);
			mController->onTableMouseButtonDown(MSB_LEFT, POINT2I(point.x, point.y));
		}
		this->clearRowButtonState();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		__super::OnLButtonDblClk(nFlags, point);
		if( mController != NULL )
		{
			this->ClientToScreen(&point);
			this->GetParent()->ScreenToClient(&point);
			mController->onTableMouseButtonDBClick(MSB_LEFT, POINT2I(point.x, point.y));
		}
		this->clearRowButtonState();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnRButtonDown(UINT nFlags, CPoint point)
	{
		__super::OnLButtonDown(nFlags, point);
		if( mController != NULL )
		{
			this->ClientToScreen(&point);
			this->GetParent()->ScreenToClient(&point);
			if (!mController->onTableMouseButtonDown(MSB_RIGHT, POINT2I(point.x, point.y)) && this->hasRowEditing() && (mStyle&ITableWindow::TES_CONTEXT_MENU))
			{
				//note: popup to parent (CUIFrameWnd) which can handle menu command
				MenuManager::getSingleton().popupMenu(msRowEditMenu, point.x, point.y, this->GetParent());
			}
		}
		this->clearRowButtonState();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnRButtonDblClk(UINT nFlags, CPoint point)
	{
		__super::OnLButtonDblClk(nFlags, point);
		if( mController != NULL )
		{
			this->ClientToScreen(&point);
			this->GetParent()->ScreenToClient(&point);
			mController->onTableMouseButtonDBClick(MSB_RIGHT, POINT2I(point.x, point.y));
		}
		this->clearRowButtonState();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnMouseMove(UINT nFlags, CPoint point)
	{
		__super::OnMouseMove(nFlags, point);
		this->clearRowButtonState();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
	{
		//__super::OnKeyDown(nChar, nRepCnt, nFlags); //disable CListCtrl default behavior

		//record own mask in case global keyboard is not available (when used as a plugin for 3rd party app, i.e. max exporter)
		if( nChar == VK_SHIFT || nChar == VK_LSHIFT || nChar == VK_RSHIFT )
			mKeyModifier |= KM_SHIFT;
		else if( nChar == VK_CONTROL || nChar == VK_LCONTROL || nChar == VK_RCONTROL )
			mKeyModifier |= KM_CTRL;

		assert(mKeyMap != NULL);

		if( mKeyMap != NULL )
		{
			uint32 globalMask = IUIService::getSingleton().getGlobalKeyboard()->getKeyModifier();
			mController->onTableKeyDown(mKeyMap[nChar], mKeyModifier|globalMask);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnKeyUp(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
	{
		//__super::OnKeyUp(nChar, nRepCnt, nFlags);	//disable CListCtrl default behavior

		//record own mask in case global keyboard is not available (when used as a plugin for 3rd party app, i.e. max exporter)
		if( nChar == VK_SHIFT || nChar == VK_LSHIFT || nChar == VK_RSHIFT )
			mKeyModifier &= ~KM_SHIFT;
		else if( nChar == VK_CONTROL || nChar == VK_LCONTROL || nChar == VK_RCONTROL )
			mKeyModifier &= ~KM_CTRL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNMDisableDefaultSelectState(NMHDR* /*pNMHDR*/, LRESULT *pResult)
	{
		//disable select state, we will do our own high light in OnNMCustomDraw
		*pResult = TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnItemChanged(NMHDR *pNMHDR, LRESULT * /*pResult*/)
	{
		LPNMLISTVIEW nmLv = (LPNMLISTVIEW)pNMHDR;
		//refresh row header
		if( (nmLv->uChanged&LVIF_STATE) )
			this->updateRowHeader();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
	{
		NMLVCUSTOMDRAW* cd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
		*pResult = CDRF_DODEFAULT;

		//note: LVS_EX_BORDERSELECT is only for icon view so we don't use it 

		index_t row = (index_t)cd->nmcd.dwItemSpec;
		index_t col = (index_t)cd->iSubItem;

		if(row == mSelRow && col == mSelCol)
		{
			cd->clrFace = THEMECOLOR(TC_HILIGHTBK);
			cd->clrTextBk = THEMECOLOR(TC_HILIGHTBK);
		}
		else
		{
			cd->clrFace = THEMECOLOR(TC_CTLBK);
			cd->clrTextBk = THEMECOLOR(TC_CTLBK);
		}
		cd->clrText = THEMECOLOR(TC_TEXT);

		switch( cd->nmcd.dwDrawStage )
		{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
			*pResult = CDRF_DODEFAULT;
			break;
		default:
			break;
		}//switch
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNMEndScroll(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
	{
		this->updateRowHeader();
	}

	//////////////////////////////////////////////////////////////////////////
	int		TableView::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int ret = __super::OnCreate(lpCreateStruct);
		this->EnableOwnerDraw(FALSE);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnPaint()
	{
		//reposition active/selected control
		if( mSelRow < this->getRowCount() && mSelCol < this->getColumnCount() )
			this->RepositionControl();
		else
		{
			for(int i = CUIH_START+1; i < CUIH_COUNT; ++i)
				mConfigControls[i]->GetWindow()->ShowWindow(SW_HIDE);
		}
		this->Default();
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
	{
		__super::OnNcCalcSize(bCalcValidRects, lpncsp);
		if( this->hasRowHeader() )
		{
			CRect rc(&lpncsp->rgrc[0]);

			int RowHeight, ColWidth;
			this->GetItemSpacing(TRUE, &ColWidth, &RowHeight);
			//square for add/delete button (insertion is not frequently used, should be in context menu)
			mRowHeaderWidth = this->hasRowEditingHelper() ? RowHeight + (RowHeight+1)/2 : 0;

			if( this->hasRowNumber() )
			{
				CWindowDC dc(this);
				CFont* font = dc.SelectObject( this->GetFont() );
				CRect textRect;
				//TString rowNO = TStringHelper::fromInt(this->GetItemCount()+1);
				TString rowNO = BTString("9999");
				dc.DrawText( rowNO.c_str(), (int)rowNO.size(), &textRect, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_CALCRECT);
				mRowHeaderWidth += textRect.Width() + 3;
				dc.SelectObject(font);
			}
			rc.DeflateRect(mRowHeaderWidth, 0, 0, 0);
			lpncsp->rgrc[0] = rc;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNcPaint()
	{
		__super::OnNcPaint();
		mTopIndex = -1;
		mDeletingIndex = -1;
		mHitAppend = false;
		this->updateRowHeader();
	}

	//////////////////////////////////////////////////////////////////////////
	LRESULT	TableView::OnNcHitTest(CPoint point)
	{
		if( !this->hasRowHeader() )
			return __super::OnNcHitTest(point);

		CRect rect;
		this->GetWindowRect(&rect);
		rect.right = rect.left + mRowHeaderWidth;
		if(!rect.PtInRect(point))
			return __super::OnNcHitTest(point);

		if(this->hasRowEditingHelper() )
			this->updateRowHeader(&point);
		return HTCAPTION;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNcLButtonDown(UINT /*nHitTest*/, CPoint point)
	{
		if( !this->hasRowHeader() )
		{
			//note: using default because this function may be called by Other message handler (OnNcLButtonDBClk)
			Default();
			return;
		}

		CRect rect;
		this->GetWindowRect(&rect);
		rect.right = rect.left + mRowHeaderWidth;
		if(!rect.PtInRect(point))
		{
			//note: using default because this function may be called by Other message handler (OnNcLButtonDBClk)
			Default();
			return;
		}

		if(this->hasRowEditingHelper() )
			this->updateRowHeader(&point, DOWN);
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNcLButtonUp(UINT nHitTest, CPoint point)
	{
		__super::OnNcLButtonUp(nHitTest, point);
		if( !this->hasRowEditingHelper() )
			return;

		CRect rect;
		this->GetWindowRect(&rect);
		rect.right = rect.left + mRowHeaderWidth;
		if(!rect.PtInRect(point))
			return;

		bool addNewRow = mHitAppend; 
		bool deleteCurRow = mDeletingIndex != -1;

		this->updateRowHeader(&point, UP);

		if( mDeletingIndex != -1 || mHitAppend )
		{
			mDeletingIndex = -1;
			mHitAppend = false;
			return;
		}
		if(addNewRow || deleteCurRow && this->GetSelectedCount() > 1)
			return;

		uint32 globalMask = IUIService::getSingleton().getGlobalKeyboard()->getKeyModifier();
		bool bShift = ((mKeyModifier|globalMask)&KM_SHIFT) != 0;
		bool bCtrl = ((mKeyModifier|globalMask)&KM_CTRL) != 0;
		ESelectMethod sm = bShift ? SM_RANGE : (bCtrl ? SM_ADD : SM_SINGLE);

		int start = this->GetTopIndex();
		int count = min(start+this->GetCountPerPage()+1, this->GetItemCount());
		for(int i = start; i < count; ++i)
		{
			CRect rcItem;
			this->GetItemRect(i, &rcItem, LVIR_BOUNDS);
			this->ClientToScreen(&rcItem);
			rcItem.left = rect.left;
			rcItem.right = rect.right;
			if( rcItem.PtInRect(point) )
			{
				this->SetFocus();
				index_t col = mSelCol > this->getColumnCount() ? 0 : mSelCol;
				this->endEditing(true);
				mSelCol = INVALID_INDEX;
				this->selectCell((index_t)i, col, false, sm);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TableView::OnNcRButtonUp(UINT nHitTest, CPoint point)
	{
		__super::OnNcRButtonUp(nHitTest, point);
		if( !this->hasRowEditing() )
			return;

		CRect rect;
		this->GetWindowRect(&rect);
		rect.right = rect.left + mRowHeaderWidth;
		if(!rect.PtInRect(point))
			return;

		if( mDeletingIndex != -1 || mHitAppend )
		{
			mDeletingIndex = -1;
			mHitAppend = false;
			return;
		}

		int start = this->GetTopIndex();
		int count = min(start+this->GetCountPerPage()+1, this->GetItemCount());
		for(int i = start; i < count; ++i)
		{
			CRect rcItem;
			this->GetItemRect(i, &rcItem, LVIR_BOUNDS);
			this->ClientToScreen(&rcItem);
			rcItem.left = rect.left;
			rcItem.right = rect.right;
			if( rcItem.PtInRect(point) )
			{
				UINT state = this->GetItemState(i, LVIS_SELECTED);
				ESelectMethod sm = (!(state&LVIS_SELECTED)) ? SM_SINGLE : SM_ADD;	//if selected, use SM_ADD to set focus
				this->SetFocus();
				index_t col = mSelCol > this->getColumnCount() ? 0 : mSelCol;
				this->endEditing(true);
				mSelCol = INVALID_INDEX;
				this->selectCell((index_t)i, col, false, sm);
				//popup context menu
				//note: popup to parent (CUIFrameWnd) which can handle menu command
				if( (mStyle&(ITableWindow::TES_CONTEXT_MENU)) )
				{
					this->GetParent()->ScreenToClient(&point);
					MenuManager::getSingleton().popupMenu(msRowEditMenu, point.x, point.y, this->GetParent());
				}
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL	TableView::PreTranslateMessage(MSG* pMsg)
	{
		if( pMsg->message != WM_KEYDOWN )
			return __super::PreTranslateMessage(pMsg);

		index_t row = mSelRow;
		index_t col = mSelCol;
		if( row == INVALID_INDEX || col == INVALID_INDEX )
			return __super::PreTranslateMessage(pMsg);

		if( ((pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) && mEditing)
			|| pMsg->wParam == VK_TAB || pMsg->wParam == VK_NEXT || pMsg->wParam == VK_PRIOR )	//why we can't get VK_NEXT' VK_PRIOR messages?
		{
			this->SetFocus();
			this->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
		return __super::PreTranslateMessage(pMsg);
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL	TableView::Create(CWnd* pParent,const RECT& rect,UINT nID)
	{
		DWORD dwStyle = LVS_SHAREIMAGELISTS | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOLABELWRAP | LVS_ALIGNLEFT 
			| WS_CHILD | WS_VISIBLE;
		DWORD dwExLVStyle = LVS_EX_FLATSB | LVS_EX_LABELTIP;

		//this style cannot put into dialog. if table view has no focusing child, windows will enter a infinite loop
		//DWORD dwExWinStyle |= WS_EX_CONTROLPARENT;

		BOOL ret = this->CListCtrl::CreateEx(0, dwStyle, rect, pParent, nID);

		if(ret)
		{
			this->SetExtendedStyle(dwExLVStyle);
			this->SetOutlineColor(RGB(0xFF,0,0));

			mSelCol = mSelRow = mRangeStart = INVALID_INDEX;

			CRect ctrlRect(0, 0, 0, 0);
			for(int i = CUIH_START+1; i < CUIH_COUNT; ++i)
			{
				mConfigControls[i] = ConfigControlUtil::CreateConfigControl(this->getHintType((EConfigUIHint)i), this, ctrlRect, 0);
				mConfigControls[i]->GetWindow()->ShowWindow(SW_HIDE);
			}
		}
		
		assert(ret);
		return ret;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool		TableView::initialize(const HCONFIG* columnDesc, size_t columnCount, int style)
	{
		if( columnDesc == NULL || this->GetSafeHwnd() == NULL || mColumnDesc.size() != 0 || mContent.size() != 0 )
		{
			assert(false);
			return false;
		}
		mStyle = style;

		DWORD dwStyle = WS_HSCROLL | WS_VSCROLL | LVS_SHOWSELALWAYS;
		DWORD dwExLVStyle = LVS_EX_FLATSB;

		if( (style&ITableWindow::TES_FULL_ROW_SELECTION) || (style&ITableWindow::TES_MULTI_SELECTION)
			/*|| (style&ITableWindow::TES_SHOW_ROWNO) */)
			dwExLVStyle |= LVS_EX_FULLROWSELECT;

		if( !(style & ITableWindow::TES_MULTI_SELECTION) )
			dwStyle |= LVS_SINGLESEL;

		if( style & ITableWindow::TES_GRIDVIEW )
			dwExLVStyle |= LVS_EX_GRIDLINES;

		this->ModifyStyle(0, dwStyle);
		this->SetExtendedStyle(dwExLVStyle | this->GetExtendedStyle());

		mColumnDesc.reserve(columnCount);
		for(size_t i = 0; i < columnCount; ++i)
		{
			const HCONFIG& config = *(columnDesc + i);
			const TString& columnText = BTString2Lang(config->getName());
			this->InsertColumn((int)i, columnText.c_str(), LVCFMT_LEFT);
			mColumnDesc.push_back( config );
		}

		for(size_t i = 0; i < columnCount; ++i)
		{
			this->SetColumnWidth((int)i, LVSCW_AUTOSIZE);
			this->SetColumnWidth((int)i, LVSCW_AUTOSIZE_USEHEADER);
		}

		mSelRow = mSelCol = mRangeStart = INVALID_INDEX;
		mReDrawEnableCount = 1;
		this->SetWindowPos(NULL, 0, 0, 0, 0, SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
		//this->SetRedraw(TRUE);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		TableView::findColumn(const TString& header) const
	{
		FnHConfigFindByName finder(header);
		ColumnList::const_iterator i = std::find_if(mColumnDesc.begin(), mColumnDesc.end(), finder);
		if( i != mColumnDesc.end() )
			return index_t(i - mColumnDesc.begin());
		else
			return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::addColumn(index_t index, HCONFIG columnDesc)
	{
		if( columnDesc == NULL || this->GetSafeHwnd() == NULL || index > mColumnDesc.size() )
		{
			assert(false);
			return false;
		}

#if BLADE_DEBUG
		CHeaderCtrl* header = this->GetHeaderCtrl();
		int nCount = header->GetItemCount(); 
		assert( (int)mColumnDesc.size() == nCount );
		//for(size_t i = 0; i < mContent.size(); ++i)
		//	assert( mContent[i]->size() == mColumnDesc.size() );
		assert( mContent.size() == 0 || mContent[0]->size() == mColumnDesc.size() );
#endif
		RedrawHelper helper(this);

		const TString& columnText = columnDesc->getName();
		if( this->InsertColumn((int)index, columnText.c_str()) != -1 )
		{
			Variant v = columnDesc->getBinaryValue();
			v.fromTString( columnDesc->getDefaultValue() );
			for(size_t i = 0; i < mContent.size(); ++i)
			{
				RowItem* item = mContent[i];
				assert( item->size() == mColumnDesc.size() );
				item->insert(item->begin()+(indexdiff_t)index, v);
			}
			mColumnDesc.insert(mColumnDesc.begin() + (indexdiff_t)index, columnDesc);
			this->refreshColumn(index);
			return true;
		}
		else
		{
			assert(false && "insert column failed.");
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::refreshColumn(index_t index)
	{
		if( index >= this->getColumnCount() )
		{
			assert(false);
			return false;
		}

		RedrawHelper helper(this);

		size_t rowCount = this->getRowCount();
		for(size_t i = 0; i < rowCount; ++i)
			this->SetItemText((int)i, (int)index, this->getCell(i, index)->toTString().c_str() );
		this->AutoSizeColumn();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::deleteColumn(index_t index)
	{
		if( this->GetSafeHwnd() == NULL || index > mColumnDesc.size() )
		{
			assert(false);
			return false;
		}

#if BLADE_DEBUG
		CHeaderCtrl* header = this->GetHeaderCtrl();
		int nCount = header->GetItemCount(); 
		assert( (int)mColumnDesc.size() == nCount );
		//for(size_t i = 0; i < mContent.size(); ++i)
		//	assert( mContent[i]->size() == mColumnDesc.size() );
		assert( mContent.size() == 0 || mContent[0]->size() == mColumnDesc.size() );
#endif
		RedrawHelper helper(this);
		BOOL ret = this->DeleteColumn((int)index);
		if(ret)
		{
			for(size_t i = 0; i < mContent.size(); ++i)
			{
				RowItem* item = mContent[i];
				assert( item->size() == mColumnDesc.size() );
				item->erase(item->begin()+(indexdiff_t)index);
			}

			mColumnDesc.erase(mColumnDesc.begin() + (indexdiff_t)index);
			this->AutoSizeColumn();
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::clearColumns()
	{
		if( this->GetSafeHwnd() == NULL )
		{
			mColumnDesc.clear();
			return true;
		}

#if BLADE_DEBUG
		CHeaderCtrl* header = this->GetHeaderCtrl();
		int nCount = header->GetItemCount(); 
		assert( (int)mColumnDesc.size() == nCount );
#endif

		while( this->DeleteColumn(0) == TRUE );
		mColumnDesc.clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant*	TableView::getRow(index_t index) const
	{
		if( this->GetSafeHwnd() == NULL )
		{
			assert(false);
			return NULL;
		}

		if( mColumnDesc.size() == 0 )
		{
			assert(false && "no columns defined, all item are empty.");
			return NULL;
		}

		if( index >= mContent.size() )
		{
			assert(false && "out of range");
			return NULL;
		}

		RowItem& item = *(mContent[index]);
		assert( item.size() == mColumnDesc.size() );
		return &item[0];
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::addRow(size_t index, const Variant* cols, size_t colCount, IconIndex icon/* = INVALID_ICON_INDEX*/)
	{
		if( this->GetSafeHwnd() == NULL || index > this->getRowCount() )
		{
			assert(false);
			return false;
		}

		if( colCount != mColumnDesc.size() || mColumnDesc.size() == 0 || cols == NULL)
		{
			assert(false);
			return false;
		}

		//verify types
		for(size_t i = 0; i < colCount; ++i)
		{
			IConfig* column = mColumnDesc[i];
			const Variant& typeCheck = column->getBinaryValue();
			if( cols[i].getType() != typeCheck.getType() )
			{
				assert(false && "column type mismatch.");
				return false;
			}
		}

		if( icon != INVALID_ICON_INDEX && this->GetImageList(LVSIL_SMALL) == NULL )
		{
			this->SetImageList( IconManager::getSingleton().getIconList(IS_16), LVSIL_SMALL);
			this->SetImageList( IconManager::getSingleton().getIconList(IS_24), LVSIL_NORMAL);
		}

		//RedrawHelper helper(this);
		int itemIndex = this->InsertItem((int)index, cols[0].toTString().c_str(), (icon==INVALID_ICON_INDEX?I_IMAGENONE:icon));
		if( itemIndex == (int)index )
		{
			//add subitem (columns)
			for(size_t i = 1; i < colCount; ++i)
				this->SetItemText(itemIndex, (int)i, cols[i].toTString().c_str());

			RowItem* item = BLADE_NEW RowItem();
			item->resize(colCount);
			for(size_t i = 0; i < colCount; ++i )
				(*item)[i].reset(cols[i]);

			mContent.insert( mContent.begin() + (indexdiff_t)index, item);
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::deleteRow(index_t index)
	{
		if( this->GetSafeHwnd() == NULL || index >= mContent.size() )
		{
			assert(false);
			return false;
		}

		//RedrawHelper helper(this);
		if( this->DeleteItem((int)index) )
		{
			RowItem* item = mContent[index];
			BLADE_DELETE item;
			mContent.erase( mContent.begin() + (indexdiff_t)index);
			//this->AutoSizeColumn();
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::clearRows()
	{
		for(size_t i = 0; i < mContent.size(); ++i)
			BLADE_DELETE mContent[i];
		mContent.clear();

		if( this->GetSafeHwnd() == NULL )
			return true;

		BOOL ret = this->DeleteAllItems();
		assert(ret);
		return ret != FALSE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::updateRowHeader(POINT* pt/* = NULL*/, EMouseState mouseState/* = 0*/)
	{
		if( !this->hasRowHeader() )
			return false;

		CWindowDC dc(this);
		int start = this->GetTopIndex();

		CRect rcWindow;
		this->GetWindowRect(&rcWindow);

		int RowButtonWidth = 0;
		if( this->hasRowEditingHelper() )
		{
			int RowHeight = 0, ColWidth = 0;
			this->GetItemSpacing(TRUE, &ColWidth, &RowHeight);
			RowButtonWidth = RowHeight + (RowHeight+1)/2;	//make it square
		}
		CRect rcRowHeader = rcWindow;
		rcRowHeader.OffsetRect(-rcRowHeader.TopLeft());
		rcRowHeader.right = (mTopIndex != start) ? mRowHeaderWidth : RowButtonWidth;
		//draw row header
		dc.FillSolidRect(&rcRowHeader, THEMECOLOR(TC_BTN));
		CBrush::DeleteTempMap();

		CPoint point;
		if(pt != NULL)
		{
			point = *pt;
			point -= rcWindow.TopLeft();
		}

		int count = min(start+this->GetCountPerPage()+1, this->GetItemCount());
		int mode = dc.GetBkMode();
		dc.SetBkMode(TRANSPARENT);
		CFont* font = dc.SelectObject( this->GetFont() );

		for(int i = start; i < count; ++i)
		{
			CRect rcItem;
			this->GetItemRect(i, &rcItem, LVIR_BOUNDS);
			this->ClientToScreen(&rcItem);
			rcItem.OffsetRect(-rcWindow.TopLeft());

			//draw row/line number
			if( this->hasRowNumber() && mTopIndex != start )
			{
				rcItem.left = this->hasRowEditingHelper() ? RowButtonWidth : 0;
				rcItem.right = mRowHeaderWidth-1;
				TString rowNO =  TStringHelper::fromInt(i+1);
				COLORREF oc = dc.SetTextColor(THEMECOLOR(TC_TEXT));
				dc.DrawText(rowNO.c_str(), (int)rowNO.size(), &rcItem, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
				dc.SetTextColor(oc);
			}

			if( !this->hasRowEditingHelper() )
				continue;
			UINT state = this->GetItemState(i, LVIS_SELECTED);
			if( !(state&LVIS_SELECTED) )
				continue;

			//draw delete button
			rcItem.left = 0;
			rcItem.right = RowButtonWidth;
			rcItem.top += 1;
			rcItem.bottom -= 1;
			rcItem.DeflateRect(2,2,RowButtonWidth/2,2);
			bool hit = (pt != NULL && rcItem.PtInRect(point));
			int hover = (hit && mouseState != DOWN && mDeletingIndex == -1) ? 0 : DFCS_FLAT;

			{
				CRect rcMask(0, 0, rcItem.Width(), rcItem.Height());
				CDC maskMemDC;
				maskMemDC.CreateCompatibleDC(&dc);
				CBitmap maskBitmap;
				maskBitmap.CreateCompatibleBitmap(&dc, rcMask.Width(), rcMask.Height());
				maskMemDC.SelectObject(&maskBitmap);
				maskMemDC.DrawFrameControl(&rcMask, DFC_CAPTION, UINT(DFCS_CAPTIONCLOSE | hover));

				dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &maskMemDC, 0, 0, SRCINVERT);
				dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &maskMemDC, 0, 0, SRCAND);
				dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &maskMemDC, 0, 0, SRCINVERT);
			}
			

			if( hit )
			{
				if( mouseState == DOWN )
					mDeletingIndex = i;
				else if( mouseState == UP )
				{
					if( mDeletingIndex == i )
					{
						this->endEditing(true);
						this->deleteRow((index_t)i);
					}
					mDeletingIndex = -1;
				}
			}

			//draw insert(after) button
			rcItem.OffsetRect(RowButtonWidth/2-1, 0);
			hit = (pt != NULL && rcItem.PtInRect(point));
			hover = (hit && mouseState != DOWN && !mHitAppend) ? 0 : DFCS_FLAT;
			//dc.DrawFrameControl(&rcItem, DFC_SCROLL, UINT(DFCS_SCROLLDOWN|hover));

			{
				CRect rcMask(0, 0, rcItem.Width(), rcItem.Height());
				CDC maskMemDC;
				maskMemDC.CreateCompatibleDC(&dc);
				CBitmap maskBitmap;
				maskBitmap.CreateCompatibleBitmap(&dc, rcMask.Width(), rcMask.Height());
				maskMemDC.SelectObject(&maskBitmap);
				maskMemDC.DrawFrameControl(&rcItem, DFC_SCROLL, UINT(DFCS_SCROLLDOWN | hover));

				dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &maskMemDC, 0, 0, SRCINVERT);
				dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &maskMemDC, 0, 0, SRCAND);
				dc.BitBlt(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &maskMemDC, 0, 0, SRCINVERT);
			}

			if( hit )
			{
				if( mouseState == DOWN )
					mHitAppend = true;
				else if( mouseState == UP && mHitAppend)
				{
					mHitAppend = false;
					this->insertRow((index_t)i + 1);
				}
			}
		}

		dc.SetBkMode(mode);
		dc.SelectObject(font);
		mTopIndex = start;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		TableView::selectCell(index_t row, index_t col, bool edit, ESelectMethod method)
	{
		if( row >= this->getRowCount() || col >= this->getColumnCount() )
		{
			assert(false);
			return false;
		}
		this->EnsureVisible((int)row, FALSE);
		//note: ensure visible may scroll the view, so we need update the row header
		this->updateRowHeader();

		if( mSelRow == row && mSelCol == col && mEditing == edit)
			return true;

		CRect rect;
		this->GetSubItemRect((int)mSelRow, (int)mSelCol, LVIR_BOUNDS, rect);
		rect.InflateRect(1,1,1,1);
		this->InvalidateRect(rect);

		if( (this->GetStyle()&LVS_SINGLESEL) || method == SM_SINGLE || method == SM_RANGE )
		{
			//clear all
			POSITION pos = this->GetFirstSelectedItemPosition();
			while(pos != NULL)
			{
				int index = this->GetNextSelectedItem(pos);
				this->SetItemState(index, 0, LVIS_SELECTED|LVIS_FOCUSED);
			}
		}

		if( mRangeStart != INVALID_INDEX && method == SM_RANGE && !(this->GetStyle()&LVS_SINGLESEL) )
		{
			int start = std::min<int>((int)mRangeStart, (int)row);
			int end = std::max<int>((int)mRangeStart, (int)row);
			for(int i = start; i <= end; ++i)
				this->SetItemState(i, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		}
		else
			mRangeStart = row;

		mSelRow = row;
		mSelCol = col;

		this->SetItemState((int)mSelRow, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		this->GetSubItemRect((int)row, (int)col, LVIR_BOUNDS, rect);
		rect.InflateRect(1,1,1,1);
		this->InvalidateRect(rect);

		if( edit )
			this->startEditing();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Variant*	TableView::getCell(index_t row, index_t col) const
	{
		if( row >= mContent.size() || col >= mColumnDesc.size() )
		{
			assert(false && "out of range" );
			return NULL;
		}

		RowItem* item = mContent[row];
		assert( col < item->size() );
		return &(*item)[col];
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&	TableView::getHintType(EConfigUIHint hint) const
	{
		if(hint < CUIH_START || hint >= CUIH_COUNT )
			return HCONFIG::EMPTY;

		if( msHintTypes[hint] == NULL )
		{
			ConfigAtom* cfg = BLADE_NEW ConfigAtom(TString::EMPTY);

			Variant var = TString::EMPTY;
			if(hint == CUIH_INPUT_RANGE )
				var.reset(int(0));
			else if( hint == CUIH_CHECK )
				var.reset(false);
			else if( hint == CUIH_COLOR )
				var.reset(Color::WHITE);

			cfg->initBinaryValue(var, hint, CAF_READWRITE);

			switch(hint)
			{
			case CUIH_INPUT_RANGE:
				{
					TStringParam list;
					list.push_back(BTString("0"));
					list.push_back(BTString("1"));
					list.push_back(BTString("1"));
					cfg->setOptions(list,0);
				}
				break;
			case CUIH_RANGE:
				{
					TStringParam list;
					list.push_back(BTString(""));
					list.push_back(BTString(" "));
					cfg->setOptions(list,0);
				}
				break;
			default:
				break;
			}

			msHintTypes[hint].bind( cfg );
		}
		return msHintTypes[hint];
	}
	
}//namespace Blade