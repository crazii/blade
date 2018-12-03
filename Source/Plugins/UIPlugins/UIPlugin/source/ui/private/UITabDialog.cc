/********************************************************************
	created:	2013/04/28
	filename: 	UITabDialog.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/

#include "stdafx.h"
#include <ui/private/UITabDialog.h>
#include <BladeBase_blang.h>

using namespace Blade;

IMPLEMENT_DYNAMIC(CUITabDialog, CUIDialog)

//////////////////////////////////////////////////////////////////////////
CUITabDialog::CUITabDialog(CWnd* pParent /*=NULL*/)
	: CUIDialog(CUITabDialog::IDD, pParent)
{

}

//////////////////////////////////////////////////////////////////////////
CUITabDialog::~CUITabDialog()
{
	for(size_t i = 0; i < mTabs.size(); ++i)
	{
		BLADE_DELETE mTabs[i];
	}
}

/************************************************************************/
/* custom methods                                                                     */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
index_t	CUITabDialog::addTab(ITabPage* page)
{
	if( page == NULL || page->getWindow() == NULL)
		return INVALID_INDEX;

	if( mTabCtrl.GetSafeHwnd() != NULL )
		this->addTabItem(page);

	mTabs.push_back(page);
	return mTabs.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
void	CUITabDialog::addTabItem(ITabPage* page)
{
	mTabCtrl.AssertValid();

	CString caption;
	CWnd* child = page->createWindow( &mTabCtrl );
	child->ShowWindow(SW_HIDE);
	child->GetWindowText(caption);
	child->SetParent(&mTabCtrl);
	mTabCtrl.InsertItem( mTabCtrl.GetItemCount(), caption.GetString() );

	CRect rect;
	mTabCtrl.GetClientRect(&rect);

	CRect headerRect;
	mTabCtrl.GetItemRect(0,&headerRect);

	const int EDGE = 10;
	rect.DeflateRect(EDGE,EDGE,EDGE,EDGE);
	rect.top += headerRect.Height();

	child->MoveWindow(&rect);
	page->onWindowResize();
}

//////////////////////////////////////////////////////////////////////////
ITabPage*		CUITabDialog::getTab(index_t index)
{
	if( index >= mTabs.size() )
	{
		assert(false);
		return NULL;
	}
	return mTabs[index];
}

//////////////////////////////////////////////////////////////////////////
void			CUITabDialog::setCaption(const Blade::TString& caption)
{
	mCaption = caption;
}

/************************************************************************/
/* MFC related                                                                     */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
void CUITabDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_DIALOG_TAB_CTRL, mTabCtrl);
}

//////////////////////////////////////////////////////////////////////////
BOOL CUITabDialog::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if( !ret )
		return ret;

	mTabCtrl.EnableCustomLook(TRUE, ETC_COLOR|ETC_FLAT);
	mTabCtrl.EnableDraw();
	this->SetWindowText( mCaption.c_str() );

	for(size_t i = 0; i < mTabs.size(); ++i)
	{
		ITabPage* page = mTabs[i];
		this->addTabItem(page);
	}

	mTabCtrl.SetCurSel(0);
	mTabs[0]->getWindow()->ShowWindow(SW_SHOW);

	CString text;
	this->GetDlgItem(IDOK)->GetWindowText(text);
	this->GetDlgItem(IDOK)->SetWindowText( BTString2Lang(TString(text.GetBuffer())).c_str() );

	this->GetDlgItem(IDCANCEL)->GetWindowText(text);
	this->GetDlgItem(IDCANCEL)->SetWindowText( BTString2Lang(TString(text.GetBuffer())).c_str() );

	this->GetDlgItem(IDC_APPLY)->GetWindowText(text);
	this->GetDlgItem(IDC_APPLY)->SetWindowText( BTString2Lang(TString(text.GetBuffer())).c_str() );
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CUITabDialog::OnOK()
{
	index_t i = (index_t)mTabCtrl.GetCurSel();
	assert( i < mTabs.size() );
	if( mTabs[i]->onOK() )
		__super::OnOK();
}

//////////////////////////////////////////////////////////////////////////
void CUITabDialog::OnCancel()
{
	index_t i = (index_t)mTabCtrl.GetCurSel();
	assert( i < mTabs.size() );
	if( mTabs[i]->onCancel() )
		__super::OnCancel();
}

BEGIN_MESSAGE_MAP(CUITabDialog, CUIDialog)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB_DIALOG_TAB_CTRL, &CUITabDialog::OnTcnSelchangingUITabDialogTabCtrl)
	ON_BN_CLICKED(IDC_APPLY, &CUITabDialog::OnBnClickedApply)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
void CUITabDialog::OnTcnSelchangingUITabDialogTabCtrl(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	index_t sel = (index_t)mTabCtrl.GetCurSel();
	assert( sel < mTabs.size() );
	for(size_t i = 0; i < mTabs.size(); ++i)
	{
		ITabPage* page = mTabs[i];
		CWnd* window = page->getWindow();
		window->ShowWindow(SW_HIDE);
	}
	mTabs[sel]->getWindow()->ShowWindow(SW_SHOW);
}

//////////////////////////////////////////////////////////////////////////
void CUITabDialog::OnBnClickedApply()
{
	index_t i = (index_t)mTabCtrl.GetCurSel();
	assert( i < mTabs.size() );
	mTabs[i]->onApply();
}
