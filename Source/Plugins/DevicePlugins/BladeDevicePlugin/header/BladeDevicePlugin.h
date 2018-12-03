/********************************************************************
	created:	2011/08/13
	filename: 	BladeDevicePlugin.h
	author:		Crazii
	purpose:	
*********************************************************************/

#ifndef __Blade_BladeDevicePlugin_h__
#define __Blade_BladeDevicePlugin_h__
#include <BladePlatform.h>
#include <BladeTypes.h>
#include <BladeException.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_DEVICEPLUGIN_EXPORTS
#			define BLADE_DEVICE_PLUGIN_API __declspec( dllexport )
#		else
#			define BLADE_DEVICE_PLUGIN_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_DEVICE_PLUGIN_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_DEVICE_PLUGIN_API	//static link lib

#endif

#endif // __Blade_BladeDevicePlugin_h__