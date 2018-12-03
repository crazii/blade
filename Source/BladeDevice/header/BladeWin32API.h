/********************************************************************
	created:	2015/10/25
	filename: 	Win32API.h
	author:		Crazii
	
	purpose:	wrapper for Win32 to resolve TEXT macro conflict
*********************************************************************/
#ifndef __Blade_Win32API_h__
#define __Blade_Win32API_h__

#include <BladeTypes.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#ifndef _WINDOWS_
#	define NOMINMAX
#	undef TEXT
#	include <Windows.h>
#endif

#endif


#endif // __Blade_Win32API_h__
