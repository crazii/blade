/********************************************************************
	created:	2011/05/07
	filename: 	IEditorTool.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IEditorTool_h__
#define __Blade_IEditorTool_h__
#include <BladeEditor.h>
#include <interface/IConfig.h>
#include <interface/IHotkeyManager.h>
#include <utility/String.h>
#include <utility/StringList.h>
#include <interface/public/IEditorFile.h>

namespace Blade
{

	class BLADE_EDITOR_API IEditorTool
	{
	public:
		virtual ~IEditorTool()	{}

		/**
		@describe tool name identifier
		@param 
		@return 
		*/
		virtual const TString&		getName() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual IconIndex			getToolIcon() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HOTKEY				getHotkey() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const HCONFIG&		getToolConfig() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isEnable() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				setEnable(bool enabled) = 0;

		/**
		@describe 
		@param 
		@return 
		@see Blade::EditorToolTypes
		*/
		virtual const TString&		getType() const = 0;

		/**
		@describe 
		@param 
		@return 
		@note if a tool is immediate command (only executed once, on tool selected),
		it commonly should return false, or it will be marked in selected status( like a check button ),
		but still, the editor tool manager will not selected it.
		*/
		virtual bool				setSelected(bool select) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool				isSelected() const = 0;

		/**
		@describe file switch notification, this usually happens when open/create a file,\n
		or user click the sub window tabs of the editor to switch to another sub window
		@param 
		@return 
		@note DO NOT keep any reference of the params 'prevFile' nor 'currentFile'
		you should delete any related data on prevFile when switch happens,
		because currently, there's no 'file close' notification yet.
		*/
		virtual void				onFileSwitch(const IEditorFile* prevFile,const IEditorFile* currentFile)
		{
			BLADE_UNREFERENCED(prevFile);
			BLADE_UNREFERENCED(currentFile);
		}

		/**
		@describe 
		@param [in] x, y : normalized mouse position
		@return reserved
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file) = 0;

		/**
		@describe 
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier, IEditorFile* file) = 0;

		/**
		@describe 
		@param 
		@return reserved
		@note this function is called though main synchronous state
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file) = 0;

		/**
		@describe
		@param [in] x, y : normalized mouse position
		@param [in] deltaMove : mouse movement delta in pixel from last update
		@return reserved
		@note update in the task 'run' step
		*/
		virtual bool				update(scalar x, scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file) = 0;

		/**
		@describe 
		@param 
		@return 
		@note update in the main synchronous state
		*/
		virtual bool				mainUpdate(scalar /*x*/,scalar /*y*/,uint32 /*keyModifier*/,scalar /*deltaTime*/,IEditorFile* /*file*/)	{return true;}

		/**
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param 
		@return 
		@note return original language of category. the translation will be done by the framework.
		*/
		virtual const TString&		getMannaulCategory() const = 0;

		/**
		@describe get the types of file can be applied by this tool
		@param 
		@return supported count.if return value is -1,then this tool support all kinds of files
		@see SEditorFileInfoEx::mFileClassName
		*/
		virtual size_t				getSupportedFiles(TStringParam& outFiles) = 0;

	};
	

}//namespace Blade



#endif // __Blade_IEditorTool_h__