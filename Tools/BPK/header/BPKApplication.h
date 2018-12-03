/********************************************************************
	created:	2014/02/15
	filename: 	BPKApplication.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BPKApplication_h__
#define __Blade_BPKApplication_h__
#include <ToolApplication.h>
#include <AppConfig.h>
#include <ICmdParser.h>

namespace Blade
{
	class BPKAppConfig : public AppConfig
	{
	public:
		BPKAppConfig(const TString& name,const TString &desc,Version ver)
			:AppConfig(name,desc,ver)	{}

		/* @brief  */
		virtual void		setupAppCmdArgument()
		{
			AppConfig::setupAppCmdArgument();

			ICmdParser::getSingleton().addOption( 
				BTString("verbose"),
				BTString("show packing file information."),
				TEXT('V'),
				CAT_BOOL,
				BTString("false"));

			ICmdParser::getSingleton().addOption( 
				BTString("progress"),
				BTString("show packaging progress (n%), work with --verbose."),
				TEXT('P'),
				CAT_BOOL,
				BTString("false"));


			ICmdParser::getSingleton().addOption(
				BTString("rootdir"),
				BTString("if input file/folder to pack is relative path, use the root dir as the root of input path."),
				TEXT('r'),
				CAT_STRING,
				BTString("./"));

			ICmdParser::getSingleton().addOption( 
				BTString("append"),
				BTString("enable append mode - reserve original data in package."),
				TEXT('A'),
				CAT_BOOL,
				BTString("false"));

			ICmdParser::getSingleton().addOption(
				BTString("destpath"),
				BTString("add srouce files into specified path in package, when this parameter is unspecicied (default behavior):\
						 \n if input file contains a relative path then use the same path as input file; if input file contains an absolute path put file in root '/' .\
						 \n i.e C:/folder/a.txt will be packed to /a.txt; and folder/a.txt will be added to /folder/a.txt"),
				TEXT('D'),
				CAT_STRING,
				BTString(""));

			ICmdParser::getSingleton().addOption(
				BTString("output"),
				BTString("target file path to output the packed package file."),
				TEXT('o'),
				CAT_STRING,
				BTString("./"));

			ICmdParser::getSingleton().addOption(
				BTString("filter"),
				BTString("filter-out unwanted files. \
					\n	i.e. -filter=*.tmp;*.temp will avoid packing .tmp & .temp files in source folder."),
				TEXT('F'),
				CAT_STRING,
				BTString(""));
		}
	};


	
	class BPKApplication : public ToolApplication
	{
	public:
		BPKApplication();
		~BPKApplication();

	public:

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

#endif //  __Blade_BPKApplication_h__