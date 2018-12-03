/********************************************************************
created:	2010/04/28
filename: 	BladeUI.h
author:		Crazii

purpose:	
*********************************************************************/
#ifndef __Blade_BladeUI_h__
#define __Blade_BladeUI_h__
#include <BladePlatform.h>
#include <utility/String.h>
#include <interface/public/ISubsystem.h>
#include <ExceptionEx.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_UI_EXPORTS
#			define BLADE_UI_API __declspec( dllexport )
#		else
#			define BLADE_UI_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_UI_API __attribute__ ((visibility("default")))
#	endif

#else
#	define BLADE_UI_API	//static link lib
#endif

#endif //__Blade_BladeUI_h__