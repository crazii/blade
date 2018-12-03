// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once
#include <BladePCH.h>
//#define NOMINMAX
#undef TEXT

#define BLADE_USE_MFC_FEATURE_PACK 0

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4263)	//member function does not override base class virtual member function(MFC)
#pragma warning(disable:4264)	//no override available for virtual virtual member function from base(MFC)
#pragma warning(disable:4266)	//no override available for virtual virtual member function from base(MFC)
#pragma warning(disable:4191)	//GUID (MFC)
#pragma warning(disable:4127)	//conditional expression is constant(MFC)
#pragma warning(push)
#pragma warning(disable:4365)	//argument: conversion from . to ., signed/unsigned mismatch
#pragma warning(disable:4917)
#pragma warning(disable:4986)
#pragma warning(disable:4471)
#endif

#define _SCB_REPLACE_MINIFRAME 1
#define _SCB_MINIFRAME_CAPTION 1

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _AFX_NO_DB_SUPPORT

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <Windows.h>
#include <commctrl.h>
#include <atlimage.h>
#define ENABLE_THEME 0
#include <ui/private/theme/ThemeManager.h>
#if BLADE_USE_MFC_FEATURE_PACK
#include <afxdialogex.h>
#include <afxmenubar.h>
#include <afxwinappex.h>
#include <afxframewndex.h>
#include <afxtooltipctrl.h>
#include <afxtooltipmanager.h>
#include <afxvisualmanager.h>
#include <afxtoolbarmenubutton.h>

typedef CWinAppEx MFCApp;
typedef CFrameWndEx MFCFrameWindow;
typedef CMFCToolTipCtrl MFCToolTip;
#else
typedef CWinApp MFCApp;
typedef CFrameWnd MFCFrameWindow;
typedef CToolTipCtrl MFCToolTip;
#endif

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <interface/public/ui/IConfigDialog.h>
#include <interface/IResourceManager.h>
#include <interface/public/window/IWindowEventHandler.h>
#include <interface/public/ui/IUIService.h>
#include <ExceptionEx.h>

namespace Blade
{
	template <typename T, typename CT>
	class WindowCreator : public FactoryUtil::Creator<T>
	{
		virtual T*	create(const char* file,int line) const
		{
			BLADE_UNREFERENCED(file);
			BLADE_UNREFERENCED(line);
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			return BLADE_NEW CT();
		}

		virtual T* create() const
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			return BLADE_NEW CT();
		}

		virtual void destroy(T* p) const
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			BLADE_DELETE p;
		}
	};

}//namespace Blade

#define NameRegisterWindowFactory(_type, _basetype, _name) const Blade::FactoryUtil::RegisterProxy<_basetype, Blade::WindowCreator<_basetype,_type> > _type##_reg(_name)


#include "BladeUIPlugin.h"

//lang files
#include <BladeBase_blang.h>
#include <BladeFramework_blang.h>
#include <UIPlugin_blang.h>

//replace MFC new object with BLADE_NEW
#define BLADE_IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject* PASCAL class_name::CreateObject() \
		{ return BLADE_NEW class_name; }\
		IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::CreateObject, NULL)