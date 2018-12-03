/********************************************************************
	created:	2011/05/25
	filename: 	TerrainDeformTool.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainDeformTool.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	static const int16 MAX_I16 = (int16)0x7FFF;
	static const int16 MIN_I16 = -(int16)0x7FFF;

	//////////////////////////////////////////////////////////////////////////
	TerrainDeformOperation::TerrainDeformOperation(EditorBrushTool::BrushData& data)
		:mBrushDataRef(data)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	TerrainDeformOperation::process(const PosInfo& x,const PosInfo& z,TerrainEditable* te,scalar deltaTime,bool inverse)
	{
		assert( x.mEnd - x.mStart == x.mTerranEnd - x.mTerranStart);
		assert( z.mEnd - z.mStart == z.mTerranEnd - z.mTerranStart);

		size_t rowCount = z.mEnd - z.mStart;
		size_t colCount = x.mEnd - x.mStart;

		const int32* brushdata = mBrushDataRef.getBrushData();
		size_t brush_offset = z.mStart*mBrushDataRef.getBrushSize() + x.mStart;
		assert(brush_offset < mBrushDataRef.getBrushSize()*mBrushDataRef.getBrushSize() );
		const int32* brush_pointer = brushdata + brush_offset;

		//terrain size is in quads,not vertices, we need vertex count
		size_t terrainSize = ITerrainConfigManager::getInterface().getTerrainInfo().mTileSize+1;
		pint16 buffer = te->getHeightBuffer();
		size_t buffer_offset = z.mTerranStart*terrainSize + x.mTerranStart;
		assert( buffer_offset < terrainSize*terrainSize );
		pint16	buffer_pointer = buffer + buffer_offset;
		//size_t intensity = mBrushData.getIntensity();
#if BLADE_DEBUG
		const pint16 buffer_end = buffer + terrainSize*terrainSize;
		const int32* brush_end = brushdata + mBrushDataRef.getBrushSize()*mBrushDataRef.getBrushSize();
#endif
		for( size_t i = 0; i <= rowCount; ++i )
		{
			for(size_t j = 0; j <= colCount; ++j )
			{
#if BLADE_DEBUG
				//data area overflow check
				assert( brush_pointer+j < brush_end );
				assert( buffer_pointer+j < buffer_end );
#endif

				//TODO: buffer int16 overflow check
				int val = (int)( (scalar)(*(brush_pointer+j)) * deltaTime );

				if( val == 0 )
					continue;

				if( inverse )
				{
					int result = *(buffer_pointer+j) - val;
					*(buffer_pointer+j) = (int16) ( ( result < MIN_I16 ) ? MIN_I16 : result );
				}
				else
				{
					int result = *(buffer_pointer+j) + val;
					*(buffer_pointer+j) = (int16) ( ( result > (int)MAX_I16 ) ? MAX_I16 : result );
				}
			}
			buffer_pointer += terrainSize;
			brush_pointer += mBrushDataRef.getBrushSize();
		}

		te->updateHeightBuffer(x.mTerranStart, z.mTerranStart, colCount, rowCount);
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TerrainDeformTool::TerrainDeformTool(IconIndex icon,HOTKEY hotkey)
		:TerrainBrushBaseTool(BXLang(BLANG_DEFORM),icon,hotkey)
		,mBrushOP( mBrushData )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TerrainDeformTool::~TerrainDeformTool()
	{

	}


}//namespace Blade
