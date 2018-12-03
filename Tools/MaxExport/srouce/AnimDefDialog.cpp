/********************************************************************
	created:	2015/09/13
	filename: 	AnimDefDialog.cpp
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include "AnimDefDialog.h"
#include <utility/Variant.h>
#include <interface/public/window/IWindow.h>
#include <ConfigTypes.h>

namespace Blade
{
	IMPLEMENT_DYNAMIC(AnimDefDialog, CDialog);
	BEGIN_MESSAGE_MAP(AnimDefDialog, CDialog)
		ON_WM_WINDOWPOSCHANGING()
	END_MESSAGE_MAP()

	/************************************************************************/
	/* common functions                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	AnimDefDialog::AnimDefDialog()
		:CDialog(IDD, CWnd::GetActiveWindow())
	{
		mTableWindow = NULL;
		mList = NULL;
		mResourceHandle = NULL;
		mTableHandle = NULL;
		mMaxFrame = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	AnimDefDialog::~AnimDefDialog()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool AnimDefDialog::onItemChange(index_t row, index_t col, const Variant& val)
	{
		//verify data
		switch(col)
		{
			//name
		case 0:
			for(size_t i = 0; i < mTableWindow->getRowCount(); ++i)
			{
				if( i == row )
					break;
				const TString& name = mTableWindow->getCell(i, col);
				if( name == val )
				{
					::MessageBox( ::GetActiveWindow(), TEXT("Animation Name already exist."), TEXT("Reduplicated Value"), MB_ICONEXCLAMATION );
					return false;
				}
			}
			break;
			//start & end
		case 1:
		case 2:
			{
				uint frame = val;
				if( frame >= mMaxFrame )
				{
					::MessageBox( ::GetActiveWindow(), TEXT("Frame number out of range."), TEXT("Invalid Value"), MB_ICONEXCLAMATION );
					return false;
				}
			}
			break;
			//loop
		case 3:
			break;
		default:
			assert(false);
		}
		return true;
	}

	/************************************************************************/
	/* MFC section                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void AnimDefDialog::OnWindowPosChanging(WINDOWPOS* lpwndpos)
	{
		CDialog::OnWindowPosChanging(lpwndpos);
		if( this->GetSafeHwnd() == NULL )
			return;

		static const int SPACING = 5;
		CRect rect;
		CRect rcButton;
		this->GetClientRect(&rect);

		CWnd* button = this->GetDlgItem(IDOK);
		button->GetWindowRect(&rcButton);
		this->ScreenToClient(&rcButton);

		int offy = rect.Height()- rcButton.Height() - rcButton.top;
		int offx = rect.Width()- rcButton.Width() - rcButton.left;
		rcButton.OffsetRect(offx, offy);
		button->MoveWindow(&rcButton);

		button = this->GetDlgItem(IDCANCEL);
		button->GetWindowRect(&rcButton);
		this->ScreenToClient(&rcButton);

		offy = rect.Height()- rcButton.Height() - rcButton.top;
		offx = rect.Width()- rcButton.Width() - rcButton.left - rcButton.Width() - SPACING;
		rcButton.OffsetRect(offx, offy);
		button->MoveWindow(&rcButton);

		::MoveWindow(mTableHandle, rect.left, rect.top, rect.Width(), rect.Height() - rcButton.Height()-SPACING, TRUE);
	}

	//////////////////////////////////////////////////////////////////////////
	BOOL AnimDefDialog::OnInitDialog()
	{
		BOOL ret = CDialog::OnInitDialog();
		if( !ret )
			return ret;

		mConfig[0].bind(BLADE_NEW ConfigAtom(BTString("Name"), BTString("New Animation"), CAF_READWRITE) );
		mConfig[1].bind(BLADE_NEW ConfigAtom(BTString("StartFrame"), uint32(0), CAF_READWRITE) );
		mConfig[2].bind(BLADE_NEW ConfigAtom(BTString("EndFrame"), (uint32)mMaxFrame, CAF_READWRITE) );
		mConfig[3].bind(BLADE_NEW ConfigAtom(BTString("Loop"), false, CAF_READWRITE) );

		AfxSetResourceHandle(mResourceHandle);

		mTableWindow = static_cast<ITableWindow*>( BLADE_FACTORY_CREATE(IUIWindow, ITableWindow::TABLE_WINDOW_TYPE) );
		mTableWindow->initWindow((uintptr_t)this->GetSafeHwnd(), INVALID_ICON_INDEX, BTString("Animation Definitions"));
		mTableWindow->initialize(mConfig, 4, 
			ITableWindow::TES_GRIDVIEW | ITableWindow::TES_ROWEDIT | ITableWindow::TES_ROWEDIT_HELPER | ITableWindow::TES_SHOW_ROWNO | ITableWindow::TES_CONTEXT_MENU,
			this);
		IWindow* window = mTableWindow->getWindow();
		mTableHandle = (HWND)(void*)window->getNativeHandle();

		if(mList == NULL )
			return ret;

		if(mList->size() == 0)
		{
			AnimationDef def;
			def.Name = BTString("New Animation");
			def.StartFrame = 0;
			def.EndFrame = mMaxFrame;
			def.Loop = true;
			mList->push_back(def);
		}

		mTableWindow->beginAddRow();
		for(size_t i = 0; i < mList->size(); ++i)
		{
			const AnimationDef& def = (*mList)[i];
			const Variant row[4] = {def.Name, def.StartFrame, def.EndFrame, true};
			mTableWindow->appendRow(row, 4);
		}
		mTableWindow->endAddRow();
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void AnimDefDialog::OnOK()
	{
		if( mList != NULL )
		{
			size_t count = mTableWindow->getRowCount();
			mList->clear();
			mList->resize(count);
			for(size_t i = 0; i < count; ++i)
			{
				const Variant* row = mTableWindow->getRow(i);
				AnimationDef def;
				def.Name = row[0];
				def.StartFrame = row[1];
				def.EndFrame = row[2];
				def.Loop = row[3];
				(*mList)[i] = def;
			}
		}
		CDialog::OnOK();
	}
	
}//namespace Blade