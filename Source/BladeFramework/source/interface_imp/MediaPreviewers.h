/********************************************************************
	created:	2011/08/23
	filename: 	MediaPreviewers.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MediaPreviewers_h__
#define __Blade_MediaPreviewers_h__
#include <interface/IMediaLibrary.h>
#include <memory/BladeMemory.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class ImagePreviewer : public IMediaLibrary::IMediaPreviewer, public StaticAllocatable
	{
	public:
		ImagePreviewer()	{}
		~ImagePreviewer()	{}

		/************************************************************************/
		/* IMediaLibrary::IMediaPreviewer interface                                                                      */
		/************************************************************************/
		/** @brief  */
		virtual HIMAGE	generatePreview(const TString& mediaFile, size_t size, IMAGE_POOL pool);

		/** @brief  */
		virtual void	play(IWindow* targetWindow,const TString& mediaFile)	{	BLADE_UNREFERENCED(targetWindow);	BLADE_UNREFERENCED(mediaFile);	}

		/** @brief  */
		virtual void	updatePlay()	{}

		/** @brief  */
		virtual void	stop()			{}

	};//class ImagePreviewer 

}//namespace Blade



#endif // __Blade_MediaPreviewers_h__