/********************************************************************
	created:	2010/06/06
	filename: 	GameLibrary.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GameLibrary_h__
#define __Blade_GameLibrary_h__
#include <BladePlatform.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_GAME_LIBRARY_EXPORTS
#			define BLADE_GAME_LIBRARY_API __declspec( dllexport )
#		else
#			define BLADE_GAME_LIBRARY_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_GAME_LIBRARY_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_GAME_LIBRARY_API	//static link lib

#endif


namespace Blade
{

	void BLADE_GAME_LIBRARY_API initializeGameLibrary();
	
}//namespace Blade

#endif //__Blade_GameLibrary_h__