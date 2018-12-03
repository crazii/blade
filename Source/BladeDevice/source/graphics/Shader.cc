/********************************************************************
	created:	2014/12/22
	filename: 	Shader.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/Shader.h>
#include <graphics/ShaderIntermediate.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Shader::Shader(const ShaderIntermediate* intermediate)
	{
		for(int i = SHADER_TYPE_BEGIN; i < SHADER_TYPE_COUNT; ++i)
		{
			EShaderType type = EShaderType(i);
			mProfile[i] = intermediate->getProfile(type);
			mEntryPoint[i] = intermediate->getEntry(type);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Shader::~Shader()
	{

	}

	/************************************************************************/
	/* IShader interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString& Shader::getProfileString(EShaderType type) const
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			return mProfile[type];
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Shader::getEntry(EShaderType type) const
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			return mEntryPoint[type];
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}

	
}//namespace Blade