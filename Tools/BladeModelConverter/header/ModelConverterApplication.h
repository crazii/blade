/********************************************************************
	created:	2015/09/23
	filename: 	ModelConverterApplication.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelConverterApplication_h__
#define __Blade_ModelConverterApplication_h__
#include <ToolApplication.h>
#include <AppConfig.h>
#include <ICmdParser.h>
#include <interface/ILangTable.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	
	class ModelConverterConfig : public AppConfig
	{
	public:
		ModelConverterConfig(const TString& name,const TString &desc,Version ver)
			:AppConfig(name,desc,ver)	
		{
		}

		/* @brief  */
		virtual void		setupAppCmdArgument()
		{
			AppConfig::setupAppCmdArgument();

			ICmdParser::getSingleton().addOption(
				BTString("output"),
				BTString("target file to save the converted file."),
				TEXT('o'),
				CAT_STRING,
				BTString("./"));

			ICmdParser::getSingleton().addOption(
				BTString("skeleton"),
				BTString("write skeleton/animation file if any."),
				TEXT('s'),
				CAT_BOOL,
				BTString("true"));

			ICmdParser::getSingleton().addOption(
				BTString("mesh"),
				BTString("write mesh file if any."),
				TEXT('m'),
				CAT_BOOL,
				BTString("true"));
		}
	};//LangBuildConfig

	class ModelConverterApplication : public ToolApplication
	{
	public:
		ModelConverterApplication()	{}
		~ModelConverterApplication()	{}

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
		typedef TStringMap<TString> SourceImporterMap;	//source extension to importer class name
		typedef TStringMap<SourceImporterMap> FormtMap;	//target extension to SourceImporterMap

		FormtMap mFormatMap;
	};


}//namespace Blade


#endif // __Blade_ModelConverterApplication_h__
