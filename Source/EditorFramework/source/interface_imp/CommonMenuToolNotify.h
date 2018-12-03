/********************************************************************
	created:	2011/05/26
	filename: 	CommonMenuToolNotify.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_CommonMenuToolNotify_h__
#define __Blade_CommonMenuToolNotify_h__
#include <utility/BladeContainer.h>
#include <interface/IFileUINotify.h>

namespace Blade
{

	class CommonNotify : public IFrameworkUINotify
	{
	public:
		CommonNotify();
		virtual ~CommonNotify();

		/************************************************************************/
		/* IFileUINotify interface                                                                     */
		/************************************************************************/

		/*
		@describe when files switch, the corresponding UI may change status
		@param 
		@return 
		@remark efOld is NULL means the first file is created/opened, \n
		efNew is NULL means the last file is closed
		*/
		virtual void	onFileSwitch(const IEditorFile* efOld,const IEditorFile* efNew);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/** @brief  */
		virtual bool			addCommonFileMenu(IMenu* menu);

		/** @brief  */
		virtual bool			addViewRelatedMenu(IMenu* menu);

		/** @brief  */
		virtual bool			addCommonFileTool(IEditorTool* tool);

		/** @brief  */
		virtual bool			addFileTypeTool(const TString& filetype, IEditorTool* tool);

		/** @brief  */
		virtual bool			addFileTypeMenu(const TString& filetype, IMenu* menu);

	protected:
		typedef Set<IMenu*>			MenuList;
		typedef Set<IEditorTool*>	ToolList;

		MenuList	mNotifiedMenus;
		MenuList	mViewMenus;
		ToolList	mNotifiedTools;

		typedef TStringMap<MenuList>	FileTypeMenu;
		typedef TStringMap<ToolList>	FileTypeTool;

		FileTypeMenu	mTypedMenu;
		FileTypeTool	mTypedTool;
	};
	

}//namespace Blade



#endif // __Blade_CommonMenuToolNotify_h__