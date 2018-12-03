/********************************************************************
	created:	2015/01/29
	filename: 	ImageFile.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ImageFile_h__
#define __Blade_ImageFile_h__
#include "IImageFile.h"

namespace Blade
{

	class ImageFileDDS : public IImageFile
	{
	public:
		static const TString IMAGEFILE_DDS;
	public:
		/** @brief extension/factory typename */
		virtual const TString& getTypeExtension() const
		{
			return IMAGEFILE_DDS;
		}

		/** @brief  */
		virtual bool	loadImage(const HSTREAM& stream, ImageBase* image);

		/** @brief  */
		virtual bool	saveImage(const HSTREAM& stream, ImageBase* image);
	};//class ImageFileDDS

	class ImageFileKTX : public IImageFile
	{
	public:
		static const TString IMAGEFILE_KTX;
	public:
		/** @brief extension/factory typename */
		virtual const TString& getTypeExtension() const
		{
			return IMAGEFILE_KTX;
		}

		/** @brief  */
		virtual bool	loadImage(const HSTREAM& stream, ImageBase* image);

		/** @brief  */
		virtual bool	saveImage(const HSTREAM& stream, ImageBase* image);
	};//class ImageFileKTX

}//namespace Blade

#endif//__Blade_ImageFile_h__
