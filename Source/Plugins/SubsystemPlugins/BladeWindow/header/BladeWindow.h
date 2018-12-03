/********************************************************************
	created:	2009/02/27
	created:	27:2:2009   8:32
	filename: 	BladeWindow.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeWindow_h__
#define __Blade_BladeWindow_h__
#include <BladePlatform.h>
#include <utility/String.h>
#include <interface/public/window/IWindowdevice.h>
#include <interface/public/window/IWindowService.h>
#include <interface/public/ISubsystem.h>
#include <ExceptionEx.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_WINDOW_EXPORTS
#			define BLADE_WINDOW_API __declspec( dllexport )
#		else
#			define BLADE_WINDOW_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_WINDOW_API __attribute__ ((visibility("default")))
#	endif

#else
#	define BLADE_WINDOW_API	//static link lib
#endif

#endif // __Blade_BladeWindow_h__