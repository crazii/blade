/********************************************************************
	created:	2015/01/29
	filename: 	IImageFile.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IImageFile_h__
#define __Blade_IImageFile_h__
#include <memory/BladeMemory.h>
#include <BladeImageFI.h>
#include <interface/public/file/IStream.h>

namespace Blade
{
	class ImageBase;

	class IImageFile : public TempAllocatable
	{
	public:
		virtual ~IImageFile() {}

		/** @brief extension/factory typename */
		virtual const TString& getTypeExtension() const = 0;

		/** @brief  */
		virtual bool	loadImage(const HSTREAM& stream, ImageBase* image) = 0;

		/** @brief  */
		virtual bool	saveImage(const HSTREAM& stream, ImageBase* image) = 0;
	};//class IImageFile

	extern template class BLADE_IMAGEFI_API Factory<IImageFile>;
	typedef Factory<IImageFile> ImageFileFactory;

}//namespace Blade

#endif//__Blade_IImageFile_h__
