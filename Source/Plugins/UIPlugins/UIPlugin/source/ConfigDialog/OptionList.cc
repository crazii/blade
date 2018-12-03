// OptionList.cc : implementation file
//

#include "stdafx.h"
#include <ConfigDialog/OptionList.h>
#include <interface/public/ui/UIEvents.h>
#include <interface/IEventManager.h>
#include <interface/IEnvironmentManager.h>
#include <ConstDef.h>

#define DEFAULT_ROW_HEIGHT	(20)
#define MIN_COLUM_WIDTH		(50)
#define EXP_BUTTON_ID_BASE	(0xD000)	//Command ID , MFC need it ranges 0x8000-0xDFFFF
#define CONTROL_ID_BASE		(0x8000)	//control ID, MFC need it ranges 8-0xDFFF
static const int INDENT_SIZE = EXP_BUTTON_SIZE+3;

using namespace Blade;
// COptionList
BEGIN_MESSAGE_MAP(COptionList, CUIListCtrl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_NCDESTROY()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &COptionList::OnNMClick)
END_MESSAGE_MAP()

class UIExpandData : public IConfigUIData, public Blade::Allocatable
{
};


COptionList::COptionList()
:mForceShowAll(false)
,mRefreshExpandIndex(INVALID_INDEX)
,mTimerID( UINT_PTR(-1) )
{
	mPaintPhase = 0;

	mColumnRate = 0.5f;
	mDraging = false;
	mExpandAll = false;

	mUIExpanded.bind( BLADE_NEW UIExpandData() );
	mUINotExpanded.bind( BLADE_NEW UIExpandData() );
}

COptionList::~COptionList()
{
	this->clear();
	this->DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////
BOOL	COptionList::Create(CWnd* pParent,const RECT& rect,UINT nID,DWORD dwExWinStyle/* = 0*/,DWORD dwExLVStyle/* = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT*/)
{
	//
	DWORD dwStyle = LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOLABELWRAP | LVS_OWNERDRAWFIXED | LVS_ALIGNLEFT | 
		/*WS_EX_CONTROLPARENT |*/ WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER;
	dwStyle |= LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS;

	dwExLVStyle |= LVS_EX_FLATSB | LVS_EX_FULLROWSELECT/* | LVS_EX_GRIDLINES*/;

	BOOL ret = this->CListCtrl::CreateEx(dwExWinStyle,dwStyle,rect,pParent,nID);
	mColumnRate = 0.5;

	if( ret )
	{
		this->insertColumn(0, TEXT(""), mColumnRate);
		this->insertColumn(1, TEXT(""), 1-mColumnRate);
		this->SetExtendedStyle(dwExLVStyle);
#if ENABLE_THEME
		this->SetBkColor(  THEMECOLOR(TC_CTLBK) );
#endif
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void		COptionList::insertColumn(index_t nCol, LPCTSTR lpszColumnHeading, float width, int nSubItem/* = -1*/, int nFormat/* = LVCFMT_LEFT*/)
{
	CRect rect;
	this->GetClientRect(&rect);

	ASSERT( width > 0 );
	float fColWidth = (float)rect.Width()*width;

	__super::InsertColumn((int)nCol,lpszColumnHeading, nFormat,(int)( fColWidth ) - 1,nSubItem);
}

//////////////////////////////////////////////////////////////////////////
size_t		COptionList::getIdealWidth()
{
	int count = this->GetHeaderCtrl()->GetItemCount();
	assert(count == 2);
	for(int i = 0; i < count; ++i)
		this->SetColumnWidth(i, LVSCW_AUTOSIZE);

	size_t indent = 1;
	for(size_t i = 0; i < mListTypeInfo.size(); ++i)
		indent = max(indent, (size_t)mListTypeInfo[i].depth*(size_t)INDENT_SIZE);

	size_t border = (size_t)::GetSystemMetrics(SM_CYBORDER)*2;
	size_t middleLine = border;

	int width = 0;
	float columnRate[] = {mColumnRate, 1-mColumnRate, 0};
	for(int i = 0; i < count; ++i)
		width = max(width, int((float)this->GetColumnWidth(i)/columnRate[i]));
	return (size_t)width + border + middleLine + indent;
}

//////////////////////////////////////////////////////////////////////////
void		COptionList::refresh()
{
	this->SetRedraw(FALSE);
	for(size_t i = 0; i < mListControls.size(); ++i)
	{
		ConfigControl* control = static_cast<ConfigControl*>( mListControls[i] );
		IConfig* config = mConfigList[i];

		size_t subCount = config->getSubConfigCount();
		bool updated;
		if( control != NULL)
			updated = control->updateData();
		else
			updated = config->updateData(false);

		//collapse data & expand again to refresh
		IConfig::IDataHandler* handler = config->getDataHandler();
		bool needExpand = subCount > 0 && mListExpandButton[i] != NULL && mListExpandButton[i]->GetExpanded()
			&& handler != NULL && updated;

		if( needExpand )
			this->expandMultiRows(i, FALSE);

		if( !updated )
			continue;

		//handle special cases first

		//config added new sub config, but previously has none
		if( config->getSubConfigCount() > 0 && mListExpandButton[i] == NULL )
		{
			CExpButton* button = BLADE_NEW CExpButton();
			mListExpandButton[i] = button;
			CRect rect;
			rect.left = rect.top = 0;
			rect.right = rect.bottom = EXP_BUTTON_SIZE;
			button->Create(rect, this, EXP_BUTTON_ID_BASE + (UINT)i);
			button->EnableWindow(TRUE);
			button->SetExpanded(TRUE);
		}
		//or previous has some sub config, but now has none.
		else if ( config->getSubConfigCount() == 0 && mListExpandButton[i] != NULL )
		{
			BLADE_DELETE mListExpandButton[i];
			mListExpandButton[i] = NULL;
		}

		if( needExpand || config->getSubConfigCount() > 0 )
		{
			if( config->getSubConfigCount() > 0 )
				this->expandMultiRows(i, TRUE);
			else
				mListExpandButton[i]->SetExpanded(FALSE);
		}
		
		//refresh text
		{
			CString text;
			if( mConfigList[i]->getValue() != NULL )
				text = BTString2Lang( config->getValue() ).c_str();
			CString oldText = this->GetItemText((int)i,1);

			if( text != oldText  )
			{
				//this->SetItemText(i,1,text);
				LVITEM item;
				item.iItem = (int)i;
				item.iSubItem = 1;
				item.mask = LVIF_PARAM | LVIF_TEXT;
				item.lParam = NULL;
				item.pszText = text.GetBuffer();
				this->SendMessage(LVM_SETITEMTEXT,i,(LPARAM)&item);
			}
		}
	}
	this->SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::clear()
{
	//clear UIHint 
	//because hint object is created by UIPlugin.
	//UIHint will be invalid after DLL unloaded.
	TempVector<IConfig*> configs(mConfigList.begin(), mConfigList.end());
	while(!configs.empty())
	{
		IConfig* cfg = configs.back();
		configs.pop_back();

		ConfigUIHint hint = cfg->getUIHint();
		hint.mUIData = HCONFIGUIDATA::EMPTY;
		cfg->setUIHint(hint);

		for(size_t i = 0; i < cfg->getSubConfigCount(); ++i)
			configs.push_back(cfg->getSubConfig(i));
	}

	if( mTimerID != UINT_PTR(-1) )
	{
		this->KillTimer(mTimerID);
		mTimerID = UINT_PTR(-1);
	}

	//hide all controls in case it post any message on destroy
	for(size_t i = 0; i < mListControls.size(); ++i)
	{
		if(mListControls[i] != NULL && mListControls[i]->GetWindow() != NULL )
			mListControls[i]->GetWindow()->ShowWindow(SW_HIDE);
	}

	for(size_t i = 0; i < mListControls.size(); ++i)
		BLADE_DELETE mListControls[i];

	for( size_t i = 0; i < mListExpandButton.size(); ++i )
		BLADE_DELETE mListExpandButton[i];

	mListControls.clear();
	mListExpandButton.clear();
	mListTypeInfo.clear();
	mConfigList.clear();
	mListAccess.clear();
	mColumnRate = 0.5f;
	mDraging = false;
}

//////////////////////////////////////////////////////////////////////////
static size_t countTotal(IConfig* root)
{
	size_t count = root->getSubConfigCount();
	size_t total = count;
	for(size_t i = 0; i < count; ++i)
	{
		IConfig* subConfig = root->getSubConfig(i);
		total += countTotal(subConfig);
	}
	return total;
}
void	COptionList::setSubConfigs(IConfig* root)
{
	//prepare arrays size
	{
		size_t total = ::countTotal(root);
		mListTypeInfo.reserve(total);
		mListAccess.reserve(total);
		mListControls.reserve(total);
		mListExpandButton.reserve(total);
		mConfigList.reserve(total);
	}

	assert(!root->getBinaryValue().isCollection());

	//do add
	CONFIG_ACCESS access = root->getAccess();
	size_t count = root->getSubConfigCount();

	bool devMode = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvBool::DEVELOPER_MODE);

	this->SetRedraw(FALSE);
	
	for( size_t i = 0; i < count; ++i)
	{
		IConfig* subConfig = root->getSubConfig(i);
		this->addConfig(subConfig, access, devMode);
	}
	assert( mListExpandButton.size() == mConfigList.size() );
	assert( this->GetItemCount() == (int)mConfigList.size() );

	if( mExpandAll )
	{
		for(size_t i = 0; i < mConfigList.size(); ++i )
		{
			IConfig* config = mConfigList[i];
			if( config->getSubConfigCount() > 0 )
			{
				CExpButton* button = mListExpandButton[i];
				if( !button->GetExpanded() ) //button already expanded through cascade
				{
					button->SetExpanded(true);
					this->expandMultiRows(i, true);
				}
			}
		}
	}

	this->SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
bool COptionList::addConfig(IConfig* config, CONFIG_ACCESS parentAccess, bool devMode)
{
	if( config == NULL )
		return false;

	CONFIG_ACCESS access = config->getAccess();

	if( !(parentAccess&CAF_WRITE) && !(access&CAF_NOCASCADE) )
		access = CONFIG_ACCESS(access&(~CAF_WRITE));

	if( (access&CAF_READWRITE) == 0 || (!devMode && (access&CAF_DEVMODE)) )
		return true;

	CONFIG_UIHINT hint = config->getUIHint();
	hint.mUIData = mUINotExpanded;
	config->setUIHint(hint);

	mConfigList.push_back(config);
	mListAccess.push_back(access);

	size_t subCount = config->getSubConfigCount();
	if( this->addListItem(config) == -1 && subCount == 0 )
	{
		assert(false);
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
uint	COptionList::addListItem(IConfig* config)
{
	int row = this->GetItemCount();
	return this->insertListItem((index_t)row, config);
}


//////////////////////////////////////////////////////////////////////////
uint		COptionList::insertListItem(index_t row,IConfig* config, bool bExpandSub/* = false*/)
{
	ASSERT(config != NULL );
	ASSERT( row < mConfigList.size() && mConfigList[row] != NULL );
	const TString& name = BTString2Lang( config->getName() );

	//add subitem
	LVITEM item;
	item.iItem = (int)row;
	item.iSubItem = 1;
	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.lParam = (INT_PTR)mConfigList[row];
	CString cstring;
	if( !config->getValue().empty() )
		cstring = BTString2Lang( config->getValue() ).c_str();

	item.pszText = cstring.GetBuffer();

	//prepare for add control
	if( !(row <= mListControls.size()) || row == 0)
	{
		ASSERT( row == mListControls.size() );
		ASSERT( row == mListExpandButton.size() );
		ASSERT( row == mListTypeInfo.size() );
		mListControls.insert( mListControls.begin() + (indexdiff_t)row , (ConfigControl*)NULL);
		SControlTypeInfo info = {0,(uint16)config->getUIHint() };
		mListTypeInfo.insert( mListTypeInfo.begin() + (indexdiff_t)row ,info);
		mListExpandButton.insert( mListExpandButton.begin() + (indexdiff_t)row ,(CExpButton*)NULL);
	}
	else
	{
		SControlTypeInfo info = mListTypeInfo[row-1];//parent

		if( bExpandSub )
			++info.depth;

		info.controlType = (uint8)config->getUIHint();
		mListExpandButton.insert( mListExpandButton.begin() + (indexdiff_t)row ,(CExpButton*)NULL);
		mListControls.insert( mListControls.begin() + (indexdiff_t)row , (ConfigControl*)NULL);
		mListTypeInfo.insert( mListTypeInfo.begin() + (indexdiff_t)row ,info);

		//offset the button id of the below buttons
		for(size_t i = row + 1; i < mListExpandButton.size(); ++i)
		{
			CExpButton* button = mListExpandButton[i];
			if( button != NULL )
				button->SetDlgCtrlID( button->GetDlgCtrlID() + 1 );
		}
		//offset the control id of the belows
		for(index_t i = (index_t)row + 1; i < mListControls.size(); ++i)
		{
			CWnd* wnd;
			if( mListControls[i] != NULL && (wnd = mListControls[i]->GetWindow()) != NULL )
			{
				int ID = wnd->GetDlgCtrlID()+1;
				wnd->SetDlgCtrlID( ID );
			}
		}
	}

	if( config->getSubConfigCount() > 0 )
	{
		//ASSERT( this->GetItemCount() > 0 );
		ASSERT( mListExpandButton[row] == NULL );
		ASSERT( row < (int)mListExpandButton.size() );
		CExpButton* button = BLADE_NEW CExpButton();
		mListExpandButton[row] = button;
		CRect rect;
		rect.left = rect.top = 0;
		rect.right = rect.bottom = EXP_BUTTON_SIZE;
		button->Create(rect, this, EXP_BUTTON_ID_BASE + (UINT)row );
		button->EnableWindow(TRUE);
	}

	//insert a item with name
	if( this->InsertItem((int)row, name.c_str()) == -1 )
		BLADE_EXCEPT(EXC_API_ERROR,BTString("MFC CListCtrl Error: Cannot Insert Item."));

	if(this->SendMessage(LVM_SETITEMTEXT,row,(LPARAM)&item) != TRUE)
	{
		//
		return uint(-1);
	}

	CWnd* Wnd = mListControls[row] == NULL ? NULL : mListControls[row]->GetWindow();

	if( Wnd != NULL )
		ASSERT( Wnd->GetDlgCtrlID() == CONTROL_ID_BASE+(int)row );
	else
		return (uint)row;

	if( mForceShowAll )
	{
		//this->SetItemText(row,1,TEXT(""));
		cstring = TEXT("");
		item.iItem = (int)row;
		item.iSubItem = 1;
		item.mask = LVIF_PARAM | LVIF_TEXT;
		item.lParam = NULL;
		item.pszText = cstring.GetBuffer();
		this->SendMessage(LVM_SETITEMTEXT,row,(LPARAM)&item);

		Wnd->ShowWindow(SW_SHOW);
	}
	else
		Wnd->ShowWindow(SW_HIDE);

	return (uint)row;
}


//////////////////////////////////////////////////////////////////////////
void	COptionList::refreshControl()
{
	POSITION pos = this->GetFirstSelectedItemPosition();
	index_t currentIndex = (index_t)this->GetNextSelectedItem(pos);

	CRect client;
	this->GetClientRect(&client);

	if( this->GetHeaderCtrl() != NULL && this->GetHeaderCtrl()->IsWindowVisible() )
	{
		CRect headerRect;
		this->GetHeaderCtrl()->GetWindowRect(&headerRect);
		client.top += headerRect.Height() + 2;
	}

	LV_COLUMN lvc, lvcprev ;
	::ZeroMemory(&lvc, sizeof(lvc));
	::ZeroMemory(&lvcprev, sizeof(lvcprev));
	lvc.mask = LVCF_WIDTH;
	lvcprev.mask = LVCF_WIDTH;

	size_t itemCount = (size_t)this->GetItemCount();
	for(size_t i = 0; i < itemCount; ++i )
	{
		CRect rcItem;
		this->GetItemRect((int)i,&rcItem,LVIR_BOUNDS);
		rcItem.DeflateRect(1,0,1,1);
		//position the expand button
		CButton* expButton = mListExpandButton[i];
		if( expButton != NULL )
		{
			CRect rc(rcItem);
			rc.left = 0;
			rc.right = rcItem.left + EXP_BUTTON_SIZE-1;
			rc.left += (INDENT_SIZE-EXP_BUTTON_SIZE)/2;
			rc.top += (rc.Height()-(EXP_BUTTON_SIZE) )/2;
			rc.bottom = rc.top + EXP_BUTTON_SIZE-2;
			rc.OffsetRect(-this->GetScrollPos( SB_HORZ ), 0);

			int offset = mListTypeInfo[i].depth*INDENT_SIZE;
			rc.OffsetRect(offset,0);
			expButton->MoveWindow(&rc);
			expButton->Invalidate(FALSE);
			expButton->ShowWindow(SW_SHOW);
		}

		CWnd* ctrl = mListControls[i] == NULL ? NULL : mListControls[i]->GetWindow();
		if( ctrl == NULL )
			continue;

		rcItem.DeflateRect(1,0,0,0);	//for col 0
		for (int nCol = 1; this->GetColumn(nCol, &lvc) == TRUE; nCol++)
		{
			this->GetColumn(nCol-1, &lvcprev);
			rcItem.left += lvcprev.cx;
			rcItem.right = rcItem.left + lvc.cx;
			rcItem.DeflateRect(1,0,0,0);

			if ( nCol == 1 ) 
			{
				CRect rcTest;
				rcTest.IntersectRect(&rcItem,&client);
				if( rcTest.IsRectEmpty() || rcTest.Width() < MIN_COLUM_WIDTH )
				{
					if(ctrl->IsWindowVisible())
						ctrl->ShowWindow(SW_HIDE);
				}
				else
				{
					if( mForceShowAll || i == currentIndex )
					{
						ASSERT( ctrl != NULL );
						CRect rc(rcItem);
						rc.DeflateRect(0, 0, 2, 0);

						ctrl->SetWindowPos(NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_NOZORDER);
						ctrl->Invalidate(FALSE);

						if( !ctrl->IsWindowVisible() )
							ctrl->ShowWindow(SW_SHOW);
						CWnd* focus = CWnd::GetFocus();
						bool hasFocus = ctrl == focus;
						while(focus != NULL && !hasFocus)
						{
							hasFocus = (focus == this);
							focus = focus->GetParent();
						}
						if(hasFocus)
							ctrl->SetFocus();
					}
					else if(ctrl->IsWindowVisible())
					{
						ctrl->ShowWindow(SW_HIDE);
						CString text;
						if( mConfigList[i]->getValue() != NULL )
							text = BTString2Lang( mConfigList[i]->getValue() ).c_str();

						CString oldText = this->GetItemText((int)i,1);
						if( text != oldText  )
						{
							LVITEM item;
							item.iItem = (int)i;
							item.iSubItem = 1;
							item.mask = LVIF_PARAM | LVIF_TEXT;
							item.lParam = NULL;
							item.pszText = text.GetBuffer();
							this->SendMessage(LVM_SETITEMTEXT,i,(LPARAM)&item);
						}
					}
				}

			}//column=1

		}//column
	}//row

	//on some environment (i.e. 3ds max) this will call paint and then call refresh control, 
	//resulting a dead recursion
	//this->SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
void		COptionList::expandMultiRows(index_t index,BOOL bExpand, BOOL bCascade/* = FALSE*/)
{
	ASSERT( index < mListExpandButton.size() && mListExpandButton[index] != NULL );
	ASSERT ( index < mConfigList.size() && (size_t)this->GetItemCount() > index );
	ASSERT( index < mListTypeInfo.size() );

	IConfig* config = mConfigList[index];
	ASSERT( config != NULL && config->getSubConfigCount() > 0 );

	CExpButton* button = mListExpandButton[index];

	bool devMode = IEnvironmentManager::getSingleton().getVariable(ConstDef::EnvBool::DEVELOPER_MODE);
	if( bExpand )
	{
		ConfigUIHint hint = config->getUIHint();
		//UIData is expand state
		hint.mUIData = mUIExpanded;
		config->setUIHint(hint);

		if( bCascade )
			button->SetExpanded(TRUE);
		else
			assert( button->GetExpanded() );

		const IConfig::IList* list = config->getSubConfigs();
		CONFIG_ACCESS parentAccess = config->getAccess();

		size_t count = list->getCount();
		for(size_t i = count-1; i < count && count > 0; --i)
		{
			IConfig* opt = list->getConfig(i);
			CONFIG_ACCESS subAccess = opt->getBinaryValue().isCollection() ? CAF_READ : opt->getAccess();

			if( !(parentAccess&CAF_WRITE) && !(subAccess&CAF_NOCASCADE) )
				subAccess = CONFIG_ACCESS(subAccess&(~CAF_WRITE));

			if( !(subAccess&CAF_READ) || !devMode && (subAccess&CAF_DEVMODE) )
				continue;

			mConfigList.insert(mConfigList.begin()+(indexdiff_t)index+1,opt);
			mListAccess.insert(mListAccess.begin()+(indexdiff_t)index+1,subAccess);
			this->insertListItem(index+1, opt, true);

			//recursively expand sub if sub state is expanded
			if( opt->getSubConfigCount() > 0 )
			{
				ASSERT( mListExpandButton[index+1] != NULL);
				if( opt->getUIHint().mUIData == mUIExpanded )
					this->expandMultiRows(index+1, TRUE, TRUE);
			}
		}
	}
	else
	{
		ConfigUIHint hint = config->getUIHint();
		//UIData is expand state

		//when parent collapse: sub tree's internal state is not changed,
		//so that when parent expand again, sub tree will restore expand too.
		//ASSERT( hint.mUIData == TRUE );
		if( !bCascade )
		{
			hint.mUIData = mUINotExpanded;
			config->setUIHint(hint);
		}

		const IConfig::IList* list = config->getSubConfigs();
		CONFIG_ACCESS parentAccess = config->getAccess();

		size_t count = list->getCount();
		for( size_t i = 0; i < count; ++i)
		{
			IConfig* opt = list->getConfig(i);
			CONFIG_ACCESS subAccess = opt->getBinaryValue().isCollection() ? CAF_READ : opt->getAccess();

			if( !(parentAccess&CAF_WRITE) && !(subAccess&CAF_NOCASCADE) )
				subAccess = CONFIG_ACCESS(subAccess&(~CAF_WRITE));

			if( !(subAccess&CAF_READ) || !devMode && (subAccess&CAF_DEVMODE) )
			{
				assert((int)mConfigList.size() <= index+1 || mConfigList[index+1] != opt);
				continue;
			}

			//recursively collapse group
			//note: use fixed offset '1' because the previous is deleted before each iteration
			IConfig* subConfig = mConfigList[index+1];
			//assert(subConfig == opt);
			//config list has changed: added more data: opt, we don't have it, skip
			if(subConfig != opt )
				continue;
			if( subConfig->getSubConfigCount() > 0 )
			{
				assert(mListExpandButton[index + 1] != NULL);

				if( subConfig->getUIHint().mUIData == mUIExpanded )
					this->expandMultiRows(index+1, FALSE, TRUE);
			}
			this->DeleteItem((int)index+1);
		}
	}
	assert( mListExpandButton.size() == mConfigList.size() );
	assert( this->GetItemCount() == (int)mConfigList.size() );
}

//////////////////////////////////////////////////////////////////////////
bool	COptionList::isInCenterLine(const CPoint& pt)
{
	static const int LINE_ERROR = 3;
	CRect rcClient;
	this->GetClientRect(&rcClient);
	int width = (int)( (float)rcClient.Width() * mColumnRate );
	if( pt.x >= width - LINE_ERROR && pt.x <= width + LINE_ERROR )
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
size_t		COptionList::getItemHeight() const
{
	return DEFAULT_ROW_HEIGHT+1;
}

//////////////////////////////////////////////////////////////////////////
index_t		COptionList::buttonIDToIndex(index_t expandButtonID)
{
	if(expandButtonID < EXP_BUTTON_ID_BASE)
	{
		assert(false);
		return 0;
	}

	index_t index = expandButtonID - EXP_BUTTON_ID_BASE;

	//calculate upper expand count
	index_t realindex = index;
	CExpButton* expandButton = NULL;

	for(size_t i = 0; i < mListExpandButton.size(); ++i )
	{
		expandButton = mListExpandButton[i];
		if( expandButton == NULL )
			continue;

		if( (index_t)expandButton->GetDlgCtrlID()-EXP_BUTTON_ID_BASE == index )
		{
			realindex = i;
			break;
		}
	}
	return realindex;
}

//////////////////////////////////////////////////////////////////////////
void		COptionList::updateColumnWidth(int center/* = 0*/)
{
	CRect rect;
	this->GetClientRect(&rect);
	int cx = rect.Width();
	if(cx == 0)
		return;

	int width;
	if(center == 0)
		width = min( max( (int)((float)cx * mColumnRate), MIN_COLUM_WIDTH), cx - MIN_COLUM_WIDTH);
	else
	{
		width = min( max( (int)center, MIN_COLUM_WIDTH), cx - MIN_COLUM_WIDTH);
		mColumnRate = (float)width / (float)cx;
	}
	
	this->SetColumnWidth(0, width);
	this->SetColumnWidth(1, cx - width);

	this->ShowScrollBar(SB_HORZ,FALSE);
	this->EnableScrollBar(SB_HORZ, FALSE);
}

//////////////////////////////////////////////////////////////////////////
void COptionList::onConfigChanging(index_t ctlID)
{
	if( ctlID < CONTROL_ID_BASE )
	{
		assert(false);
		return;
	}

	index_t item = ctlID - CONTROL_ID_BASE;
	item = this->buttonIDToIndex( EXP_BUTTON_ID_BASE + item );

	//if this option could be expanded ?
	if( item > mListExpandButton.size() )
		return;

	//if current status is expanded
	CExpButton* expButton = mListExpandButton[item];
	if( expButton == NULL || !expButton->GetExpanded() )
		return;

	IConfig* config = mConfigList[item];
	ConfigControl* control = mListControls[item];
	assert( config == control->GetConfig() );
	BLADE_UNREFERENCED(config);

	//collapse sub lists
	control->::ConfigControl::GetWindow()->EnableWindow(FALSE);
	this->SetRedraw(FALSE);
	this->expandMultiRows(item, FALSE);
	expButton->EnableWindow(FALSE);
	this->Invalidate(FALSE);
	this->SetRedraw(TRUE);

	//auto re-expand this option
	mRefreshExpandIndex = item;
	mTimerID = this->SetTimer(1, 100, NULL);
}

//////////////////////////////////////////////////////////////////////////
void COptionList::onConfigChanged(index_t /*ctlID*/)
{

}

//////////////////////////////////////////////////////////////////////////
LRESULT COptionList::DefWindowProc(UINT message,WPARAM wParam,LPARAM lParam)
{
	LRESULT lret= __super::DefWindowProc(message, wParam, lParam);

	switch(message)
	{
	case WM_COMMAND:
		{
			switch( HIWORD(wParam) )
			{
			case BN_CLICKED:
			case BN_DBLCLK:
				{
					if( LOWORD(wParam) >= WORD(EXP_BUTTON_ID_BASE) )
					{
						index_t index = this->buttonIDToIndex( (index_t)LOWORD(wParam) );
						assert( index < (int)mListExpandButton.size() && mListExpandButton[index] != NULL );
						this->SetRedraw(FALSE);
						this->expandMultiRows( index, mListExpandButton[index]->GetExpanded() );
						this->SetRedraw(TRUE);
						this->Invalidate(FALSE);
					}
					else
					{
						//currently check box only
					}
				}
				break;
			}

		}//WM_COMMAND
		break;
	case LVM_SETITEMTEXT:
		{
			LPLVITEM pItem = (LPLVITEM)lParam;
			if( pItem->iSubItem == 0 || pItem->lParam == 0 )
				break;

			CRect rect;
			{
				//this->GetItemRect(pItem->iItem, &rect,LVIR_BOUNDS);
				rect.top = pItem->iSubItem*(DEFAULT_ROW_HEIGHT+1) - 1;
				rect.bottom = rect.top + DEFAULT_ROW_HEIGHT + 3;
				LV_COLUMN lvc;
				lvc.mask = LVCF_WIDTH;
				if( this->GetColumn(pItem->iSubItem, &lvc) == TRUE )
					rect.right = rect.left + lvc.cx;
			}

			IConfig* config = reinterpret_cast<IConfig*>(pItem->lParam);
			ConfigUIHint hint = config->getUIHint();
			if( config->getBinaryValue().isCollection() )
				hint = CUIH_NONE;
			if( (mListAccess[(index_t)pItem->iItem]&CAF_WRITE) == 0 &&
				//read-only image has controls not only texts
				hint.mHint != CUIH_IMAGE)
			{
				hint = CUIH_NONE;
			}

			ConfigControl* newControl = ConfigControlUtil::CreateConfigControl(config, this, rect, (UINT)(CONTROL_ID_BASE + pItem->iItem), &hint, this);
			if( newControl != NULL )
			{
				mListControls[(index_t)pItem->iItem] = newControl;
			}
		}//case LVM_INSERTITEM

		break;
	case LVM_DELETEALLITEMS:
			this->clear();
		break;
	case LVM_DELETEITEM:
		{
			index_t item = (index_t)wParam;
			BLADE_DELETE mListControls[item];
			BLADE_DELETE mListExpandButton[item];
			mListTypeInfo.erase( mListTypeInfo.begin() + (indexdiff_t)item );
			mListAccess.erase(mListAccess.begin() + (indexdiff_t)item);
			mConfigList.erase( mConfigList.begin() + (indexdiff_t)item );
			mListControls.erase( mListControls.begin() + (indexdiff_t)item );
			mListExpandButton.erase( mListExpandButton.begin() + (indexdiff_t)item );
			for(index_t i = item; i < mListControls.size(); ++i)
			{
				if( mListControls[i] != NULL )
					mListControls[i]->GetWindow()->SetDlgCtrlID( mListControls[i]->GetWindow()->GetDlgCtrlID()-1 );
			}
		}
		break;
	}
	return lret;
}

//////////////////////////////////////////////////////////////////////////
void COptionList::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC* pDC;
	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CFont* oldFont = (CFont*)pDC->SelectObject( this->GetFont() );
	LV_COLUMN lvc, lvcprev ;
	::ZeroMemory(&lvc, sizeof(lvc));
	::ZeroMemory(&lvcprev, sizeof(lvcprev));
	lvc.mask = LVCF_WIDTH | LVCF_FMT;
	lvcprev.mask = LVCF_WIDTH | LVCF_FMT;

	CRect itemRect(&lpDrawItemStruct->rcItem);
	itemRect.InflateRect(0,1,0,0);
	pDC->FrameRect(itemRect, &THEMEBRUSH(TC_LINE));
	itemRect.DeflateRect(1,1,1,1);

	for (int nCol = 0; this->GetColumn(nCol, &lvc); ++nCol)
	{
		// Get Previous Column Width in order to move the next display item
		this->GetColumn(nCol-1, &lvcprev);
		itemRect.left += lvcprev.cx;
		itemRect.right = itemRect.left + lvc.cx + 1;
		pDC->FrameRect(itemRect, &THEMEBRUSH(TC_LINE));
		itemRect.DeflateRect(1,0,0,0);

		int indentWidth = (mListTypeInfo[lpDrawItemStruct->itemID].depth)*INDENT_SIZE;
		bool active = lpDrawItemStruct->itemState & ODS_SELECTED;
		bool readOnly = (mListAccess[lpDrawItemStruct->itemID]&CAF_WRITE) == 0;
		bool isTitle = mListExpandButton[lpDrawItemStruct->itemID] != NULL;

		//expandable title?
		if( isTitle && nCol == 0)
		{
			pDC->FillSolidRect(&lpDrawItemStruct->rcItem, THEMECOLOR(TC_LINE));
			pDC->SetTextColor(THEMECOLOR(TC_LINE));
			if ( active )
			{
				CRect titleRect(itemRect);
				titleRect.left += indentWidth + INDENT_SIZE;
				titleRect.DeflateRect(1,1,1,2);
				pDC->DrawFocusRect(&titleRect);
			}
		}
		else if( !isTitle )
		{
			COLORREF color = (nCol == 0 && active) ? THEMECOLOR(TC_HILIGHTBK) : THEMECOLOR(TC_CTLBK);
			pDC->FillSolidRect(&itemRect, color);
		}

		CRect textRect(itemRect);
		//draw left border 
		if( nCol == 0 )
		{
			if( isTitle )
				indentWidth += INDENT_SIZE;
			CRect rect(itemRect);
			rect.right = rect.left + indentWidth;
			pDC->FillSolidRect(&rect, THEMECOLOR(TC_LINE));
			textRect.left += indentWidth;
		}

		ConfigControl* control = mListControls[lpDrawItemStruct->itemID];
		if ( nCol == 0 || control == NULL || !control->GetWindow()->IsWindowVisible() )//has control, don't draw text
		{
			CString text = this->GetItemText((int)lpDrawItemStruct->itemID,nCol);
			UINT uFormat  = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
			COLORREF oldCOlor = pDC->SetTextColor((!readOnly || isTitle || active && nCol == 0) ? THEMECOLOR(TC_TEXT) : THEMECOLOR(TC_GRAYTEXT));
			::DrawText(lpDrawItemStruct->hDC, text.GetString(), text.GetLength(), &textRect, uFormat) ;
			pDC->SetTextColor(oldCOlor);
		}
	}
	pDC->SelectObject(oldFont);
	CDC::DeleteTempMap();
}

// COptionList message handlers
void COptionList::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	lpMeasureItemStruct->itemHeight = DEFAULT_ROW_HEIGHT + 1;
	lpMeasureItemStruct->itemWidth += this->GetHeaderCtrl()->GetItemCount() + 1;
}

void COptionList::OnPaint()
{
	//hide/show control will invalid some client area, and that may call OnPaint again
	//so refresh the control first and then call the default paint so that
	//the dirty regions are cleared
#if 0
	if(mPaintPhase == 0)
	{
		this->SetRedraw(FALSE);
		this->refreshControl();
		this->SetRedraw(TRUE);
		this->Invalidate(FALSE);
		++mPaintPhase;
	}
	else
	{
		mPaintPhase = 0;
		this->Default();
	}
#else
	this->refreshControl();
	this->Default();
#endif
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnNcDestroy()
{
	__super::OnNcDestroy();

	// TODO: Add your message handler code here
	for(size_t i = 0; i < mListControls.size(); ++i)
	{
		if( mListControls[i] != NULL )
			BLADE_DELETE mListControls[i];
	}
	mListControls.clear();
	mListTypeInfo.clear();
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent != mTimerID )
		return;

	assert( mRefreshExpandIndex != -1 );

	this->KillTimer(mTimerID);
	mListControls[mRefreshExpandIndex]->GetWindow()->EnableWindow(TRUE);
	mListExpandButton[mRefreshExpandIndex]->EnableWindow(TRUE);
	this->SetRedraw(FALSE);
	this->expandMultiRows(mRefreshExpandIndex,TRUE);
	this->SetRedraw(TRUE);
	mRefreshExpandIndex = INVALID_INDEX;
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanging(lpwndpos);

	if( !(lpwndpos->flags & SWP_NOSIZE) )
		this->updateColumnWidth();
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanged(lpwndpos);
	this->updateColumnWidth();
}

//////////////////////////////////////////////////////////////////////////
BOOL	COptionList::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint pt;
	::GetCursorPos(&pt);
	this->ScreenToClient(&pt);

	if( this->isInCenterLine(pt) || mDraging )
	{
		::SetCursor( ::LoadCursor(NULL,IDC_SIZEWE) );
		return TRUE;
	}
	else
		return __super::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( this->isInCenterLine(point) )
	{
		mDraging = true;
		this->SetCapture();
	}
	else
		return __super::OnLButtonDown(nFlags,point);
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( mDraging )
		::ReleaseCapture();
	return __super::OnLButtonDown(nFlags,point);
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnCaptureChanged(CWnd* /*pWnd*/)
{
	mDraging = false;
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnMouseMove(UINT nFlags, CPoint point)
{
	if( mDraging )
		this->updateColumnWidth(point.x);

	return __super::OnLButtonDown(nFlags,point);
}

//////////////////////////////////////////////////////////////////////////
void	COptionList::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	//new item selected
	if( pNMItemActivate->iItem != -1 
		&& (pNMItemActivate->uChanged & LVIF_STATE) 
		&& (pNMItemActivate->uNewState & LVNI_SELECTED)
		)
	{
		if( (int)mConfigList.size() > pNMItemActivate->iItem )
		{
			IConfig* config = mConfigList[(index_t)pNMItemActivate->iItem];
			if( config != NULL && config->getDataHandler() != NULL && config->getDataHandler()->getTarget() != NULL )
			{
				const TString& value = config->getValue();
				const Bindable* bindable = config->getDataHandler()->getTarget();
				OptionHighLightEvent state(bindable, value, config->getUIHint().getUserData() );
				IEventManager::getSingleton().dispatchEvent(state);
			}
		}
		else
			assert(false);
	}
}
