/********************************************************************
	created:	2014/12/26
	filename: 	GLESShaderIntermediate.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESShaderIntermediate.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const TString	GLESShaderIntermediate::GLES_SHADER_ID = BTString("GLES");

	//////////////////////////////////////////////////////////////////////////
	GLESShaderIntermediate::GLESShaderIntermediate(const TString& name, GLuint program/* = 0*/)
		:ShaderIntermediate(name)
		,mProgram(program)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GLESShaderIntermediate::~GLESShaderIntermediate()
	{
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES