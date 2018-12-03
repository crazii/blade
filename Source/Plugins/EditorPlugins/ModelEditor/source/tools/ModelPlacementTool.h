/********************************************************************
	created:	2017/2/27
	filename: 	ModelPlacementTool.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelPlacementTool_h__
#define __Blade_ModelPlacementTool_h__
#include <BladeModelEditor.h>
#include <BladeWorldEditor.h>
#include <interface/public/EditorToolTypes.h>
#include <utility/StringList.h>
#include <utility/BladeContainer.h>
#include "ModelToolBase.h"

namespace Blade
{

	class ModelPlacementTool : public EditorCommonTool, public ModelToolBase, public StaticAllocatable
	{
	protected:
		static const TString	MODEL_INDICATOR_ENTITY;
	public:
		static const TString	MODEL_PLACEMENT_TOOL_NAME;
	public:
		ModelPlacementTool();
		~ModelPlacementTool();

		/*
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param
		@return
		*/
		virtual const TString&		getMannaulCategory() const
		{
			return ModelEditorConsts::MODEL_EDITOR_CATEGORY;
		}

		/*
		@describe get the types of file can be applied by this tool
		@param
		@return supported count.if return value is -1,then this tool support all kinds of files
		@see SEditorFileInfoEx::mFileClassName
		*/
		virtual size_t				getSupportedFiles(TStringParam& outFiles)
		{
			outFiles.clear();
			outFiles.push_back(WORLD_FILE_TYPE);
			return 1;
		}

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		*/
		virtual bool				onMouseLeftClick(scalar x, scalar y, uint32 keyModifier, IEditorFile* file);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		*/
		virtual bool				onMouseLeftDown(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, IEditorFile* /*file*/) { return false; }

		/*
		@describe
		@param
		@return reserved
		*/
		virtual bool				onMouseLeftUp(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, IEditorFile* /*file*/) { return false; }

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier, scalar deltaTime, IEditorFile* file);

		/**
		@describe file switch notification, this usually happens when open/create a file,\n
		or user click the sub window tabs of the editor to switch to another sub window
		@param
		@return
		@note DO NOT keep any reference of the params 'prevFile' nor 'currentFile'
		you should delete any related data on prevFile when switch happens,
		because currently, there's no 'file close' notification yet.
		*/
		virtual void				onFileSwitch(const IEditorFile* /*prevFile*/, const IEditorFile* currentFile);

	protected:
		/*
		@describe
		@param
		@return
		*/
		virtual void				onEnable(bool bEnabled);

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void				onSelection();

		/** @brief  */
		void	checkEntityResource();

		/** @brief ui notification  */
		void	onConfigChange(void * data);

	protected:
		//current file data
		IStage*		mStage;
		IEntity*	mHelperEntity;
		//config
		TString		mSelectedModel;
		//runtime data
		bool		mRandomRotation;
	};
	
}//namespace Blade

#endif//__Blade_ModelPlacementTool_h__