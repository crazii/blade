/********************************************************************
	created:	2015/08/11
	filename: 	BladeLangEditor.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeLangEditor_h__
#define __Blade_BladeLangEditor_h__
#include <BladePlatform.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_LANGEDITOR_EXPORTS
#			define BLADE_LANGEDITOR_API __declspec( dllexport )
#		else
#			define BLADE_LANGEDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_LANGEDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_LANGEDITOR_API	//static link lib

#endif



#endif // __Blade_BladeLangEditor_h__