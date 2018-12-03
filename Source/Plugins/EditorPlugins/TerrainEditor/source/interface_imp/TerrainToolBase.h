/********************************************************************
	created:	2011/05/25
	filename: 	TerrainToolBase.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainToolBase_h__
#define __Blade_TerrainToolBase_h__
#include <utility/StringList.h>
#include <interface/public/EditorToolTypes.h>
#include <BladeWorldEditor.h>
#include <BladeTerrainEditor_blang.h>
#include "TerrainBrushRect.h"

namespace Blade
{

	class TerrainCommandBaseTool : public EditorCommandTool
	{
	public:
		static const TString CATEGORY;
	public:
		TerrainCommandBaseTool(const TString& name,IconIndex icon,HOTKEY hotkey);
		virtual ~TerrainCommandBaseTool();


		/*
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param 
		@return 
		*/
		virtual const TString&		getMannaulCategory() const
		{
			return CATEGORY;
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

	protected:
	};//class TerrainCommandBaseTool




	class TerrainBrushBaseTool : public EditorBrushTool
	{
	public:
		static const TString CATEGORY;
	public:
		TerrainBrushBaseTool(const TString& name,IconIndex icon,HOTKEY hotkey,const TString& defaultBrushImage = TString::EMPTY);
		virtual ~TerrainBrushBaseTool();

		/*
		@describe use defined category,\n
		tools with the same category will be placed in the same toolbox
		@param 
		@return 
		*/
		virtual const TString&		getMannaulCategory() const
		{
			return CATEGORY;
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
		virtual bool				onMouseLeftClick(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param [in] x, y : normalized mouse position
		@return whether tool needs update
		if return true, IEditorTool::update will be called on each loop, until onMouseLeftUp() event
		*/
		virtual bool				onMouseLeftDown(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe 
		@param 
		@return reserved
		*/
		virtual bool				onMouseLeftUp(scalar x,scalar y,uint32 keyModifier,IEditorFile* file);

		/*
		@describe
		@param [in] x, y : normalized mouse position
		@return reserved
		*/
		virtual bool				update(scalar x ,scalar y, const POINT3& deltaMove, uint32 keyModifier,scalar deltaTime,IEditorFile* file);

	protected:
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void				onEnable(bool bEnabled);

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual ITerrainBrushOperation&	getBrushOperation() = 0;

		AppFlag				mTerrainAppFlag;
		scalar				mAccumulateTime;
		bool				mEditing;

		static TerrainBrushRect		msRect;
		static bool					msInited;
	};//class TerrainBrushBaseTool

	

}//namespace Blade




#endif // __Blade_TerrainToolBase_h__