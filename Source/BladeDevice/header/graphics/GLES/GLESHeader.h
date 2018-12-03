/********************************************************************
	created:	2014/12/20
	filename: 	GLESHeader.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESHeader_h__
#define __Blade_GLESHeader_h__

#include <BladePlatform.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID || BLADE_PLATFORM == BLADE_PLATFORM_IOS
#define BLADE_USE_GLES 1
#elif !defined(BLADE_USE_GLES)
#define BLADE_USE_GLES 0
#endif


#if BLADE_USE_GLES

//config
#define BLADE_GLES_SHARED_CONTEXT 0	//experimental, not working (not needed anymore)

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#else
//TODO: iOS
#endif

#endif//BLADE_USE_GLES


#endif // __Blade_GLESHeader_h__