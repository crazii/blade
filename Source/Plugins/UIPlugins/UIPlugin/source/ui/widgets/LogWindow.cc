/********************************************************************
	created:	2011/05/05
	filename: 	LogWindow.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "LogWindow.h"
#include <ui/IconManager.h>
#include <interface/ILogManager.h>

#define TOP_EDGE (2)
#define LEFT_EDGE (0)
#define RIGHT_EDGE (0)
#define BOTTOM_EDGE (6)
#define TAB_COLOR (COLORREF(0xEAEDEF))
#define TAB_ID (1032)

namespace Blade
{

	BEGIN_MESSAGE_MAP(LogWindow, DockWindowBase)
		//{{AFX_MSG_MAP(LogWindow)
		ON_WM_CREATE()
		ON_WM_SIZE()
		ON_NOTIFY(TCN_SELCHANGE, TAB_ID, OnTabSelChange)
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	const Blade::TString LogWindow::WIDGET_TYPE = BXLang(BLANG_LOG);

	//////////////////////////////////////////////////////////////////////////
	LogWindow::LogWindow()
		:DockWindowBase(WIDGET_TYPE)
	{
		hSelf.bind(this);
		ILogManager::getSingleton().redirectLogs(hSelf, true);
		IFramework::getSingleton().addUpdater(this);
	}

	//////////////////////////////////////////////////////////////////////////
	LogWindow::~LogWindow()
	{
		ILogManager::getSingleton().redirectLogs(HLOGREDIRECTOR::EMPTY);
		hSelf.unbind();
		mLogTypeTab.SetImageList(NULL);
		mBuffer.clear();
		IFramework::getSingleton().removeUpdater(this);
	}

	/************************************************************************/
	/* IUILogView interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool LogWindow::enableLogRedirection(bool enable/* = true*/)
	{
		return ILogManager::getSingleton().redirectLogs(enable ? hSelf : HLOGREDIRECTOR::EMPTY);
	}

	/************************************************************************/
	/* ILogRedirector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void LogWindow::outputLog(ILog::ELogLevel level, const tchar* content)
	{

		//output to system debugger
		::OutputDebugString(content);

		//buffer log data , to avoid multi-thread SendMessage dead-lock
		//use LogWindow::flushLogBuffer in main thread instead

		//no lock here, because log object will lock before call this::outputLog
		SLogItem item;
		item.level = level;

		//add extra lock in case that background thread is outputting log while main thread is flushLogBuffer
		ScopedLock lock(mBufferLock);

		mBuffer.push_back(item);
		(*mBuffer.rbegin()).logText = content;
	}

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	int		LogWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int result = __super::OnCreate(lpCreateStruct);

		this->initialize();

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	void	LogWindow::OnSize(UINT nType, int cx, int cy)
	{
		__super::OnSize(nType, cx, cy);

		if (mLogTypeTab.GetSafeHwnd() == NULL)
			return;

		CRect clientRect;
		this->GetClientRect(&clientRect);

		RECT rect;
		rect.left = LEFT_EDGE;
		rect.right = cx - RIGHT_EDGE;
		rect.top = cy - clientRect.Height();
		rect.bottom = cy;
		mLogTypeTab.MoveWindow(&rect);
		//move it to the bottom
		::SetWindowPos(mLogTypeTab.m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		rect.top += TOP_EDGE;
		rect.left += LEFT_EDGE;
		rect.right -= RIGHT_EDGE;
		rect.bottom -= mLogTypeTab.GetMaxTabHeight() + BOTTOM_EDGE;

		for (size_t i = ILog::LL_BEGIN; i < ILog::LL_COUNT; ++i)
		{
			mLogContent[i].MoveWindow(&rect);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	void	LogWindow::OnTabSelChange(NMHDR*pNotifyStruct, LRESULT*result)
	{
		if (pNotifyStruct->code == TCN_SELCHANGE)
		{
			CTabCtrl* tab = static_cast<CTabCtrl*>(this->GetDlgItem((int)pNotifyStruct->idFrom));
			int index = tab->GetCurSel();

			assert(index >= 0 && index < ILog::LL_COUNT);

			mLogContent[index].ShowWindow(SW_SHOW);
			this->setActiveChild(&mLogContent[index]);

			for (int i = ILog::LL_BEGIN; i < ILog::LL_COUNT; ++i)
			{
				if (i != index)
					mLogContent[i].ShowWindow(SW_HIDE);
			}
			mLogTypeTab.SetFocus();
		}

		*result = 0;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		LogWindow::flushLogBuffer()
	{
		ScopedLock lock(mBufferLock);

		bool hasOutput[ILog::LL_COUNT] = { false, false, false, false, false };
		bool scrollOutput[ILog::LL_COUNT] = { false, false, false, false, false };
		index_t oldPos[ILog::LL_COUNT] = { 0 };

		for (LogBuffer::iterator i = mBuffer.begin(); i != mBuffer.end(); ++i)
		{
			const SLogItem& item = *i;

			size_t size = item.logText.size();
			const tchar* content = item.logText.c_str();
			ILog::ELogLevel level = item.level;
			int firstVisibleLine;

			assert(level > 0 && level < ILog::LL_COUNT);

			{
				long start, end, lastSel;
				mLogContent[level].GetSel(start, end);
				if (start != end)
					lastSel = 0;
				else
					lastSel = start;

				//de-select text && set cursor
				//don't scroll if user manual selecting text
				scrollOutput[level] = (lastSel == (long)mTextCursor[level]);
				oldPos[level] = mTextCursor[level];

				//CRichEdit won't auto scroll if it doesn't have focus
				//so we don't need disable redraw 
				if (mLogContent[level].GetSafeHwnd() == ::GetFocus())
				{
					mLogContent[level].HideCaret();
					mLogContent[level].SetRedraw(FALSE);
				}

				firstVisibleLine = mLogContent[level].GetFirstVisibleLine();

				mLogContent[level].SetSel((long)mTextCursor[level], (long)mTextCursor[level]);
				mLogContent[level].ReplaceSel(content, FALSE);
				mTextCursor[level] += size;

				if (!scrollOutput[level])
				{
					mLogContent[level].SetSel(start, end);

					int newVisibleLine = mLogContent[level].GetFirstVisibleLine();
					if (newVisibleLine != firstVisibleLine)
						mLogContent[level].LineScroll(firstVisibleLine - newVisibleLine);
				}
				else
					mLogContent[level].SetSel((long)mTextCursor[level], (long)mTextCursor[level]);

				hasOutput[level] = true;
			}

			{
				long start, end, lastSel;
				mLogContent[0].GetSel(start, end);
				if (start != end)
					lastSel = 0;
				else
					lastSel = start;

				//de-select text && set cursor
				//don't scroll if user manual selecting text
				scrollOutput[0] = (lastSel == (long)mTextCursor[0]);
				oldPos[0] = mTextCursor[0];

				if (mLogContent[0].GetSafeHwnd() == ::GetFocus())
				{
					mLogContent[0].HideCaret();
					mLogContent[0].SetRedraw(FALSE);
				}

				firstVisibleLine = mLogContent[0].GetFirstVisibleLine();

				mLogContent[0].SetSel((long)mTextCursor[0], (long)mTextCursor[0]);
				mLogContent[0].ReplaceSel(content, FALSE);
				mLogContent[0].SetSel((long)mTextCursor[0], (long)(mTextCursor[0] + size));
				mTextCursor[0] += size;

				CHARFORMAT cf;
				::ZeroMemory(&cf, sizeof(cf));
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_COLOR;
				cf.crTextColor = mTextColor[level];
				mLogContent[0].SetSelectionCharFormat(cf);

				if (!scrollOutput[0])
				{
					mLogContent[0].SetSel(start, end);
					int newVisibleLine = mLogContent[0].GetFirstVisibleLine();
					if (newVisibleLine != firstVisibleLine)
						mLogContent[0].LineScroll(firstVisibleLine - newVisibleLine);
				}
				else
					mLogContent[0].SetSel((long)mTextCursor[0], (long)mTextCursor[0]);

				hasOutput[0] = true;
			}
		}

		for (index_t i = 0; i < ILog::LL_COUNT; ++i)
		{
			if (scrollOutput[i])
				mLogContent[i].SendMessage(WM_VSCROLL, MAKELONG(SB_BOTTOM, 0), NULL);

			if (hasOutput[i] && mLogContent[i].GetSafeHwnd() == ::GetFocus())
			{
				mLogContent[i].SetRedraw(TRUE);

				CPoint pos = mLogContent[i].GetCharPos((long)oldPos[i]);
				CRect rect;
				mLogContent[i].GetRect(&rect);
				if (rect.PtInRect(pos))
				{
					//rect.left = newPos.x;
					//rect.top = newPos.y;
					//mLogContent[i].InvalidateRect(&rect);
					mLogContent[i].Invalidate();
					mLogContent[i].UpdateWindow();
				}

				//TODO: caret is blinking to frequently
				mLogContent[i].ShowCaret();
			}
		}
		mBuffer.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void		LogWindow::initialize()
	{

		//tab control
		DWORD style = TCS_BOTTOM | TCS_RIGHTJUSTIFY |/*TCS_FLATBUTTONS | TCS_BUTTONS */ TCS_TABS | WS_CHILD | WS_VISIBLE;
		RECT rect;
		this->GetClientRect(&rect);
		mLogTypeTab.Create(style, rect, this, TAB_ID);

		TString names[ILog::LL_COUNT];
		names[0] = BTLang(BLANG_ALL);
		names[1] = BTLang(BLANG_ERROR);
		names[2] = BTLang(BLANG_WARNING);
		names[3] = BTLang(BLANG_INFO);
		names[4] = BTLang(BLANG_DEBUG);

		TString icons[ILog::LL_COUNT];
		icons[0] = BTString("detail");
		icons[1] = BTString("error");
		icons[2] = BTString("warning");
		icons[3] = BTString("Info");
		icons[4] = BTString("bug");

		for (size_t i = ILog::LL_BEGIN; i < ILog::LL_COUNT; ++i)
		{
			IconIndex index = IIconManager::getSingleton().loadSingleIconImage(icons[i] + BTString(".png"));
			mLogTypeTab.InsertItem(mLogTypeTab.GetItemCount(), names[i].c_str(), index);
		}

		//mLogTypeTab.SetBkgndColor( TAB_COLOR );
		mLogTypeTab.EnableCustomLook(TRUE, ETC_SELECTION | ETC_FLAT| ETC_COLOR);
		mLogTypeTab.EnableDraw();
		//set space for icons
		//mLogTypeTab.SetMinTabWidth( mLogTypeTab.GetMaxTabWidth() + 32 );
		mLogTypeTab.SetImageList(IconManager::getSingleton().getIconList(IS_16));

		//text box
		mTextColor[0] = RGB(255, 255, 255);		//runtime
		mTextColor[1] = RGB(255, 0, 0);			//red for error
		mTextColor[2] = RGB(255, 255, 0);		//yellow for warning
		mTextColor[3] = RGB(255, 255, 255);		//white for info
		mTextColor[4] = RGB(0, 255, 0);			//green for debug

		style = ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL;
		for (size_t i = ILog::LL_BEGIN; i < ILog::LL_COUNT; ++i)
		{
			mLogContent[i].Create(style, rect, this, TAB_ID + 1 + (UINT)i);
			mTextCursor[i] = 0;

			//mLogContent[i].ModifyStyleEx(0,WS_EX_CLIENTEDGE,SWP_NOACTIVATE);
			mLogContent[i].SetBackgroundColor(FALSE, THEMECOLOR(TC_TEXTBKDARK));
			mLogContent[i].SetTextMode(TM_RICHTEXT | TM_SINGLELEVELUNDO | TM_MULTICODEPAGE);
			mLogContent[i].SetOptions(ECOOP_OR, ECO_AUTOWORDSELECTION | ECO_SELECTIONBAR);
			//mLogContent[i].SetOptions(ECOOP_XOR, ECO_AUTOVSCROLL|ECO_AUTOHSCROLL);

			CHARFORMAT2 cf;
			::ZeroMemory(&cf, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_COLOR | CFM_WEIGHT | CFM_SPACING | CFM_FACE | CFM_SIZE;
			cf.crTextColor = mTextColor[i];
			cf.wWeight = FW_NORMAL;
			cf.sSpacing = 0;
			cf.yHeight = 180;
			//memcpy(cf.szFaceName,TEXT("Courier"),sizeof(TEXT("Courier")) );
			::memcpy(cf.szFaceName, TEXT("Consolas"), sizeof(TEXT("Consolas")));
			BOOL ret = mLogContent[i].SetDefaultCharFormat(cf);
			assert(ret);

			PARAFORMAT2   pf;
			::ZeroMemory(&pf, sizeof(pf));
			pf.cbSize = sizeof(PARAFORMAT2);
			pf.dwMask = PFM_LINESPACING | PFM_SPACEAFTER | PFM_SPACEBEFORE;
			pf.dyLineSpacing = 4;
			pf.dySpaceAfter = 3;
			pf.dySpaceBefore = 3;
			pf.bLineSpacingRule = 5;
			ret = mLogContent[i].SetParaFormat(pf);
			assert(ret);
		}

		mLogTypeTab.SetCurSel(0);
		for (size_t i = ILog::LL_BEGIN + 1; i < ILog::LL_COUNT; ++i)
		{
			mLogContent[i].ShowWindow(SW_HIDE);
		}
		this->setActiveChild(&mLogContent[0]);
	}

	
}//namespace Blade