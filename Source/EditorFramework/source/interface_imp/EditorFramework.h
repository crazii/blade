/********************************************************************
	created:	2010/06/12
	filename: 	EditorFramework.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_EditorFramework_h__
#define __Blade_EditorFramework_h__
#include <utility/BladeContainer.h>
#include <RefCount.h>
#include <interface/public/window/IWindowEventHandler.h>
#include <interface/IEditorFramework.h>
#include <interface/public/ui/IUILayoutManager.h>
#include "CommonMenuToolNotify.h"

namespace Blade
{
	#define CHECK_EDITOR() do{if( mEditorUI == NULL ){BLADE_EXCEPT(EXC_NEXIST,BTString("editor framework not initialized.") );}}while(false)

	class IEditorTool;


	class EditorFramework : public IEditorFramework, public IWindowEventHandler, public Singleton<EditorFramework>
	{
	public:
		using Singleton<EditorFramework>::getSingleton;
		using Singleton<EditorFramework>::getSingletonPtr;
	public:
		EditorFramework();
		~EditorFramework();

		/************************************************************************/
		/* IUIListener interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void onWindowDestroy(IWindow* window);

		/** @brief  */
		virtual bool onWindowClose(IWindow* window);

		/** @brief  */
		virtual void onWindowShow(IWindow* window);

		/************************************************************************/
		/* IEditorFramework interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual void	initialize(IUIMainWindow* ui, const TString& icon, Version version);

		/*
		@describe
		@param
		@return
		*/
		virtual void	deinitialize();

		/*
		@describe
		@param
		@return
		*/
		virtual IUIMainWindow*	getEditorUI() const			{return mEditorUI;}

		/*
		@describe
		@param
		@return
		*/
		virtual bool	addEditor(IEditor* editor);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	removeEditor(IEditor* editor);

		/*
		@describe
		@param
		@return
		*/
		virtual void	registerFileType(const SEditorFileInfoEx& fileinfo);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	unregisterFileType(const TString& fileClassName);

		/*
		@describe
		@param
		@return
		*/
		virtual size_t	getFileTypeCount() const					{return mFileTypeList.size();}

		/*
		@describe get file info by type id. the type id ranges [0, getFileTypeCount() )
		@param
		@return
		*/
		virtual const SEditorFileInfoEx* getFileType(FileTypeID ftid) const	{return ftid < mFileTypeList.size() ? mFileTypeList[ftid] : NULL;}

		/*
		@describe
		@param
		@return
		*/
		virtual const SEditorFileInfoEx* getFileType(const TString& fileClassName) const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	addFileNotification(IFileUINotify* notif);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeFileNotification(IFileUINotify* notif);

		/*
		@describe get the opend file
		@param 
		@return 
		*/
		virtual IEditorFile*	getEditorFile(FileInstanceID id) const;

		/*
		@describe get the current active opened file on focus
		@param 
		@return 
		*/
		virtual IEditorFile*	getCurrentFile() const					
		{
			//note: on some ui implementations, show event are dispatched before window become visible,
			//so mEditorUI->getLayoutManager()->getActiveWindow() is the prev window. so we cannot verify it
			//TODO:
			//assert(mActiveFile == NULL ||
			//	(mEditorUI != NULL && mEditorUI->getLayoutManager()->getActiveWindow() != NULL &&
			//		mEditorUI->getLayoutManager()->getActiveWindow()->getFrameworkData() == mActiveFile) );
			return mActiveFile;
		}

		/*
		@describe get the currently loading/opening file, none-NULL only on opening file
		@param 
		@return 
		*/
		virtual IEditorFile*	getLoadingFile() const					{return mCurrentOpeningFile;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual IFrameworkUINotify*	getFrameworkNotification() const	{return &mInternalNotify;}

		/*
		@describe the original prefix is used to skip loading/saving by terminal apps
		element/entity with this prefix will be loaded/saved by editor, but not loaded by terminal apps
		@param 
		@return 
		*/
		virtual const TString&	getOriginalEntitySkipPrefix() const		{return mOriginalEntityPrefix;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getOrininalElementSkipPrefix() const	{return mOriginalElementPrefix;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getOriginalSceneTypeSkipPrefix() const	{return mOriginalSceneTypePrefix;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			initStageConfig(IStage* stage, const HCONFIG& config) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		IEditorFile*		getOpenedFile(FileInstanceID fileID);
		SEditorFileInfoEx*	getFileType(FileTypeID ftid)	{return ftid < mFileTypeList.size() ? mFileTypeList[ftid] : NULL;}
		SEditorFileInfoEx*	getFileType(const TString& fileClassName)
		{
			FileTypeRegistry::iterator i = mFileTypeRegistry.find(fileClassName);
			if( i!= mFileTypeRegistry.end() )
				return &(i->second);
			return NULL;
		}
		SEditorFileInfoEx* getFileTypeByExtension(const TString& extension) const
		{
			for (size_t i = 0; i < mFileTypeList.size(); ++i)
			{
				if (mFileTypeList[i]->mFileExtension == extension)
					return mFileTypeList[i];
			}
			return NULL;
		}
		/** @brief  */
		void			setupMenus();
		/** @brief  */
		void			createWidgets();
		/** @brief  */
		void			setupToolbox();
		/** @brief  */
		void			setupUILayout();
		/** @brief  */
		void			resetUILayout();
		/** @brief  */
		void			saveLayout();
		/** @brief  */
		void			saveFile();
		/** @brief  */
		bool			isSaveAllNeeded() const;
		/** @brief  */
		void			saveAll();

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4127)		//while(true)
#endif

		/** @brief  */
		inline void		creatFile()		{CHECK_EDITOR();this->newFileInstance(true);}
		/** @brief  */
		inline void		openFile()		{CHECK_EDITOR();this->newFileInstance(false);}
		/** @brief  */
		inline void		exit()			{CHECK_EDITOR();mEditorUI->getWindow()->closeWindow();}

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

	protected:
		/** @brief  */
		void		newFileInstance(bool bCreate);

		typedef	Set<IEditor*>	EditorList;
		typedef Map<FileTypeID,SEditorFileInfoEx*>	FileTypeInfo;
		typedef TStringMap<SEditorFileInfoEx>		FileTypeRegistry;
		typedef Blade::Vector<Blade::SEditorFileInfoEx*>	EditorFileTypeList;
		typedef Map<FileInstanceID,IEditorFile*>	OpenedFileMap;
		typedef Map<IWindow*, IUIWindow*>			UIWindowMap;

		IUIMainWindow*		mEditorUI;
		EditorList			mEditors;
		FileTypeRegistry	mFileTypeRegistry;
		EditorFileTypeList	mFileTypeList;
		OpenedFileMap		mOpenedFiles;
		UIWindowMap			mUIWindowMap;
		RefCount			mFielHandleCounter;

		typedef Vector<IEditorTool*>	CommonToolList;
		CommonToolList		mCommonTools;

		TString				mOriginalEntityPrefix;
		TString				mOriginalElementPrefix;
		TString				mOriginalSceneTypePrefix;

		typedef	Set<IFileUINotify*>		FileNotification;

		FileNotification	mNotifications;
		IEditorFile*		mActiveFile;
		IEditorFile*		mCurrentOpeningFile;

		mutable CommonNotify	mInternalNotify;
	};//class EditorFramework 
	
}//namespace Blade


#endif //__Blade_EditorFramework_h__