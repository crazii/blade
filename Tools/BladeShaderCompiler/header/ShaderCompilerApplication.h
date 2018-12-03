/********************************************************************
	created:	2013/05/19
	filename: 	ShaderCompilerApplication.h
	author:		Crazii
	purpose:	
*********************************************************************/
#include <ToolApplication.h>
#include <AppConfig.h>
#include <ICmdParser.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	class ShaderCompilerConfig : public AppConfig
	{
	public:
		ShaderCompilerConfig(const TString& name,const TString &desc,Version ver)
			:AppConfig(name,desc,ver)	{}

		/* @brief  */
		virtual void		setupAppCmdArgument()
		{
			AppConfig::setupAppCmdArgument(); 

			ICmdParser::getSingleton().addOption(
				BTString("target"),
				BTString("set output shader target type"),
				TEXT('T'),
				CAT_STRING,
				BTString("Win32"));

			ICmdParser::getSingleton().addOption(
				BTString("api"),
				BTString("set shader api. for specific platform, there may be multiple apis available, i.e. VULKAN & D3D for windows, VULKAN & GLES for Android,\n\
					VULKAN, GLES, METAL for iOS, ett."),
				TEXT('i'),
				CAT_STRING,
				BTString(""));

			ICmdParser::getSingleton().addOption( 
				BTString("include"),
				BTString("set compiling include path."),
				TEXT('I'),
				CAT_STRING,
				BTString("./"));

			ICmdParser::getSingleton().addOption(
				BTString("output"),
				BTString("target file path to output the compiled shader object."),
				TEXT('o'),
				CAT_STRING,
				BTString("./"));

			ICmdParser::getSingleton().addOption(
				BTString("optimize"),
				BTString("shader compiling optimization level."),
				TEXT('O'),
				CAT_INT,
				BTString("0"));
		}
	};//ClientConfig

	class ShaderCompilerApplication : public ToolApplication
	{
	public:
		ShaderCompilerApplication();
		~ShaderCompilerApplication()	{}

		/*
		@describe run the application
		@param [in] bModal whether in modal mode(internal loop ) or just run for once
		@return run result
		@retval true run succeeded
		@retval false run failed
		@remark this will call beginRun at first,and endRun last
		*/
		virtual bool	run(bool bModal);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	doInitialize();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void	initPlugins();

	protected:
		typedef StaticTStringMap<TStringList> TargetAPIList;
		TargetAPIList mSupportedAPI;
	};
	
}//namespace Blade