/********************************************************************
	created:	2009/02/22
	filename: 	BladeDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeDevice_h__
#define __Blade_BladeDevice_h__
#include <BladePlatform.h>
#include <BladeException.h>

#if !defined(BLADE_STATIC) && defined(BLADE_DEVICE_EXPORTS) //dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_DEVICE_EXPORTS
#			define BLADE_DEVICE_API __declspec( dllexport )
#		else
#			define BLADE_DEVICE_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_DEVICE_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_DEVICE_API	//static link lib

#endif

#endif // __Blade_BladeDevice_h__