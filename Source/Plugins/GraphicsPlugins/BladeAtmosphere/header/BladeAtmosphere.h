/********************************************************************
created:	2011/09/01
filename: 	BladeAtmosphere.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_BladeAtmosphere_h__
#define __Blade_BladeAtmosphere_h__
#include <utility/String.h>
#include <BladeAtmosphere_blang.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADEATMOSPHERE_EXPORTS
#			define BLADE_ATMOSPHERE_API __declspec( dllexport )
#		else
#			define BLADE_ATMOSPHERE_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_ATMOSPHERE_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_ATMOSPHERE_API	//static link lib

#endif


namespace Blade
{

	namespace AtmosphereConsts
	{
		static const TString ATMOSPHERE_ELEMENT_TYPE = BTString("AtmosphereElement");
		static const TString ATMOSPHERE_STAGE_DATA_TYPE = BTString(BLANG_ATMOSPHERE);
		static const TString ATMOSPHERE_SKY_TEXTURE_IMAGE = BTString("Sky Textures");
		static const TString ATMOSPHERE_SKYBOX_TEXTURE_IMAGE = BTString("Sky Cube Textures");
	}//namespace Atmosphere
	
}//namespace Blade



#endif //__Blade_BladeAtmosphere_h__
