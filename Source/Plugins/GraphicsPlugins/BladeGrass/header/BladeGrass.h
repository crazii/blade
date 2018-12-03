/********************************************************************
created:	2010/05/09
filename: 	BladeGrass.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_BladeGrass_h__
#define __Blade_BladeGrass_h__
#include <utility/String.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_GRASS_EXPORTS
#			define BLADE_GRASS_API __declspec( dllexport )
#		else
#			define BLADE_GRASS_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_GRASS_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_GRASS_API	//static link lib

#endif


namespace Blade
{

	namespace GrassConsts
	{
		//element type
		const TString GRASS_ELEMENT_TYPE = BTString("GrassElement");

		//resource types
		const TString GRASS_RESOURCE_TYPE = BTString("GrassResource");

		//media type hint
		const TString GRASS_MODEL_HINT = BTString("Grass Model");

		//resource parameters
		const TString GRASS_PARAM_SIZE = BTString("Size");		//uint32 shared density size in world units. used only on creating a resource
		const TString GRASS_PARAM_X = BTString("OffsetX");		//uint32 load offset x in world units (meters)
		const TString GRASS_PARAM_Z = BTString("OffsetZ");		//uint32 load offset z in world units (meters)
		const TString GRASS_PARAM_RANGE = BTString("Range");	//uint32 loading range in world units
		const TString GRASS_PARAM_SHARED_DATA = BTString("SharedResource");	//TString shared density resource file
		const TString GRASS_PARAM_MESH_COUNT = BTString("MeshCount");	//uint32 grass mesh count, max to GrassConfig::MAX_LAYERS
		const TString GRASS_PARAM_MESH_PREFIX = BTString("Mesh_");	//TString param name prefix, real param name is prefix + i
		
	}//namespace GrassConsts

}//namespace Blade

#endif//__Blade_BladeGrass_h__