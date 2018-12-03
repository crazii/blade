/********************************************************************
	created:	2011/11/02
	filename: 	BladeEditor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeEditor_h__
#define __Blade_BladeEditor_h__
#include <BladeTypes.h>
#include <interface/public/ui/UITypes.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_EDITOR_EXPORTS
#			define BLADE_EDITOR_API __declspec( dllexport )
#		else
#			define BLADE_EDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_EDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_EDITOR_API	//static link lib

#endif


namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	extern "C" BLADE_EDITOR_API bool initializeEditorFramework();

}//namespace Blade

#endif // __Blade_BladeEditor_h__