/********************************************************************
	created:	2013/10/30
	filename: 	LangBuildApplication.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LangBuildApplication_h__
#define __Blade_LangBuildApplication_h__
#include <ToolApplication.h>
#include <AppConfig.h>
#include <ICmdParser.h>
#include <interface/ILangTable.h>

namespace Blade
{
	class LangBuildConfig : public AppConfig
	{
	public:
		LangBuildConfig(const TString& name,const TString &desc,Version ver)
			:AppConfig(name,desc,ver)	
		{
		}

		/* @brief  */
		virtual void		setupAppCmdArgument()
		{
			AppConfig::setupAppCmdArgument();

			ICmdParser::getSingleton().addOption( 
				BTString("lang"),
				BTString("set output language (file name only) to build."),
				TEXT('L'),
				CAT_STRING,
				ILangTableManager::DEFAULT_LANG);

			ICmdParser::getSingleton().addOption(
				BTString("output"),
				BTString("target file folder to save the build language file."),
				TEXT('o'),
				CAT_STRING,
				BTString("./"));

			ICmdParser::getSingleton().addOption(
				BTString("skip"),
				BTString("skip parse any folders with specific names, multpile folder names can be seperated by ';'."),
				TEXT('s'),
				CAT_STRING,
				BTString(""));
		}
	};//LangBuildConfig

	class LangBuildApplication : public ToolApplication
	{
	public:
		LangBuildApplication()	{}
		~LangBuildApplication()	{}

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

	};
	
}//namespace Blade

#endif //  __Blade_LangBuildApplication_h__