/********************************************************************
	created:	2018/05/27
	filename: 	BladeGrassEditor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeGrassEditor_h__
#define __Blade_BladeGrassEditor_h__
#include <BladePlatform.h>


#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_GRASSEDITOR_EXPORTS
#			define BLADE_GRASSEDITOR_API __declspec( dllexport )
#		else
#			define BLADE_GRASSEDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_GRASSEDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_GRASSEDITOR_API	//static link lib

#endif


#endif // __Blade_BladeGrassEditor_h__