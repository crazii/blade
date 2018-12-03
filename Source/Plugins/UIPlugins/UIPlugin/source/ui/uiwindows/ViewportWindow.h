/********************************************************************
	created:	2010/05/23
	filename: 	ViewportWindow.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ViewportWindow_h__
#define __Blade_ViewportWindow_h__
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <interface/public/input/IMouse.h>

class CUIViewport;

namespace Blade
{
	class IRenderWindow;
	class ViewportToolBar;

	class ViewportWindow : public CUIFrameWnd, public IViewportWindow, public Allocatable
	{
	public:
		using Allocatable::operator new;
		using Allocatable::operator delete;
		using Allocatable::operator new[];
		using Allocatable::operator delete[];
	public:
		ViewportWindow();
		virtual ~ViewportWindow();
		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void PostNcDestroy();
		/** @brief  */
		afx_msg int	OnCreate(LPCREATESTRUCT lpCreateStruct);
		/** @brief  */
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		/** @brief  */
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		/** @brief  */
		afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);

		/************************************************************************/
		/* IUIWindow interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool		initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption = TString::EMPTY);

		/** @brief  */
		virtual IKeyboard*	getKeyboard() const;

		/** @brief  */
		virtual IMouse*		getMouse() const;

		/** @brief  */
		virtual IUIToolBox*	createToolBox(const TString& name, ICONSIZE is);

		/** @brief  */
		virtual IUIToolBox*	getToolBox(const TString& name) const;

		//the fields above should be available right after this IUIWindow
		//instance is created
		/*
		@describe
		@param
		@return
		*/
		virtual index_t		getCurrentViewportLayout() const;

		/*
		@describe
		@param
		@return
		*/
		virtual bool		setCurrentViewportLayout(index_t index);

		/*
		@describe get window's view port set
		@param
		@return
		*/
		virtual IViewportSet*	getViewportSet() const;

		/*
		@describe maximize/restore the active view port
		@param
		@return
		*/
		virtual bool			maximizeViewport();

		/*
		@describe whether active view port is maximized
		@param
		@return
		*/
		virtual bool			isViewportMaximized(index_t index) const;

		/*
		@describe add mouse listeners for all viewport mice
		@param
		@return
		*/
		virtual IMouse*		getViewportMouse(index_t index) const;

		/*
		@describe
		@param
		@return
		*/
		virtual IKeyboard*	getViewportKeyboard(index_t index) const;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void		updateViewportLayout();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

	protected:

		DECLARE_MESSAGE_MAP()
		IViewportSet*	mViewportSet;
		CUIViewport*	mUIViewport[MAX_VIEWPORT_COUNT];
		IWindow*		mViewportWindow[MAX_VIEWPORT_COUNT];
		IKeyboard*		mGlobalKeyboard;
		IKeyboard*		mKeyboard[MAX_VIEWPORT_COUNT];
		IMouse*			mMouse[MAX_VIEWPORT_COUNT];
		//view port
		index_t			mViewportLayout;

		typedef Vector<ViewportToolBar*>	ViewportToolbarList;
		ViewportToolbarList		mToolbars;

		static const TCHAR*	msWindowClassName;
	};


}//namespace Blade



#endif //__Blade_ViewportWindow_h__