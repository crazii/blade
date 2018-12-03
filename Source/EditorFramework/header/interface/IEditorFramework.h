/********************************************************************
	created:	2010/06/12
	filename: 	IEditorFramework.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEditorFramework_h__
#define __Blade_IEditorFramework_h__
#include <interface/InterfaceSingleton.h>
#include <interface/public/ui/IUIMainWindow.h>
#include <interface/public/IEditor.h>
#include <interface/public/IEditorFile.h>
#include <interface/IFileUINotify.h>
#include <utility/Version.h>

namespace Blade
{
	struct SEditorFileInfoEx : public SEditorFileInfo
	{
		//for factory creation
		TString			mFileClassName;
		TString			mWindowType;	//window type class name
	};

	class IEditorFramework : public InterfaceSingleton<IEditorFramework>
	{
	public:
		///predefined media types:
		BLADE_EDITOR_API static const TString MEDIA_TYPE_BRUSH;
		///editor resource path
		BLADE_EDITOR_API static const TString EDITOR_RESOURCE_PATH;
		BLADE_EDITOR_API static const TString EDITOR_ICON_PATH;

		/**
		@describe the editor prefix is used to skip loading/saving by editor,
		element/entity with the prefix will not be saved, thus will not be loaded by terminal apps
		@param 
		@return 
		*/
		BLADE_EDITOR_API static const TString EDITOR_ELEMENT_PREFIX;
		BLADE_EDITOR_API static const TString EDITOR_ENTITY_PREFIX;
		BLADE_EDITOR_API static const TString EDITOR_SCENE_PREFIX;

		//pre-created widget
		BLADE_EDITOR_API static const TString EDITOR_WIDGET_TOOL;
		BLADE_EDITOR_API static const TString EDITOR_WIDGET_PROPERTY;
		BLADE_EDITOR_API static const TString EDITOR_WIDGET_MEDIAVIEW;
		BLADE_EDITOR_API static const TString EDITOR_WIDGET_LOGVIEW;
		BLADE_EDITOR_API static const TString EDITOR_WIDGET_TRACKVIEW;

	public:

		/**
		@describe
		@param
		@return
		*/
		virtual void	initialize(IUIMainWindow* ui, const TString& icon, Version version) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void	deinitialize() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IUIMainWindow*	getEditorUI() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool	addEditor(IEditor* editor) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool	removeEditor(IEditor* editor) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void	registerFileType(const SEditorFileInfoEx& fileinfo) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool	unregisterFileType(const TString& fileClassName) = 0;

		inline bool		unregisterFileType(const SEditorFileInfoEx& fileinfo)
		{
			return this->unregisterFileType(fileinfo.mFileClassName);
		}

		/**
		@describe
		@param
		@return
		*/
		virtual size_t	getFileTypeCount() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const SEditorFileInfoEx* getFileType(FileTypeID ftid) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const SEditorFileInfoEx* getFileType(const TString& fileClassName) const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	addFileNotification(IFileUINotify* notif) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	removeFileNotification(IFileUINotify* notif) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IEditorFile*	getEditorFile(FileInstanceID id) const = 0;

		/**
		@describe get the current active opened file on focus
		@param 
		@return 
		*/
		virtual IEditorFile*	getCurrentFile() const = 0;

		/**
		@describe get the currently loading/opening file, none-NULL only on opening file
		@param 
		@return 
		*/
		virtual IEditorFile*	getLoadingFile() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IFrameworkUINotify*	getFrameworkNotification() const = 0;

		/**
		@describe the original prefix is used to skip loading/saving by terminal apps
		element/entity with this prefix will be loaded/saved by editor, but not loaded by terminal apps
		@param 
		@return 
		*/
		virtual const TString&	getOriginalEntitySkipPrefix() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getOrininalElementSkipPrefix() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getOriginalSceneTypeSkipPrefix() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			initStageConfig(IStage* stage, const HCONFIG& config) const = 0;

		/** @brief helper functions */
		static inline IEditorFile*	getFileFromWindow(IUIWindow* window)
		{
			//explicit use framework data from ui window.
			return window != NULL ? static_cast<IEditorFile*>(window->getFrameworkData()) : NULL;
		}

	};//class IEditorFramework

	extern template class  BLADE_EDITOR_API Factory<IEditorFramework>;
	
}//namespace Blade


#endif //__Blade_IEditorFramework_h__