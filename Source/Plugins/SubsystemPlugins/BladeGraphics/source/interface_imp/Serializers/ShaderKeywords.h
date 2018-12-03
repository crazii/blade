/********************************************************************
	created:	2014/12/31
	filename: 	ShaderKeywords.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ShaderKeywords_h__
#define __Blade_ShaderKeywords_h__
#include <utility/String.h>

namespace Blade
{

	namespace ShaderKeywords
	{
		static const TString HEADER_MARK = BTString("//!");
		static const TString HEADER_SIGNATURE = BTString("BladeShaderHeader");

		static const TString SHADER_SECTION = BTString("Shader");
		static const TString SUBSHADER_SECTION = BTString("SubShaders");

		static const TString SEMANTIC_SECTION = BTString("Semantics");
		static const TString CUSTOM_SEMANTIC_SECTION = BTString("Custom Smantics");

		static const TString SHADER_NAME = BTString("Name");
		static const TString ENTRY_POINT = BTString("Entry");
		static const TString SHADER_PROFILE = BTString("Profiles");
		static const TString SHADER_OUTPUT = BTString("Output");

		static const TString MACRO_PROFILE = BTString("BLADE_PROFILE");
	}//namespace ShaderKeywords
	
}//namespace Blade


#endif // __Blade_ShaderKeywords_h__