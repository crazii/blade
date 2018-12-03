/********************************************************************
	created:	2010/06/12
	filename: 	BladeWorldEditor.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeWorldEditor_h__
#define __Blade_BladeWorldEditor_h__
#include <utility/String.h>
#include <WorldEditor_blang.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_WORLDEDITOR_EXPORTS
#			define BLADE_WORLDEDITOR_API __declspec( dllexport )
#		else
#			define BLADE_WORLDEDITOR_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_WORLDEDITOR_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_WORLDEDITOR_API	//static link lib

#endif

namespace Blade
{

	//namespace WorldEditor
	static const TString WORLD_FILE_TYPE = BXLang(BLANG_WORLD);
	static const TString WORLD_FILE_EXT = BTString("bstage");

}//namespace Blade



#endif //__Blade_BladeWorldEditor_h__