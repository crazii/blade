/********************************************************************
	created:	2010/01/16
	filename: 	BladeBase.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeBase_h__
#define __Blade_BladeBase_h__
#include <BladeTypes.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_BASE_EXPORTS
#			define BLADE_BASE_API __declspec( dllexport )
#		else
#			define BLADE_BASE_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_BASE_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_BASE_API	//static link lib

#endif

namespace Blade
{

	extern "C"
	{
		BLADE_BASE_API bool	initializeFoundation();

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

		BLADE_BASE_API void BladeAssert(const char* file, int line, const char* func, const char* msg);

#ifndef NDEBUG
#	undef assert
#	define	assert(e)	((e) ? (void)0 : Blade::BladeAssert(__FILE__, __LINE__, BLADE_FUNCTION, #e))
#endif

#endif//BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

	}

	
}//namespace Blade
 
#endif //__Blade_BladeBase_h__
