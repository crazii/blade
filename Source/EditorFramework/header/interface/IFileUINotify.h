/********************************************************************
	created:	2011/05/26
	filename: 	IFileUINotify.h
	author:		Crazii
	purpose:	this class is for file switching notification for UI
*********************************************************************/
#ifndef __Blade_IFileUINotify_h__
#define __Blade_IFileUINotify_h__
#include <interface/public/IEditorFile.h>
#include <interface/public/IEditorTool.h>


namespace Blade
{
	class IMenu;

	class IFileUINotify
	{
	public:
		
		/**
		@describe when files switch, the corresponding UI may change status
		@param 
		@return 
		@remark efOld is NULL means the first file is created/opened, \n
		efNew is NULL means the last file is closed
		*/
		virtual void	onFileSwitch(const IEditorFile* efOld,const IEditorFile* efNew) = 0;

	};

	///integrated global notification for common UIs
	class IFrameworkUINotify : public IFileUINotify
	{
	public:

		/** @brief  */
		virtual bool			addCommonFileMenu(IMenu* menu) = 0;

		/** @brief  */
		virtual bool			addViewRelatedMenu(IMenu* menu) = 0;

		/** @brief  */
		virtual bool			addCommonFileTool(IEditorTool* tool) = 0;

		/** @brief  */
		virtual bool			addFileTypeTool(const TString& filetype,IEditorTool* tool) = 0;

		/** @brief  */
		virtual bool			addFileTypeMenu(const TString& filetype,IMenu* menu) = 0;
	};
	

}//namespace Blade



#endif // __Blade_IFileUINotify_h__