/********************************************************************
	created:	2015/01/14
	filename: 	TexCompressorApp.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_TexCompressorApp_h__
#define __Blade_TexCompressorApp_h__
#include <ToolApplication.h>
#include <AppConfig.h>
#include <ICmdParser.h>
#include <interface/public/file/IStream.h>
#include <interface/public/graphics/IImage.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class	TexCompressorConfig : public AppConfig
	{
	public:
		TexCompressorConfig(const TString& name,const TString &desc,Version ver)
			:AppConfig(name,desc,ver)	{}

		/* @brief  */
		virtual void		setupAppCmdArgument()
		{
			AppConfig::setupAppCmdArgument();

			ICmdParser::getSingleton().addOption(
				BTString("target"),
				BTString("set output texture target - which's deciding output extension"),
				TEXT('T'),
				CAT_STRING,
				BTString("Win32"));

			ICmdParser::getSingleton().addOption(
				BTString("filter"),
				BTString("input file extension filter."),
				TEXT('f'),
				CAT_STRING,
				BTString("PNG,BMP,DDS,TGA"));

			ICmdParser::getSingleton().addOption(
				BTString("recursive"),
				BTString("recursively process input folders."),
				TEXT('R'),
				CAT_BOOL,
				BTString("false"));

			ICmdParser::getSingleton().addOption(
				BTString("format"),
				BTString("compressed file format."),
				TEXT('F'),
				CAT_STRING,
				BTString("DDS"));

			ICmdParser::getSingleton().addOption(
				BTString("normalmap"),
				BTString("compressed file format."),
				TEXT('N'),
				CAT_BOOL,
				BTString("false"));

			ICmdParser::getSingleton().addOption(
				BTString("mipmaps"),
				BTString("set mip map count, -1 = full mipmap chain, 0 = count in original file."),
				TEXT('M'),
				CAT_INT,
				BTString("-1"));

			ICmdParser::getSingleton().addOption(
				BTString("scale"),
				BTString("set scale factor to scale the ouput.."),
				TEXT('S'),
				CAT_FLOAT,
				BTString("1.0"));

			ICmdParser::getSingleton().addOption(
				BTString("verbose"),
				BTString("print file names."),
				TEXT('V'),
				CAT_BOOL,
				BTString("false"));

			ICmdParser::getSingleton().addOption(
				BTString("output"),
				BTString("output folder to save the compressed texture."),
				TEXT('o'),
				CAT_STRING,
				BTString("./"));

			//ICmdParser::getSingleton().addOption(
			//	BTString("quality"),
			//	BTString("compression quality."),
			//	TEXT('Q'),
			//	CAT_STRING,
			//	BTString("low"));
		}
	};//ClientConfig

	//////////////////////////////////////////////////////////////////////////
	class TexCompressorApp : public ToolApplication
	{
	public:
		class TexConfig
		{
		public:
			TexConfig() {}

			TexConfig(EPixelFormat fmt, IMG_ORIENT orient)
			{
				format = fmt;
				orientation = orient;
			}
			PixelFormat format;
			IMG_ORIENT	orientation;
		};

		typedef Map<EPixelFormat, EPixelFormat> PixelFmtMap;
		struct TexConversion
		{
			TexConfig mDefaultConfig;
			PixelFmtMap mColor2RGBAMap;
			PixelFmtMap mColor2NormalMap;
			PixelFmtMap mColor2AlphaMap;

			TexConversion(EPixelFormat fmt, IMG_ORIENT orient) :mDefaultConfig(fmt, orient) {}
		};

	public:
		TexCompressorApp();
		~TexCompressorApp();

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
		/*
		@describe 
		@param [in] filePrefix: output file base without extension, may have relative path prefix
		@return
		*/
		bool	compressFile(const HSTREAM& stream, const TString& filePrefix, const TexConversion& conversion, const TString& fileExt, int mipmapCount, float scale, bool normalMap);

		typedef TStringMap<TexConversion*> TargetTexConfig;

		TexConversion mBCConfig;
		TexConversion mETCConfig;
		TargetTexConfig mTargetConfig;
	};
	
}//namespace Blade


#endif // __Blade_TexCompressorApp_h__