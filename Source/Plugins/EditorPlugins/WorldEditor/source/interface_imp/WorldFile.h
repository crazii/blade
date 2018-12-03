/********************************************************************
	created:	2010/06/14
	filename: 	WorldFile.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_WorldFile_h__
#define __Blade_WorldFile_h__
#include <interface/public/IEditorFile.h>
#include <interface/public/EditorFileBase.h>
#include <interface/IConfig.h>
#include <interface/public/logic/ILogicWorld.h>

namespace Blade
{
	class IStage;

	class WorldFile : public EditorFileBase, public IEditorFileUpdater, public Allocatable
	{
	public:
		WorldFile();
		~WorldFile();

		/************************************************************************/
		/* IEditorFile interface                                                                     */
		/************************************************************************/

		/** @brief  */
		virtual void				openFile(const TString& dir,const TString& name);

		/** @brief  */
		virtual void				saveFile();

		/** @brief  */
		virtual void				newFile(const TString& dir,const TString& name);

		/** @brief  */
		virtual void				closeFile();

		/************************************************************************/
		/* IEditorFileUpdater interface                                                                     */
		/************************************************************************/
		/** @brief  */
		virtual void				onActivate(IEditorFile* /*file*/, bool /*active*/) {}
		/** @brief  */
		virtual void				update(IEditorFile* file);
		/** @brief setup window related params after file is loaded */
		virtual void				setupWindow(IUIWindow* window);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		static const HCONFIG&			getFileConfig()
		{
			return WorldFile::operateFileConfig(NULL);
		}
		/** @brief  */
		static const HCONFIG&			createFileConfig(const TStringList& sceneList)
		{
			return WorldFile::operateFileConfig(&sceneList);
		}
		/** @brief  */
		static void						clearFileConfig()
		{
			WorldFile::operateFileConfig(NULL, true);
		}

	protected:
		/** @brief  */
		static const HCONFIG&			operateFileConfig(const TStringList* list, bool bDelete = false);

		TString			mFile;
	};//class WorldFile

}//namespace Blade


#endif //__Blade_WorldFile_h__