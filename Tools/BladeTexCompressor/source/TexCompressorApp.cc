/********************************************************************
	created:	2015/01/14
	filename: 	TexCompressorApp.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <TexCompressorApp.h>
#include <BladeStdTypes.h>
#include <interface/IFramework.h>
#include <interface/IPlatformManager.h>
#include <interface/IResourceManager.h>
#include <interface/public/graphics/IImageManager.h>
#include <utility/StringHelper.h>
#include <utility/StringList.h>
#include <math/BladeMath.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TexCompressorApp::TexCompressorApp()
		:mBCConfig(PF_BC1, IMGO_TOP_DOWN)
		,mETCConfig(PF_RGB_ETC2, IMGO_BOTTOM_UP)
	{
		mBCConfig.mColor2RGBAMap[PF_RGB_ETC2] = PF_BC1;
		mBCConfig.mColor2RGBAMap[PF_BC1] = PF_BC3;
		mBCConfig.mColor2RGBAMap[PF_BC2] = PF_BC2;
		mBCConfig.mColor2RGBAMap[PF_BC3] = PF_BC3;
		mBCConfig.mColor2RGBAMap[PF_RGBA_ETC2EAC] = PF_BC3;

		mBCConfig.mColor2NormalMap[PF_RGB_ETC2] = PF_BC5;
		mBCConfig.mColor2NormalMap[PF_BC1] = PF_BC5;
		mBCConfig.mColor2NormalMap[PF_BC5] = PF_BC5;

		mBCConfig.mColor2AlphaMap[PF_RGB_ETC2] = PF_BC3;
		mBCConfig.mColor2AlphaMap[PF_BC1] = PF_BC4;


		mETCConfig.mColor2RGBAMap[PF_RGB_ETC2] = PF_RGBA_ETC2EAC;
		mETCConfig.mColor2RGBAMap[PF_BC1] = PF_RGBA_ETC2EAC;
		mETCConfig.mColor2RGBAMap[PF_BC2] = PF_RGBA_ETC2EAC;
		mETCConfig.mColor2RGBAMap[PF_BC3] = PF_RGBA_ETC2EAC;
		mETCConfig.mColor2RGBAMap[PF_RGBA_ETC2EAC] = PF_RGBA_ETC2EAC;

		mETCConfig.mColor2NormalMap[PF_RG_EAC] = PF_RG_EAC;
		mETCConfig.mColor2NormalMap[PF_RGB_ETC2] = PF_RG_EAC;
		mETCConfig.mColor2NormalMap[PF_BC1] = PF_RG_EAC;

		mETCConfig.mColor2AlphaMap[PF_RGB_ETC2] = PF_R_EAC;
		mETCConfig.mColor2AlphaMap[PF_BC1] = PF_R_EAC;

		mTargetConfig[BTString("Win32")] = &mBCConfig;
		mTargetConfig[BTString("x86")] = &mBCConfig;
		mTargetConfig[BTString("x64")] = &mBCConfig;

		mTargetConfig[BTString("Android")] = &mETCConfig;
		mTargetConfig[BTString("iOS")] = &mETCConfig;
	}

	//////////////////////////////////////////////////////////////////////////
	TexCompressorApp::~TexCompressorApp()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TexCompressorApp::run(bool /*bModal*/)
	{
		const TStringList& files = ICmdParser::getSingleton().getFileList();
		const TString& output = ICmdParser::getSingleton().getOption(BTString("output"))->getValue();
		const TString& target = ICmdParser::getSingleton().getOption(BTString("target"))->getValue();
		//const TString& quality = ICmdParser::getSingleton().getOption(BTString("quality"))->getValue();
		const TString& filter = ICmdParser::getSingleton().getOption(BTString("filter"))->getValue();
		int mipmapCount = ICmdParser::getSingleton().getOption(BTString("mipmaps"))->getValue();
		bool normalMap = ICmdParser::getSingleton().getOption(BTString("normalmap"))->getValue();
		bool recursive = ICmdParser::getSingleton().getOption(BTString("recursive"))->getValue();
		scalar scale = ICmdParser::getSingleton().getOption(BTString("scale"))->getValue();
		bool verbose = ICmdParser::getSingleton().getOption(BTString("verbose"))->getValue();
		const TString& _format = ICmdParser::getSingleton().getOption(BTString("format"))->getValue();
		TString format = TStringHelper::getLowerCase(_format);
		

		TStringTokenizer filters;
		filters.tokenize(filter, TEXT(","));
		if( filters.size() == 0 )
			return false;

		//check target
		const TexConversion* conversion = NULL;
		{
			TargetTexConfig::const_iterator configIter = mTargetConfig.find( target );
			if( configIter == mTargetConfig.end() )
			{
				std::tcout << TEXT("error: un-supported target:") << target.c_str() << std::endl;
				return false;
			}
			conversion = configIter->second;
		}

		if( scale <= 0 )
		{
			std::tcout << TEXT("error: invalid parameter scale:") << scale << std::endl;
			return false;
		}

		#if 0
		//check output folder (root must exists)
		{
			HFILEDEVICE outFolder = IResourceManager::getSingleton().loadFolder(output);
			if( outFolder == NULL || !outFolder->isLoaded() )
			{
				std::tcout << TEXT("error: unable to open output folder:") << output.c_str() << std::endl;
				return false;
			}
		}
		#endif

		int findFlag = recursive ? (IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE) : IFileDevice::FF_FILE;

		for(size_t i = 0; i < files.size(); ++i)
		{
			const TString& file = files[i];
			bool isFolder = false;

			//check dir
			HFILEDEVICE folder = IResourceManager::getSingleton().loadFolder(file);
			if( folder != NULL && folder->isLoaded() )
			{
				isFolder = true;
				TStringParam fileList;
				for(size_t j = 0; j < filters.size(); ++j)
				{
					folder->findFile(fileList, BTString("*.") + TStringHelper::getUpperCase(filters[j]), findFlag);
					folder->findFile(fileList, BTString("*.") + TStringHelper::getLowerCase(filters[j]), findFlag);
				}
				
				for(size_t j = 0; j < fileList.size(); ++j)
				{
					const TString& subfile = fileList[j];
					if(verbose)
						std::tcout << subfile.c_str() << std::endl;

					HSTREAM stream = folder->openFile(subfile);
					TString base,ext;
					TStringHelper::getFileInfo(subfile, base, ext);
					this->compressFile(stream, output + BTString("/") + base, *conversion, format, mipmapCount, scale, normalMap);
				}
			}

			//check file
			TString name, dir, base,ext;
			TStringHelper::getFilePathInfo(file, dir, name, base, ext);
			TString UpperExt = TStringHelper::getUpperCase(ext);
			TString LowerExt = TStringHelper::getLowerCase(ext);

#if 0
			bool supported = false;
			for(size_t j = 0; j < filters.size(); ++j)
			{
				if( ext == filters[j] || UpperExt == filters[i] || LowerExt == filters[i] )
				{
					supported = true;
					break;
				}
			}

			if( !supported )
				return true;
#endif
			if(verbose)
				std::tcout << file.c_str() << std::endl;

			const HSTREAM& stream = IResourceManager::getSingleton().loadStream(file);

			if( stream != NULL )
				this->compressFile(stream, output + BTString("/") + base, *conversion, format, mipmapCount, scale, normalMap);
			else if( !isFolder )
			{
				std::tcout << TEXT("\terror: unable to load file.") << std::endl;
				return false;
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TexCompressorApp::doInitialize()
	{
		IFramework& fm = IFramework::getSingleton();
		fm.addSubSystem(BTString("BladeGraphicsSystem"));
		fm.addSubSystem(BTString("BladeWindowSystem"));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TexCompressorApp::initPlugins()
	{
		ParamList params;
		params[BTString("BladeImageFI")] = true;	//mandatory for compressing
		params[BTString("BladeDevice")] = true;
		params[BTString("BladeWindow")] = true;
		params[BTString("BladeGraphics")] = true;
		//UI system for configs if needed
		params[BTString("BladeUI")] = true;
		params[BTString("BladeUIPlugin")] = true;
		IPlatformManager::getSingleton().initialize(params, IResourceManager::getSingleton().schemePathToRealPath(BTString("plugins:")));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	TexCompressorApp::compressFile(const HSTREAM& stream, const TString& filePrefix, const TexConversion& conversion, const TString& fileExt, int mipmapCount, float scale, bool normalMap)
	{
		if( stream == NULL )
		{
			std::tcout << TEXT("\terror: unable to load file.") << std::endl;
			return false;
		}

		HIMAGE img = IImageManager::getSingleton().loadImage(stream, IP_TEMPORARY, PF_UNDEFINED, IImage::MIPLOD_DEFAULT);

		if( img == NULL )
		{
			BLADE_LOGU(ToolLog::Error, TEXT("\terror: unable to load file as image."));
			return false;
		}

		//note: cube map use 3d dir vector to sample, UVs are consistent, no need to flip
		if (img->getFaceCount() == 1 && img->getOrientation() != conversion.mDefaultConfig.orientation)
			img = IImageManager::getSingleton().flipImage(img, true, false, IP_TEMPORARY);

		size_t scaledWidth = size_t((scalar)img->getImageWidth()*scale);
		size_t scaledHeight = size_t((scalar)img->getImageHeight()*scale);
		if( scaledWidth != img->getImageWidth() || scaledHeight != img->getImageHeight() )
			img = IImageManager::getSingleton().scaleImage(img, SIZE2I((int)scaledWidth, (int)scaledHeight), IP_TEMPORARY);

		size_t width = img->getImageWidth();
		size_t height = img->getImageHeight();

		if( !Math::IsPowerOf2((uint)width) == 1 || !Math::IsPowerOf2((uint)height) || width < 4 || height < 4 )//check power of 2
		{
			BLADE_LOGU(ToolLog::Error, TEXT("\terror: width/height is not power of 2, or not greater than 4:") << width << TEXT("x") << height
				<< TEXT("."));
			return false;
		}

		TexConfig config = conversion.mDefaultConfig;
		if (normalMap)
		{
			PixelFmtMap::const_iterator i = conversion.mColor2NormalMap.find(conversion.mDefaultConfig.format);
			if (i == conversion.mColor2NormalMap.end())
			{
				BLADE_LOGU(ToolLog::Error, 
					TEXT("error: no suitable normal map format for ") << fileExt.c_str() << TEXT(", format: ") << config.format.getString().c_str());
				return false;
			}
			else
				config.format = i->second;
		}
		else
		{
			PixelFormat pf = config.format;
			bool hasAlpha = img->hasAlpha();
			if (!hasAlpha && img->isCompressed())
			{
				hasAlpha = img->getFormat() == PF_BC2 || img->getFormat() == PF_BC3 || img->getFormat() == PF_RGBA_ETC2EAC;
				//use original format
				if (hasAlpha && !normalMap)
					pf = img->getFormat();
			}

			if (hasAlpha && !pf.hasAlpha())
			{
				PixelFmtMap::const_iterator i = conversion.mColor2RGBAMap.find(pf);
				if (i == conversion.mColor2RGBAMap.end())
					std::tcout << TEXT("\twarning: compress '") << img->getFormat().getString().c_str() <<
					TEXT("' into '") << pf.getString().c_str() << TEXT("', dropping alpha channel.") << std::endl;
				else
				{
					PixelFormat fmt = i->second;
					//std::tcout << TEXT("\twarning: image has alpha channel, change output format '") << pf.getString().c_str() 
					//	<< TEXT("' into '") << fmt.getString().c_str() << TEXT("'.") << std::endl;
					pf = fmt;
				}
			}
			config.format = pf;
		}

		if (img->getBPP() == 8 && !img->isCompressed()) //single channel
		{
			PixelFmtMap::const_iterator i = conversion.mColor2AlphaMap.find(config.format);
			if (i == conversion.mColor2AlphaMap.end())
				std::tcout << TEXT("\twarning: compress '") << img->getFormat().getString().c_str() <<
				TEXT("' into '") << config.format.getString().c_str() << TEXT("'.") << std::endl;
			else
			{
				PixelFormat fmt = i->second;
				//std::tcout << TEXT("\timage is single channel ( grayscale ), change output format '") << pf.getString().c_str() 
				//	<< TEXT("' into '") << fmt.getString().c_str() << TEXT("'.") << std::endl;
				config.format = fmt;
			}
		}

		img = IImageManager::getSingleton().convertToFormat(img, config.format, IP_TEMPORARY, mipmapCount);

		HSTREAM outputStream = IResourceManager::getSingleton().openStream( filePrefix + BTString(".") + fileExt, true);
		bool ret = IImageManager::getSingleton().saveImage(img,  outputStream, fileExt);
		if( !ret )
			BLADE_LOGU(ToolLog::Error, TEXT("\tunable to save compressed file."));

		return ret;

	}
	
}//namespace Blade