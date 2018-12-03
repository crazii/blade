/********************************************************************
	created:	2011/08/23
	filename: 	BladeImageFI.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeImageFI_h__
#define __Blade_BladeImageFI_h__

#include <BladePlatform.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADEIMAGEFI_EXPORTS
#			define BLADE_IMAGEFI_API __declspec( dllexport )
#		else
#			define BLADE_IMAGEFI_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_IMAGEFI_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_IMAGEFI_API	//static link lib

#endif



#endif // __Blade_BladeImageFI_h__