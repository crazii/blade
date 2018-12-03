/********************************************************************
	created:	2010/05/24
	filename: 	ViewportWindow.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <interface/public/ui/IViewportManager.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/ui/IUIService.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/ui/IMenu.h>
#include <interface/public/ui/IMenuManager.h>
#include <ui/private/UIViewport.h>

#include "resource.h"
#include "ViewportWindow.h"
#include "UIViewportCommand.h"
#include "ViewportToolBar.h"

namespace Blade
{
	const TCHAR*		ViewportWindow::msWindowClassName = NULL;

	BEGIN_MESSAGE_MAP(ViewportWindow, CUIFrameWnd)
		ON_WM_CREATE()
		ON_WM_LBUTTONDOWN()
		ON_WM_RBUTTONDOWN()
		ON_WM_WINDOWPOSCHANGED()
	END_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	ViewportWindow::ViewportWindow()
	{
		mGlobalKeyboard = NULL;

		mViewportLayout = INVALID_INDEX;
		mViewportSet = NULL;

		for (size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			mUIViewport[i] = NULL;
			mViewportWindow[i] = NULL;
			mMouse[i] = NULL;
			mKeyboard[i] = NULL;
		}

		//add internal menus
		IMenu* menu = IMenuManager::getSingleton().findMenu(BTString(BLANG_VIEWPORT));
		if (menu->findMenu(BTString(BLANG_OPTIONS)) == NULL)
		{
			menu->addSeparator();
			menu->addItem(BTString(BLANG_OPTIONS), UIViewportCommand::getSingletonPtr(), INVALID_ICON_INDEX, true);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	ViewportWindow::~ViewportWindow()
	{
		for (size_t i = 0; i < mToolbars.size(); ++i)
			BLADE_DELETE mToolbars[i];

		for (size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			if (mViewportWindow[i] != NULL)
				IWindowService::getSingleton().destroyWindow(mViewportWindow[i]->getUniqueName());
		}
	}

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void ViewportWindow::PostNcDestroy()
	{
		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
		{
			IWindowService::getSingleton().destroyWindow(mViewportWindow[i]->getUniqueName());
			mViewportWindow[i] = NULL;
		}

		IViewportManager::getSingleton().destroyViewportSet(mViewportSet);

		__super::PostNcDestroy();
		//this is keeps the same behavior as CFrameWnd: delete this on post PostNcDestroy
		//or we have to deal with each type of window (whether delete this window or not, from other place)
		//BLADE_DELETE this; //now ViewportWindow is inherited from CFrameWnd
	}

	//////////////////////////////////////////////////////////////////////////
	int ViewportWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int ret = __super::OnCreate(lpCreateStruct);

		if (this->GetSafeHwnd() == NULL)
			return ret;

		for (size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			CUIViewport* ui = BLADE_NEW CUIViewport();
			ui->Create(this);
			mUIViewport[i] = ui;
			ui->ShowWindow(SW_HIDE);
		}

		mGlobalKeyboard = IUIService::getSingleton().getGlobalKeyboard();
		for (size_t i = 0; i < MAX_VIEWPORT_COUNT; ++i)
		{
			CUIViewport* ui = mUIViewport[i];

			IWindow* ui_window = IWindowService::getSingleton().attachWindow((uintptr_t)ui->getContentHWND());
			mViewportWindow[i] = ui_window;

			mMouse[i] = IUIService::getSingleton().getInputMouse(ui_window);
			if (mMouse[i] == NULL)
				mMouse[i] = IUIService::getSingleton().createInputMouse(ui_window);
			if (mGlobalKeyboard == NULL)
			{
				mKeyboard[i] = IUIService::getSingleton().getInputKeyboard(ui_window);
				if (mKeyboard[i] == NULL)
					mKeyboard[i] = IUIService::getSingleton().createInputKeyboard(ui_window);
			}
		}
		assert(mToolbars.size() == 0);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void ViewportWindow::OnLButtonDown(UINT nFlags, CPoint point)
	{
		this->SetFocus();
		__super::OnLButtonDown(nFlags, point);
	}

	//////////////////////////////////////////////////////////////////////////
	void ViewportWindow::OnRButtonDown(UINT nFlags, CPoint point)
	{
		this->SetFocus();
		__super::OnRButtonDown(nFlags, point);
	}

	//////////////////////////////////////////////////////////////////////////
	void ViewportWindow::OnWindowPosChanged(WINDOWPOS* lpwndpos)
	{
		__super::OnWindowPosChanged(lpwndpos);

		if (mViewportSet == NULL)
			return;

		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
			mUIViewport[i]->updateLayout();
	}

	/************************************************************************/
	/* IUIWindow interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				ViewportWindow::initWindow(uintptr_t parentWindowImpl, IconIndex icon, const TString& caption/* = TString::EMPTY*/)
	{
		HWND hWndParent = (HWND)(void*)parentWindowImpl;
		if (hWndParent == NULL)
		{
			assert(false);
			return false;
		}

		if (msWindowClassName == NULL)
			msWindowClassName = ::AfxRegisterWndClass(0, LoadCursor(NULL, IDC_ARROW), THEMEBRUSH(TC_WINBK), NULL);

		CWnd* parent = CWnd::FromHandle(hWndParent);
		BOOL ret = CWnd::Create(msWindowClassName, TEXT(""), WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), parent, (UINT)IDC_VIEWPORT_WINDOW)
			&& this->setCurrentViewportLayout(IViewportManager::getSingleton().getCurrentLayout());

		if (ret)
			this->init((uintptr_t)m_hWnd, icon, caption, false, false);
		assert(ret);

		return ret == TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	IKeyboard*			ViewportWindow::getKeyboard() const
	{
		if (mGlobalKeyboard != NULL)
			return mGlobalKeyboard;
		else
			return mKeyboard[mViewportSet->getActiveViewportIndex()];
	}

	//////////////////////////////////////////////////////////////////////////
	IMouse*				ViewportWindow::getMouse() const
	{
		return mMouse[mViewportSet->getActiveViewportIndex()];
	}

	//////////////////////////////////////////////////////////////////////////
	IUIToolBox*			ViewportWindow::createToolBox(const TString& name, ICONSIZE is)
	{
		AFX_MANAGE_STATE(::AfxGetStaticModuleState());

		size_t count = MAX_VIEWPORT_COUNT;
		CFrameWnd* frmWnd[MAX_VIEWPORT_COUNT];
		for (size_t i = 0; i < count; ++i)
			frmWnd[i] = mUIViewport[i];

		size_t width = 0;
		for (size_t i = 0; i < mToolbars.size(); ++i)
			width += mToolbars[i]->getWidth();

		ViewportToolBar* toolbar = BLADE_NEW ViewportToolBar(name, count, width, frmWnd);
		toolbar->setIconSize(is);
		mToolbars.push_back(toolbar);
		return toolbar;
	}

	//////////////////////////////////////////////////////////////////////////
	Blade::IUIToolBox*	ViewportWindow::getToolBox(const TString& name) const
	{
		for (size_t i = 0; i < mToolbars.size(); ++i)
		{
			if (mToolbars[i]->getName() == name)
				return mToolbars[i];
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		ViewportWindow::getCurrentViewportLayout() const
	{
		return mViewportLayout;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportWindow::setCurrentViewportLayout(index_t index)
	{
		if (mViewportLayout == index)
			return false;
		mViewportLayout = index;

		if (mViewportLayout >= IViewportManager::getSingleton().getLayoutCount())
		{
			assert(false);
			return false;
		}
		const VIEWPORT_LAYOUT& layout = IViewportManager::getSingleton().getLayout(mViewportLayout);
		if (mViewportSet == NULL)
			mViewportSet = IViewportManager::getSingleton().createViewportSet(layout);
		else
			mViewportSet->resetLayout(&layout);

		index_t maximizedIndex = INVALID_INDEX;
		index_t activeIndex = INVALID_INDEX;
		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
		{
			IViewport* viewport = mViewportSet->getViewport(i);
			mUIViewport[i]->setViewport(viewport);
			mUIViewport[i]->updateLayout();
			viewport->setWindow(mViewportWindow[i]);

			//only find the first one
			if (viewport->isMaximized() && maximizedIndex == INVALID_INDEX)
				maximizedIndex = i;
			mUIViewport[i]->EnableWindow(TRUE);
			mUIViewport[i]->ShowWindow(SW_SHOW);

			if (viewport->isActive())
				activeIndex = i;
		}

		for (size_t i = mViewportSet->getViewportCount(); i < MAX_VIEWPORT_COUNT; ++i)
		{
			mUIViewport[i]->setViewport(NULL);
			mUIViewport[i]->ShowWindow(SW_HIDE);
			mUIViewport[i]->EnableWindow(FALSE);
		}

		if (maximizedIndex != INVALID_INDEX)
		{
			for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
				mUIViewport[i]->ShowWindow(maximizedIndex == i ? SW_SHOW : SW_HIDE);
		}

		if (activeIndex != INVALID_INDEX)
			mUIViewport[activeIndex]->activate();

		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
			mUIViewport[i]->draw();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IViewportSet*	ViewportWindow::getViewportSet() const
	{
		return mViewportSet;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportWindow::maximizeViewport()
	{
		size_t count = mViewportSet->getViewportCount();
		index_t index = mViewportSet->getActiveViewportIndex();
		assert(index < count);
		bool maximized = !mUIViewport[index]->isMaximized();
		mUIViewport[index]->setMaximize(maximized);

		for (index_t i = 0; i < count; ++i)
		{
			if (i == index)
				mUIViewport[i]->ShowWindow(SW_SHOW);
			else
				mUIViewport[i]->ShowWindow(maximized ? SW_HIDE : SW_SHOW);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ViewportWindow::isViewportMaximized(index_t index) const
	{
		if(index < mViewportSet->getViewportCount())
			return mUIViewport[index]->isMaximized();
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IMouse*				ViewportWindow::getViewportMouse(index_t index) const
	{
		assert(index < mViewportSet->getViewportCount());
		return mMouse[index];
	}

	//////////////////////////////////////////////////////////////////////////
	IKeyboard*			ViewportWindow::getViewportKeyboard(index_t index) const
	{
		assert(index < mViewportSet->getViewportCount());
		return mGlobalKeyboard != NULL ? mGlobalKeyboard : mKeyboard[index];
	}

	//////////////////////////////////////////////////////////////////////////
	void		ViewportWindow::updateViewportLayout()
	{
		//if view-port is maximized, we need up date graphics view after graphics view is created
		for (size_t i = 0; i < mViewportSet->getViewportCount(); ++i)
			mUIViewport[i]->updateLayout();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade