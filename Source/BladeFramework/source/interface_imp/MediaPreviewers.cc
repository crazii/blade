/********************************************************************
	created:	2011/08/23
	filename: 	MediaPreviewers.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "MediaPreviewers.h"
#include <utility/Profiling.h>
#include <interface/IResourceManager.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	HIMAGE	ImagePreviewer::generatePreview(const TString& mediaFile, size_t size, IMAGE_POOL pool)
	{
		//the brush format should be 8 bit luminance
		HSTREAM stream = IResourceManager::getSingleton().loadStream(mediaFile);

		if( size == (size_t)-1 )
		{
			BLADE_LW_PROFILING_FUNCTION();
			HIMAGE image = IImageManager::getSingleton().loadDecompressedImage(stream, pool, PF_UNDEFINED, 1);
			return image;
		}
		else
		{
			HIMAGE imageCache = IImageManager::getSingleton().loadDecompressedImage(stream, IP_TEMPORARY, PF_UNDEFINED, 1);
			SIZE2I scaleSize;
			scaleSize.x = scaleSize.y = (int)size;
			HIMAGE image = IImageManager::getSingleton().scaleImage(imageCache, scaleSize, pool);
			return image;
		}
	}
}//namespace Blade
