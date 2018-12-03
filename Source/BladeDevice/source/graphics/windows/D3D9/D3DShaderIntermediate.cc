/********************************************************************
	created:	2010/08/23
	filename: 	D3DShaderIntermediate.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <BladeWin32API.h>
#include <graphics/windows/D3DShaderIntermediate.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString D3DShaderIntermediate::HLSL_ID = BTString("HLSL");

	//////////////////////////////////////////////////////////////////////////
	D3DShaderIntermediate::D3DShaderIntermediate(const TString& name)
		:ShaderIntermediate(name)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	D3DShaderIntermediate::~D3DShaderIntermediate()
	{
	}

	/************************************************************************/
	/* IShaderIntermediate interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	DWORD				D3DShaderIntermediate::getCompilingFlags(EShaderType type) const
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			return mFlags[type];
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}

	//////////////////////////////////////////////////////////////////////////
	void				D3DShaderIntermediate::setCompilingFlags(EShaderType type, DWORD flags)
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			mFlags[type] = flags;
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}

	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
