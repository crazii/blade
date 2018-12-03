/********************************************************************
	created:	2017/1/24
	filename: 	BladePostFX.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladePostFX_h__
#define __Blade_BladePostFX_h__
#include <utility/String.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_POSTFX_EXPORTS
#			define BLADE_POSTFX_API __declspec( dllexport )
#		else
#			define BLADE_POSTFX_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_POSTFX_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_POSTFX_API	//static link lib

#endif

#endif//__Blade_BladePostFX_h__