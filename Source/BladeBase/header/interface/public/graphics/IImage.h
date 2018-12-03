/********************************************************************
	created:	2011/08/22
	filename: 	IImage.h
	author:		Crazii
	purpose:	the definition of bitmap , and DXT image (known as MS direct draw surface,DDS)
	log:		move from BladeFramework to BladeBase 11/29/2012
*********************************************************************/
#ifndef __Blade_IImage_h__
#define __Blade_IImage_h__
#include <Handle.h>
#include "PixelFormat.h"
#include "Color.h"

namespace Blade
{
	/************************************************************************/
	/* image pool                                                                     */
	/************************************************************************/
	typedef enum EImagePool
	{
		IP_DEFAULT = 0,
		IP_TEMPORARY,
	}IMAGE_POOL;


	/************************************************************************/
	/* image info                                                                     */
	/************************************************************************/
	typedef struct BladeImageInfo
	{
		PixelFormat	format;
		uint8		depth;	///volume or normal 2D
		uint8		faces;	///cube or normal 2D, must be 0 or 6
		uint8		mipmap;	///mipmap level, if exist
		uint8		pool;	///IMAGE_POOL
	}IMG_INFO;

	typedef struct BladeImageMipInfo
	{
		///image dimension
		///pixel dimension for non-compressed image, or block dimension for (block)compressed image
		uint32		width;
		uint32		height;
		///original dimension
		uint32		oriWidth;
		uint32		oriHeight;

		uint32	rowPitch;	///row pitch in bytes
		uint32	slicePitch;
		size_t	bytes;		//compressed size
		uint8*	data;		///image data
	}IMG_MIPINFO;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API IImage
	{
	public:
		static const size_t IMAGE_ALIGNMENT = 64;	///cache line alignment for x86 processors
		///special value for LOD level count
		///note: level 0 is the best level.
		static const int MIPLOD_MAX = -1;		//max mip map count based on size
		static const int MIPLOD_DEFAULT = 0;	//default mip count stored in image

		enum EScaleFilter
		{
			SF_BOX		  = 0,	/// Box, pulse, Fourier window, 1st order (constant) b-spline
			SF_BICUBIC	  = 1,	/// Mitchell & Netravali's two-param cubic filter
			SF_BILINEAR   = 2,	/// Bilinear filter
			SF_BSPLINE	  = 3,	/// 4th order (cubic) b-spline
			SF_CATMULLROM = 4,	/// Catmull-Rom spline, Overhauser spline
			SF_LANCZOS3	  = 5	/// Lanczos3 filter
		};
	public:
		virtual ~IImage()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const IMG_INFO&		getInfo() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual IMG_ORIENT			getOrientation() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool				setOrientation(IMG_ORIENT layout) = 0;

		/**
		@describe 
		@param [in] pxLayout: target layout to set
		@param [in] reformat: whether to change the content according layout. you can set reformat to false
		if the image is empty and you should fill the pixel data with the right layout
		@return 
		@note this is only available on 32BPP 8888 format
		*/
		virtual bool				setByteOrder(PIXEL_ORDER pxLayout, bool reformat = false) = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual const IMG_MIPINFO*	getMipInfo(index_t LOD, size_t face = 0) const = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual bool				getPixelColor(index_t x, index_t y, Color::RGBA& outColor) const = 0;

		/**
		@describe set linear flag. this WON'T change image data, only tagged for future texture generation
		@param 
		@return 
		*/
		virtual void				setLinearColorSpace(bool linear) = 0;

		/** @brief  */
		inline IMAGE_POOL	getPool() const
		{
			return (IMAGE_POOL)this->getInfo().pool;
		}

		/** @brief  */
		inline uint8		getMipmapCount() const
		{
			return this->getInfo().mipmap;
		}

		/** @brief  */
		inline uint8		getDepth() const
		{
			return this->getInfo().depth;
		}

		/** @brief  */
		inline uint8		getFaceCount() const
		{
			return this->getInfo().faces;
		}

		/** @brief  */
		inline bool			isCube() const
		{
			return this->getFaceCount() == 6;
		}

		/** @brief  */
		inline bool			isVolume() const
		{
			return this->getDepth() > 1;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline uint8*	getBits() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->data: NULL;
		}

		/** @brief  */
		inline uint32		getImageWidth() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->oriWidth: 0;
		}

		/** @brief get the original image height */
		inline uint32		getImageHeight() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->oriHeight: 0;
		}

		/** @brief  */
		inline uint32		getWidth() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->width: 0;
		}

		/** @brief get the image height */
		inline uint32		getHeight() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->height: 0;
		}

		/** @brief get bytes used for mip 0 */
		inline size_t		getBytes() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->bytes: 0;
		}

		/** @brief  */
		inline uint32		getPitch() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->rowPitch: 0;
		}

		/** @brief  */
		inline uint32		getSlicePitch() const
		{
			const IMG_MIPINFO* mip = this->getMipInfo(0);
			return mip != NULL ? mip->slicePitch: 0;
		}

		/** @brief  */
		inline PixelFormat	getFormat() const
		{
			return this->getInfo().format;
		}

		/** @brief  */
		inline bool			isCompressed() const
		{
			return this->getFormat().isCompressed();
		}

		/** @brief  */
		inline bool			hasByteOrder() const
		{
			return this->getFormat().hasByteOrder();
		}

		/** @brief  */
		inline PIXEL_ORDER		getByteOrder() const
		{
			return this->getFormat().getByteOrder();
		}

		/** @brief  */
		inline bool			hasAlpha() const
		{
			return this->getFormat().hasAlpha();
		}

		/** @brief  */
		inline size_t		getComponents() const
		{
			return this->getFormat().getComponents();
		}

		/** @brief bytes per pixel, or bytes per compressed block */
		inline ushort		getBytesPerUnit() const
		{
			return (ushort)this->getFormat().getSizeBytes();
		}

		/** @brief bit per pixel or bit per compressed block */
		inline ushort		getBPP() const
		{
			return this->getBytesPerUnit()*8u;
		}

	};//class IImage

	typedef Handle<IImage>	HIMAGE;

}//namespace Blade



#endif // __Blade_IImage_h__