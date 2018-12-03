/********************************************************************
	created:	2015/08/17
	filename: 	BladeBPKExplorer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeBPKExplorer_h__
#define __Blade_BladeBPKExplorer_h__


#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_BPKEXPLORER_EXPORTS
#			define BLADE_BPKEXPLORER_API __declspec( dllexport )
#		else
#			define BLADE_BPKEXPLORER_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_BPKEXPLORER_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_BPKEXPLORER_API	//static link lib

#endif


#endif // __Blade_BladeBPKExplorer_h__