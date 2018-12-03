/********************************************************************
	created:	2015/01/01
	filename: 	HLSL2GLSLConvert.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <HLSL2GLSLConvert.h>
#include <GLSLAttributeTable.h>
#include <ToolLog.h>

namespace Blade
{
	//#include callback for hlsl2glsl
	static bool C_DECL Blade_IncludeOpenFunc(bool /*isSystem*/, const char* fname, std::string& output, void* data)
	{
		Blade::HLSL2GLSLIncludeLoader* loader = (Blade::HLSL2GLSLIncludeLoader*)data;
		return loader->open(fname, output);
	}

	//////////////////////////////////////////////////////////////////////////
	HLSL2GLSLConvert::HLSL2GLSLConvert()
	{
		mContext = NULL;
		mInited = false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	HLSL2GLSLConvert::initialize()
	{
		if( !mInited )
		{
			::Hlsl2Glsl_Initialize();
			mContext = ::glslopt_initialize(kGlslTargetOpenGLES30);
			mInited = true;
		}
		return mInited;
	}

	//////////////////////////////////////////////////////////////////////////
	void	HLSL2GLSLConvert::shutdown()
	{
		if( mInited )
		{
			::Hlsl2Glsl_Shutdown();
			::glslopt_cleanup(mContext);
			mContext = NULL;
			mInited = false;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	bool	HLSL2GLSLConvert::convert(const TString& sourceDir, SubShader* subShader, String& outputContent, ParamList& predefinedMacros)
	{
		if( !mInited )
		{
			assert(false);
			return false;
		}

		ShHandle GLESVS = ::Hlsl2Glsl_ConstructCompiler(EShLangVertex);
		ShHandle GLESFS = ::Hlsl2Glsl_ConstructCompiler(EShLangFragment);

		Hlsl2Glsl_ParseCallbacks callback = {&Blade_IncludeOpenFunc, &msIncludeLoader};
		msIncludeLoader.addPath(sourceDir);

		const TString& vsEntry = subShader->getShader()->getEntry(SHADER_VERTEX);
		const TString& fsEntry = subShader->getShader()->getEntry(SHADER_FRAGMENT);
		const String& originalSource = subShader->getSourceText();
		assert(originalSource != String::EMPTY );

		int result = ::Hlsl2Glsl_SetUserAttributeNames(GLESVS, semantics, variableNames, _countof(semantics) );
		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, ::Hlsl2Glsl_GetInfoLog(GLESVS) );
			return false;
		}

		String defines;
#if 1
		for (size_t i = 0; i < predefinedMacros.size(); ++i)
		{
			const NamedVariant param = predefinedMacros.at(i);
			const TString& paramName = param.getName();
			if (TStringHelper::isStartWith(paramName, BTString("-D")))
			{
				String macro = StringConverter::TStringToString(paramName.substr_nocopy(2));
				String macroVal = StringConverter::TStringToString(param.mValue);
				defines += BString("#define ") + macro + BString(" ") + macroVal + BString("\n");
			}
		}
#else
		BLADE_UNREFERENCED(predefinedMacros);
#endif
		String source = defines + originalSource;

		//convert VS
		String entry = StringConverter::TStringToString(vsEntry);
		result = result && ::Hlsl2Glsl_Parse(GLESVS, source.c_str(), ETargetGLSL_ES_300, &callback, ETranslateOpAvoidBuiltinAttribNames);
		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, ::Hlsl2Glsl_GetInfoLog(GLESVS) );
			return false;
		}

		result = result && ::Hlsl2Glsl_Translate(GLESVS, entry.c_str(), ETargetGLSL_ES_300, ETranslateOpAvoidBuiltinAttribNames);
		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, ::Hlsl2Glsl_GetInfoLog(GLESVS) );
			return false;
		}

		//Note: macros are already in original text
		//pre-pend macros
		String extras = BString("#version 300 es\n");

		//optimize
		String vsText = String( ::Hlsl2Glsl_GetShader(GLESVS) );
		vsText = extras + vsText;
		
		glslopt_shader* optShader = NULL;
		if (true)
		{
			optShader = ::glslopt_optimize(mContext, kGlslOptShaderVertex, vsText.c_str(), kGlslOptionSkipPreprocessor);
			result = result && ::glslopt_get_status(optShader);
			if (!result)
			{
				BLADE_LOGU(ToolLog::Error, ::glslopt_get_log(optShader));
				return false;
			}
			vsText = String(::glslopt_get_output(optShader));
			::glslopt_shader_delete(optShader);
		}

		//StringHelper::replace(vsText, BString("void main"), BString("void ") + entry);
		StringHelper::replace(vsText, BString("#version 300 es\n"), BString("") );

		//convert FS
		entry = StringConverter::TStringToString(fsEntry);
		result = result && ::Hlsl2Glsl_Parse(GLESFS, source.c_str(), ETargetGLSL_ES_300, &callback, ETranslateOpAvoidBuiltinAttribNames|ETranslateOpPropogateOriginalAttribNames);
		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, ::Hlsl2Glsl_GetInfoLog(GLESFS) );
			return false;
		}

		result = result && ::Hlsl2Glsl_Translate(GLESFS, entry.c_str(), ETargetGLSL_ES_300, ETranslateOpAvoidBuiltinAttribNames|ETranslateOpPropogateOriginalAttribNames);
		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, ::Hlsl2Glsl_GetInfoLog(GLESFS) );
			return false;
		}
		//optimize
		String fsText = String( ::Hlsl2Glsl_GetShader(GLESFS) );
		fsText = extras + fsText;

		static const size_t MAX_MRT_COUNT = 4;
		//note: GLSL 3.0 DOESN't have built in gl_FragColor or gl_FragData
		//it uses user defined "out" variable like HLSL.
		//but Hlsl2Glsl have this problem and we have to do it on our own.
		index_t out = fsText.find("void main()");
		if( out != TString::npos )
		{
			//MRT output
			String extraOutput("layout(location=0) out vec4 outBladeColor0;\n");
			StringHelper::replace(fsText, BString("gl_FragColor"), BString("outBladeColor0"));
			for(size_t i = 0; i < MAX_MRT_COUNT; ++i)
			{
				StringStream ss0, ss1;
				ss0.format("gl_FragData[%d]", (int)i);
				ss1.format("outBladeColor%d", (int)i);
				size_t count = StringHelper::replace(fsText, ss0.str(), ss1.str());

				if( i > 0 && count > 0 )
				{
					assert(count == 1);
					ss0.format("layout(location=%d) out vec4 outBladeColor%d;\n", (int)i, (int)i);
					extraOutput += ss0.str();
				}
			}
			fsText = fsText.substr(0, out) + extraOutput + fsText.substr(out);
		}
		optShader = ::glslopt_optimize(mContext, kGlslOptShaderFragment, fsText.c_str(), kGlslOptionSkipPreprocessor);
		result = result && ::glslopt_get_status(optShader);
		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, ::glslopt_get_log(optShader) );
			return false;
		}
		fsText = String( ::glslopt_get_output(optShader) );
		::glslopt_shader_delete(optShader);

		//StringHelper::replace(fsText, BString("void main"), BString("void ") + entry );
		StringHelper::replace(fsText, BString("#version 300 es\n"), BString("") );

		if( !result )
		{
			BLADE_LOGU(ToolLog::Error, BTString("unable to convert HLSL to GLSL ES."));
			return false;
		}
		//outputContent = vsText + BString("\n") + fsText;

		outputContent = BString("#if defined(") + GLSL_VS_SWITCH + BString(")\n") + 
			//BString("invariant gl_Position;\n") +
			vsText + 
			BString("#elif defined(") + GLSL_FS_SWITCH + BString(")\n") + 
			fsText +
			BString("#else\n#error switch not defined.\n#endif\n");

		msIncludeLoader.clearPaths();
		::Hlsl2Glsl_DestructCompiler(GLESVS);
		::Hlsl2Glsl_DestructCompiler(GLESFS);
		return true;
	}
	
}//namespace Blade