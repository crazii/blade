/********************************************************************
	created:	2011/08/23
	filename: 	IImageManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IImageManager_h__
#define __Blade_IImageManager_h__
#include <BladeBase.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/file/IStream.h>
#include <interface/public/graphics/IImage.h>
#include <math/Vector2i.h>

namespace Blade
{

	class IImageManager : public InterfaceSingleton<IImageManager>
	{
	public:
		virtual ~IImageManager() {}

		/**
		@describe load image with specific format override
		@param
		@return
		*/
		virtual HIMAGE			loadImage(const HSTREAM& stream, IMAGE_POOL pool, PixelFormat fmt, 
			int LOD = IImage::MIPLOD_DEFAULT, IMG_ORIENT layout = IMGO_DEFAULT) = 0;

		/** @brief load image with original format */
		inline HIMAGE			loadImage(const HSTREAM& stream, IMAGE_POOL pool,
			PIXEL_ORDER order = PO_NONE, IMG_ORIENT layout = IMGO_DEFAULT, 
			int LOD = IImage::MIPLOD_DEFAULT)
		{
			HIMAGE img = this->loadImage(stream, pool, PixelFormat(PF_UNDEFINED), LOD, layout);
			if( img != NULL && img->getFormat().hasByteOrder() )
				img->setByteOrder(order);
			return img;
		}

		/** @brief load image and decompress it, if the image is not compressed, then keep its original format */
		inline HIMAGE			loadDecompressedImage(const HSTREAM& stream, IMAGE_POOL pool, PixelFormat fmt, 
			int LOD = IImage::MIPLOD_DEFAULT, IMG_ORIENT layout = IMGO_DEFAULT)
		{
			HIMAGE img = this->loadImage(stream, IP_TEMPORARY, PixelFormat(PF_UNDEFINED), LOD, layout);
			if( img == NULL )
			{
				assert(false);
				return img;
			}

			if( (fmt != PF_UNDEFINED && img->getFormat() != fmt) || img->getFormat().isCompressed() || img->getFormat().isPacked() || img->getFormat().isFloat() )
			{
				if( fmt == PF_UNDEFINED )
					fmt = PF_ARGB;
				assert( !fmt.isCompressed() && !fmt.isPacked() );
				img = this->convertToFormat(img, fmt, pool, LOD);
				return img;
			}
			else if( pool == IP_TEMPORARY )
				return img;
			else
				return this->cloneImage(img, pool);
		}

		/**
		@describe save image as specified format
		@param
		@return
		*/
		virtual bool			saveImage(const HIMAGE& img, const HSTREAM& stream, const TString& format) = 0;

		/**
		@describe create a 32bpp empty image
		@param 
		@return 
		*/
		virtual HIMAGE			createImage(size_t width, size_t height, IMAGE_POOL pool, PixelFormat format, IMG_ORIENT layout, int mipLODCount, int depth = 1, int faces = 1) = 0;

		/**
		@describe
		@param LOD levels to specify, must be 
		@return
		*/
		virtual HIMAGE			cloneImage(const HIMAGE& src, IMAGE_POOL pool) = 0;

		/**
		@describe extract one mip level as a new image. the new image has one one mip level
		@param
		@return
		*/
		virtual HIMAGE			extractMipmap(const HIMAGE& src, int mipLevel, IMAGE_POOL pool) = 0;

		/**
		@describe 
		@param
		@return mip map count of image
		*/
		virtual int				generateMipmaps(const HIMAGE& image, int LOD) = 0;

		/**
		@describe scale the original image and generate a new image ( the original is unchanged)
		@param 
		@return 
		*/
		virtual HIMAGE			scaleImage(const HIMAGE& image, const SIZE2I& targetSize, IMAGE_POOL pool, IImage::EScaleFilter filter = IImage::SF_BILINEAR) = 0;

		/**
		@describe flip a image, new instance created
		@param
		@return
		*/
		virtual HIMAGE			flipImage(const HIMAGE& image, bool bVflip, bool bHflip, IMAGE_POOL pool) = 0;

		/**
		@describe create a gray-scale image, with each of R,G,B channel the same value, alpha channel unchanged
		@param 
		@return
		@remark if you want to change to L8 or A8L8 grayscale, use IImageManager::grayscaleImage() then IImageManager::convertToFormat()
		*/
		virtual HIMAGE			grayscaleImage(const HIMAGE& image, IMAGE_POOL pool) = 0;

		/**
		@describe convert image format, new instance created
		@param
		@return
		@remarks
		*/
		virtual HIMAGE			convertToFormat(const HIMAGE& original, PixelFormat format, IMAGE_POOL pool, int LOD = IImage::MIPLOD_DEFAULT) = 0;

		/**
		@describe get the total bytes needed by format of image
		@param
		@return
		*/
		virtual size_t			calcImageSize(size_t width,size_t height, size_t depth, PixelFormat format) = 0;

		/**
		@describe 
		@note 
		@param 
		@return 
		*/
		virtual bool			compressBlock(uint8* dest, const uint8* src, size_t srcWidth, PixelFormat srcFormat, PixelFormat destFormat) = 0;

		/**
		@describe 
		@note 
		@param 
		@return 
		*/
		virtual bool			decompressBlock(uint8* dest, const uint8* src, size_t destWidth, PixelFormat srcFormat, PixelFormat compressedFormat) = 0;
	};


	extern template class BLADE_BASE_API Factory<IImageManager>;

}//namespace Blade



#endif // __Blade_IImageManager_h__