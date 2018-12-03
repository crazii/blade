/********************************************************************
	created:	2016/06/09
	filename: 	MainWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <stdafx.h>
#include <afxpriv.h>
#include <CdErr.h>
#include <interface/IPlatformManager.h>
#include <interface/public/window/IWindowService.h>

#include <ui/MainWindow.h>
#include <ui/IconManager.h>
#include <ui/MenuManager.h>
#include <ui/private/UISplashWindow.h>
#include <ui/private/UIMenu.h>
#include <ui/private/UIToolTip.h>
#include <ui/private/UIFileNewDialog.h>

#include <ConfigDialog/ConfigDialog.h>
#include <ui/UIToolBox.h>
#include <ui/SplashOutput.h>
#include <ui/LoadingProgressWindow.h>
#include <ui/LayoutManager.h>

#include <Win32UIEntry.h>
extern CWin32UIEntryApp theApp;

namespace Blade
{
	const TString MainWindow::MAIN_WINDOW_TYPE = BTString("MFC_MainWindow");

	/////////////////////////////////////////////////////////////////////////////
	BEGIN_MESSAGE_MAP(MainWindow, CUIFrameWnd)
		ON_WM_DESTROY()
		ON_WM_CLOSE()
		ON_WM_CREATE()
		ON_WM_NCPAINT()
		ON_WM_ACTIVATE()
	END_MESSAGE_MAP()

	/************************************************************************/
	/* IMainWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	MainWindow::MainWindow()
	{
		//no auto menu enable
		//menus are enable/disable at runtime
		m_bAutoMenuEnable = FALSE;


		mTitle = BTString("MainWindow");
		mLoadingProgress = NULL;
		mFrameworkData = NULL;
		mUserData = NULL;

		mToolboxes.reserve(16);
	}

	//////////////////////////////////////////////////////////////////////////
	MainWindow::~MainWindow()
	{
	}

	/************************************************************************/
	/* IUIWindow interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			MainWindow::initWindow(uintptr_t parentWindowImpl, IconIndex iconIndex, const TString& caption/* = TString::EMPTY*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		mTitle = caption;
		mIcon = iconIndex;

		LPCTSTR className = ::AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), THEMEBRUSH(TC_WINBK), NULL);

		CWnd* parent = NULL;
		if (parentWindowImpl != NULL)
		{
			assert(::IsWindow((HWND)parentWindowImpl));
			parent = CWnd::FromHandle(HWND(parentWindowImpl));
		}

		if (!this->Create(className, mTitle.c_str(), WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, rectDefault, parent, NULL/*, WS_EX_APPWINDOW|WS_EX_OVERLAPPEDWINDOW*/))
			BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("failed to create main child window."));

		CWinThread *pThread = ::AfxGetThread();
		ENSURE_VALID(pThread);
		pThread->m_pMainWnd = this;

		this->SetMenu(NULL);
		CUIToolTip::getSingleton().Create(this, TTS_ALWAYSTIP);
		CUIToolTip::getSingleton().SetDelayTime(TTDT_AUTOPOP, 6000);
		CUIToolTip::getSingleton().Activate(TRUE);

		HWND hWnd = this->GetSafeHwnd();
		mWindow = IWindowService::getSingleton().attachWindow((uintptr_t)hWnd);
		IWindowService::getSingleton().setAppWindow(mWindow);
		IWindowMessagePump::getSingleton().addMessageListener(&theApp);

		HICON hIcon = NULL;
		if (iconIndex != INVALID_ICON_INDEX)
			hIcon = IconManager::getSingleton().getIconList(IS_32)->ExtractIcon(iconIndex);

		this->SetIcon(hIcon, TRUE);

		//setup menu
		CMenu* menu = MenuManager::getSingleton().getAppMenu();
#if !BLADE_USE_MFC_FEATURE_PACK
		this->SetMenu(menu);
		this->DrawMenuBar();
#else
		mMenuBar.Create(this);
		mMenuBar.CreateFromMenu(menu->m_hMenu, TRUE);
		this->DockPane(&mMenuBar);
#endif

		//setup layouts
		mWindowManager = BLADE_NEW LayoutManager();
		mWindowManager->initialize(this, WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), IconManager::getSingleton().getIconList(IS_16), IconManager::getSingleton().getIconList(IS_16, true));

		mLoadingProgress = BLADE_NEW LoadingProgress();

		this->RecalcLayout();
		this->SetForegroundWindow();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IUIToolBox*		MainWindow::createToolBox(const TString& name, ICONSIZE is)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		size_t totalWidth = 0;
		for (size_t i = 0; i < mToolboxes.size(); ++i)
		{
			if (mToolboxes[i]->getToolbar()->m_pDockBar == this->GetControlBar(AFX_IDW_DOCKBAR_TOP))
				totalWidth += mToolboxes[i]->getWidth();
		}

		UIToolBox* tb = BLADE_NEW UIToolBox(name, is, AFX_IDW_TOOLBAR, this, totalWidth, true);
		mToolboxes.push_back(tb);
		return tb;

	}

	//////////////////////////////////////////////////////////////////////////
	IUIToolBox*		MainWindow::getToolBox(const TString& name) const
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		for (size_t i = 0; i < mToolboxes.size(); ++i)
		{
			if (mToolboxes[i]->getName() == name)
				return mToolboxes[i];
		}
		return NULL;

	}

	/************************************************************************/
	/* IUIMainWindow interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			MainWindow::showCreateFileUI(TString& outExt, TString& outDir, TString& outName, const NEW_FILE_DESC* desc, size_t count)
	{
		if (desc == NULL || count == 0)
		{
			//assert(false);
			return false;
		}
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		//TODO:
		CUIFileNewDialog dialog(this);

		for (size_t i = 0; i < count; ++i)
			dialog.AddFileTypes(desc+i);

		if (dialog.DoModal() != IDOK)
			return false;

		const NEW_FILE_DESC* info = dialog.GetFileType();
		if (info == NULL)
		{
			assert(false);
			return false;
		}
		TString filename = dialog.GetFileName();
		TStringHelper::appendFileExtension(filename, info->mDesc);

		const TString& dir = dialog.GetFileDirectory();
		const TString& cwd = IPlatformManager::getSingleton().getCurrentWorkingPath();
		outDir = TStringHelper::getRelativePath(cwd, dir);
		outName = filename;
		outExt = info->mExtension;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MainWindow::showOpenFileUI(TString& outExt, TString& outDir, TString& outName, const TStringParam& extensions, const TStringParam& descs)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		assert(descs.size() == extensions.size() || descs.size() == 0);
		TString path;
		INT_PTR result;

		{
			TString filter;
			TStringConcat concat(BTLang(BLANG_ALL_SUPPORTED_FILE) + BTString(" |"));

			for (size_t i = 0; i < extensions.size(); ++i)
				concat += TEXT("*.") + extensions[i] + TEXT(";");

			if (descs.size() != 0)
			{
				concat += TEXT("|");
				for (size_t i = 0; i < descs.size(); ++i)
				{
					concat += descs[i] +
						TEXT("( *.") + extensions[i] + TEXT(")|") +
						TEXT("*.") + extensions[i] + TEXT("|");
				}
			}

			concat += BTLang(BLANG_ALL_FILE) + TEXT(" (*.*)|*.*||");
			filter = concat;

			// TODO: Add your command handler code here
			CFileDialog fileOpen(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_DONTADDTORECENT | OFN_EXPLORER, filter.c_str(), this);
			fileOpen.GetOFN().hwndOwner = this->GetSafeHwnd();
			fileOpen.GetOFN().nFilterIndex = 1;
			fileOpen.GetOFN().lpstrInitialDir = TEXT("..\\Data");

			result = fileOpen.DoModal();
			path = TString(fileOpen.GetOFN().lpstrFile);
		}

		if (result == IDOK)
		{
			TStringHelper::getFilePathInfo(path, outDir, outName);
			TString name = outName;
			TString base, ext;
			TStringHelper::getFileInfo(name, base, ext);

			for (size_t i = 0; i < extensions.size(); ++i)
			{
				if (extensions[i].compare_no_case(ext) == 0)
				{
					//use original case
					outExt = extensions[i];
					return true;
				}
			}
			::AfxMessageBox(BTLang(BLANG_FILE_SELECTERROR).c_str());
		}
		else
		{
			DWORD val = ::CommDlgExtendedError();
			if (val == FNERR_INVALIDFILENAME)
				::AfxMessageBox(BTLang(BLANG_FILE_SELECTERROR).c_str());
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MainWindow::showSaveFileUI(TString& outDir, TString& outName, const TString& extension/* = TString::EMPTY*/)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		TString path;
		INT_PTR result;
		TString filter;

		TStringConcat concat(TString::EMPTY);
		if (extension != TString::EMPTY)
			concat += extension + TEXT(" File|") + TEXT("*.") + extension + TEXT("|");
		concat += BTLang(BLANG_ALL_FILE) + TEXT(" (*.*)|*.*||");
		filter = concat;

		CFileDialog fileOpen(TRUE, NULL, NULL, OFN_NOCHANGEDIR | OFN_DONTADDTORECENT | OFN_EXPLORER | OFN_OVERWRITEPROMPT, filter.c_str(), this);
		fileOpen.GetOFN().hwndOwner = this->GetSafeHwnd();
		fileOpen.GetOFN().nFilterIndex = 1;
		fileOpen.GetOFN().lpstrInitialDir = IPlatformManager::getSingleton().getCurrentWorkingPath().c_str();
		fileOpen.GetOFN().lpstrInitialDir = TEXT("..\\Data");

		result = fileOpen.DoModal();
		path = TString(fileOpen.GetOFN().lpstrFile);
		if (result == IDOK && !path.empty())
		{
			//why does file open dialog leaves blank background after it's closed?
			//this->RedrawWindow(NULL,NULL,RDW_ALLCHILDREN|RDW_ERASENOW|RDW_UPDATENOW|RDW_INVALIDATE);
			TStringHelper::appendFileExtension(path, extension);
			TStringHelper::getFilePathInfo(path, outDir, outName);
			return true;
		}
		else
		{
			DWORD val = ::CommDlgExtendedError();
			assert(val < FNERR_FILENAMECODES || val > FRERR_BUFFERLENGTHZERO);
			BLADE_UNREFERENCED(val);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			MainWindow::showMultipleGroupConfig(const TString& diallogName, IConfig** configs, size_t count,
		const IConfigDialog::INFO& info/* =  IConfigDialog::INFO() */)
	{
		if (configs == NULL)
			return false;
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		if (info.mModal)
		{
			CConfigDialog dialog(this);
			dialog.setCaption(diallogName);

			for (size_t i = 0; i < count; ++i)
				dialog.addConfig(configs[i]);

			bool ret = dialog.show(info, true, true, this);

			if (ret)
				info.mNoneModalDelegate.call(info.mUserData);

			return ret;
		}
		else
		{
			//TODO: this is an unsafe cast, try to figure out a better way
			//using dynamic cast is acceptable here
			CConfigDialog* dialog = dynamic_cast<CConfigDialog*>(this->FindWindow(NULL, diallogName.c_str()));

			if (dialog != NULL)
			{
				dialog->ShowWindow(SW_SHOW);
				dialog->SetActiveWindow();
				return true;
			}
			else
			{
				dialog = BLADE_NEW CConfigDialog(this);
				dialog->setCaption(diallogName);

				for (size_t i = 0; i < count; ++i)
					dialog->addConfig(configs[i]);
				return dialog->show(info, true, true, this);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IProgressCallback*	MainWindow::showProgress(const TString& title)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		mLoadingProgress->reset(this, title);
		return mLoadingProgress;
	}

	/************************************************************************/
	/* MFC overrides                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			MainWindow::OnClose()
	{
		////note: OnClose will automatically close child windows
		////but will destroy them directly and NOT send WM_CLOSE, thus framework cannot check unsaved changes
		////so we close them manually so that framework will notify close event and detect unsaved changes
		//while (mWindowManager->getActiveWindow() != NULL)
		//{
		//	IUIWindow* child = mWindowManager->getActiveWindow();
		//	IWindow* window = child->getWindow();
		//	if (window == NULL)
		//		continue;
		//	window->closeWindow();
		//	//test child window whether closed
		//	//note the order: child may not exist!
		//	if (mWindowManager->getActiveWindow() == child && child->getWindow() != NULL)
		//		return;
		//}

		__super::OnClose();
	}

	//////////////////////////////////////////////////////////////////////////
	int		MainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int ret = __super::OnCreate(lpCreateStruct);
		this->EnableDocking(CBRS_ALIGN_ANY);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void			MainWindow::OnDestroy()
	{
		for (ToolBoxList::iterator i = mToolboxes.begin(); i != mToolboxes.end(); ++i)
		{
			UIToolBox* tb = *i;
			BLADE_DELETE tb;
		}

		//close all windows
		mWindowManager->shutdown();
		BLADE_DELETE mWindowManager;
		mWindowManager = NULL;

		BLADE_DELETE mLoadingProgress;
		mLoadingProgress = NULL;

		__super::OnDestroy();
	}

	//////////////////////////////////////////////////////////////////////////
	void	MainWindow::OnNcPaint()
	{
		MFCFrameWindow::OnNcPaint();

		//CFrameWnd* wnd = (CFrameWnd*)this;
		//MENUBARINFO info;
		//info.cbSize = sizeof(info);
		//VERIFY(wnd->GetMenuBarInfo(OBJID_MENU, 0, &info));
		//CRect rc(info.rcBar);
		//CRect rcWin;
		//wnd->GetWindowRect(&rcWin);
		//rc.OffsetRect(-rcWin.left, -rcWin.top);
		//rc.OffsetRect(0, rc.Height());
		//rc.bottom = rc.top + 2;

		//CWindowDC dc(this);
		//dc.FillSolidRect(&rc, THEMECOLOR(TC_WINBK));
	}

	//////////////////////////////////////////////////////////////////////////
	void	MainWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
	{
		__super::OnActivate(nState, pWndOther, bMinimized);
		static HWND focusWnd = NULL;
		if (nState == WA_INACTIVE)
			focusWnd = ::GetFocus();
		else if ((nState == WA_ACTIVE || nState == WA_CLICKACTIVE))
		{
			if (::IsWindow(focusWnd))
				::SetFocus(focusWnd);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	LRESULT	MainWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_GETTEXT)
		{
			WPARAM cchTextMax = wParam;
			LPTSTR lpszText = (LPTSTR)lParam;
			UINT minchars = min((UINT)mTitle.size() + 1, (UINT)cchTextMax);;
			if (lpszText != NULL)
			{
				Char_Traits<TCHAR>::copy(lpszText, mTitle.c_str(), minchars);
				//if string is truncated,assure NULL char appended
				lpszText[minchars - 1] = TEXT('\0');
				return (LRESULT)minchars;
			}
		}
		else if (message == WM_GETTEXTLENGTH)
		{
			return (LRESULT)mTitle.size();
		}

		return __super::DefWindowProc(message, wParam, lParam);
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL MainWindow::PreTranslateMessage(MSG* pMsg)
	{
		CUIToolTip::getSingleton().RelayEvent(pMsg);
		return __super::PreTranslateMessage(pMsg);
	}

	//////////////////////////////////////////////////////////////////////////
	void	MainWindow::PostNcDestroy()
	{
		HWINDOW hWindow = IWindowService::getSingleton().getAppWindow();
		IWindowService::getSingleton().destroyWindow(hWindow->getUniqueName());
		IWindowMessagePump::getSingleton().removeMessageListener(&theApp);

		BLADE_DELETE this;
	}

	
}//namespace Blade