/********************************************************************
	created:	2011/08/23
	filename: 	ImageManagerFIPlugin.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ImageManagerFIPlugin.h"

#include <BladeImageFI.h>

#include "ImageManagerFI.h"
#include "ImageFile.h"
#include "ETC2EACCommon.h"
#include <interface/IPlatformManager.h>

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladeImageFI");
	BLADE_DEF_PLUGIN(ImageManagerFIPlugin);

	//////////////////////////////////////////////////////////////////////////
	ImageManagerFIPlugin::ImageManagerFIPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ImageManagerFIPlugin::~ImageManagerFIPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				ImageManagerFIPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("ImageManager"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				ImageManagerFIPlugin::getDependency(TStringParam& dependencyList) const
	{
		BLADE_UNREFERENCED(dependencyList);
		//note: image manager uses graphics interface, but it doesn't not depend on any graphics feature
		//dependencyList.push_back(BTString("GraphicsService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				ImageManagerFIPlugin::install()
	{
		RegisterSingleton(ImageManagerFI,IImageManager);
		NameRegisterFactory(ImageFileDDS, IImageFile, ImageFileDDS::IMAGEFILE_DDS);
		NameRegisterFactory(ImageFileKTX, IImageFile, ImageFileKTX::IMAGEFILE_KTX);

		//init ETC2 tables
		setupAlphaTableAndValtab();
	}

	//////////////////////////////////////////////////////////////////////////
	void				ImageManagerFIPlugin::uninstall()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void				ImageManagerFIPlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				ImageManagerFIPlugin::shutdown()
	{

	}

}//namespace Blade
