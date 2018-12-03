/********************************************************************
	created:	2011/08/22
	filename: 	UIMediaLibrary.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <stdafx.h>
#include <ui/UIMediaLibrary.h>
#include <ui/IconManager.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#undef max
#undef min

namespace Blade
{

	BEGIN_MESSAGE_MAP(UIMediaLibrary, CUIDialog)
		ON_WM_DESTROY()
		ON_NOTIFY(NM_CLICK, IDC_MEDIA_TYPE_LIST, &UIMediaLibrary::OnNMClickMediaTypeList)
		ON_NOTIFY(LVN_ITEMCHANGED, IDC_MEDIA_FILE_LIST, &UIMediaLibrary::OnLvnItemchangedMediaFileList)
	END_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	UIMediaLibrary::UIMediaLibrary(CWnd* pParent/* = NULL*/)
		:CUIDialog(UIMediaLibrary::IDD, pParent)
		, mSelectedType(0)
		, mReceiver(NULL)
		, mDefaultType(0)
		, mShowDefaultOnly(true)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	UIMediaLibrary::~UIMediaLibrary()
	{
		for (TypedIconList::iterator i = mIcons.begin(); i != mIcons.end(); ++i)
			BLADE_DELETE(UIImageList*)(i->second);
	}

	/************************************************************************/
	/* IUIMediaLibrary interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			UIMediaLibrary::initialize()
	{
		size_t types_count = IMediaLibrary::getSingleton().getMediaTypesCount();

		for (size_t i = 0; i < types_count; ++i)
		{
			const IMediaLibrary::MEDIA_TYPE& type = IMediaLibrary::getSingleton().getMediaTypeByIndex(i);

			UIImageList* list = BLADE_NEW UIImageList();
			mIcons[type.mType] = list;
			CImageList& iconList = *list;

			iconList.Create(32, 32, ILC_COLOR32, 0, 64);

			assert(type.mPreviewer != NULL);

			if (type.mSharedIcon)
			{
				//generate only one icon, that's good enough
				const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(i, 0);
				assert(file != NULL);

				HBITMAP hBitmap = IconManager::getSingleton().createBitmap(file->mPreview);

				CBitmap* bitmap = CBitmap::FromHandle(hBitmap);
				iconList.Add(bitmap, RGB(0, 0, 0));
			}
			else
			{
				//add the type string to type list
				//mTypeList.AddString( type.mType.c_str() );

				//generate icon for all files
				size_t fileCount = IMediaLibrary::getSingleton().getMediaFileCount(i);
				for (size_t n = 0; n < fileCount; ++n)
				{
					const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(i, n);
					assert(file != NULL);

					HBITMAP hBitmap = IconManager::getSingleton().createBitmap(file->mPreview);

					CBitmap* bitmap = CBitmap::FromHandle(hBitmap);
					iconList.Add(bitmap, RGB(0, 0, 0));
				}
			}
			CBitmap::DeleteTempMap();
		}

		//cannot clear the cache because the cache is used for other UI to generate bit maps.
		//TODO: render GDI directly using HIMAGE without win32 HBITMAP?
		//IMediaLibrary::getSingleton().clearImageCaches();

		return types_count > 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			UIMediaLibrary::showMediaLibrary(const TString& type, const TString& curFile,
		IMediaLibrary::ISelectionReceiver& receiver,
		bool bShowDefaultOnly/* = false*/)
	{
		try
		{
			this->prepareModal(receiver, type, curFile, bShowDefaultOnly);
			CWnd* foreground = CWnd::GetForegroundWindow();
			this->SetModalParent(foreground);
			return this->DoModal() == IDOK;
		}

		catch (...)
		{
			return false;
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		UIMediaLibrary::prepareModal(
		IMediaLibrary::ISelectionReceiver& receiver,
		const TString& defaultType, const TString& selected, bool defaultOnly)
	{
		mReceiver = &receiver;
		mDefaultType = defaultType;
		mSelectedType = defaultType;
		mShowDefaultOnly = defaultOnly;
		mSelectedFiles.clear();

		const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(defaultType, selected);
		if (file != NULL)
			mSelectedFiles[mDefaultType] = file->mIndex;
	}

	//////////////////////////////////////////////////////////////////////////
	void		UIMediaLibrary::clearPreview()
	{
		CStatic* preview = (CStatic*)this->GetDlgItem(IDC_MEDIA_PREVIEW);
		if (preview != NULL)
		{
			HBITMAP hOldBitmap = preview->SetBitmap(NULL);
			if (hOldBitmap != NULL)
				::DeleteObject(hOldBitmap);
		}
		else
			assert(false);
	}

	/************************************************************************/
	/* MFC related                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	BOOL	UIMediaLibrary::Create(UINT nIDTemplate/* = IDD_MEDIA_LIBRARY*/, CWnd* pParentWnd/* = NULL */)
	{
		return __super::Create(nIDTemplate, pParentWnd);
	}


	//////////////////////////////////////////////////////////////////////////
	BOOL UIMediaLibrary::OnInitDialog()
	{
		BOOL result = __super::OnInitDialog();

		if (!result)
			return result;

		size_t types_count = IMediaLibrary::getSingleton().getMediaTypesCount();

		CRect rect;
		mTypeList.GetWindowRect(&rect);
		mTypeList.InsertColumn(0, TEXT(""));

		CDC* pDC = mTypeList.GetDC();
		if (mShowDefaultOnly)
		{
			//add the type string to type list
			const TString& mediaTypeName = BTString2Lang(mDefaultType);
			mTypeList.InsertItem(0, mediaTypeName.c_str(), -1);
		}
		else
		{
			const TString& defaultTypeName = BTString2Lang(mDefaultType);
			for (size_t i = 0; i < types_count; ++i)
			{
				const IMediaLibrary::MEDIA_TYPE& type = IMediaLibrary::getSingleton().getMediaTypeByIndex(i);
				//add the type string to type list
				const TString& mediaTypeName = BTString2Lang(type.mType);
				int index = mTypeList.InsertItem((int)i, mediaTypeName.c_str(), -1);

				if (mediaTypeName == defaultTypeName)
					mTypeList.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
				else
					mTypeList.SetItemState(index, LVIS_CUT, LVIS_CUT);
			}
		}
		{
			CRect rcTypeList;
			mTypeList.GetClientRect(&rcTypeList);
			mTypeList.SetColumnWidth(0, rcTypeList.Width());
			mTypeList.SetExtendedStyle(mTypeList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
		}

		if (mTypeList.GetItemCount() > 0)
		{
			mMediaFileList.SetImageList(mIcons[mDefaultType], LVSIL_NORMAL);

			//add all files of the first type into the file list
			const IMediaLibrary::MEDIA_TYPE& mediaType = IMediaLibrary::getSingleton().getMediaType(mDefaultType);
			size_t fileCount = IMediaLibrary::getSingleton().getMediaFileCount(mediaType.mIndex);
			for (size_t i = 0; i < fileCount; ++i)
			{
				const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(mDefaultType, i);
				assert(file != NULL);
				mMediaFileList.InsertItem((int)i, file->mName.c_str()
					, mediaType.mSharedIcon ? 0 : (int)i);
			}
		}

		CStatic* preview = (CStatic*)this->GetDlgItem(IDC_MEDIA_PREVIEW);
		if (preview != NULL)
		{
			CRect crect;
			preview->GetWindowRect(&crect);
			int size = std::min(crect.Width(), crect.Height());
			mPreviewSize = Math::GetLowerPowerOf2((uint)size);
		}
		else
		{
			assert(false);
			result = FALSE;
		}

		//show the initial selected one
		SelectedItems::iterator i = mSelectedFiles.find(mDefaultType);
		if (i != mSelectedFiles.end())
		{
			mMediaFileList.SetItemState((int)i->second, LVIS_SELECTED, LVIS_SELECTED);
			mMediaFileList.SetSelectionMark((int)i->second);
			LRESULT lresult;
			this->OnLvnItemchangedMediaFileList(NULL, &lresult);
		}
		mTypeList.ReleaseDC(pDC);
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	void UIMediaLibrary::DoDataExchange(CDataExchange* pDX)
	{
		__super::DoDataExchange(pDX);

		DDX_Control(pDX, IDC_MEDIA_TYPE_LIST, mTypeList);
		DDX_Control(pDX, IDC_MEDIA_FILE_LIST, mMediaFileList);
	}

	//////////////////////////////////////////////////////////////////////////
	void UIMediaLibrary::OnOK()
	{
		if (mSelectedType == mDefaultType)
		{
			if (mReceiver != NULL)
			{
				::POSITION pos = mMediaFileList.GetFirstSelectedItemPosition();
				if (pos != NULL)
				{
					index_t iItem = (index_t)mMediaFileList.GetNextSelectedItem(pos);
					const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(mSelectedType, iItem);
					assert(file != NULL);
					index_t mediaTypeIndex = IMediaLibrary::getSingleton().getMediaType(mSelectedType).mIndex;
					mReceiver->setSelectedTarget(mediaTypeIndex, *file);
				}
			}
			else
				assert(false);
		}

		this->clearPreview();
		this->EndDialog(IDOK);
	}

	//////////////////////////////////////////////////////////////////////////
	void UIMediaLibrary::OnCancel()
	{
		this->clearPreview();
		this->EndDialog(IDCANCEL);
	}


	//////////////////////////////////////////////////////////////////////////
	void UIMediaLibrary::OnNMClickMediaTypeList(NMHDR *pNMHDR, LRESULT *pResult)
	{
		LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
		// TODO: Add your control notification handler code here
		*pResult = 0;

		this->clearPreview();

		if (pNMItemActivate->iItem != -1)
		{
			const IMediaLibrary::MEDIA_TYPE& mediaType = IMediaLibrary::getSingleton().getMediaTypeByIndex((index_t)pNMItemActivate->iItem);
			const TString& currentType = mediaType.mType;
			if (currentType != mDefaultType)
			{
				mMediaFileList.EnableWindow(FALSE);
				this->GetDlgItem(IDOK)->EnableWindow(FALSE);
			}
			else
			{
				mMediaFileList.EnableWindow(TRUE);
				this->GetDlgItem(IDOK)->EnableWindow(TRUE);
			}

			::POSITION pos = mMediaFileList.GetFirstSelectedItemPosition();
			index_t iItem = (index_t)mMediaFileList.GetNextSelectedItem(pos);

			mSelectedType = currentType;

			mMediaFileList.DeleteAllItems();
			mMediaFileList.SetImageList(mIcons[mSelectedType], LVSIL_NORMAL);

			//add all files of the first type into the file list
			size_t fileCount = IMediaLibrary::getSingleton().getMediaFileCount(mediaType.mIndex);
			for (size_t i = 0; i < fileCount; ++i)
			{
				const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(mediaType.mIndex, i);
				assert(file != NULL);
				mMediaFileList.InsertItem((int)i, file->mName.c_str()
					, mediaType.mSharedIcon ? 0 : (int)i);
			}

			//restore the new selected one
			if (mMediaFileList.GetItemCount() > 0)
			{
				iItem = mSelectedFiles[mSelectedType];
				mMediaFileList.SetItemState((int)iItem, LVIS_SELECTED, LVIS_SELECTED);
			}

		}
	}

	//////////////////////////////////////////////////////////////////////////
	void UIMediaLibrary::OnLvnItemchangedMediaFileList(NMHDR*, LRESULT* pResult)
	{
		//LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
		// TODO: Add your control notification handler code here
		*pResult = 0;

		if (!mMediaFileList.IsWindowEnabled())
			return;
		::POSITION pos = mMediaFileList.GetFirstSelectedItemPosition();
		if (pos == NULL)
			return;

		CStatic* preview = (CStatic*)this->GetDlgItem(IDC_MEDIA_PREVIEW);
		if (preview != NULL)
		{
			index_t iItem = (index_t)mMediaFileList.GetNextSelectedItem(pos);
			if (mSelectedFiles[mSelectedType] != iItem)
			{
				mSelectedFiles[mSelectedType] = iItem;
				const IMediaLibrary::MEDIA_TYPE& mediaType = IMediaLibrary::getSingleton().getMediaType(mSelectedType);
				const IMediaLibrary::MediaFile* file = IMediaLibrary::getSingleton().getMediaFile(mediaType.mIndex, iItem);
				assert(file != NULL);
				HIMAGE hPreview = mediaType.mPreviewer->generatePreview(file->mSchemePath, /*size*/ (size_t)-1, IP_TEMPORARY);
				HBITMAP hBitmap = IconManager::getSingleton().createBitmap(hPreview);

				HBITMAP hOldBitmap = preview->SetBitmap(NULL);
				if (hOldBitmap)
					::DeleteObject(hOldBitmap);
				preview->SetRedraw(FALSE);
				preview->SetBitmap(hBitmap);
				preview->SetRedraw(TRUE);
				preview->SetWindowPos(NULL, 0, 0, (int)mPreviewSize, (int)mPreviewSize, SWP_NOMOVE | SWP_NOZORDER);
			}
		}
		else
		{
			assert(false);
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void UIMediaLibrary::OnDestroy()
	{
		mMediaFileList.SetImageList(NULL, LVSIL_SMALL);
		mMediaFileList.SetImageList(NULL, LVSIL_NORMAL);
		__super::OnDestroy();
	}
	
}//namespace Blade