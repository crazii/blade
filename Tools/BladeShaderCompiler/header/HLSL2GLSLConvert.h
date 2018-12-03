/********************************************************************
	created:	2015/01/01
	filename: 	HLSL2GLSLConvert.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_HLSL2GLSLConvert_h__
#define __Blade_HLSL2GLSLConvert_h__
#include <ShaderResource.h>
#include <utility/StringHelper.h>
#include <hlsl2glsl/hlsl2glsl.h>
#include <glsl_optimizer/glsl_optimizer.h>
#include <HLSL2GLSLIncludeLoader.h>


namespace Blade
{

	class HLSL2GLSLConvert
	{
	public:
		HLSL2GLSLConvert();
		~HLSL2GLSLConvert()		{this->shutdown();}

		/* @brief  */
		bool	initialize();

		/* @brief  */
		void	shutdown();

		/* @brief  */
		bool	convert(const TString& sourceDir, SubShader* subShader, String& outputContent, ParamList& predefinedMacros);

	protected:
		HLSL2GLSLIncludeLoader	msIncludeLoader;
		glslopt_ctx*	mContext;
		bool			mInited;
	};

	
}//namespace Blade


#endif // __Blade_HLSL2GLSLConvert_h__