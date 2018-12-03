/********************************************************************
	created:	2017/05/14
	filename: 	ModelBrushTool.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelBrushTool_h__
#define __Blade_ModelBrushTool_h__
#include <BladeModelEditor.h>
#include <BladeWorldEditor.h>
#include <interface/public/EditorToolTypes.h>
#include <utility/StringList.h>
#include <utility/BladeContainer.h>
#include "ModelToolBase.h"

namespace Blade
{

	class ModelBrushTool : public EditorBrushTool, public ModelToolBase, public StaticAllocatable
	{
	public:
		static const TString	MODEL_BRUSH_TOOL_NAME;
	public:
		ModelBrushTool();
		~ModelBrushTool();

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
		virtual bool				onMouseLeftClick(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, IEditorFile* /*file*/) { return false; }

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		*/
		virtual bool				onMouseLeftDown(scalar x, scalar y, uint32 /*keyModifier*/, IEditorFile* file);

		/*
		@describe
		@param
		@return reserved
		*/
		virtual bool				onMouseLeftUp(scalar /*x*/, scalar /*y*/, uint32 /*keyModifier*/, IEditorFile* /*file*/);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		*/
		virtual bool				update(scalar /*x*/, scalar /*y*/, const POINT3& /*deltaMove*/, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* /*file*/) { return false; }

		/**
		@describe 
		@param
		@return
		*/
		virtual bool				mainUpdate(scalar x, scalar y, uint32 /*keyModifier*/, scalar /*deltaTime*/, IEditorFile* file);

		/**
		@describe file switch notification, this usually happens when open/create a file,\n
		or user click the sub window tabs of the editor to switch to another sub window
		@param
		@return
		@note DO NOT keep any reference of the params 'prevFile' nor 'currentFile'
		you should delete any related data on prevFile when switch happens,
		because currently, there's no 'file close' notification yet.
		*/
		virtual void				onFileSwitch(const IEditorFile* prevFile, const IEditorFile* currentFile);

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

		/** @brief calling when brush data changes. (image, intensity, size, etc.) */
		virtual void				onBrushDataChanged();

		/** @brief ui notification  */
		void onConfigChange(void * data);

		TStringList	mModels;
		uint32		mDensity;
		fp32		mMinScale;
		fp32		mMaxScale;
		int64*		mChanceDistribution;
		bool		mPainting;
		bool		mRandomRotation;
		bool		mRandomScale;
		bool		mUseScaleAsSize;
	};
	
}//namespace Blade


#endif // __Blade_ModelBrushTool_h__