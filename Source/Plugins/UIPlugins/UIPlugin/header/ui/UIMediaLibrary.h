/********************************************************************
	created:	2011/08/22
	filename: 	UIMediaLibrary.h
	author:		Crazii
	purpose:	
*********************************************************************/
#pragma once
#include <resource.h>
#include <interface/public/ui/IUIMediaLibrary.h>
#include <ui/private/UIDialog.h>
#include <ui/private/UIListCtrl.h>

namespace Blade
{
	class UIMediaLibrary : public CUIDialog, public Singleton<UIMediaLibrary>
	{
	public:
		using Singleton<UIMediaLibrary>::operator new;
		using Singleton<UIMediaLibrary>::operator delete;
		using Singleton<UIMediaLibrary>::operator new[];
		using Singleton<UIMediaLibrary>::operator delete[];
	public:
		enum { IDD = IDD_MEDIA_LIBRARY };

		UIMediaLibrary(CWnd* pParent = NULL);
		virtual ~UIMediaLibrary();

		/************************************************************************/
		/* IUIMediaLibrary interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool			initialize();

		/** @brief  */
		virtual bool			showMediaLibrary(const TString& type, const TString& curFile,
			IMediaLibrary::ISelectionReceiver& receiver,
			bool bShowDefaultOnly = false);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void			prepareModal(IMediaLibrary::ISelectionReceiver& receiver, const TString& defaultType, const TString& selected, bool defaultOnly);

		/** @brief  */
		void			clearPreview();
	protected:

	public:

		/************************************************************************/
		/* MFC related                                                                     */
		/************************************************************************/
		/** @brief  */
		void			SetModalParent(CWnd* parent)
		{
			m_pParentWnd = parent;
		}

		/** @brief  */
		virtual BOOL	Create(UINT nIDTemplate = IDD_MEDIA_LIBRARY, CWnd* pParentWnd = NULL);

	protected:
		virtual BOOL OnInitDialog();
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual void OnOK();
		virtual void OnCancel();

		DECLARE_MESSAGE_MAP();

		afx_msg void OnNMClickMediaTypeList(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnLvnItemchangedMediaFileList(NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void OnDestroy();

		typedef TStringMap<CImageList*>		TypedIconList;
		typedef TStringMap<index_t>	SelectedItems;


		CUIListCtrl		mTypeList;
		CUIListCtrl		mMediaFileList;
		SelectedItems	mSelectedFiles;
		TypedIconList	mIcons;
		IMediaLibrary::ISelectionReceiver*	mReceiver;
		TString			mDefaultType;
		TString			mSelectedType;
		bool			mShowDefaultOnly;
		uint			mPreviewSize;
	};

	///note: UIMediaLibrary constructor need AFX_MANAGE_STATE, lazy init cannot handle it, use a wrapper.
	class UIMediaLibraryImpl : public IUIMediaLibrary, public Singleton<UIMediaLibraryImpl>
	{
	public:
		/************************************************************************/
		/* IUIMediaLibrary interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual bool			initialize()
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			return UIMediaLibrary::getSingleton().initialize();
		}

		/** @brief  */
		virtual bool			showMediaLibrary(const TString& type, const TString& curFile,
			IMediaLibrary::ISelectionReceiver& receiver,
			bool bShowDefaultOnly = false)
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			return UIMediaLibrary::getSingleton().showMediaLibrary(type, curFile, receiver, bShowDefaultOnly);
		}

		/** @brief  */
		virtual bool			shutdown()	{return true;}
	};
	
}//namespace Blade
