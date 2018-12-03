/********************************************************************
	created:	2014/05/09
	filename: 	BladeFreeImage.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeFreeImage_h__
#define __Blade_BladeFreeImage_h__
#include <FreeImage.h>
#include <Singleton.h>
#include <interface/public/graphics/IImage.h>

#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE
#define FI_RGBA PF_ARGB
#define FI_RGB	PF_RGB
#else
#define FI_RGBA PF_BGRA
#define FI_RGB	PF_BGR
#endif

namespace Blade
{
	class FreeImageBridge : public Singleton<FreeImageBridge>
	{
	private:
		FreeImageIO			mIO;
		FreeImageMem		mMem[2];
		FreeImageMemType	mNormalMemType;
		FreeImageMemType	mTempMemType;
	public:
		FreeImageBridge();
		~FreeImageBridge();

		/** @brief  */
		inline FreeImageMem*		getPool(IMAGE_POOL ip)
		{
			if( ip == IP_TEMPORARY)
				return &mMem[IP_TEMPORARY];
			else
				return &mMem[IP_DEFAULT];
		}

		/** @brief  */
		inline FreeImageIO*			getIO()
		{
			return &mIO;
		}

		/** @brief  */
		inline FREE_IMAGE_FILTER	convertFilter(IImage::EScaleFilter filter)
		{
			return FREE_IMAGE_FILTER(filter);
		}

		/** @brief  */
		IPool*			getDefaultImagePool();

		/** @brief  */
		bool			isSupportedRGB(PixelFormat format);

		/** @brief  */
		PixelFormat		getPixelFormatFromDIB(FIBITMAP* dib);

		/** @brief  */
		FREE_IMAGE_TYPE	pixelFormatRGBToFIType(PixelFormat format);

		/** @brief create FIBITMAP using image's mip level, face, and all depths  */
		FIBITMAP*		createFIBitmap(const HIMAGE& image, int mipLODLevel, int face);

		/** @brief  */
		bool			destroyFIBitmap(FIBITMAP* dib);

	protected:
		IPool*	mImagePool;
	}; //class FreeImageBridge


	//////////////////////////////////////////////////////////////////////////
#define BLADE_IMAGE_ALLOC(_bytes)				BLADE_POOL_ALLOCATE( Blade::FreeImageBridge::getSingleton().getDefaultImagePool(), _bytes )
#define BLADE_IMAGE_ALLOCT(_type,_count)		(_type*)BLADE_RES_ALLOC( sizeof(_type)*_count )
#define BLADE_IMAGE_FREE(_ptr)					BLADE_POOL_DEALLOCATE( Blade::FreeImageBridge::getSingleton().getDefaultImagePool(), _ptr )
#define BLADE_IMAGE_ALIGN_ALLOC(_bytes, _align)	BLADE_POOL_ALIGN_ALLOC( Blade::FreeImageBridge::getSingleton().getDefaultImagePool(), _bytes, _align)
#define BLADE_IMAGE_ALIGN_FREE(_ptr)			BLADE_POOL_ALIGN_FREE( Blade::FreeImageBridge::getSingleton().getDefaultImagePool(), _ptr)
	
}//namespace Blade

#endif // __Blade_BladeFreeImage_h__