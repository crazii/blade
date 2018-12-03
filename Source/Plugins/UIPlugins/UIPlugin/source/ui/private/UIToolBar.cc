/********************************************************************
	created:	2011/05/11
	filename: 	UIToolBar.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ConfigDialog/ConfigControlUtil.h>
#include <ui/MenuManager.h>
#include <ui/Menu.h>
#include <ui/private/UIToolBar.h>
#include "../widgets/TabedReBar.h"

using namespace Blade;
static const int GRIPPER_HEIGHT = 15;

//
IMPLEMENT_DYNAMIC(CUIToolBar, CToolBar);

//
BEGIN_MESSAGE_MAP(CUIToolBar, CToolBar)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_GETTEXT, OnGetText)
	ON_MESSAGE(WM_GETTEXTLENGTH, OnGetTextLength)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CUIToolBar::CUIToolBar(const Blade::TString& name)
:mName(name)
,mTooTipLink(NULL)
,mColumn(1)
,mCurrentColumn(1)
{
	mTitle = BTString2Lang(mName);
	mMaxStringWidth = 0;
	mDockBarID = 0;
}

//////////////////////////////////////////////////////////////////////////
CUIToolBar::~CUIToolBar()
{
	for(size_t i = 0; i < mConfigTools.size(); ++i)
		BLADE_DELETE mConfigTools[i];
}

/************************************************************************/
/* common method                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
void		CUIToolBar::SetColumnCount(uint8 nColumn)
{
	nColumn = max(nColumn ,1u);
	if( mColumn == nColumn )
		return;
	mColumn = nColumn;
	int nCount = GetToolBarCtrl().GetButtonCount();

	int rowCount = 0;
	for (int i = 0; i < nCount; i++)
	{
		UINT nStyle = GetButtonStyle(i);
		BOOL bWrap = (((i + 1) % nColumn) == 0);
		if (bWrap)
		{
			nStyle |= TBBS_WRAPPED;
			++rowCount;
		}
		else
			nStyle &= ~TBBS_WRAPPED;
		SetButtonStyle(i, nStyle);
	}

	if( rowCount == 0 )
		mCurrentColumn = (uint8)nCount;
	else
		mCurrentColumn = mColumn;
}

//////////////////////////////////////////////////////////////////////////
int			CUIToolBar::initDropDownMenu(UINT buttonID, Menu* menu, bool text)
{
	if( menu == NULL || !menu->isGrouped() || menu->isEmbedded() )
	{
		assert(false);
		return -1;
	}

	int iStringBase = -1;
	const MENU_DATA* data = menu->getCheckedItemData();

	TStringParam list;
	int maxWidth = 0;
	if (menu->getItems(list) && data != NULL && text)
	{
		CDC* pDC = this->GetDC();
		TStringParam langList;
		for (size_t i = 0; i < list.size(); ++i)
		{
			const TString& subText = BTString2Lang(list[i]);
			langList.push_back(subText);
			int cx = pDC->GetTextExtent(subText.c_str(), (int)subText.size()).cx;
			if (maxWidth < cx)
				maxWidth = cx;
		}

		//note: hacking way to make all strings with the same length for this button
		IOBuffer strBuffer;
		for (size_t i = 0; i < langList.size(); ++i)
		{
			TString subText = (TString)langList[i];
			int cx = pDC->GetTextExtent(subText.c_str(), (int)subText.size()).cx;
			int spacex = pDC->GetTextExtent(TEXT(" "), 1).cx;
			while (cx + spacex < maxWidth)
			{
				subText += BTString(" ");
				cx = pDC->GetTextExtent(subText.c_str(), (int)subText.size()).cx;
			}
			strBuffer.writeTStringRAW(subText);
		}
		tchar end = tchar();
		strBuffer.write(&end);

		iStringBase = this->GetToolBarCtrl().AddStrings((const tchar*)strBuffer.getData());
		this->ReleaseDC(pDC);
	}
	mMaxStringWidth += (int16)maxWidth;

	DROP_DOWN_INFO& dropdownInfo = mDropDownInfo[buttonID];
	dropdownInfo.menu = menu;
	dropdownInfo.index = data != NULL ? (int)data->mIndex : -1;
	dropdownInfo.indexBase = iStringBase;

	return iStringBase;
}

//////////////////////////////////////////////////////////////////////////
bool		CUIToolBar::updateDropDownMenu(int buttonIndex, index_t instance)
{
	TBBUTTON tbb;
	VERIFY(this->GetToolBarCtrl().GetButton(buttonIndex, &tbb));

	if( !(tbb.fsStyle&BTNS_WHOLEDROPDOWN) )
		return false;

	ButtonTextMap::iterator iter = mDropDownInfo.find((UINT)tbb.idCommand);
	if( iter == mDropDownInfo.end() )
		return false;

	DROP_DOWN_INFO& dropdownInfo = iter->second;

	//update all drop down items
	for (size_t i = 0; i < dropdownInfo.menu->getSubMenuCount(); ++i)
	{
		if (dropdownInfo.menu->getSubMenu(i)->getBindingCommand() != NULL)
			dropdownInfo.menu->getSubMenu(i)->getBindingCommand()->update(dropdownInfo.menu->getSubMenu(i), instance);
	}

	Menu* subMenu = static_cast<Menu*>( dropdownInfo.menu->getCheckedSubMenu() );
	if (subMenu == NULL)
	{
		assert(false);
		return false;
	}

	const MENU_DATA* data = subMenu->getMenuData();
	int subIndex = (int)data->mIndex;
	if(dropdownInfo.index == subIndex && (tbb.iBitmap == data->mIcon) )
		return true;

	CRect rect;
	this->GetItemRect(buttonIndex, &rect);
	VERIFY(this->GetToolBarCtrl().DeleteButton(buttonIndex));
	tbb.iString = dropdownInfo.indexBase + subIndex;
	if(data->mIcon != INVALID_ICON_INDEX )
		tbb.iBitmap = data->mIcon;
	VERIFY(this->GetToolBarCtrl().InsertButton(buttonIndex, &tbb));
	dropdownInfo.index = subIndex;
	this->InvalidateRect(&rect);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool		CUIToolBar::updateCmdUI(IUICommand* cmd, index_t index, UINT_PTR id, CCmdUI* cmdui)
{
	this->updateDropDownMenu((int)cmdui->m_nIndex, index);

	Button* button = this->getButton(id);
	if (button != NULL)
	{
		cmd->update(button, index);
		cmdui->Enable((button->mState&TBSTATE_ENABLED));
		cmdui->SetCheck((button->mState&TBSTATE_CHECKED) && (button->mState&TBSTATE_ENABLED));
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool		CUIToolBar::handleToolCommand(UINT_PTR id)
{
	IUICommand* cmd = MenuManager::getSingleton().getCommand(id);
	if (cmd == NULL)
		return false;

	Button* button = this->getButton(id);
	if (button != NULL)
	{
		cmd->execute(button);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool		CUIToolBar::addConfigTool(IConfig* config, size_t width)
{
	if( config == NULL || config->getUIHint() == CUIH_NONE || config->getSubConfigCount() != 0)
	{
		assert(false);
		return false;
	}
	//check access? access could change from non writable to writable
	int index = this->GetToolBarCtrl().GetButtonCount();
	static const int DEFAULT_WIDTH = 110;
	if( (int)width < 0)
		width = DEFAULT_WIDTH;

	static const int HPADDING = 5;
	static const int VPADDING = 2;
	TBBUTTON tbb;
	::memset(&tbb, 0, sizeof(tbb));
	tbb.idCommand = 0;
	tbb.fsStyle = TBBS_SEPARATOR;
	tbb.iBitmap = (int)width + HPADDING*2;
	tbb.dwData = 0;
	tbb.fsState = 0;
	tbb.iString = -1;
	VERIFY( this->GetToolBarCtrl().AddButtons(1, &tbb) );
	this->SetButtonInfo(index, (UINT)tbb.idCommand, TBBS_SEPARATOR, (int)width + HPADDING*2);
	CRect rect;
	this->GetItemRect(index, &rect);
	rect.DeflateRect(HPADDING, VPADDING,HPADDING, VPADDING);
	ConfigControl* tool = ConfigControlUtil::CreateConfigControl(config, this, rect, (UINT)tbb.idCommand);
	if( tool == NULL )
		return false;
	tool->setEventNotify(true);
	tool->GetWindow()->ShowWindow(SW_SHOW);

	if( mTooTipLink != NULL )
	{
		const TString& title = BTString2Lang(config->getName());
		tool->AddToolTip(mTooTipLink, title);
	}
	mConfigTools.push_back(tool);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void		CUIToolBar::updateConfigTools()
{
	for(size_t i = 0; i < mConfigTools.size(); ++i)
		mConfigTools[i]->updateData(true);
}

/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
LRESULT CUIToolBar::OnSetText(WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = __super::SendMessage(WM_SETTEXT, wParam, lParam);
	this->SendMessage(WM_NCPAINT);
	return lResult;
}

//////////////////////////////////////////////////////////////////////////
LRESULT		CUIToolBar::OnGetText(WPARAM wParam, LPARAM lParam)
{
	WPARAM cchTextMax = wParam;
	LPTSTR lpszText = (LPTSTR)lParam;
	UINT minchars = (UINT)cchTextMax;
	if(lpszText != NULL)
	{
		Char_Traits<TCHAR>::copy(lpszText,mTitle.c_str(),minchars);
		//if string is truncated,assure NULL char appended
		lpszText[minchars-1] = TEXT('\0');
		return (LRESULT)minchars;
	}
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////
LRESULT		CUIToolBar::OnGetTextLength(WPARAM wParam, LPARAM lParam)
{
	BLADE_UNREFERENCED(wParam);
	BLADE_UNREFERENCED(lParam);
	return (LRESULT)mTitle.size();
}

//////////////////////////////////////////////////////////////////////////
void		CUIToolBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDockBar* oldDockBar = m_pDockBar;
	__super::OnLButtonDblClk(nFlags,point);
	bool newDockTabed = m_pDockBar != NULL && m_pDockBar->IsKindOf(RUNTIME_CLASS(TabedReBar));

	if (oldDockBar != NULL && ::IsWindow(oldDockBar->m_hWnd) )
	{
		if(oldDockBar->IsKindOf(RUNTIME_CLASS(TabedReBar)) && !newDockTabed ) 
		{
			TabedReBar* pTabedDockBar = STATIC_DOWNCAST(TabedReBar,oldDockBar);
			pTabedDockBar->UpdateTabData(this);
			this->SetColumnCount(0xFF);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnRButtonDown(UINT nFlags, CPoint point)
{
	BLADE_UNREFERENCED(nFlags);

	INT_PTR id = this->OnToolHitTest(point, NULL);
	if( id != -1 )
	{
		Button* button = this->getButton((UINT_PTR)id);
		if (button != NULL)
		{
			IUICommand* cmd = MenuManager::getSingleton().getCommand((UINT_PTR)id);
			//config the tool.
			//TODO: add pure event callback for RButton, and handle it at higher level?
			if (cmd != NULL)
				cmd->config(button);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	BLADE_UNREFERENCED(nFlags);
	BLADE_UNREFERENCED(point);
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanged(lpwndpos);

	if( !this->IsVertDocked() )
		this->SetColumnCount(0xFF);

	if( mTooTipLink == NULL )
		return;

	CToolBarCtrl& ctrl = this->GetToolBarCtrl();
	int nButtonCount = ctrl.GetButtonCount();

	if( nButtonCount > 0)
	{
		for(int i = 0; i < nButtonCount; ++i)
		{
			UINT nID = this->GetItemID(i);
			CRect rect;
			this->GetItemRect(i,&rect);

			if(nID == 0)
			{
				//separator has NO command ID even if set on addButtons
				//the Config control doesn't need adjust
			}
			else
				mTooTipLink->SetToolRect(&ctrl, nID, &rect);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanging(lpwndpos);
	mDockBarID = (UINT)this->GetParent()->GetDlgCtrlID();
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnNcPaint()
{
	CWindowDC dc(this);
	CRect rcClient, rcBar;
	this->GetClientRect(rcClient);
	this->ClientToScreen(rcClient);
	this->GetWindowRect(rcBar);
	rcClient.OffsetRect(-rcBar.TopLeft());
	rcBar.OffsetRect(-rcBar.TopLeft());

	CDC mdc;
	mdc.CreateCompatibleDC(&dc);

	CBitmap bm;
	bm.CreateCompatibleBitmap(&dc, rcBar.Width(), rcBar.Height());
	CBitmap* pOldBm = mdc.SelectObject(&bm);

	// draw borders in non-client area
	mdc.SetBkColor(THEMECOLOR(TC_WINBK));
	mdc.SetBkMode(TRANSPARENT);
	CRect rcDraw = rcBar;
	mdc.FrameRect(&rcBar, &THEMEBRUSH(TC_FRAME));

	// erase the NC background
	//mdc.FillRect(rcDraw, CBrush::FromHandle(
	//	(HBRUSH) GetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND)));
	if( !(this->IsHorzDocked() || this->IsFloating()) )
		this->NcPaintGripper(&mdc, rcClient);

	dc.IntersectClipRect(rcBar);
	dc.ExcludeClipRect(rcClient);

	dc.BitBlt(0, 0, rcBar.Width(), rcBar.Height(), &mdc, 0, 0, SRCCOPY);
	this->ReleaseDC(&dc);
	mdc.SelectObject(pOldBm);
	bm.DeleteObject();
	mdc.DeleteDC();
	CBrush::DeleteTempMap();
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	CRect rc(&lpncsp->rgrc[0]);
	rc.DeflateRect(1,1,1,1);

	if( this->IsVertDocked() && !this->IsFloating() )
		rc.DeflateRect(0, GRIPPER_HEIGHT, 0, 0);
	lpncsp->rgrc[0] = rc;
}

//////////////////////////////////////////////////////////////////////////
int	CUIToolBar::OnCreate(LPCREATESTRUCT lpcs)
{
	int ret = __super::OnCreate(lpcs);
	::SetWindowTheme(m_hWnd, L"", L"");

	COLORSCHEME cs;
	cs.dwSize = sizeof(cs);
	cs.clrBtnHighlight = THEMECOLOR(TC_HILIGHTBK);
	cs.clrBtnShadow = THEMECOLOR(TC_HILIGHTBK);
	this->GetToolBarCtrl().SetColorScheme(&cs);

	if(m_hReBarTheme != NULL)
	{
		::CloseThemeData(m_hReBarTheme);
		m_hReBarTheme = NULL;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnDestroy()
{
	this->GetToolBarCtrl().SetImageList( NULL );
	__super::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
HBRUSH	CUIToolBar::OnCtlColor(CDC* pDC, CWnd* /*pWnd*/, UINT /*nCtlColor*/)
{
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(THEMECOLOR(TC_TEXT));
	return THEMEBRUSH(TC_WINBK);
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTBCUSTOMDRAW* pNMCD = (NMTBCUSTOMDRAW*)pNMHDR;
	CDC* dc = CDC::FromHandle(pNMCD->nmcd.hdc);

	switch(pNMCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		{
			const size_t TEXT_LEN = 256;
			TCHAR strBuffer[TEXT_LEN] = { 0 };
			CToolBarCtrl& ctl = this->GetToolBarCtrl();

			DWORD_PTR id = pNMCD->nmcd.dwItemSpec;
			TBBUTTONINFO info;
			info.cbSize = sizeof(info);
			info.dwMask = TBIF_IMAGE | TBIF_STYLE | TBIF_TEXT;
			info.cchText = TEXT_LEN;
			info.pszText = strBuffer;
			ctl.GetButtonInfo((int)id, &info);

			UINT state = pNMCD->nmcd.uItemState;
			bool disable = (state&(CDIS_DISABLED|CDIS_GRAYED)) != 0;
			bool hot = (state&CDIS_HOT) != 0;
			bool checked = (state&(CDIS_CHECKED|CDIS_SELECTED)) != 0;
			CRect rect(pNMCD->nmcd.rc);

			if(hot)
				dc->FillSolidRect(&rect, THEMECOLOR(TC_HILIGHTBTN));
			else
				dc->FillSolidRect(&rect, THEMECOLOR(TC_BTN));
			if(checked)
				dc->FrameRect(&rect, &THEMEBRUSH(TC_HILIGHTBK));

			int ox = DEF_BUTTON_XEDGE/2;
			int oy = DEF_BUTTON_YEDGE/2;
			rect.DeflateRect(ox, oy, ox, oy);
			CImageList* imgList = disable ? ctl.GetDisabledImageList() : ctl.GetImageList();
			ImageList_Draw(imgList->GetSafeHandle(), info.iImage, dc->GetSafeHdc(), rect.left, rect.top, ILD_NORMAL);

			int imageWidth = ::GetSystemMetrics(SM_CXBORDER);
			if(imgList != NULL )
			{
				IMAGEINFO imgInfo;
				imgList->GetImageInfo(0, &imgInfo);
				if(imgInfo.hbmImage != NULL)
				{
					BITMAP bmp;
					::GetObject(imgInfo.hbmImage, sizeof(bmp), &bmp);
					imageWidth += bmp.bmWidth;
				}
			}
			rect.left += imageWidth;

			DWORD exStyle = ctl.GetExtendedStyle();
			CSize textExt(0,0);
			if((info.fsStyle&BTNS_SHOWTEXT) && *info.pszText != TEXT('\0') && (exStyle&TBSTYLE_EX_MIXEDBUTTONS))
			{
				CFont* oldFont = dc->SelectObject(this->GetFont());
				COLORREF oldColor = dc->SetTextColor(THEMECOLOR(TC_TEXT));
				size_t len = ::_tcslen(info.pszText);
				dc->DrawText(info.pszText, (int)len, &rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE);
				textExt = dc->GetTextExtent(info.pszText, (int)len);
				dc->SelectObject(oldFont);
				dc->SetTextColor(oldColor);
			}

			if((info.fsStyle&BTNS_DROPDOWN) && (exStyle&TBSTYLE_EX_DRAWDDARROWS))
			{
				rect.left += textExt.cx;

				CDC maskMemDC;
				maskMemDC.CreateCompatibleDC(dc);
				CBitmap maskBitmap;
				maskBitmap.CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
				maskMemDC.SelectObject( &maskBitmap );
				CRect rcMask(0, 0, rect.Width(), rect.Height());
				
				//back ground white: mask
				if(!hot)
					maskMemDC.FillSolidRect(&rcMask, RGB(0xFF,0xFF,0xFF));
				CRect rcClipBorder(rcMask);
				int cx = ::GetSystemMetrics(SM_CXBORDER) + 1;
				int cy = ::GetSystemMetrics(SM_CYBORDER) + 1;
				//don't need arrow with dark button border
				rcClipBorder.DeflateRect(cx, cy, cx, cy);
				maskMemDC.IntersectClipRect(&rcClipBorder);
				//draw arrow only
				maskMemDC.DrawFrameControl(&rcMask, DFC_SCROLL, DFCS_SCROLLDOWN);
				if(hot)
				{
					maskMemDC.BitBlt(0, 0, rect.Width(), rect.Height(), NULL, 0, 0, DSTINVERT);
					dc->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &maskMemDC, 0, 0, SRCPAINT);
				}
				else
				{
					dc->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &maskMemDC, 0, 0, SRCINVERT);
					dc->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &maskMemDC, 0, 0, SRCAND);
					dc->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &maskMemDC, 0, 0, SRCINVERT);
				}
			}
		}
		*pResult = CDRF_SKIPDEFAULT;
		break;
	default:
		*pResult = CDRF_SKIPDEFAULT;
	}
}

//////////////////////////////////////////////////////////////////////////
CSize	CUIToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize size = __super::CalcFixedLayout(bStretch,bHorz);

	if( !(this->IsFloating() || this->IsHorzDocked()) )
		size += CSize(10, GRIPPER_HEIGHT+5);
	else if( mMaxStringWidth != 0 )
		size.cx += mMaxStringWidth;
	if( mColumn != 0xFFu && mCurrentColumn < mColumn )
	{
		DWORD buttonSize = this->GetToolBarCtrl().GetButtonSize();
		WORD buttonWidth = LOWORD(buttonSize);
		TBMETRICS metric;
		::ZeroMemory(&metric, sizeof(metric));
		metric.cbSize = sizeof(metric);
		metric.dwMask = TBMF_BUTTONSPACING;
		this->GetToolBarCtrl().GetMetrics(&metric);
		size.cx += (buttonWidth + metric.cxButtonSpacing)*(mColumn - mCurrentColumn);
	}
	else
	{
		bool hasDropDown = false;
		for(int i = 0; i < this->GetToolBarCtrl().GetButtonCount(); ++i)
		{
			UINT ID, style;
			int image;
			this->GetButtonInfo(0, ID, style, image);
			if( (style&BTNS_WHOLEDROPDOWN) )
			{
				hasDropDown = true;
				break;
			}
		}

		if( hasDropDown )
		{
			CSize controlSize;
			this->GetToolBarCtrl().GetMaxSize(&controlSize);
			LONG cx = controlSize.cx + 2*2;
			if( size.cx < cx )
				size.cx = cx;
		}
	}
	return size;
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	__super::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
	for(size_t i = 0; i < mConfigTools.size(); ++i)
		mConfigTools[i]->updateData();
}

//////////////////////////////////////////////////////////////////////////
void	CUIToolBar::NcPaintGripper(CDC* pDC, CRect rcClient)
{
	// compute the caption rectangle
	BOOL bHorz = this->IsHorzDocked();
	BOOL bVert = this->IsVertDocked();
	CRect rcGrip = rcClient;

	if(bVert)
	{
		rcGrip.bottom = rcGrip.top;
		rcGrip.top -= GRIPPER_HEIGHT + 1;
	}
	rcGrip.InflateRect(bHorz ? 1 : 0, bHorz ? 0 : 1);

	// draw the caption background
	//CBrush br;
	COLORREF clrCptn = THEMECOLOR(TC_FRAME);

	pDC->FillSolidRect(&rcGrip, clrCptn); // solid color

	// draw the caption text - first select a font
	COLORREF clrCptnText = THEMECOLOR(TC_TEXT);
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor(clrCptnText);

	CPoint ptOrg = bHorz ? CPoint(rcGrip.left - 1, rcGrip.bottom - 3) : CPoint(rcGrip.left + 3, rcGrip.top - 1);

	CFont* oldFont = pDC->SelectObject( this->GetFont() );
	pDC->ExtTextOut(ptOrg.x, ptOrg.y, ETO_CLIPPED, rcGrip, mTitle.c_str(), (UINT)mTitle.size(), NULL);

	pDC->SetBkMode(nOldBkMode);
	pDC->SetTextColor(clrOldText);
	pDC->SelectObject(oldFont);
}