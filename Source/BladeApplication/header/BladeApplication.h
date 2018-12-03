/********************************************************************
	created:	2010/04/02
	filename: 	BladeApplication.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeApplication_h__
#define __Blade_BladeApplication_h__

#include <BladePlatform.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_APPLICATION_EXPORTS
#			define BLADE_APPLICATION_API __declspec( dllexport )
#		else
#			define BLADE_APPLICATION_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_APPLICATION_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_APPLICATION_API	//static link lib

#endif

#endif //__Blade_BladeApplication_h__