/********************************************************************
	created:	2011/05/25
	filename: 	TerrainDeformTool.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainDeformTool_h__
#define __Blade_TerrainDeformTool_h__
#include "TerrainToolBase.h"
#include <memory/BladeMemory.h>
#include <interface/ITerrainConfigManager.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class TerrainDeformOperation : public ITerrainBrushOperation
	{
	public:
		TerrainDeformOperation(EditorBrushTool::BrushData& data);

	protected:
		// per terrain process
		virtual bool	process(const PosInfo& x, const PosInfo& z, TerrainEditable* te, scalar deltaTime, bool inverse);

		/** @brief  */
		EditorBrushTool::BrushData&	getBurhDataRef()	{return mBrushDataRef;}

	protected:

		EditorBrushTool::BrushData&	mBrushDataRef;
	private:
		TerrainDeformOperation&	operator=(const TerrainDeformOperation&);
	};//TerrainDeformOperation


	//////////////////////////////////////////////////////////////////////////
	class TerrainDeformTool : public TerrainBrushBaseTool , public StaticAllocatable
	{
	public:
		TerrainDeformTool(IconIndex icon,HOTKEY hotkey);
		~TerrainDeformTool();

	protected:
		/*
		@describe 
		@param 
		@return 
		*/
		virtual ITerrainBrushOperation&	getBrushOperation()	{return mBrushOP;}

		TerrainDeformOperation	mBrushOP;
	};
	

}//namespace Blade



#endif // __Blade_TerrainDeformTool_h__