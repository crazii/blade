/********************************************************************
	created:	2009/02/27
	filename: 	BladeGraphics.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeGraphics_h__
#define __Blade_BladeGraphics_h__
#include <BladePlatform.h>
#include <utility/String.h>
#include <ExceptionEx.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_GRAPHICS_EXPORTS
#			define BLADE_GRAPHICS_API __declspec( dllexport )
#		else
#			define BLADE_GRAPHICS_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_GRAPHICS_API __attribute__ ((visibility("default")))
#	endif

#else
#	define BLADE_GRAPHICS_API	//static link lib
#endif


#include <interface/IGraphicsSystem.h>

#define BLADE_TS_CHECK_GRAPHICS_CONTEXT() (IGraphicsSystem::getSingleton().isGraphiscContext())
//public(access by other systems) reads/writes (changing data) only available in async run state
//public writes will be queued using graphics commands
#define BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS() (assert(BLADE_TS_INCLUDE(TS_ASYNC_RUN) || BLADE_TS_CHECK_GRAPHICS_CONTEXT()))
//write/change data only happens in async update state
#define BLADE_TS_VERIFY_GRAPHICS_WRITE() (assert((BLADE_TS_INCLUDE(TS_ASYNC_UPDATE) && BLADE_TS_CHECK_GRAPHICS_CONTEXT())))


#endif // __Blade_BladeGraphics_h__