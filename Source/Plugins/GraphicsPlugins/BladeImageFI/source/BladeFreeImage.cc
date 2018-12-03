/********************************************************************
	created:	2014/05/10
	filename: 	BladeFreeImage.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BladeFreeImage.h"
#include <interface/IPoolManager.h>
#include <interface/public/graphics/PixelColorFormatter.h>

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	static inline unsigned DLL_CALLCONV Blade_FIReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
	{
		IStream* stream = static_cast<IStream*>(handle);

		IStream::Off pos = stream->tell();
		if( stream->readData(buffer,size*count) )
			return size*count;
		else
		{
			assert(false);
			return unsigned(stream->tell() - pos);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	static inline unsigned DLL_CALLCONV Blade_FIWriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
	{
		IStream* stream = static_cast<IStream*>(handle);

		IStream::Off pos = stream->tell();
		if( stream->writeData(buffer,size*count) )
			return size*count;
		else
		{
			assert(false);
			return unsigned(stream->tell() - pos);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	static int DLL_CALLCONV Blade_FISeekProc(fi_handle handle, long offset, int origin)
	{
		IStream* stream = static_cast<IStream*>(handle);

		IStream::EStreamPos pos = 
			origin == SEEK_SET ? IStream::SP_BEGIN : (origin == SEEK_CUR ? IStream::SP_CURRENT : IStream::SP_END );

		return (int)stream->seek(pos,offset);
	}

	//////////////////////////////////////////////////////////////////////////
	static long DLL_CALLCONV Blade_FITellProc(fi_handle handle)
	{
		IStream* stream = static_cast<IStream*>(handle);
		return long(stream->tell());
	}

	//////////////////////////////////////////////////////////////////////////
	static void* DLL_CALLCONV Blade_FIMalloc(size_t size, fi_mem_handle handle)
	{
		assert( FreeImageBridge::getSingleton().getDefaultImagePool() == handle );
		return BLADE_POOL_ALLOCATE(static_cast<IPool*>(handle), size);
	}

	//////////////////////////////////////////////////////////////////////////
	static void DLL_CALLCONV Blade_FIFree(void* ptr, fi_mem_handle handle)
	{
		assert( FreeImageBridge::getSingleton().getDefaultImagePool() == handle );
		return BLADE_POOL_DEALLOCATE(static_cast<IPool*>(handle), ptr);
	}

	//////////////////////////////////////////////////////////////////////////
	static void* DLL_CALLCONV Blade_FIAlignMalloc(size_t size,size_t align, fi_mem_handle handle)
	{
		assert( FreeImageBridge::getSingleton().getDefaultImagePool() == handle );
		return AlignMalloc(size,align, static_cast<IPool*>(handle) );
	}

	//////////////////////////////////////////////////////////////////////////
	static void DLL_CALLCONV Blade_FIAlignFree(void* ptr, fi_mem_handle handle)
	{
		assert( FreeImageBridge::getSingleton().getDefaultImagePool() == handle );
		return AlignFree(ptr, static_cast<IPool*>(handle) );
	}

	//////////////////////////////////////////////////////////////////////////
	static void* DLL_CALLCONV Blade_FITempMalloc(size_t size, fi_mem_handle handle)
	{
		assert( Memory::getTemporaryPool() == handle );
		return BLADE_POOL_ALLOCATE(static_cast<IPool*>(handle), size);
	}

	//////////////////////////////////////////////////////////////////////////
	static void DLL_CALLCONV Blade_FITempFree(void* ptr, fi_mem_handle handle)
	{
		assert( Memory::getTemporaryPool() == handle );
		return BLADE_POOL_DEALLOCATE(static_cast<IPool*>(handle), ptr);
	}

	//////////////////////////////////////////////////////////////////////////
	static void* DLL_CALLCONV Blade_FITempAlignMalloc(size_t size,size_t align, fi_mem_handle handle)
	{
		assert( Memory::getTemporaryPool() == handle );
		return AlignMalloc(size,align, static_cast<IPool*>(handle) );
	}

	//////////////////////////////////////////////////////////////////////////
	static void DLL_CALLCONV Blade_FITempAlignFree(void* ptr, fi_mem_handle handle)
	{
		assert( Memory::getTemporaryPool() == handle );
		return AlignFree(ptr, static_cast<IPool*>(handle) );
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	FreeImageBridge::FreeImageBridge()
	{
		mIO.read_proc = Blade_FIReadProc;
		mIO.seek_proc = Blade_FISeekProc;
		mIO.tell_proc = Blade_FITellProc;
		mIO.write_proc = Blade_FIWriteProc;

		mNormalMemType.align_free_proc = Blade_FIAlignFree;
		mNormalMemType.align_malloc_proc = Blade_FIAlignMalloc;
		mNormalMemType.malloc_proc = Blade_FIMalloc;
		mNormalMemType.free_proc = Blade_FIFree;
		mNormalMemType.mem_handle = Memory::getResourcePool();

		mTempMemType.align_free_proc = Blade_FITempAlignFree;
		mTempMemType.align_malloc_proc = Blade_FITempAlignMalloc;
		mTempMemType.malloc_proc = Blade_FITempMalloc;
		mTempMemType.free_proc = Blade_FITempFree;
		mTempMemType.mem_handle = Memory::getTemporaryPool();

		mMem[IP_DEFAULT].normal_mem = &mNormalMemType;
		mMem[IP_DEFAULT].temp_mem = &mTempMemType;

		mMem[IP_TEMPORARY].normal_mem = &mTempMemType;
		mMem[IP_TEMPORARY].temp_mem = &mTempMemType;

		mImagePool = BLADE_FACTORY_CREATE(IPool, BTString("Misc"));
		mImagePool->initialize(1, 16*16*4, 4*1024);
	}

	//////////////////////////////////////////////////////////////////////////
	FreeImageBridge::~FreeImageBridge()
	{
		BLADE_DELETE mImagePool;
	}

	//////////////////////////////////////////////////////////////////////////
	IPool*			FreeImageBridge::getDefaultImagePool()
	{
		return mImagePool;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FreeImageBridge::isSupportedRGB(PixelFormat format)
	{
		//FI supported direct scale format
		return !format.isCompressed() && !format.isPacked() /*&& format.getComponents() >= 3*/ && !format.isFloat16();
	}

#define ENABLE_FI_UNSUPPORTED 0

	//////////////////////////////////////////////////////////////////////////
	PixelFormat		FreeImageBridge::getPixelFormatFromDIB(FIBITMAP* dib)
	{
		PixelFormat format = PF_UNDEFINED;
		unsigned int BPP = ::FreeImage_GetBPP(dib);
		unsigned int redMask = ::FreeImage_GetRedMask(dib);
		unsigned int greenMask = ::FreeImage_GetGreenMask(dib);
		unsigned int blueMask = ::FreeImage_GetBlueMask(dib);
		unsigned int alphaMask = 0;
		FREE_IMAGE_TYPE type = ::FreeImage_GetImageType(dib);

		if( type == FIT_BITMAP )
		{
			if( BPP == 16 )
			{
				if( redMask == FI16_565_RED_MASK && greenMask == FI16_565_GREEN_MASK && blueMask == FI16_565_BLUE_MASK )
					format = PF_RGB565;
				else if( alphaMask == 0x00008000 && redMask == 0x00007C00 && greenMask == 0x000003E0 && blueMask == 0x0000001F )
					format = PF_B5G5R5A1;
				else if( alphaMask == 0x00000000 && redMask == 0x00007c00 && greenMask == 0x000003E0 && blueMask == 0x0000001F )
					format = PF_B5G5R5A1;
#if ENABLE_FI_UNSUPPORTED
				else if( alphaMask == 0xF0 )
					format = PF_R8G8;
				else if( alphaMask == 0x0000F000 && redMask == 0x00000F00 && greenMask == 0x000000F0 && blueMask == 0x0000000F )
					format = PF_B4G4R4A4;
				else if( alphaMask == 0x00000000 && redMask == 0x00000F00 && greenMask == 0x000000F0 && blueMask == 0x0000000F )
					format = PF_B4G4R4A4;
#endif
			}
			else if( BPP == 24 )
			{
				//R8G8B8
				if( redMask == 0x00FF0000 && greenMask == 0x0000FF00 && blueMask == 0x000000FF )
					format = PF_B8G8R8;
				else if( redMask == 0x000000FF && greenMask == 0x0000FF00 && blueMask == 0x00FF0000 )
					format = PF_R8G8B8;
				else
					format = FI_RGB;//use default
			}
			else if( BPP == 32 )
			{
				if( alphaMask == 0xFF000000 && redMask == 0x00FF0000 && greenMask == 0x0000FF00 && blueMask == 0x000000FF )
					format = PF_ARGB;
				else if( alphaMask == 0x00000000 && redMask == 0x00FF0000 && greenMask == 0x0000FF00 && blueMask == 0x000000FF )
					format = PF_ARGB;
				else if( alphaMask == 0xFF000000 && redMask == 0x000000FF && greenMask == 0x0000FF00 && blueMask == 0x00FF0000 )
					format = PF_ABGR;
				else if( alphaMask == 0x00000000 && redMask == 0x000000FF && greenMask == 0x0000FF00 && blueMask == 0x00FF0000 )
					format = PF_ABGR;
				else
					format = FI_RGBA;//use default

				//note: for PF_A2R10G10B10, should be alphaMask == 0xC0000000 && redMask == 0x3FF00000 && greenMask == 0xFFC00 && blueMask == 0x3FF, 
				//but for most libs, they use the latter. do the same for compatible reason.
				//PF_A2B10G10R10 is the same
#if ENABLE_FI_UNSUPPORTED
				else if( alphaMask == 0xC0000000 && redMask == 0x000003FF && greenMask == 0x000FFC00 && blueMask == 0x3FF00000 )
					format = PF_B10G10R10A2;
				//else if( alphaMask == 0xC0000000 && redMask == 0x3FF00000 && greenMask == 0x000FFC00 && blueMask == 0x000003FF )
				//	format = PF_A2B10G10R10;
#endif
			}
			else if( BPP == 8 )
			{
				if( alphaMask == 0 )
					format = PF_R8;
#if ENABLE_FI_UNSUPPORTED
				//else if( alphaMask == 0xF0)
				//	format = PF_A4L4;
				else
				{
					assert(alphaMask == 0xFF );
					format = PF_A8;
				}
#endif
			}
		}
		else if( type == FIT_UINT16 )
				format = PF_R16;
		else if( type == FIT_RGB16 )
			format = PF_UNDEFINED;
		else if( type == FIT_RGBA16 )
			format = PF_R16G16B16A16;
		else if( type == FIT_UINT32)
			assert(false);
		else if( type == FIT_FLOAT )
			format = PF_R32F;
		else if( type == FIT_RGBF )
			format = PF_R32G32B32F;
		else if( type == FIT_RGBAF )
			format = PF_R32G32B32A32F;

		return format;
	}

	//////////////////////////////////////////////////////////////////////////
	FREE_IMAGE_TYPE	FreeImageBridge::pixelFormatRGBToFIType(PixelFormat format)
	{
		assert( this->isSupportedRGB(format) );
		size_t components = format.getComponents();
		assert( components != 2 && components >= 1 && components <= 4 );
		if( format.isUint8() )
		{
			assert( !format.isPacked() );
			return FIT_BITMAP;
		}
		else if( format.isUint16() )
		{
			FREE_IMAGE_TYPE types[4] = {FIT_UINT16, FIT_RGBA16, FIT_RGB16, FIT_RGBA16};
			return types[components-1];
		}
		else if( format.isFloat32() )
		{
			FREE_IMAGE_TYPE types[4] = {FIT_FLOAT, FIT_RGBAF, FIT_RGBF, FIT_RGBAF};
			return types[components-1];
		}
		return FIT_UNKNOWN;
	}

	//////////////////////////////////////////////////////////////////////////
	FIBITMAP*		FreeImageBridge::createFIBitmap(const HIMAGE& image, int mipLODLevel, int face)
	{
		const IMG_MIPINFO* mip = NULL;
		if( image != NULL && this->isSupportedRGB( image->getFormat() )
			&& (mip = image->getMipInfo((index_t)mipLODLevel, (size_t)face)) != NULL )
		{
			FreeImageMem* cache = FreeImageBridge::getSingleton().getPool( IP_TEMPORARY );
			FREE_IMAGE_TYPE type = this->pixelFormatRGBToFIType( image->getFormat() );
			int BPP = image->getBPP();
			FIBITMAP* dib = ::FreeImage_AllocateT(cache, type, (int)mip->oriWidth, (int)mip->oriHeight*(int)image->getDepth(), (int)BPP );

			size_t destPitch = (size_t)::FreeImage_GetPitch(dib);
			size_t sourcePitch = (size_t)mip->rowPitch;
			char* dest = (char*)::FreeImage_GetBits(dib);
			char* source = (char*)mip->data;
			size_t sourceStride = image->getFormat().getSizeBytes();
			size_t destStride = (size_t)::FreeImage_GetBPP(dib) / 8;

			if( destPitch == sourcePitch )
				std::memcpy(dest, source, mip->bytes);
			else
			{
				size_t bytes = std::min<size_t>( destPitch, sourcePitch );
				for(size_t i = 0; i < mip->height; ++i)
				{
					if( sourceStride == destStride )
						std::memcpy(dest, source, bytes);
					else
					{
						//note: pixelFormatRGBToFIType may return an enlarged format to hold the data, not precisely match.
						//i.e. FIT_RGBA16 for R16G16
						//if not match, use color formatter to convert format. (simply clamp min stride is incorrect)
						//size_t strideBytes = std::min<size_t>(destStride, sourceStride);
						Color c;
						PixelFormat destFormat = FreeImageBridge::getSingleton().getPixelFormatFromDIB(dib);
						for (size_t j = 0; j < mip->width; ++j)
						{
							ColorFormatter::readColor(source + j * sourceStride, sourceStride, c, image->getFormat());
							ColorFormatter::writeColor(dest + j * destStride, destStride, c, destFormat);
						}
					}
					dest += destPitch;
					source += sourcePitch;
				}
			}
			return dib;
		}
		assert(false);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FreeImageBridge::destroyFIBitmap(FIBITMAP* dib)
	{
		if( dib != NULL )
		{
			::FreeImage_Unload( FreeImageBridge::getSingleton().getPool(IP_TEMPORARY), dib);
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

}//namespace Blade
