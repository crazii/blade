/********************************************************************
	created:	2011/04/13
	filename: 	BladeTerrainEditor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeTerrainEditor_h__
#define __Blade_BladeTerrainEditor_h__
#include <BladePlatform.h>


#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_TERRAINEDITOR_EXPORTS
#			define BLADE_TERRAINEDITOR_API __declspec( dllexport )
#		else
#			define BLADE_TERRAINEDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_TERRAINEDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_TERRAINEDITOR_API	//static link lib

#endif


#endif // __Blade_BladeTerrainEditor_h__