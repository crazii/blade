// Win32DialogMFC.h : main header file for the Win32DialogMFC DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include <interface/public/window/IWindowMessagePump.h>

// CWin32DialogMFCApp
// See Win32DialogMFC.cpp for the implementation of this class

class CWin32UIEntryApp : public MFCApp,public Blade::IWindowMessagePump::IMessageListener
{
public:
	CWin32UIEntryApp();

	/************************************************************************/
	/* IMessageListener interface                                                                     */
	/************************************************************************/
	/** @brief  */
	virtual bool preProcessMessage(void* msg);
	/** @brief  */
	virtual void postProcessMessage(void* msg);
	/** @brief called on each loop before processing all messages */
	virtual void preProcessMessages()	{}
	/** @brief called on each loop after processing all messages */
	virtual void postProcessMessages()	{}

	/** @brief  */
	virtual bool isIdle();
	/** @brief  */
	virtual bool onIdleUpdate(bool& bContinueUpdate);

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/

// Overrides
public:
	virtual BOOL	InitInstance();
	virtual int		ExitInstance();

	DECLARE_MESSAGE_MAP()
protected:
	LONG	mIdleCount;
	bool	mIdle;
};
