/********************************************************************
	created:	2013/05/19
	filename: 	ShaderCompilerApplication.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <ShaderCompilerApplication.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <BladeStdTypes.h>
#include <interface/ILog.h>
#include <ShaderResource.h>
#include <utility/StringHelper.h>
#include <interface/IFramework.h>
//3rd party dependencies
#include <hlsl2glsl/hlsl2glsl.h>
#include <glsl_optimizer/glsl_optimizer.h>
#include <HLSL2GLSLConvert.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ShaderCompilerApplication::ShaderCompilerApplication()
	{
		TStringList& win32 = mSupportedAPI[BTString("Win32")];
		win32.push_back(BTString("D3D"));
		win32.push_back(BTString("VULKAN"));

		TStringList& x64 = mSupportedAPI[BTString("x64")];
		x64.push_back(BTString("D3D"));
		x64.push_back(BTString("VULKAN"));

		TStringList& android = mSupportedAPI[BTString("Android")];
		android.push_back(BTString("VULKAN"));
		android.push_back(BTString("GLES"));

		TStringList& iOS = mSupportedAPI[BTString("iOS")];
		iOS.push_back(BTString("VULKAN"));
		iOS.push_back(BTString("GLES"));
		iOS.push_back(BTString("METAL"));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderCompilerApplication::run(bool /*bModal*/)
	{
		const TStringList& files = ICmdParser::getSingleton().getFileList();
		const TString& output = ICmdParser::getSingleton().getOption(BTString("output"))->getValue();
		const TString& includePath = ICmdParser::getSingleton().getOption(BTString("include"))->getValue();
		const TString& target = ICmdParser::getSingleton().getOption(BTString("target"))->getValue();
		const TString& api = ICmdParser::getSingleton().getOption(BTString("api"))->getValue();
		int optimization = ICmdParser::getSingleton().getOption( BTString("optimize") )->getValue();

		TargetAPIList::iterator iter = mSupportedAPI.find(target);
		if (iter == mSupportedAPI.end())
		{
			BLADE_LOGU(ToolLog::Error, TEXT("error: target platform not specified."));
			return false;
		}
		const TStringList& apiList = iter->second;

		if (api.empty())
		{
			BLADE_LOGU(ToolLog::Error, TEXT("error: api not specified."));
			return false;
		}
		if (apiList.find(api) == INVALID_INDEX)
		{
			BLADE_LOGU(ToolLog::Error, TEXT("error: api not supported on target platform: ") << target.c_str() << TEXT("."));
			return false;
		}

		ParamList shaderParam;
		shaderParam[BTString("OptimizationLevel")] = (int)0;
		shaderParam[ BTString("IncludePath") ] = includePath;

		TString ResourceType;
		TString LoaderType;
		TString	SaverType;
		HRESOURCE resource;

		shaderParam[BTString("-DBLADE_PLATFORM_WINDOWS")] = TStringHelper::fromUInt( BLADE_PLATFORM_WINDOWS );
		shaderParam[BTString("-DBLADE_PLATFORM_WINPHONE")] = TStringHelper::fromUInt( BLADE_PLATFORM_WINPHONE );
		shaderParam[BTString("-DBLADE_PLATFORM_LINUX")] = TStringHelper::fromUInt( BLADE_PLATFORM_LINUX );
		shaderParam[BTString("-DBLADE_PLATFORM_ANDROID")] = TStringHelper::fromUInt( BLADE_PLATFORM_ANDROID );
		shaderParam[BTString("-DBLADE_PLATFORM_IOS")] = TStringHelper::fromUInt( BLADE_PLATFORM_IOS );
		shaderParam[BTString("-DBLADE_PLATFORM_MAC")] = TStringHelper::fromUInt( BLADE_PLATFORM_MAC );

		//profile macro definitions
		uint profileNum = 0;
		shaderParam[BTString("-Dprofile_1_1")] = TStringHelper::fromUInt(profileNum++);
		shaderParam[BTString("-Dprofile_2_0")] = TStringHelper::fromUInt(profileNum++);
		shaderParam[BTString("-Dprofile_2_a")] = TStringHelper::fromUInt(profileNum++);
		shaderParam[BTString("-Dprofile_2_b")] = TStringHelper::fromUInt(profileNum++);
		shaderParam[BTString("-Dprofile_3_0")] = TStringHelper::fromUInt(profileNum++);
		shaderParam[BTString("-Dprofile_4_0")] = TStringHelper::fromUInt(profileNum++);
		shaderParam[BTString("-Dprofile_5_0")] = TStringHelper::fromUInt(profileNum++);

		HLSL2GLSLConvert converter;

		if( target == BTString("Win32") || target == BTString("x64") )
		{
			ResourceType = BTString("Shader");
			LoaderType = BTString("SourceShader");
			SaverType = BTString("Shader");
			shaderParam[ BTString("-DBLADE_PLATFORM") ] = BTString("BLADE_PLATFORM_WINDOWS");
			shaderParam[BTString("OptimizationLevel")] = optimization;
		}
		else if( target == BTString("Android") || target == BTString("iOS") )
		{
			ResourceType = BTString("Shader");
			LoaderType = BTString("SourceShader");
			SaverType = BTString("HybridShader");
			converter.initialize();
		}
		else
		{
			BLADE_LOGU(ToolLog::Error, BTString("invalid paltform:") + target);
			return false;
		}

		bool bOutputPath = output[output.size()-1] == TEXT('\\') || output[output.size()-1] == TEXT('/');
		for(size_t i = 0; i < files.size(); ++i)
		{
			TString dir,filename,base,ext;
			TString fullpath = files[i];

			TStringHelper::getFilePathInfo(fullpath, dir, filename);
			TStringHelper::getFileInfo(filename, base, ext);
			if(dir == TString::EMPTY)
				dir = BTString(".");

			//note: it is possible that we add text only serializer and do not compile d3d shader for other platforms,
			//thus speed up the shader compiling process, but it will skip error checking for d3d if not built for win32/d3d
			resource = IResourceManager::getSingleton().loadResourceSync(ResourceType, files[i], LoaderType, &shaderParam);
			if( this->hasError() )
				break;

			if( resource == NULL )
				continue;

			TString path = output;
			if( bOutputPath )
				path += base + BTString(".blshader");

			shaderParam.clear();

			//hard coded.  TODO: better ways?
			if( target != BTString("Win32") && target != BTString("x64") )
			{
				shaderParam[BTString("-DBLADE_PLATFORM")] = BTString("BLADE_PLATFORM_") + TStringHelper::getUpperCase(target);
				shaderParam[BTString("-DBLADE_") + api] = BTString("1");

				//convert HLSL source to GLSL
				ShaderResource* shaderRes = static_cast<ShaderResource*>(resource);
				size_t profileCount = shaderRes->getProfileCount();
				for(size_t j = 0; j < profileCount; ++j)
				{
					const TString& profile = shaderRes->getProfileName(j);
					size_t count = shaderRes->getSubShaderCount(profile);
					SubShader* subShaders = shaderRes->getSubShaders(profile);
					for(size_t k = 0; k < count; ++k)
					{
						SubShader* subShader = subShaders + k;
						String fullContent;
						if (!converter.convert(dir, subShader, fullContent, shaderParam))
						{
							BLADE_LOGU(ToolLog::Error, BTString("error: failed to compile sub shader: ") << subShader->getName()
								<< BTString(", profile: ") << profile
								<< BTString("."));
							IResourceManager::getSingleton().unloadResource(resource);
							return false;
						}
						subShader->setSourceText(fullContent);
					}//for each sub shader

				}
			}//GLESES

			IResourceManager::getSingleton().saveResourceSync(resource, path, false, SaverType);

			IResourceManager::getSingleton().unloadResource(resource);
		} //for each input 

		return this->hasError();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ShaderCompilerApplication::doInitialize()
	{
		IFramework& fm = IFramework::getSingleton();
		fm.addSubSystem(BTString("BladeGraphicsSystem"));
		fm.addSubSystem(BTString("BladeWindowSystem"));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ShaderCompilerApplication::initPlugins()
	{
		ParamList params;
		params[BTString("BladeImageFI")] = true;
		params[BTString("BladeDevice")] = true;
		params[BTString("BladeWindow")] = true;
		params[BTString("BladeGraphics")] = true;
		//UI system for configs if needed
		params[BTString("BladeUI")] = true;
		params[BTString("BladeUIPlugin")] = true;
		IPlatformManager::getSingleton().initialize(params, IResourceManager::getSingleton().schemePathToRealPath(BTString("plugins:")));
	}
}//namespace Blade