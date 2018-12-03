/********************************************************************
	created:	2011/08/23
	filename: 	ImageManagerFI.h
	author:		Crazii
	purpose:	FreeImage implementation of IImageManager
*********************************************************************/
#ifndef __Blade_ImageManagerFI_h__
#define __Blade_ImageManagerFI_h__
#include <interface/public/graphics/IImageManager.h>
#include "BladeFreeImage.h"

namespace Blade
{
	class ImageFI;
	class ImageDDSBase;

	class ImageManagerFI : public IImageManager, public Singleton<ImageManagerFI>
	{
	public:
		using Singleton<ImageManagerFI>::getSingleton;
		using Singleton<ImageManagerFI>::getSingletonPtr;
	public:
		ImageManagerFI()	{}
		~ImageManagerFI()	{}

		/************************************************************************/
		/* IImageManager interface                                                                     */
		/************************************************************************/
		/*
		@describe load image with specific format override
		@param
		@return
		*/
		virtual HIMAGE			loadImage(const HSTREAM& stream, IMAGE_POOL pool, PixelFormat fmt, 
			int mipLODCount = IImage::MIPLOD_DEFAULT, IMG_ORIENT layout = IMGO_DEFAULT);

		/*
		@describe save image as specified format
		@param
		@return
		*/
		virtual bool			saveImage(const HIMAGE& img, const HSTREAM& stream, const TString& format);

		/*
		@describe create a 32bpp empty image
		@param 
		@return 
		*/
		virtual HIMAGE			createImage(size_t width, size_t height, IMAGE_POOL pool, PixelFormat format, IMG_ORIENT layout, int mipLODCount, int depth = 1, int faces = 1);

		/*
		@describe
		@param
		@return
		*/
		virtual HIMAGE			cloneImage(const HIMAGE& src, IMAGE_POOL pool);

		/*
		@describe 
		@param
		@return
		*/
		virtual HIMAGE			extractMipmap(const HIMAGE& src, int mipLevel, IMAGE_POOL pool);

		/*
		@describe 
		@param
		@return mip map count of image
		*/
		virtual int				generateMipmaps(const HIMAGE& image, int mipLODCount);

		/*
		@describe scale the original image and generate a new image ( the original is unchanged)
		@param 
		@return 
		*/
		virtual HIMAGE			scaleImage(const HIMAGE& image, const SIZE2I& targetSize, IMAGE_POOL pool, IImage::EScaleFilter filter = IImage::SF_BILINEAR);

		/*
		@describe flip a image, new instance created
		@param
		@return
		*/
		virtual HIMAGE			flipImage(const HIMAGE& image, bool bVflip, bool bHflip, IMAGE_POOL pool);

		/*
		@describe create a gray-scale image, with each of R,G,B channel the same value, alpha channel unchanged
		@param 
		@return
		@remark if you want to change to L8 or A8L8 grayscale, use IImageManager::grayscaleImage() then IImageManager::convertToFormat()
		*/
		virtual HIMAGE			grayscaleImage(const HIMAGE& image, IMAGE_POOL pool);

		/*
		@describe convert a 32bpp RGB/32bpp RGBA image to target format, new IImage instance created
		@param
		@remarks
		*/
		virtual HIMAGE			convertToFormat(const HIMAGE& original, PixelFormat format, IMAGE_POOL pool, int mipLODCount = IImage::MIPLOD_DEFAULT);

		/*
		@describe get the total bytes needed by format of image
		@param
		@return
		*/
		virtual size_t			calcImageSize(size_t width, size_t height, size_t depth, PixelFormat format);

		/*
		@describe 
		@note 
		@param 
		@return 
		*/
		virtual bool			compressBlock(uint8* dest, const uint8* src, size_t srcWidth, PixelFormat srcFormat, PixelFormat compressedFormat);

		/*
		@describe 
		@note 
		@param 
		@return 
		*/
		virtual bool			decompressBlock(uint8* dest, const uint8* src, size_t destWidth, PixelFormat destFormat, PixelFormat compressedFormat);

	protected:

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		size_t			calcMaxMipCount(PixelFormat format, size_t width,size_t height);

		/** @brief create RGB(A) image with mip count 1 */
		/* @note: dibs will be destroyed inside the function call  */
		HIMAGE			createImageRGBA(FIBITMAP** dibs, IMAGE_POOL pool, PixelFormat destFormat, IMG_ORIENT layout, int depth, int faces);

		/** @brief  */
		HIMAGE			compressImage(const HIMAGE& sourceImage, PixelFormat format, IMAGE_POOL pool, int mipLODCount, bool bHighQuality = false);

		/** @brief  */
		HIMAGE			decompressImage(const HIMAGE& sourceImage, PixelFormat format, IMAGE_POOL pool, int mipLODCount);

		/** @brief conversion between non compressed format*/
		HIMAGE			convertToFormatNoCompressed(const HIMAGE& original, PixelFormat format, IMAGE_POOL pool, int mipLODCount = IImage::MIPLOD_DEFAULT);

		/** @brief scale image in RGB(A) mode */
		HIMAGE			scaleImageRGBA(const HIMAGE& original, const SIZE2I& targetSize, IMAGE_POOL pool, IImage::EScaleFilter filter = IImage::SF_BILINEAR);

		/** @brief generate mip map from an-RGB(A) image as the current existing lowest level*/
		bool			generateMipmapsRGBA(const HIMAGE& image, int mipLODCount, const HIMAGE& minMipImage);
	};
	

}//namespace Blade



#endif // __Blade_ImageManagerFI_h__