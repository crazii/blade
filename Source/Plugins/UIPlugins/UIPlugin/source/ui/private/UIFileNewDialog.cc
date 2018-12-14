/********************************************************************
	created:	2010/05/27
	filename: 	UIFileNewDialog.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include <ConfigDialog/ConfigControlUtil.h>
#include <ui/IconManager.h>
#include <ui/private/UIFileNewDialog.h>

using namespace Blade;

//////////////////////////////////////////////////////////////////////////
CUIFileNewDialog::CUIFileNewDialog(CWnd* pParent /* = NULL */)
:CUIDialog(CUIFileNewDialog::IDD,pParent)
,mFileInfo(BLADE_TEMPCONT_INIT)
,mSelectFile(INVALID_INDEX)
,mNewFileName(TEXT(""))
,mFilePathConfig(BTString("PATH"))
{
	mFilePathConfig.initStringValue(BTString("../Data"),CUIH_PATH);
	//mFileOption.SetAllVisible(true);
}

//////////////////////////////////////////////////////////////////////////
CUIFileNewDialog::~CUIFileNewDialog()
{
}

//////////////////////////////////////////////////////////////////////////
void CUIFileNewDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_FILE_TYPE_LIST, mFileTypeList);
	DDX_Text(pDX,IDC_NEWFILE_NAME, mNewFileName);
}

//////////////////////////////////////////////////////////////////////////
void			CUIFileNewDialog::AddFileTypes(const IUIMainWindow::NEW_FILE_DESC* info)
{
	mFileInfo.push_back(info);
}

//////////////////////////////////////////////////////////////////////////
const IUIMainWindow::NEW_FILE_DESC*	CUIFileNewDialog::GetFileType() const
{
	return mSelectFile < mFileInfo.size() ? mFileInfo[mSelectFile] : NULL;
}

//////////////////////////////////////////////////////////////////////////
TString	CUIFileNewDialog::GetFileName() const
{
	return TString( mNewFileName.GetString() );
}

//////////////////////////////////////////////////////////////////////////
const TString&	CUIFileNewDialog::GetFileDirectory() const
{
	return mFilePathConfig.getValue();
}

//////////////////////////////////////////////////////////////////////////
void			CUIFileNewDialog::RefreshOption(Blade::index_t fileOptIndex)
{
	if( mSelectFile != fileOptIndex )
	{
		mSelectFile = fileOptIndex;
	}
	else
		return;

	mFileOption.SetRedraw(FALSE);
	mFileOption.DeleteAllItems();
	mFileOption.setDefaultExpand(true);
	if( mSelectFile < mFileInfo.size() )
	{
		if( mFileTypeList.GetItemState( (int)mSelectFile,UINT(-1) ) & LVIS_CUT )
		{
			mSelectFile = INVALID_INDEX;
			return;
		}

		IConfig* config = mFileInfo[mSelectFile]->mNewFileConfig;

		if( config != NULL )
			mFileOption.setSubConfigs(config);

		mFilePathConfig.setValue(mFileInfo[mSelectFile]->mDefaultPath);
		mDirectoryPicker.updateData(true);
	}
	mFileOption.Invalidate(FALSE);
	mFileOption.SetRedraw(TRUE);
}

//////////////////////////////////////////////////////////////////////////
BOOL CUIFileNewDialog::OnInitDialog()
{
	CRect client;
	this->GetClientRect(&client);
	BOOL ret = __super::OnInitDialog();
	if( !ret )
		return ret;

	//sort file info: stabilize list order
	std::sort(mFileInfo.begin(), mFileInfo.end(), IUIMainWindow::NEW_FILE_DESC::comparePtr);

	CRect rect;
	mFileTypeList.GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	rect.left = rect.right + 20;
	rect.right = client.right;
	mFileOption.Create(this,rect,IDC_FILE_NEW_OPTION);
	rect.right = client.right-20;
	mFileOption.MoveWindow(&rect);

	//calculate the file directory control 's position
	int offsety;
	{
		CRect rcTextName;
		CRect rcTextDir;
		this->GetDlgItem(IDC_STATIC_FILE_NAME)->GetWindowRect(&rcTextName);
		this->ScreenToClient(&rcTextName);
		this->GetDlgItem(IDC_STATIC_FILE_DIR)->GetWindowRect(&rcTextDir);
		this->ScreenToClient(&rcTextDir);
		offsety = rcTextDir.top - rcTextName.top;
	}

	//create the file directory control
	CWnd* fileName = this->GetDlgItem(IDC_NEWFILE_NAME);
	fileName->GetClientRect(&rect);
	fileName->ClientToScreen(&rect);
	this->ScreenToClient(&rect);
	rect.OffsetRect(0,offsety);
	mDirectoryPicker.Create(rect,this,IDC_DIRECTORY_PICKER,&mFilePathConfig);
	mDirectoryPicker.SetTextWidth(rect.Width());
	mDirectoryPicker.ShowWindow(SW_SHOW);
	mFileTypeList.SetImageList(IconManager::getSingleton().getIconList(IS_24),LVSIL_SMALL);
	mFileTypeList.SetImageList(IconManager::getSingleton().getIconList(IS_32),LVSIL_NORMAL);

	int count = 0;
	int width = 0;

	mFileTypeList.InsertColumn(0,TEXT(""));
	CDC* dc = mFileTypeList.GetDC();
	for( FileInfoList::iterator i = mFileInfo.begin(); i != mFileInfo.end(); ++i )
	{
		const IUIMainWindow::NEW_FILE_DESC* info = (*i);
		mFileTypeList.InsertItem( count++, info->mDesc.c_str(), info->mIcon );
		if( info->mEnable )
			mFileTypeList.SetItemState(count - 1, 0, LVIS_SELECTED);
		else
			mFileTypeList.SetItemState(count-1, LVIS_CUT, LVIS_CUT);
			
		CSize sz = dc->GetTextExtent( info->mDesc.c_str(), (int)info->mDesc.size() );
		if( width < sz.cx )
			width = sz.cx;
	}
	mFileTypeList.SetColumnWidth(0, width + (int)IconManager::getSingleton().getIconSize(IS_32) );
	mFileTypeList.ReleaseDC(dc);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
void CUIFileNewDialog::OnOK()
{
	UpdateData(TRUE);

	if( mSelectFile == INVALID_INDEX )
	{
		::AfxMessageBox(TEXT("Please Select the file type."),IDOK);
	}
	else if( mNewFileName == TEXT("") )
	{
		::AfxMessageBox(TEXT("Please input a name for new file"),IDOK);
	}
	else if( mNewFileName.FindOneOf(TEXT("<>/\\:.\"|?*")) != -1 || mNewFileName.GetLength() < 2 )
	{
		::AfxMessageBox(TEXT("Invalid file name"),IDOK);
	}
	else if( mFilePathConfig.getValue() == TEXT("") )
	{
		::AfxMessageBox(TEXT("Please set the directory to create new file."),IDOK);
	}
	else
	{
		__super::OnOK();
	}
}


//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CUIFileNewDialog, CUIDialog)
	ON_NOTIFY(NM_CLICK, IDC_FILE_TYPE_LIST, &CUIFileNewDialog::OnNMClickFileTypeList)
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CUIFileNewDialog::OnNMClickFileTypeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	index_t currentItem = (index_t)pNMItemActivate->iItem;
	this->RefreshOption(currentItem);
}

//////////////////////////////////////////////////////////////////////////
void CUIFileNewDialog::OnDestroy()
{
	mFileTypeList.SetImageList(NULL, LVSIL_SMALL);
	mFileTypeList.SetImageList(NULL, LVSIL_NORMAL);
}