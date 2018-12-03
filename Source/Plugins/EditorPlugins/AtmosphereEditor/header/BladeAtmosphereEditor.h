/********************************************************************
created:	2010/09/03
filename: 	BladeAtmosphereEditorEditor.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_BladeAtmosphereEditor_h__
#define __Blade_BladeAtmosphereEditor_h__
#include <BladePlatform.h>


#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_ATMOSPHEREEDITOR_EXPORTS
#			define BLADE_ATMOSPHERE_EDITOR_API __declspec( dllexport )
#		else
#			define BLADE_ATMOSPHERE_EDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_ATMOSPHERE_EDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_ATMOSPHERE_EDITOR_API	//static link lib

#endif

#endif //__Blade_BladeAtmosphereEditor_h__
