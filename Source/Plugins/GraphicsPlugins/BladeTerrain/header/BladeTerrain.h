/********************************************************************
	created:	2010/05/19
	filename: 	BladeTerrain.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeTerrain_h__
#define __Blade_BladeTerrain_h__
#include <utility/String.h>
#include <BladeTerrain_blang.h>


#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_TERRAIN_EXPORTS
#			define BLADE_TERRAIN_API __declspec( dllexport )
#		else
#			define BLADE_TERRAIN_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_TERRAIN_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_TERRAIN_API	//static link lib

#endif


namespace Blade
{
	namespace TerrainConsts
	{
		static const TString TERRIAN_PLUGIN_NAME = BTString("BladeTerrainPlugin");
		static const TString TERRAIN_ELEMENT = BXLang(BLANG_TERRAIN_ELEMENT);
		static const TString TERRAIN_MEDIA_IMAGE_HINT = BTString("Terrain Textures");

	}//namespace Terrain
	
	//block index helper for internal & client use.
	typedef struct SBlockIndex
	{
		uint32  mIndex;		//linear index: z*row + x
		uint16	mX;
		uint16	mZ;
		SBlockIndex()	{}
		SBlockIndex(uint16 x,uint16 z) :mX(x),mZ(z)	{}
	}BLOCK_INDEX;

	//terrain layer data
	typedef uint8 LAYER_ID;
	static const uint8 INVALID_LAYER_ID = 0xFF;


	//LOD difference (1 level) between blocks
	typedef enum ELODDifferenceIndex
	{
		LODDI_NONE = 0,

		//one neighbor diff
		LODDI_UP = 1,
		LODDI_LEFT = 2,
		LODDI_DOWN = 4,
		LODDI_RIGHT = 8,

		//two neighbor diff
		LODDI_UPLEFT = 3,
		LODDI_UPDOWN = 5,
		LODDI_UPRIGHT = 9,

		LODDI_LEFT_DOWN = 6,
		LODDI_LEFT_RIGHT = 10,

		LODDI_DOWN_RIGHT = 12,

		//three neighbor diff
		LODDI_LEFT_DOWN_RIGHT = 14,
		LODDI_UPDOWNRIGHT = 13,
		LODDI_UPLEFTRIGHT = 11,
		LODDI_UPLEFTDOWN = 7,

		LODDI_ALL = 15,

		LODDI_COUNT = 16,
	}LOD_DI;

}//namespace Blade


#endif //__Blade_BladeTerrain_h__
