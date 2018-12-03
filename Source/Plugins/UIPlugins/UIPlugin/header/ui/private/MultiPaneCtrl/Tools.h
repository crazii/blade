//==========================================================
// Author: Borodenko Oleg
// 20/07/2009 <oktamail@gmail.com>
//==========================================================
// 
/////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable : 4702)	// unreachable code.
#pragma warning(disable : 4786)	// too long identifiers.
#pragma warning(disable : 4018)	// signed/unsigned mismatch.
#pragma warning(disable : 4284)
#pragma warning(disable : 4062)
#pragma warning(disable : 4365)
#pragma warning(disable : 4640 4302 4311)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
extern "C"
{
#if(WINVER < 0x0500)
	typedef HANDLE HMONITOR;
#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTONEAREST    0x00000002
	// 
	typedef struct tagMONITORINFO
	{	DWORD   cbSize;
	RECT    rcMonitor;
	RECT    rcWork;
	DWORD   dwFlags;
	} MONITORINFO, *LPMONITORINFO;
	// 
	HMONITOR WINAPI MonitorFromWindow(HWND hwnd, DWORD dwFlags);
	HMONITOR WINAPI MonitorFromRect(LPCRECT lprc, DWORD dwFlags);
	// 
	BOOL WINAPI GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi);
	BOOL WINAPI GetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpmi);
	// 
#if BLADE_UNICODE
#define GetMonitorInfo  GetMonitorInfoW
#else
#define GetMonitorInfo  GetMonitorInfoA
#endif
#endif
	// 
#if (_WIN32_WINNT <= 0x0400)
#ifndef ULONG_PTR
#if defined(_WIN64)
	typedef unsigned __int64 ULONG_PTR;
#else
	typedef unsigned long ULONG_PTR;
#endif
#endif
	// 
	typedef struct tagMOUSEINPUT
	{	LONG    dx;
	LONG    dy;
	DWORD   mouseData;
	DWORD   dwFlags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
	} MOUSEINPUT, *PMOUSEINPUT, FAR* LPMOUSEINPUT;
	// 
	typedef struct tagKEYBDINPUT
	{	WORD    wVk;
	WORD    wScan;
	DWORD   dwFlags;
	DWORD   time;
	ULONG_PTR dwExtraInfo;
	} KEYBDINPUT, *PKEYBDINPUT, FAR* LPKEYBDINPUT;
	// 
	typedef struct tagHARDWAREINPUT
	{	DWORD   uMsg;
	WORD    wParamL;
	WORD    wParamH;
	} HARDWAREINPUT, *PHARDWAREINPUT, FAR* LPHARDWAREINPUT;
	// 
#define INPUT_MOUSE     0
#define INPUT_KEYBOARD  1
#define INPUT_HARDWARE  2
	// 
	typedef struct tagINPUT
	{	DWORD   type;
	union
	{	MOUSEINPUT      mi;
	KEYBDINPUT      ki;
	HARDWAREINPUT   hi;
	};
	} INPUT, *PINPUT, FAR* LPINPUT;
	// 
	UINT WINAPI SendInput(UINT cInputs, LPINPUT pInputs, int cbSize);
#endif // (_WIN32_WINNT > 0x0400)
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct Tools
{		// 
	static void ShowWindow(CWnd *wnd, bool show)
	{	ShowWindow(wnd->m_hWnd,show);
	}
	// 
	static void ShowWindow(HWND hWnd, bool show)
	{	(show==true ? ::ShowWindow(hWnd,SW_SHOWNA) : ::ShowWindow(hWnd,SW_HIDE));
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static void GetWindowRectParentClient(CWnd *wnd, RECT *rc/*out*/)	// get window rect in parent's client space.
	{	GetWindowRectParentClient(wnd->m_hWnd,rc);
	}
	// 
	static void GetWindowRectParentClient(HWND wnd, RECT *rc/*out*/)	// get window rect in parent's client space.
	{	::GetWindowRect(wnd,rc);
	::MapWindowPoints(HWND_DESKTOP,::GetParent(wnd),(POINT *)rc,2);
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static bool HasWindowPos(HWND wnd, RECT const *rc)	// return 'true' if window has 'rc' position.
	{	CRect rcNow;
	GetWindowRectParentClient(wnd,&rcNow);
	return (rcNow==*rc)!=0;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static void MoveChangedWindow(CWnd *wnd, RECT const *rc, bool redraw)
	{	MoveChangedWindow(wnd->m_hWnd,rc,redraw);
	}
	// 
	static void MoveChangedWindow(HWND wnd, RECT const *rc, bool redraw)
	{	if(HasWindowPos(wnd,rc)==false)
	::MoveWindow(wnd,rc->left,rc->top,rc->right-rc->left,rc->bottom-rc->top,(redraw==true ? TRUE : FALSE));
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static void MoveWindowDWP(HDWP dwp, CWnd *wnd, RECT const *rc, bool redraw)
	{	MoveWindowDWP(dwp,wnd->m_hWnd,rc,redraw);
	}
	// 
	static void MoveWindowDWP(HDWP dwp, HWND wnd, RECT const *rc, bool redraw)
	{	if(HasWindowPos(wnd,rc)==false)
	::DeferWindowPos(dwp,wnd,0,
	rc->left,rc->top,rc->right-rc->left,rc->bottom-rc->top,
	SWP_NOZORDER | SWP_NOACTIVATE | (redraw==true ? 0 : SWP_NOREDRAW));
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static void MoveShowWindowDWP(HDWP dwp, CWnd *wnd, RECT const *rc, bool redraw)
	{	MoveShowWindowDWP(dwp,wnd->m_hWnd,rc,redraw);
	}
	// 
	static void MoveShowWindowDWP(HDWP dwp, HWND wnd, RECT const *rc, bool redraw)
	{	bool needShow = (::IsWindowVisible(wnd)==0);
	// 
	if(HasWindowPos(wnd,rc)==false || needShow==true)
		::DeferWindowPos(dwp,wnd,0,
		rc->left,rc->top,rc->right-rc->left,rc->bottom-rc->top,
		SWP_NOZORDER | SWP_NOACTIVATE | (needShow==true ? SWP_SHOWWINDOW : 0) |
		(redraw==true ? 0 : SWP_NOREDRAW));
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static BOOL(__stdcall *GetLayerFunctionPtr())(HWND,COLORREF,BYTE,DWORD)	// get address of SetLayeredWindowAttributes function.
	{	HMODULE hModule = ::LoadLibrary(_T("User32.dll"));
	if(hModule==NULL) return NULL;
	BOOL (__stdcall *ptr)(HWND,COLORREF,BYTE,DWORD) = 
		(BOOL(__stdcall *)(HWND,COLORREF,BYTE,DWORD))::GetProcAddress(hModule,"SetLayeredWindowAttributes");
	::FreeLibrary(hModule);
	return ptr;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static void SetLayered(CWnd *wnd, bool layered)
	{	LONG styleEx = ::GetWindowLong(wnd->m_hWnd,GWL_EXSTYLE);
	// 
	if(layered==true)
	{	if((styleEx & 0x00080000/*WS_EX_LAYERED*/)==0)
	::SetWindowLong(wnd->m_hWnd,GWL_EXSTYLE,(styleEx | 0x00080000/*WS_EX_LAYERED*/));
	}
	else
		if((styleEx & 0x00080000/*WS_EX_LAYERED*/)!=0)
		{	::SetWindowLong(wnd->m_hWnd,GWL_EXSTYLE,(styleEx & ~0x00080000/*WS_EX_LAYERED*/));
	::RedrawWindow(wnd->m_hWnd,NULL,NULL,RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
	}
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static bool IsDescendantWindow(HWND wndDst, HWND wndSrc)
	{	for(HWND h=wndSrc; h!=NULL; h=::GetParent(h))
	if(h==wndDst) return true;
	return false;
	}
	// 
	static bool IsPointInWindow(HWND wndDst, CPoint pt)
	{	return IsDescendantWindow(wndDst, ::WindowFromPoint(pt) );
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	// return 0 - wnd1=wnd2.
	// return >0 - wnd1 above wnd2.
	// return <0 - wnd2 above wnd1.
	static int CompareZOrderWindows(HWND wnd1, HWND wnd2)
	{	_ASSERTE(::GetParent(wnd1)==::GetParent(wnd2));
	// 
	if(wnd1==wnd2) return 0;
	// 
	for(HWND h=::GetNextWindow(wnd1,GW_HWNDNEXT); h!=NULL; h=::GetNextWindow(h,GW_HWNDNEXT))
		if(h==wnd2) return 1;
	return -1;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static bool HasWindowRegion(HWND wnd)
	{	bool res = false;
	// 
	HRGN rgn = ::CreateRectRgn(0,0,0,0);
	if(rgn!=NULL)
	{	int regionType = ::GetWindowRgn(wnd,rgn);
	res = (regionType!=ERROR);
	::DeleteObject(rgn);
	}
	return res;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static CRect GetMonitorRect(CWnd *wnd)
	{	HMONITOR hMonitor = ::MonitorFromWindow(wnd->m_hWnd,MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	::GetMonitorInfo(hMonitor,&info);
	return info.rcMonitor;
	}
	// 
	static CRect GetMonitorWorkAreaRect(CWnd *wnd)
	{	HMONITOR hMonitor = ::MonitorFromWindow(wnd->m_hWnd,MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	::GetMonitorInfo(hMonitor,&info);
	return info.rcWork;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static bool CorrectWindowPos(bool smallCaption, CRect *rc/*in,out*/)
	{	CRect rect(*rc);
	rect.top += ::GetSystemMetrics(SM_CXBORDER);
	rect.bottom = rect.top + 
		(smallCaption==false ? ::GetSystemMetrics(SM_CYCAPTION) : ::GetSystemMetrics(SM_CYSMCAPTION));
	// 
	if(::MonitorFromRect(&rect,MONITOR_DEFAULTTONULL)==NULL)
	{	HMONITOR hMonitor = ::MonitorFromRect(&rect,MONITOR_DEFAULTTONEAREST);
	// 
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	::GetMonitorInfo(hMonitor,&info);
	// 
	rect = rc;
	rc->top = max(info.rcWork.top+3,(info.rcWork.top+info.rcWork.bottom-rect.Height())/2);
	rc->bottom = rc->top + rect.Height();
	rc->left = max(info.rcWork.left+3,(info.rcWork.left+info.rcWork.right-rect.Width())/2);
	rc->right = rc->left + rect.Width();
	// 
	return true;
	}
	return false;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static CString RegisterWndClass(TCHAR const *namePrefix)
	{	return RegisterWndClass(namePrefix,CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),NULL,NULL);
	}
	// 
	static CString RegisterWndClass(TCHAR const *namePrefix, UINT classStyle, HCURSOR cursor, HBRUSH background, HICON icon)
	{	_ASSERTE(namePrefix!=NULL);
	// 
	HINSTANCE hInst = AfxGetInstanceHandle();
	// 
	CString className;
	className.Format(_T("%s:%x:%x:%x:%x:%x"),namePrefix,(UINT)hInst,classStyle,(UINT)cursor,(UINT)background,(UINT)icon);
	// 
	WNDCLASS wndcls;
	if(::GetClassInfo(hInst,className,&wndcls)==0)
	{	wndcls.style = classStyle;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hInstance = hInst;
	wndcls.hIcon = icon;
	wndcls.hCursor = cursor;
	wndcls.hbrBackground = background;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = className;
	// 
	if(AfxRegisterClass(&wndcls)==0)
		return AfxRegisterWndClass(classStyle,cursor,background,icon);
	}
	return className;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static void SendMouseMessage(UINT msg, int x, int y, bool absolute)
	{	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.mouseData = 0;
	input.mi.dwFlags = (absolute==true ? MOUSEEVENTF_ABSOLUTE : 0);
	input.mi.time = 0;
	input.mi.dwExtraInfo = 0;
	// 
	switch(msg)
	{	case WM_MOUSEMOVE: input.mi.dwFlags |= MOUSEEVENTF_MOVE; break;
	case WM_LBUTTONDOWN: input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN; break;
	case WM_LBUTTONUP: input.mi.dwFlags |= MOUSEEVENTF_LEFTUP; break;
	case WM_RBUTTONDOWN: input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN; break;
	case WM_RBUTTONUP: input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP; break;
	case WM_MBUTTONDOWN: input.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN; break;
	case WM_MBUTTONUP: input.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP; break;
	}
	::SendInput(1,&input,sizeof(input));
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static bool PtInTriangle(CPoint const &pt, CPoint const &v1, CPoint const &v2, CPoint const &v3)
	{	POINT pts[3];
	pts[0] = v1; pts[1] = v2; pts[2] = v3;
	CRgn rgn;
	rgn.CreatePolygonRgn(pts,sizeof(pts)/sizeof(POINT),WINDING);
	return rgn.PtInRegion(pt)!=0;
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	static CRect RotateRect90(CRect const &rc)
	{	return CRect(rc.top,rc.right,rc.bottom,rc.left);
	}
	// 
	static bool IsIntersectRect(RECT const *rc1, RECT const *rc2)
	{	return ((rc2->left<rc1->left && rc2->right>rc1->left) ||	// compare X.
	(rc2->left<rc1->right && rc2->right>rc1->right) ||
	(rc2->left>=rc1->left && rc2->right<=rc1->right)) &&
	((rc2->top<rc1->top && rc2->bottom>rc1->top) ||		// compare Y.
	(rc2->top<rc1->bottom && rc2->bottom>rc1->bottom) ||
	(rc2->top>=rc1->top && rc2->bottom<=rc1->bottom));
	}
	// 
	// return true - rcDst contains rcSrc rect.
	static bool IsContainRect(RECT const *rcDst, RECT const *rcSrc)
	{	return (rcSrc->left>=rcDst->left && rcSrc->right<=rcDst->right) &&
	(rcSrc->top>=rcDst->top && rcSrc->bottom<=rcDst->bottom);
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
#pragma warning(push)
#pragma warning(disable : 4201)	// nonstandard extension used.
// 
#include <MMSystem.h>
#pragma comment (lib, "Winmm.lib")	// for Multimedia Timers.
// 
struct Timer : private CWnd
{	interface INotify
{	virtual void OnTimer(Timer *timer, int id, int time) = 0;
};

public:
	Timer()
	{	m_pINotify = NULL;
	m_iID = m_iTime = 0;
	m_uTimerID = 0;
	}
	~Timer()
	{	KillTimer();
	}
	// 
	bool SetTimer(INotify *p, int id, int time/*ms*/)
	{	_ASSERTE(p!=NULL);
	_ASSERTE(id>0 && time>0);
	// 
	if(IsActive()==true) KillTimer();
	// 
	m_pINotify = p;
	m_iID = id;
	m_iTime = time;
	// 
	if(CWnd::Create(Tools::RegisterWndClass(_T("Timer")),_T(""),WS_CHILD,CRect(0,0,0,0),CWnd::GetDesktopWindow(),100)==0 ||
		(m_uTimerID = timeSetEvent(time,0,TimeProc,(DWORD_PTR)this,TIME_PERIODIC))==0)
	{
		KillTimer();
		return false;
	}
	// 
	return true;
	}
	void KillTimer()
	{	if(m_uTimerID!=0)
	{	timeKillEvent(m_uTimerID);
	m_uTimerID = 0;
	}
	if(m_hWnd!=NULL) DestroyWindow();
	//  
	m_pINotify = NULL;
	m_iID = m_iTime = 0;
	}
	// 
	bool IsActive() const
	{	return m_hWnd!=NULL;
	}
	// 
	INotify *GetINotify() const
	{	return m_pINotify;
	}
	int GetID() const
	{	return m_iID;
	}
	int GetTime() const
	{	return m_iTime;
	}

protected:
	INotify *m_pINotify;
	int m_iID, m_iTime;
	// 
	UINT m_uTimerID;

protected:
	static void CALLBACK TimeProc(UINT /*uID*/, UINT /*uMsg*/, DWORD_PTR dwUser, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
	{	Timer *p = (Timer *)dwUser;
	::SendMessage(p->m_hWnd,WM_USER,0,0);
	}
	// 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{	LRESULT res = CWnd::WindowProc(message,wParam,lParam);
	if(message==WM_USER) m_pINotify->OnTimer(this,m_iID,m_iTime);
	return res;
	}
};
// 
#pragma warning(pop)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct PaintFinishWaiter : private CWnd
{	bool Wait()
{	if(CWnd::Create(Tools::RegisterWndClass(_T("PaintFinishWaiter")),_T(""),WS_CHILD,
CRect(0,0,0,0),CWnd::GetDesktopWindow(),100)==0)
return false;
*GetFlagPtr() = true;
// 
SetTimer(1,1,NULL);
RunModalLoop(MLF_NOKICKIDLE);
KillTimer(1);
// 
DestroyWindow();
*GetFlagPtr() = false;
return true;
}
// 
static bool IsWait()
{	return *GetFlagPtr()==true;
}

protected:
	static bool *GetFlagPtr()
	{	static bool bWaiting = false;
	return &bWaiting;
	}

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{	if(message==WM_TIMER)
	if(HIWORD(GetQueueStatus(QS_PAINT))==0)
		EndModalLoop(0);
	return CWnd::WindowProc(message,wParam,lParam);
	}
};
// 
struct PaintFinishWaiterEx
{	bool Wait()
{	*GetFlagPtr() = true;
// 
MSG msg;		// process of all WM_PAINT messages.
while(PeekMessage(&msg,NULL,0,0,PM_REMOVE | (0x0020<<16)/*PM_QS_PAINT*/)!=0)
{	::TranslateMessage(&msg);
::DispatchMessage(&msg);
} 
// 
*GetFlagPtr() = false;
return true;
}
// 
static bool IsWait()
{	return *GetFlagPtr()==true;
}

protected:
	static bool *GetFlagPtr()
	{	static bool bWaiting = false;
	return &bWaiting;
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct DeferWindow
{	~DeferWindow()
{	Process();
}

public:
	bool MoveWindow(CWnd *wnd, RECT const *rc, bool redraw)	// move window.
	{	_ASSERTE(wnd!=NULL);
	// 
	return Add(wnd->m_hWnd,rc,redraw,false);
	}
	// 
	bool MoveWindow(HWND wnd, RECT const *rc, bool redraw)	// move window.
	{	_ASSERTE(::IsWindow(wnd)!=0);
	// 
	return Add(wnd,rc,redraw,false);
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	bool MoveShowWindow(CWnd *wnd, RECT const *rc, bool redraw)	// move and show window.
	{	_ASSERTE(wnd!=NULL);
	// 
	return Add(wnd->m_hWnd,rc,redraw,true);
	}
	// 
	bool MoveShowWindow(HWND wnd, RECT const *rc, bool redraw)	// move and show window.
	{	_ASSERTE(::IsWindow(wnd)!=0);
	// 
	return Add(wnd,rc,redraw,true);
	}
	/////////////////////////////////////////////////////////////////////////////
	// 
	int Process()
	{	int count = (int)windows.size();
	// 
	if(count>0)
	{	HDWP dwp = ::BeginDeferWindowPos(count);
	// 
	if(dwp!=NULL)
	{	for(i_windows i=windows.begin(), e=windows.end(); i!=e; ++i)
	{	Window &w = *i;
	bool needShow = (w.show==true && ::IsWindowVisible(w.wnd)==0);
	// 
	if(HasWindowPos(w.wnd,&w.rc)==false || needShow==true)
		::DeferWindowPos(dwp,w.wnd,0,
		w.rc.left,w.rc.top,w.rc.Width(),w.rc.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE | (needShow==true ? SWP_SHOWWINDOW : 0) |
		(w.redraw==true ? 0 : SWP_NOREDRAW));
	}
	// 
	::EndDeferWindowPos(dwp);
	}
	else
		for(i_windows i=windows.begin(), e=windows.end(); i!=e; ++i)
		{	Window &w = *i;
	bool needShow = (w.show==true && ::IsWindowVisible(w.wnd)==0);
	// 
	if(HasWindowPos(w.wnd,&w.rc)==false || needShow==true)
		::SetWindowPos(w.wnd,0,
		w.rc.left,w.rc.top,w.rc.Width(),w.rc.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE | (needShow==true ? SWP_SHOWWINDOW : 0) |
		(w.redraw==true ? 0 : SWP_NOREDRAW));
	}
	// 
	windows.clear();
	}
	return count;
	}

	/////////////////////////////////////////////////////////////////////////////
private:
	/////////////////////////////////////////////////////////////////////////////
	// 
	struct Window
	{	HWND wnd;
	CRect rc;
	bool redraw, show;
	};
	// 
	Blade::List<Window> windows;
	typedef Blade::List<Window>::iterator i_windows;

	/////////////////////////////////////////////////////////////////////////////
	// 
	bool Add(HWND wnd, RECT const *rc, bool redraw, bool show)
	{	try
	{	windows.push_back(Window());
	Window &w = windows.back();
	w.wnd = wnd;
	w.rc = rc;
	w.redraw = redraw;
	w.show = show;
	}
	catch(std::bad_alloc &)
	{	return false;
	}
	return true;
	}
	// 
	bool HasWindowPos(HWND wnd, RECT const *rc) const	// return 'true' if window has 'rc' position.
	{	CRect rcNow;
	::GetWindowRect(wnd,&rcNow);
	::MapWindowPoints(HWND_DESKTOP,::GetParent(wnd),(POINT *)&rcNow,2);	// screen to parent's client.
	return (rcNow==*rc)!=0;
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct MouseHook
{	struct Notify
{	virtual void OnMouseMove(CPoint /*point*/) {}	// x,y in screen coordinates.
virtual void OnLButtonDown(CPoint /*point*/) {}	// x,y in screen coordinates.
virtual void OnLButtonUp(CPoint /*point*/) {}	// x,y in screen coordinates.
virtual void OnMButtonDown(CPoint /*point*/) {}	// x,y in screen coordinates.
virtual void OnRButtonDown(CPoint /*point*/) {}	// x,y in screen coordinates.
};

public:
	bool Add(Notify *p)
	{	_ASSERTE(IsExist(p)==false);
	// 
	try
	{	Lock();
	GetHookDataPtr()->clients.insert(p);
	}
	catch(std::bad_alloc &)
	{	UnLock();
	return false;
	}
	UnLock();
	return true;
	}
	// 
	void Delete(Notify *p)
	{	_ASSERTE(IsExist(p)==true);
	// 
	Lock();
	GetHookDataPtr()->clients.erase(p);
	UnLock();
	}
	// 
	bool IsExist(Notify *p) const
	{	_ASSERTE(p!=NULL);
	// 
	Lock();
	HookData *phd = GetHookDataPtr();
	bool res = (phd->clients.find(p)!=phd->clients.end());
	UnLock();
	return res;
	}

private:
	struct HookData
	{	HookData()
	{	hook = ::SetWindowsHookEx(WH_MOUSE,(HOOKPROC)HookProc,NULL,::GetCurrentThreadId());
	::InitializeCriticalSection(&critSect);
	}
	~HookData()
	{	if(hook!=NULL) ::UnhookWindowsHookEx(hook);
	::DeleteCriticalSection(&critSect);
	}
	// 
	CRITICAL_SECTION critSect;
	HHOOK hook;
	Blade::Set<Notify *> clients;
	};
	friend HookData;

private:
	static LRESULT __stdcall HookProc(int code, WPARAM wParam, LPARAM lParam)
	{	HookData *phd = GetHookDataPtr();
	// 
	if(code==HC_ACTION)
	{	Lock();
	// 
	Blade::Set<Notify *>::iterator i,n,e;
	// 
	switch(wParam)
	{	case WM_MOUSEMOVE: case WM_NCMOUSEMOVE:
	for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
	{	n = i;
	++n;
	(*i)->OnMouseMove(((MOUSEHOOKSTRUCT *)lParam)->pt);
	}
	break;
	// 
	case WM_LBUTTONDOWN: case WM_NCLBUTTONDOWN:
		for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
		{	n = i;
		++n;
		(*i)->OnLButtonDown(((MOUSEHOOKSTRUCT *)lParam)->pt);
		}
		break;
		// 
	case WM_LBUTTONUP: case WM_NCLBUTTONUP:
		for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
		{	n = i;
		++n;
		(*i)->OnLButtonUp(((MOUSEHOOKSTRUCT *)lParam)->pt);
		}
		break;
		// 
	case WM_RBUTTONDOWN: case WM_NCRBUTTONDOWN:
		for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
		{	n = i;
		++n;
		(*i)->OnRButtonDown(((MOUSEHOOKSTRUCT *)lParam)->pt);
		}
		break;
		// 
	case WM_MBUTTONDOWN: case WM_NCMBUTTONDOWN:
		for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
		{	n = i;
		++n;
		(*i)->OnMButtonDown(((MOUSEHOOKSTRUCT *)lParam)->pt);
		}
		break;
	}
	UnLock();
	}
	return ::CallNextHookEx(phd->hook,code,wParam,lParam);
	}
	// 
	static HookData *GetHookDataPtr()
	{	static HookData hd;
	return &hd;
	}
	// 
	static void Lock()
	{	::EnterCriticalSection( &GetHookDataPtr()->critSect );
	}
	static void UnLock()
	{	::LeaveCriticalSection( &GetHookDataPtr()->critSect );
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct KeyboardHook
{	struct Notify
{	virtual void OnKeyDown(UINT /*keyCode*/, UINT /*msgFlag*/) {}
};

public:
	bool Add(Notify *p)
	{	_ASSERTE(IsExist(p)==false);
	// 
	try
	{	Lock();
	GetHookDataPtr()->clients.insert(p);
	}
	catch(std::bad_alloc &)
	{	UnLock();
	return false;
	}
	UnLock();
	return true;
	}
	// 
	void Delete(Notify *p)
	{	_ASSERTE(IsExist(p)==true);
	// 
	Lock();
	GetHookDataPtr()->clients.erase(p);
	UnLock();
	}
	// 
	bool IsExist(Notify *p) const
	{	_ASSERTE(p!=NULL);
	// 
	Lock();
	HookData *phd = GetHookDataPtr();
	bool res = (phd->clients.find(p)!=phd->clients.end());
	UnLock();
	return res;
	}

private:
	struct HookData
	{	HookData()
	{	hook = ::SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)HookProc,NULL,::GetCurrentThreadId());
	::InitializeCriticalSection(&critSect);
	}
	~HookData()
	{	if(hook!=NULL) ::UnhookWindowsHookEx(hook);
	::DeleteCriticalSection(&critSect);
	}
	// 
	CRITICAL_SECTION critSect;
	HHOOK hook;
	Blade::Set<Notify *> clients;
	};
	friend HookData;

private:
	static LRESULT __stdcall HookProc(int code, WPARAM wParam, LPARAM lParam)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());
		HookData *phd = GetHookDataPtr();
	// 
	if(code==HC_ACTION &&
		(lParam & 0x80000000)==0)	// key is down.
	{
		Lock();
		// 
		Blade::Set<Notify *>::iterator i,n,e;
		for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
		{	n = i;
		++n;
		(*i)->OnKeyDown( (UINT)wParam, (UINT)lParam);
		}
		// 
		UnLock();
	}
	return ::CallNextHookEx(phd->hook,code,wParam,lParam);
	}
	// 
	static HookData *GetHookDataPtr()
	{	static HookData hd;
	return &hd;
	}
	// 
	static void Lock()
	{	::EnterCriticalSection( &GetHookDataPtr()->critSect );
	}
	static void UnLock()
	{	::LeaveCriticalSection( &GetHookDataPtr()->critSect );
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct ActivityHook
{	interface INotify
{	virtual void OnActive(bool active, HWND wnd) = 0;
};

public:
	bool Add(INotify *p, HWND wnd)
	{	_ASSERTE(IsExist(p)==false);
	_ASSERTE(wnd!=NULL && ::IsWindow(wnd)!=0);
	// 
	try
	{	Lock();
	GetHookDataPtr()->clients[p] = wnd;
	}
	catch(std::bad_alloc &)
	{	UnLock();
	return false;
	}
	UnLock();
	return true;
	}
	// 
	void Delete(INotify *p)
	{	_ASSERTE(IsExist(p)==true);
	// 
	Lock();
	GetHookDataPtr()->clients.erase(p);
	UnLock();
	}
	// 
	bool IsExist(INotify *p) const
	{	_ASSERTE(p!=NULL);
	// 
	Lock();
	HookData *phd = GetHookDataPtr();
	bool res = (phd->clients.find(p)!=phd->clients.end());
	UnLock();
	return res;
	}

private:
	struct HookData
	{	HookData()
	{	hook = ::SetWindowsHookEx(WH_CALLWNDPROC,(HOOKPROC)HookProc,NULL,::GetCurrentThreadId());
	::InitializeCriticalSection(&critSect);
	}
	~HookData()
	{	if(hook!=NULL) ::UnhookWindowsHookEx(hook);
	::DeleteCriticalSection(&critSect);
	}
	// 
	CRITICAL_SECTION critSect;
	HHOOK hook;
	Blade::Map<INotify *,HWND> clients;
	};
	friend HookData;

private:
	static LRESULT __stdcall HookProc(int code, WPARAM wParam, LPARAM lParam)
	{	HookData *phd = GetHookDataPtr();
	// 
	if(code==HC_ACTION)
	{	CWPSTRUCT *info = (CWPSTRUCT *)lParam;
	// 
	if(info->message==WM_SETFOCUS || info->message==WM_KILLFOCUS)
	{	Lock();
	if(info->message==WM_SETFOCUS)
		SetFocusProc(phd,info);
	else if(info->message==WM_KILLFOCUS)
		KillFocusProc(phd,info);
	UnLock();
	}
	}
	return ::CallNextHookEx(phd->hook,code,wParam,lParam);
	}
	// 
	static void SetFocusProc(HookData *phd, CWPSTRUCT *info)
	{	Blade::Map<INotify *,HWND>::iterator i,n,e;
	for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
	{	n = i;
	++n;
	// 
	HWND wnd = i->second;
	_ASSERTE(::IsWindow(wnd)!=0);
	INotify *p = i->first;
	// 
	for(HWND h=info->hwnd; h!=NULL; h=::GetParent(h))
		if(wnd==h)
			p->OnActive(true,info->hwnd);
	}
	}
	static void KillFocusProc(HookData *phd, CWPSTRUCT *info)
	{	Blade::Map<INotify *,HWND>::iterator i,n,e;
	for(i=phd->clients.begin(), e=phd->clients.end(); i!=e; i=n)
	{	n = i;
	++n;
	// 
	HWND wnd = i->second;
	_ASSERTE(::IsWindow(wnd)!=0);
	INotify *p = i->first;
	// 
	for(HWND h=info->hwnd; h!=NULL; h=::GetParent(h))
		if(wnd==h)
			p->OnActive(false,info->hwnd);
	}
	}

private:
	static HookData *GetHookDataPtr()
	{	static HookData hd;
	return &hd;
	}
	// 
	static void Lock()
	{	::EnterCriticalSection( &GetHookDataPtr()->critSect );
	}
	static void UnLock()
	{	::LeaveCriticalSection( &GetHookDataPtr()->critSect );
	}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct CriticalSection
{	CriticalSection()
{	::InitializeCriticalSection(&m_sec);
}
~CriticalSection()
{	::DeleteCriticalSection(&m_sec);
}
void Lock()
{	::EnterCriticalSection(&m_sec);
}
void Unlock()
{	::LeaveCriticalSection(&m_sec);
}

private:
	CRITICAL_SECTION m_sec;
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// 
struct RgnEx : public CRgn
{	RgnEx()
{	CRgn::CreateRectRgn(0,0,0,0);
}
RgnEx(int l, int t, int r, int b)
{	CRect rc(l,t,r,b);
CRgn::CreateRectRgnIndirect(&rc);
}
RgnEx(CRect const &rc)
{	CRgn::CreateRectRgnIndirect(&rc);
}
RgnEx(RgnEx const &obj)
{	CRgn::CreateRectRgn(0,0,0,0);
CRgn::CopyRgn((CRgn *)&obj);
}
RgnEx(CRgn const &obj)
{	CRgn::CreateRectRgn(0,0,0,0);
CRgn::CopyRgn((CRgn *)&obj);
}
// 
operator CRgn *()
{	return this;
}
};
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



#pragma warning(pop)