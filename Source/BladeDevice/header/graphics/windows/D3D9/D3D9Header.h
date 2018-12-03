/********************************************************************
	created:	2012/04/25
	filename: 	D3D9Header.h
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladeTypes.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#ifndef __Blade_D3D9Header_h__
#define __Blade_D3D9Header_h__

#include <BladeWin32API.h>

#if BLADE_DEBUG
#	define D3D_DEBUG_INFO 1
#endif

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4917) //a GUID can only be associated with a class, interface or namespace
#endif


#include <d3dcompiler.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "../dxerr.h"

namespace Blade
{

	template<typename T>
	class ComPtr : public NonCopyable, public NonAssignable
	{
	public:
		ComPtr(T* p) :mPtr(p) {}
		~ComPtr() { if (mPtr != NULL) mPtr->Release(); }
	protected:
		T* mPtr;
	};
	
}//namespace Blade

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif


#endif //__Blade_D3D9Header_h__


#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS