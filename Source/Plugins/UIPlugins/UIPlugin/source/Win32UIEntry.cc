// Win32DialogMFC.cc : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "Win32UIEntry.h"
#include <afxpriv.h>
#if BLADE_USE_MFC_FEATURE_PACK
#include <afxvisualmanagervs2008.h>
#include <afxvisualmanageroffice2007.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//		If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT && 0
class CHook
{
public:
	CHook(const TCHAR * module, const char * procname, const void * pNewProc)
	{
		HINSTANCE hMod = GetModuleHandle(module);
		m_pProc = (BYTE*)GetProcAddress(hMod, procname);

		if (m_pProc)
		{
			for (int i = 0; i < 5; i++)
			{
				m_Storage[i] = m_pProc[i];
			}

			DWORD dwOldProtect;
			VirtualProtect(m_pProc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			m_pProc[0] = 0xE9;
			*(unsigned*)(m_pProc + 1) = (unsigned)pNewProc - (unsigned)(m_pProc + 5);
		}
	}

	~CHook()
	{
		if (m_pProc && (m_pProc[0] == 0xE9))
		{
			for (int i = 0; i < 5; i++)
			{
				m_pProc[i] = m_Storage[i];
			}
		}
	}

private:
	BYTE* m_pProc;
	BYTE  m_Storage[5];
};

static COLORREF WINAPI GetCustomSysColor(int nIndex)
{
	return THEMECOLOR(TC_WINBK);
}

static HBRUSH WINAPI GetCustomSysColorBrush(int nIndex)
{
	return THEMEBRUSH(TC_WINBK);
}
static CHook theHook(TEXT("user32.dll"), "GetSysColor", GetCustomSysColor);
static CHook theHook2(TEXT("user32.dll"), "GetSysColorBrush", GetCustomSysColorBrush);
#endif

// CWin32UIEntryApp

// The one and only CWin32UIEntryApp object
CWin32UIEntryApp theApp;

BEGIN_MESSAGE_MAP(CWin32UIEntryApp, CWinApp)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// CWin32UIEntryApp construction
CWin32UIEntryApp::CWin32UIEntryApp()
:mIdleCount(0)
,mIdle(true)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


/************************************************************************/
/* IMessageListener interface                                                                     */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
bool CWin32UIEntryApp::preProcessMessage(void* msg)
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

	_AFX_THREAD_STATE *pState = AfxGetThreadState();
	MSG* pMSG = (MSG*)msg;
	pState->m_msgCur = *pMSG;
	bool ret =  (pMSG->message != WM_KICKIDLE) && (AfxPreTranslateMessage(pMSG) == FALSE);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CWin32UIEntryApp::postProcessMessage(void* msg)
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	MSG* pMSG = (MSG*)msg;
	if( this->IsIdleMessage(pMSG) )
	{
		mIdleCount = 0;
		mIdle = true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CWin32UIEntryApp::isIdle()
{
	return mIdle;
}

//////////////////////////////////////////////////////////////////////////
bool CWin32UIEntryApp::onIdleUpdate(bool& bContinueUpdate)
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
	if( !this->OnIdle( mIdleCount++ ) )
		bContinueUpdate = mIdle = false;
	else
		bContinueUpdate = true;
	return true;
}


/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/
#define BLADE_COMMCTRL6 1
#if BLADE_COMMCTRL6
//note: use ComCtl32.dll 6.0 to support 32 bit(alpha/transparency) DIB image list
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//////////////////////////////////////////////////////////////////////////
// CWin32UIEntryApp initialization
BOOL CWin32UIEntryApp::InitInstance()
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

	//init common ctrl needed for 6.0 version
#if BLADE_COMMCTRL6
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
#endif
	//::SetThemeAppProperties(0);

	BOOL ret = ::AfxInitRichEdit();
	ret &= ::AfxInitRichEdit2();

	if( ret )
		ret = CWinApp::InitInstance();
	else
		return FALSE;

#if BLADE_USE_MFC_FEATURE_PACK
	this->InitContextMenuManager();
	this->InitShellManager();
	this->InitKeyboardManager();
	this->InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	this->GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);
	SetRegistryKey(TEXT("Blade"));

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
#endif

#if ENABLE_THEME
	Theme lightTheme =
	{
		RGB(0xd0,0xe0,0xf0),//TC_FRAME
		RGB(0xf9,0x69,0x29),//TC_ACTIVEFRAME
		RGB(0xec,0xec,0xec),//TC_WINBK
		RGB(0x7f,0x7f,0x7f),//TC_EMPTYBK
		RGB(0xf8,0xf8,0xff),//TC_CTLBK
		RGB(0xd0,0xe0,0xf0),//TC_HILIGHTBK
		RGB(0x00,0x00,0x00),//TC_TEXT
		RGB(0x99,0x99,0x99),//TC_GRAYTEXT
		RGB(0x7f,0x7f,0x7f),//TC_LINE
		RGB(0x55,0x55,0x55),//TC_MENULINE
		RGB(0xff,0xff,0xff),//TC_MENUBK
		RGB(0x33,0x99,0xff),//TC_HILIGHTMENUBK
		RGB(0x7f,0x7f,0x7f),//TC_GRAYMENUTEXT
		RGB(0xff,0xff,0xff),//TC_SPLITTER
		RGB(0xff,0xff,0xff),//TC_TEXTBK
		RGB(0x00,0x00,0x00),//TC_TEXTBKDARK
		RGB(0xec,0xec,0xec),//TC_BTN,
		RGB(0xff,0xff,0xff),//TC_HILIGHTBTN,
		RGB(0xec,0xec,0xec),//TC_PUSHEDBTN,

		TEXT("light")
	};
	assert(Blade::countOf(lightTheme.mColors) == TC_COUNT);
	ThemeManager::getSingleton().setTheme(lightTheme);
#endif

	return ret;
}

int		CWin32UIEntryApp::ExitInstance()
{
	AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

	return CWinApp::ExitInstance();
}