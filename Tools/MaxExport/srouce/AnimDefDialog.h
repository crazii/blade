/********************************************************************
	created:	2015/09/13
	filename: 	AnimDefDialog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_AnimDefDialog_h__
#define __Blade_AnimDefDialog_h__
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#define NOMINMAX

#include "targetver.h"

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



#include <Memory/BladeMemory.h>
#include <resource.h>
#include <interface/public/ui/uiwindows/ITableWindow.h>
#include <interface/IConfig.h>
#include "ExportData.h"

namespace Blade
{
	class AnimDefDialog : public CDialog, public ITableWindow::IListener, public TempAllocatable
	{
	public:
		using TempAllocatable::operator new;
		using TempAllocatable::operator delete;
		using TempAllocatable::operator new[];
		using TempAllocatable::operator delete[];

	public:

		/* @brief you can use it for change notification or get the modified data
		return false to discard change */
		virtual bool onItemChange(index_t row, index_t col, const Variant& val);

		/************************************************************************/
		/* common functions                                                                     */
		/************************************************************************/
		AnimDefDialog();
		virtual ~AnimDefDialog();

		/* @brief  */
		inline void setMaxFrame(uint32 count)
		{
			mMaxFrame = count;
		}

		/* @brief  */
		inline void setAnimationList(AnimationDefList* list)
		{
			mList = list;
		}

		/************************************************************************/
		/* MFC section                                                                     */
		/************************************************************************/
		DECLARE_DYNAMIC(AnimDefDialog);
		DECLARE_MESSAGE_MAP()
	public:
		enum { IDD = IDD_PANEL };

		afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

		/* @brief  */
		virtual BOOL OnInitDialog();

		/* @brief  */
		virtual void OnOK();

		/* @brief  */
		inline void SetResourceHandle(HINSTANCE hInst)
		{
			mResourceHandle = hInst;
		}

	protected:
		ITableWindow*	mTableWindow;
		AnimationDefList*	mList;
		HINSTANCE			mResourceHandle;
		HWND				mTableHandle;
		uint32				mMaxFrame;
		HCONFIG				mConfig[4];
	};
	
}//namespace Blade

#endif // __Blade_AnimDefDialog_h__