/********************************************************************
	created:	2010/03/30
	filename: 	BladeUIPlugin.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeUIPlugin_h__
#define __Blade_BladeUIPlugin_h__


#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_UIPLUGIN_EXPORT
#			define BLADE_UIPLUGIN_API __declspec( dllexport )
#		else
#			define BLADE_UIPLUGIN_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_UIPLUGIN_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_UIPLUGIN_API	//static link lib

#endif

#endif //__Blade_BladeUIPlugin_h__