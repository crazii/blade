/********************************************************************
	created:	2013/04/09
	filename: 	BladeModelViewer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeModelViewer_h__
#define __Blade_BladeModelViewer_h__
#include <utility/String.h>
#include <BladeModel_blang.h>
#include <BladeModelViewer_blang.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_MODELVIEWER_EXPORTS
#			define BLADE_MODELVIEWER_API __declspec( dllexport )
#		else
#			define BLADE_MODELVIEWER_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_MODELVIEWER_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_MODELVIEWER_API	//static link lib

#endif

namespace Blade
{
	static const TString MODEL_FILE_TYPE = BTString(BLANG_MODEL);
}//namespace Blade

#endif //  __Blade_BladeModelViewer_h__