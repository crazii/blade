/********************************************************************
	created:	2015/10/25
	filename: 	BladeWin32API.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeWin32API_h__
#define __Blade_BladeWin32API_h__

#include <BladeTypes.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	ifndef _WINDOWS_
#		define NOMINMAX
#		if defined(TEXT)
#			undef TEXT
#		endif
#		include <Windows.h>
#	endif
#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM

#endif // __Blade_BladeWin32API_h__