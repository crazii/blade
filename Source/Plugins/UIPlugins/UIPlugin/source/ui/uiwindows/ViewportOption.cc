/********************************************************************
	created:	2013/04/28
	filename: 	ViewportOption.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <interface/public/ui/IUILayoutManager.h>
#include <interface/public/ui/uiwindows/IViewportWindow.h>
#include <interface/public/ui/IViewportManager.h>
#include <ui/MainWindow.h>
#include "ViewportOption.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	BEGIN_MESSAGE_MAP(ViewButton, CButton)
		ON_CONTROL_REFLECT_EX(BN_CLICKED,OnClicked)
		ON_CONTROL_REFLECT_EX(BN_DBLCLK,OnClicked)
		ON_WM_PAINT()
	END_MESSAGE_MAP()
	//////////////////////////////////////////////////////////////////////////

	ViewButton::ViewButton(LayoutPage* parent, index_t layout, bool drawText) 
		:mParent(parent),mIndex(layout), mDrawText(drawText)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL	ViewButton::Create(CWnd* pParentWnd)
	{
		CRect rect(0,0,0, 0);
		DWORD style = BS_OWNERDRAW | WS_VISIBLE;
		BOOL ret = __super::Create(TEXT(""), style, rect, pParentWnd, 0xFFFF);
		if(ret)
			this->SetFont(pParentWnd->GetFont());
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ViewButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		CDC* pDC;
		pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		pDC->FillSolidRect(&lpDrawItemStruct->rcItem, THEMECOLOR(TC_CTLBK));

		CBrush& brush = (mParent->getCurrentLayout() == mIndex && !mDrawText ) ? THEMEBRUSH(TC_TEXT) : THEMEBRUSH(TC_WINBK);
		pDC->FrameRect(&lpDrawItemStruct->rcItem, &brush);

		const VIEWPORT_LAYOUT& layout = IViewportManager::getSingleton().getLayout(mIndex);

		CRect itemRect(lpDrawItemStruct->rcItem);
		itemRect.DeflateRect(1,1,1,1);
		for(size_t i = 0; i < layout.count; ++i )
		{
			const VIEWPORT_DESC& desc = layout.layouts[i];
			CRect rect;

			rect.left = itemRect.left + LONG(itemRect.Width()*desc.mLeft) + 1;
			rect.right = itemRect.left + LONG(itemRect.Width()*desc.mRight) - 1;
			rect.top = itemRect.top + LONG(itemRect.Height()*desc.mTop) + 1;
			rect.bottom = itemRect.top + LONG(itemRect.Height()*desc.mBottom) - 1;
			pDC->FrameRect(&rect, &brush);

			const TString layoutString[VT_COUNT] = 
			{
				BTLang(BLANG_TOP),
				BTLang(BLANG_FRONT),
				BTLang(BLANG_LEFT),
				BTLang(BLANG_PERSPECTIVE)
			};

			const TString& vpString = layoutString[desc.mType];
			if( mDrawText )
			{
				COLORREF ot = pDC->SetTextColor(THEMECOLOR(TC_TEXT));
				CFont* of = pDC->SelectObject( this->GetFont() );
				pDC->DrawText( vpString.c_str(), (int)vpString.size(), &rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
				pDC->SetTextColor(ot);
				pDC->SelectObject(of);
			}
		}
		CDC::DeleteTempMap();
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL	ViewButton::OnClicked()
	{
		if( !mDrawText )
			mParent->setCurrentLayout(mIndex);
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	LayoutPage::LayoutPage()
	{
		mLargePreview = NULL;
		mCurrentLayout = IViewportManager::getSingleton().getCurrentLayout();
	}
	LayoutPage::~LayoutPage()
	{
		for(size_t i = 0; i < mButtons.size(); ++i)
			BLADE_DELETE mButtons[i];
		BLADE_DELETE mLargePreview;
	}

	//////////////////////////////////////////////////////////////////////////
	CWnd*	LayoutPage::createWindow(CWnd* parent)
	{
		if( mWnd.GetSafeHwnd() != NULL )
			return &mWnd;

		CRect rect(0,0,0,0);
		BOOL ret = mWnd.Create(NULL, BTLang(BLANG_LAYOUT).c_str(), WS_CHILD|WS_VISIBLE, rect, parent, 0xFFFF);
		assert(ret);
		mWnd.SetFont(parent->GetFont());

		size_t layoutCount = IViewportManager::getSingleton().getLayoutCount();
		mButtons.resize(layoutCount);

		for(size_t i = 0; i < layoutCount; ++i)
		{
			mButtons[i] = BLADE_NEW ViewButton(this, i, false);
			ret = mButtons[i]->Create(&mWnd);
			assert(ret);
		}

		mLargePreview = BLADE_NEW ViewButton(this, mCurrentLayout, true);
		mLargePreview->Create(&mWnd);
		return &mWnd;
	}

	//////////////////////////////////////////////////////////////////////////}
	CWnd*	LayoutPage::getWindow()
	{
		return &mWnd;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LayoutPage::onOK()
	{
		return this->onApply();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LayoutPage::onCancel()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	LayoutPage::onApply()
	{
		IViewportManager::getSingleton().setCurrentLayout(mCurrentLayout);

		IUIWindow* window = MainWindow::getSingleton().getLayoutManager()->getActiveWindow();
		if( window->getWindowType() == IViewportWindow::VIEWPORT_WINDOW_TYPE )
		{
			IViewportWindow* win = static_cast<IViewportWindow*>(window);
			bool ret = win->setCurrentViewportLayout(mCurrentLayout);

			//set view port params
			if( ret && window->getUpdater() != NULL )
				window->getUpdater()->setupWindow(win);
			return true;
		}
		return true;
	}
	
	//////////////////////////////////////////////////////////////////////////
	void	LayoutPage::setCurrentLayout(index_t layout)
	{
		if( mCurrentLayout != layout )
		{
			mLargePreview->setLayout(layout);
			mCurrentLayout = layout;

			for(size_t i = 0; i < mButtons.size(); ++i)
				mButtons[i]->Invalidate();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	LayoutPage::onWindowResize()
	{
		const int BUTTON_SIZE = 40;
		const int BUTTON_SPACING = 5;

		CRect rect;
		mWnd.GetClientRect(&rect);

		int line = 0;
		int col = 0;
		for(size_t i = 0; i < mButtons.size(); ++i)
		{
			mButtons[i]->MoveWindow(col*(BUTTON_SIZE+BUTTON_SPACING), line*(BUTTON_SIZE+BUTTON_SPACING),BUTTON_SIZE, BUTTON_SIZE );
			++col;
			if( col*(BUTTON_SIZE+BUTTON_SPACING) > rect.Width() )
			{
				col = 0;
				++line;
			}
		}

		++line;
		mLargePreview->MoveWindow( 0, line*(BUTTON_SIZE+BUTTON_SPACING), rect.Width(), rect.Height()- line*(BUTTON_SIZE+BUTTON_SPACING) );
	}

}//namespace Blade
