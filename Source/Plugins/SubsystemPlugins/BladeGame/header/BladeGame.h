/********************************************************************
	created:	2010/04/28
	filename: 	BladeGame.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeGame_h__
#define __Blade_BladeGame_h__
#include <BladePlatform.h>
#include <utility/String.h>
#include <interface/public/ISubsystem.h>
#include <ExceptionEx.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_GAME_EXPORTS
#			define BLADE_GAME_API __declspec( dllexport )
#		else
#			define BLADE_GAME_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_GAME_API __attribute__ ((visibility("default")))
#	endif

#else
#	define BLADE_GAME_API	//static link lib
#endif

namespace Blade
{
	namespace GameConsts
	{

		static const TString GAME_SCENE_TYPE = BTString("GameScene");

	}//namespace GameConsts
}


#endif //__Blade_BladeGame_h__