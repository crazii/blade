/********************************************************************
	created:	2014/12/24
	filename: 	ShaderIntermediate.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/ShaderIntermediate.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ShaderIntermediate::ShaderIntermediate(const TString& name)
		:mName(name)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ShaderIntermediate::~ShaderIntermediate()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ShaderIntermediate::getProfile(EShaderType type) const
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			return mProfile[type];
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&		ShaderIntermediate::getEntry(EShaderType type) const
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			return mEntry[type];
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				ShaderIntermediate::setProfile(EShaderType type, const TString& profile)
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			mProfile[type] = profile;
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}
	//////////////////////////////////////////////////////////////////////////
	void				ShaderIntermediate::setEntry(EShaderType type, const TString& entry)
	{
		if( type >= SHADER_TYPE_BEGIN && type < SHADER_TYPE_COUNT )
			mEntry[type] = entry;
		else
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid shader type."));
	}


}//namespace Blade