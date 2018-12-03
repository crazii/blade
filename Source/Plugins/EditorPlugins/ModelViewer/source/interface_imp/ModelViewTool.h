/********************************************************************
	created:	2015/11/23
	filename: 	ModelViewTool.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelViewTool_h__
#define __Blade_ModelViewTool_h__
#include <BladeModelViewer.h>
#include <CommonEditorTools.h>
#include <interface/EditorElement.h>

namespace Blade
{
	extern const TString MODEL_TOOL_CATEGORY;

	class IKTestHelperElement : public EditorElement
	{
	public:
		static const TString IKHELPER_TYPE;
	public:
		IKTestHelperElement();

		virtual ~IKTestHelperElement()	{}

		TString		mEffector;
		int			mEffectorIndex;
	};

	class ModelIKTestTool : public TranslateTool, public StaticAllocatable
	{
	public:
		using StaticAllocatable::operator new;
		using StaticAllocatable::operator delete;
	public:
		ModelIKTestTool(IconIndex icon);
		~ModelIKTestTool();

		/*
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param 
		@return 
		*/
		virtual const TString&		getMannaulCategory() const
		{
			return MODEL_TOOL_CATEGORY;
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
			outFiles.push_back(MODEL_FILE_TYPE);
			return 1;
		}

		/** @brief enable/disable notification */
		virtual void				onEnable(bool bEnabled);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				onSelection();

		/**
		@describe disable click & select behavior of SelectTool
		@param 
		@return 
		*/
		virtual bool				onMouseLeftClick(scalar /*x*/,scalar /*y*/,uint32 /*keyModifier*/,IEditorFile* /*file*/)	{return false;}

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		@note update in the task 'run' step
		*/
		virtual bool				mainUpdate(scalar x,scalar y,uint32 keyModifier,scalar deltaTime,IEditorFile* file);

	protected:
		HEDITABLE		mHelper;
	};
	
}//namespace Blade

#endif // __Blade_ModelViewTool_h__
