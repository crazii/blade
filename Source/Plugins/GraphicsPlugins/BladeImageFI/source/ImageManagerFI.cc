/********************************************************************
	created:	2011/08/23
	filename: 	ImageManagerFI.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include "ImageManagerFI.h"
#include "IImageFile.h"
#include "Image.h"
#include "BC.h"
#include "ETC2EAC.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	HIMAGE		ImageManagerFI::loadImage(const HSTREAM& stream, IMAGE_POOL pool, PixelFormat fmt,
		int mipLODCount/* = IImage::MIPLOD_DEFAULT*/, IMG_ORIENT layout/* = IMGO_BOTTOM_UP*/)
	{
		if( stream == NULL || !stream->isValid() )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		HIMAGE img( ImageBase::createImage(IP_TEMPORARY) );
		ImageBase* image = static_cast<ImageBase*>(img);
		if( img == NULL )
			return img;

		const TString& name = stream->getName();
		TString dir, filename, base, ext;
		TStringHelper::getFilePathInfo(name,dir,filename, base, ext);

		TString imageFileType = TStringHelper::getUpperCase(ext);
		if( ImageFileFactory::getSingleton().isClassRegistered(imageFileType) )
		{
			IImageFile* file = BLADE_FACTORY_CREATE(IImageFile, imageFileType);
			assert( file->getTypeExtension() == imageFileType );
			bool result = file->loadImage(stream, img);
			BLADE_DELETE file;
			if( !result )
				return HIMAGE::EMPTY;
		}
		else
		{
			if( !image->load(stream) )
				return HIMAGE::EMPTY;
		}

		if( layout != IMGO_DEFAULT && layout != img->getOrientation() )
		{
			if( img->getFormat().isCompressed() && fmt != PF_UNDEFINED )
			{
				PixelFormat flippedFormat = fmt;
				int mipmap = mipLODCount;

				if( flippedFormat.isCompressed() )
					flippedFormat = FI_RGBA;
				if( fmt != flippedFormat )
					mipmap = 1;
				img = this->convertToFormat(img, flippedFormat, IP_TEMPORARY, mipmap);
			}
			img->setOrientation(layout);
		}

		if( (fmt == PF_UNDEFINED || fmt == img->getFormat() ) )
		{
			int mipCount = (int)img->getMipmapCount();
			if( (mipLODCount != IImage::MIPLOD_DEFAULT && mipLODCount > mipCount) || mipLODCount == IImage::MIPLOD_MAX)
			{
				int maxMip = (int)this->calcMaxMipCount( img->getFormat(), img->getImageWidth(), img->getImageHeight());
				if( mipLODCount == IImage::MIPLOD_MAX || mipLODCount >= maxMip )
					mipLODCount = maxMip;

				if( mipLODCount > mipCount )
					this->generateMipmaps(img, mipLODCount);
				else//DDS file may have all mip maps, including the 4x4 block's 2 mips
					mipLODCount = mipCount;
			}
			else if(mipLODCount != IImage::MIPLOD_DEFAULT && mipLODCount != mipCount)
			{
				//remove not used mip maps
				if( mipLODCount < mipCount )
					image->removeMipmaps( size_t(mipCount - mipLODCount));
				else
					assert(false);
			}
			assert(mipLODCount == IImage::MIPLOD_DEFAULT || mipLODCount == image->getMipmapCount());

			if( img->getPool() == pool )
				return img;
			else
				return this->cloneImage(img, pool);
		}
		else
			return this->convertToFormat(img, fmt, pool, mipLODCount);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ImageManagerFI::saveImage(const HIMAGE& img, const HSTREAM& stream, const TString& format)
	{
		if( img == NULL || stream == NULL )
		{
			assert(false);
			return false;
		}

		TString imageFileType = TStringHelper::getUpperCase(format);

		if( ImageFileFactory::getSingleton().isClassRegistered(imageFileType) )
		{
			IImageFile* file = BLADE_FACTORY_CREATE(IImageFile, imageFileType);
			assert( file->getTypeExtension() == imageFileType );
			bool result = file->saveImage(stream, img);
			BLADE_DELETE file;
			return result;
		}
		BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("unsupported format:")+format);
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::createImage(size_t width, size_t height, IMAGE_POOL pool, PixelFormat format, IMG_ORIENT layout, int mipLODCount, int depth/* = 1*/, int faces/* = 1*/)
	{
		ImageBase* image = ImageBase::createImage(pool);

		if( !image->create(width, height, format, layout, mipLODCount, depth, faces) )
			return HIMAGE::EMPTY;
		return HIMAGE(image);
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::cloneImage(const HIMAGE& src, IMAGE_POOL pool)
	{
		if( src == NULL )
			return HIMAGE::EMPTY;

		const ImageBase* img = static_cast<const ImageBase*>(src);
		return HIMAGE(img->clone(pool));
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::extractMipmap(const HIMAGE& src, int mipLevel, IMAGE_POOL pool)
	{
		if( src != NULL && mipLevel < src->getMipmapCount() )
		{
			if(mipLevel == IImage::MIPLOD_MAX )
				mipLevel = src->getMipmapCount()-1;

			const IMG_MIPINFO* srcMipFace0 = src->getMipInfo((size_t)mipLevel, 0);

			HIMAGE result = this->createImage(srcMipFace0->oriWidth, srcMipFace0->oriHeight, pool, src->getFormat(), src->getOrientation(), 1, src->getDepth(), src->getFaceCount() );
			//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
			for(size_t f = 0; f < result->getFaceCount(); ++f)
			{
				const IMG_MIPINFO* srcMip = src->getMipInfo((size_t)mipLevel, f);
				const IMG_MIPINFO* destMip = result->getMipInfo(0, f);
				assert( destMip->bytes == srcMip->bytes );
				std::memcpy(destMip->data, srcMip->data, srcMip->bytes);
			}
			return result;
		}
		assert(false);
		return HIMAGE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	int				ImageManagerFI::generateMipmaps(const HIMAGE& image, int mipLODCount)
	{
		if( image != NULL )
		{
			int maxMipCount = (int)this->calcMaxMipCount(image->getFormat(), image->getImageWidth(), image->getImageHeight() );
			if( mipLODCount < 0 || mipLODCount >= maxMipCount )
				mipLODCount = maxMipCount;

			uint8 mipCount = image->getMipmapCount();

			if( mipCount < mipLODCount )
			{
				HIMAGE mipImg = this->extractMipmap(image, mipCount-1, IP_TEMPORARY);
				if( !FreeImageBridge::getSingleton().isSupportedRGB( image->getFormat() ) )
					mipImg = this->convertToFormat(mipImg, FI_RGBA, IP_TEMPORARY, 1);	//prepare for scale

				if( !this->generateMipmapsRGBA(image, mipLODCount, mipImg) )
				{
					assert(false);
					return 0;
				}
			}
			return image->getMipmapCount();
		}
		assert(false);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::scaleImage(const HIMAGE& image, const SIZE2I& targetSize, IMAGE_POOL pool, IImage::EScaleFilter filter/* = IImage::SF_BICUBIC*/)
	{
		if( targetSize.x > 0 && targetSize.y > 0 )
		{
			if( targetSize.x != (int)image->getImageWidth() || targetSize.y != (int)image->getImageHeight() )
			{
				uint8 mipMap = image->getMipmapCount();

				HIMAGE source;
				if( image->isCompressed() )
					BLADE_DETAIL_LOG(Warning, BTString("scale of a compressed image: high cost.") );

				if( !FreeImageBridge::getSingleton().isSupportedRGB(image->getFormat()) )
					source = this->convertToFormat(image, FI_RGBA, IP_TEMPORARY, 1);
				else
					source = image;

				//get mip 0
				HIMAGE mip = source->getMipmapCount() == 1 ? source : this->extractMipmap(source, 0, IP_TEMPORARY);
				if( mip == NULL )
					return HIMAGE::EMPTY;

				//scale mip 0
				HIMAGE scaleMip = this->scaleImageRGBA(mip, targetSize, IP_TEMPORARY, filter);

				//generate mipmap
				if( scaleMip->getFormat() == image->getFormat() && mipMap == 1)
				{
					if(scaleMip->getPool() == pool)
						return scaleMip;
					else
						return this->cloneImage(scaleMip, pool);
				}
				else
					return this->convertToFormat(scaleMip, image->getFormat(), pool, mipMap);
			}
			else
				return this->cloneImage(image, pool);
		}
		assert(false);
		return HIMAGE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::flipImage(const HIMAGE& image, bool bVflip, bool bHflip, IMAGE_POOL pool)
	{
		if( bVflip || bHflip )
		{
			HIMAGE hImg;
			//compressed image are stored in blocks, so cannot flip directly
			if( image->isCompressed() )
			{
				BLADE_DETAIL_LOG(Warning, BTString("flip of a compressed image: high cost.") );
				hImg = IImageManager::getSingleton().convertToFormat(image, PF_R8G8B8A8, IP_TEMPORARY, 1);
			}
			else
				hImg = this->cloneImage(image, pool);

			//flip all mipmap levels
			//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
			for(size_t f = 0; f < hImg->getFaceCount(); ++f)
			{
				for(size_t i = 0; i < hImg->getMipmapCount(); ++i)
				{
					const IMG_MIPINFO* mip = hImg->getMipInfo(i, f);
					if( bVflip )
					{
						uint8* tmp = (uint8*)BLADE_TMP_ALIGN_ALLOC(mip->rowPitch, IImage::IMAGE_ALIGNMENT);
						uint8* data = mip->data;
						//flip per depth/slice, this is layout related
						for(size_t d = 0; d < hImg->getDepth(); ++d)
						{
							for(uint32 row = 0; row < mip->height/2; ++row)
							{
								uint32 flip = mip->height - row - 1;
								std::memcpy(tmp,							data + row * mip->rowPitch,		mip->rowPitch);
								std::memcpy(data + row * mip->rowPitch,	data + flip * mip->rowPitch,	mip->rowPitch);
								std::memcpy(data + flip * mip->rowPitch,	tmp,							mip->rowPitch);
							}
							data += mip->slicePitch;
						}
						BLADE_TMP_ALIGN_FREE(tmp);
					}

					if( bHflip )
					{
						uint8* data = mip->data;
						size_t uintSize = hImg->getBytesPerUnit();
						uint8* tmp = (uint8*)BLADE_TMP_ALIGN_ALLOC(uintSize, IImage::IMAGE_ALIGNMENT);
						for(uint32 row = 0; row < mip->height*hImg->getDepth(); ++row)
						{
							for(uint32 col = 0; col < mip->width/2; ++col)
							{
								uint32 flip = mip->width - col;
								std::memcpy(tmp,						data + col * uintSize,	uintSize);
								std::memcpy(data + col * uintSize,		data + flip * uintSize,	uintSize);
								std::memcpy(data + flip * uintSize,	tmp,					uintSize);
							}
							data += mip->rowPitch;
						}
						BLADE_TMP_ALIGN_FREE(tmp);
					}
				}
			}

			if( image->isCompressed() )
				return this->convertToFormat(hImg, image->getFormat(), pool, image->getMipmapCount());
			else
				return hImg;
		}
		else
			return this->cloneImage(image, pool);
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::grayscaleImage(const HIMAGE& sourceImg, IMAGE_POOL pool)
	{
		HIMAGE image;
		if( sourceImg->isCompressed() )
			BLADE_DETAIL_LOG(Warning, BTString("grayscale of a compressed image: high cost.") );

		bool supported = FreeImageBridge::getSingleton().isSupportedRGB(sourceImg->getFormat());
		if( !supported )
			image = IImageManager::getSingleton().convertToFormat(sourceImg, FI_RGBA, IP_TEMPORARY, 1);
		else
			image = this->cloneImage(sourceImg, pool);

		FreeImageMem* mem = FreeImageBridge::getSingleton().getPool(IP_TEMPORARY);

		//all mipmap levels gray
		//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
		for(size_t f = 0; f < image->getFaceCount(); ++f)
		{
			for(size_t i = 0; i < image->getMipmapCount(); ++i)
			{
				const IMG_MIPINFO* mip = image->getMipInfo(i, f);
				FIBITMAP* dib = FreeImageBridge::getSingleton().createFIBitmap(image, (int)i, (int)f);
				FIBITMAP* grayDIB = ::FreeImage_ConvertToGreyscale(mem, dib);			
				if( image->getFormat().hasAlpha() )
				{
					assert( image->getFormat().isUint8() || image->getFormat().isFloat32() || image->getFormat().isUint16() );
					FIBITMAP* gray32DIB = ::FreeImage_ConvertTo32Bits(mem, grayDIB);
					FreeImageBridge::getSingleton().destroyFIBitmap(grayDIB);
					FIBITMAP* alphaDIB = ::FreeImage_GetChannel(mem, dib, FICC_ALPHA);
					FreeImageBridge::getSingleton().destroyFIBitmap(dib);
					BOOL ret = ::FreeImage_SetChannel(gray32DIB, alphaDIB, FICC_ALPHA);
					assert(ret);
					BLADE_UNREFERENCED(ret);
					FreeImageBridge::getSingleton().destroyFIBitmap(alphaDIB);
					assert( mip->bytes == mip->oriWidth*mip->oriHeight*::FreeImage_GetBPP(gray32DIB)/8 );
					std::memcpy(mip->data, ::FreeImage_GetBits(gray32DIB), mip->bytes);
					FreeImageBridge::getSingleton().destroyFIBitmap(gray32DIB);	
				}
				else
				{
					FreeImageBridge::getSingleton().destroyFIBitmap(dib);
					assert( ::FreeImage_GetBPP(grayDIB) == 8 );
					const uint8* src = ::FreeImage_GetBits(grayDIB);
					uint8* dest = mip->data;
					if( mip->bytes == mip->oriWidth*mip->oriHeight )
						std::memcpy(dest, src, mip->bytes);
					else
					{
						size_t srcPitch = ::FreeImage_GetPitch(grayDIB);
						size_t unitSize = image->getBytesPerUnit();
						for(size_t j = 0; j < mip->oriHeight*image->getDepth(); ++j)
						{
							for(size_t k = 0; k < mip->oriWidth; ++k)
								std::memset(dest+unitSize*k, *(src+k), unitSize);
							dest += unitSize*mip->oriWidth;	//image's pitch is right one line
							src += srcPitch;
						}
					}//else
				}
			}//for
		}

		if( !supported )
			return this->convertToFormat(image, sourceImg->getFormat(), pool, sourceImg->getMipmapCount() );
		else if( image->getPool() == pool )
			return image;
		else
			return this->cloneImage(image, pool);
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::convertToFormat(const HIMAGE& original, PixelFormat format, IMAGE_POOL pool, int mipLODCount/* = IImage::MIPLOD_DEFAULT*/)
	{
		if( original != NULL )
		{
			size_t mipMap = (size_t)original->getMipmapCount();
			size_t maxMip = this->calcMaxMipCount(original->getFormat(), original->getImageWidth(), original->getImageHeight());
			if ((mipLODCount < 0 || mipLODCount > (int)maxMip))
				mipLODCount = (int)maxMip;

			if( original->getFormat() == format )
			{
				HIMAGE img = this->cloneImage(original, pool);
				if(mipLODCount == IImage::MIPLOD_DEFAULT || mipLODCount == (int)mipMap)
					return img;
				if((int)mipMap < mipLODCount)
					this->generateMipmaps(img, mipLODCount);
				else
				{
					ImageBase* imgb = static_cast<ImageBase*>(img);
					imgb->removeMipmaps(mipMap - (size_t)mipLODCount);
				}
				return img;
			}

			if( original->isCompressed() )
			{
				if( !format.isCompressed() )
					return this->decompressImage(original, format, pool, mipLODCount);
				else
				{
					HIMAGE image = this->decompressImage(original, PF_R8G8B8A8, IP_TEMPORARY, 1);
					return this->compressImage(image, format, pool, mipLODCount);
				}
			}

			if( format.isCompressed() )
				return this->compressImage(original, format, pool, mipLODCount);

			//non compressed convert to non compressed
			assert( !format.isCompressed() && !original->isCompressed() );
			return this->convertToFormatNoCompressed(original, format, pool, mipLODCount);
		}
		assert(false);
		return HIMAGE::EMPTY;
	}


	//////////////////////////////////////////////////////////////////////////
	size_t			ImageManagerFI::calcImageSize(size_t width,size_t height, size_t depth, PixelFormat format)
	{
		if( !format.isCompressed() )
			return width*height*format.getSizeBytes();
		else
		{
			width = (width+3)/4;
			height = (height+3)/4;
			return width*height*depth*format.getSizeBytes();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ImageManagerFI::compressBlock(uint8* dest, const uint8* src, size_t srcWidth, PixelFormat srcFormat, PixelFormat compressedFormat)
	{
		if( src == NULL || dest == NULL || srcFormat.isCompressed() || !compressedFormat.isCompressed() )
		{
			assert(false);
			return false;
		}

		size_t pixelBytes = srcFormat.getSizeBytes();
		uint8* buffer = (uint8*)BLADE_TMP_ALIGN_ALLOC( pixelBytes*16, IImage::IMAGE_ALIGNMENT);
		//form a linear sequence from source
		for(size_t i = 0; i < 4; ++i)
		{
			const uint8* srcline = src + i * srcWidth * pixelBytes;
			uint8* bufferline = buffer + i * 4 * pixelBytes;
			std::memcpy(bufferline, srcline, pixelBytes*4);
		}

		//compress
		switch( compressedFormat )
		{
		case PF_BC1:
			BC::compressBlockBC1(dest, buffer, srcFormat);
			break;
		case PF_BC2:
			BC::compressBlockBC2(dest, buffer, srcFormat);
			break;
		case PF_BC3:
			BC::compressBlockBC3(dest, buffer, srcFormat);
			break;
		case PF_BC4:
			BC::compressBlockBC4(dest, buffer, srcFormat);
			break;
		case PF_BC5:
			BC::compressBlockBC5(dest, buffer, srcFormat, true);
			break;
		case PF_RGB_ETC2:
			ETC2EAC::compressBlockETC2(dest, buffer, srcFormat);
			break;
		case PF_RGBA_ETC2EAC:
			ETC2EAC::compressBlockETC2EAC(dest, buffer, srcFormat);
			break;
		case PF_R_EAC:
			ETC2EAC::compressBlockR11EAC(dest, buffer, srcFormat);
			break;
		case PF_RG_EAC:
			ETC2EAC::compressBlockRG11EAC(dest, buffer, srcFormat, true);
			break;
		default:
			assert(false);
			BLADE_TMP_ALIGN_FREE(buffer);
			return false;
		}

		BLADE_TMP_ALIGN_FREE(buffer);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ImageManagerFI::decompressBlock(uint8* dest, const uint8* src, size_t destWidth, PixelFormat destFormat, PixelFormat compressedFormat)
	{
		if( src == NULL || dest == NULL || !compressedFormat.isCompressed() || destFormat.isCompressed() )
		{
			assert(false);
			return false;
		}

		size_t pixelBytes = destFormat.getSizeBytes();
		uint8* buffer = (uint8*)BLADE_TMP_ALIGN_ALLOC( pixelBytes*16, IImage::IMAGE_ALIGNMENT);
		//decompress
		switch( compressedFormat )
		{
		case PF_BC1:
			BC::decompressBlockBC1(buffer, src, destFormat);
			break;
		case PF_BC2:
			BC::decompressBlockBC2(buffer, src, destFormat);
			break;
		case PF_BC3:
			BC::decompressBlockBC3(buffer, src, destFormat);
			break;
		case PF_BC4:
			BC::decompressBlockBC4(buffer, src, destFormat);
			break;
		case PF_BC5:
			BC::decompressBlockBC5(buffer, src, destFormat);
			break;
		case PF_RGB_ETC2:
			ETC2EAC::decompressBlockETC2(buffer, src, destFormat);
			break;
		case PF_RGBA_ETC2EAC:
			ETC2EAC::decompressBlockETC2EAC(buffer, src, destFormat);
			break;
		case PF_R_EAC:
			ETC2EAC::decompressBlockR11EAC(buffer, src, destFormat);
			break;
		case PF_RG_EAC:
			ETC2EAC::decompressBlockRG11EAC(buffer, src, destFormat);
			break;
		default:
			assert(false);
			BLADE_TMP_ALIGN_FREE(buffer);
			return false;
		}

		for(size_t i = 0; i < 4; ++i)
		{
			uint8* destline = dest + i * destWidth * pixelBytes;
			const uint8* bufferline = buffer + i * 4 * pixelBytes;
			std::memcpy(destline, bufferline, pixelBytes*4);
		}

		BLADE_TMP_ALIGN_FREE(buffer);
		return true;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t			ImageManagerFI::calcMaxMipCount(PixelFormat format, size_t width,size_t height)
	{
		size_t mipCount = Math::CalcMaxMipCount(width, height);
		if( format.isCompressed() )
		{
			if( mipCount > 2 )
				mipCount -= 2;		//minimal 4x4
			else
				mipCount = 1;
		}
		return std::max<size_t>(mipCount,1);
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::createImageRGBA(FIBITMAP** dibs, IMAGE_POOL pool, PixelFormat destFormat, IMG_ORIENT layout, int depth, int faces)
	{
		assert( faces == 1 || faces == 6 );
		assert( depth == 1 || faces == 1 );

		if( dibs != NULL )
		{
			PixelFormat format = FreeImageBridge::getSingleton().getPixelFormatFromDIB(dibs[0]);
			assert( format != PF_UNDEFINED );

			HIMAGE image = this->createImage( 
				(size_t)::FreeImage_GetWidth(dibs[0]), 
				(size_t)::FreeImage_GetHeight(dibs[0])/depth, 
				pool, destFormat, layout, 1, depth, faces);

			for(int f = 0; f < faces; ++f)
			{
				FIBITMAP* dib = dibs[f];
				assert( ::FreeImage_GetWidth(dib) == image->getWidth() && ::FreeImage_GetHeight(dib) == image->getHeight()*depth );
				const IMG_MIPINFO* mip = image->getMipInfo(0, (size_t)f);
				uint8* dest = mip->data;
				size_t destPitch = (size_t)mip->rowPitch;
				size_t sourcePitch = (size_t)::FreeImage_GetPitch(dib);
				uint8* source = (uint8*)::FreeImage_GetBits(dib);

				if (::FreeImage_GetBPP(dib) == 32 || ::FreeImage_GetBPP(dib) == 24 || ::FreeImage_GetBPP(dib) == 8)
				{
					if (destPitch == sourcePitch)
						std::memcpy(dest, source, mip->bytes);
					else
					{
						size_t bytes = std::min<size_t>(destPitch, sourcePitch);
						for (size_t i = 0; i < mip->height*depth; ++i)
						{
							std::memcpy(dest, source, bytes);
							dest += destPitch;
							source += sourcePitch;
						}
						assert(dest - mip->data == (intptr_t)mip->bytes);
					}
				}
				else
				{
					size_t sourceStride = ::FreeImage_GetBPP(dib) / 8;
					size_t destStride = destFormat.getSizeBytes();
					Color c;
					for (size_t i = 0; i < mip->height*depth; ++i)
					{
						for (size_t j = 0; j < mip->width; ++j)
						{
							ColorFormatter::readColor(source + j * sourceStride, sourceStride, c, format);
							ColorFormatter::writeColor(dest + j * destStride, destStride, c, destFormat);
						}
						dest += destPitch;
						source += sourcePitch;
					}
					assert(dest - mip->data == (intptr_t)mip->bytes);
				}
				
				FreeImageBridge::getSingleton().destroyFIBitmap(dib);
			}
			return image;
		}

		assert(false);
		return HIMAGE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::compressImage(const HIMAGE& sourceImage, PixelFormat format, IMAGE_POOL pool, int mipLODCount, bool bHighQuality/* = false*/)
	{
		//TODO:
		BLADE_UNREFERENCED(bHighQuality);

		if( sourceImage == NULL )
			return HIMAGE::EMPTY;

		PixelFormat srcFormat = sourceImage->getFormat();
		if( !format.isCompressed() || srcFormat.isCompressed() )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		HIMAGE original = this->extractMipmap(sourceImage, 0, IP_TEMPORARY);
		if( !FreeImageBridge::getSingleton().isSupportedRGB(srcFormat) )
			original = this->convertToFormat(original, FI_RGB, IP_TEMPORARY, 1);

		//RGB without alpha can be compressed into BC1
		if( srcFormat.getComponents() >= 3 && format == PF_BC1 )
		{
			//empty
		}
		else if( srcFormat.getComponents() < format.getComponents() )
		{
			//this is a special hack for Crytek Sponza testing scene
			//it contains bump map that blade current doesn't support.
			//temporary hack for bump mapping: bump map(height map) to normal map
#define HACK_BUMP2NORMAL 1
#if HACK_BUMP2NORMAL
			//special hack: 1 channel bump map to 2 channel normal map
			if( srcFormat == PF_R8 && (format == PF_BC5 || format == PF_RG_EAC) )
			{
			}
			else
#endif //HACK_BUMP2NORMAL
			{
				assert(false);	//missing components
				return HIMAGE::EMPTY;
			}
		}
		else if( srcFormat.getComponents() > format.getComponents() )
		{
			//most possibly to BC4 / BC5
			//warning: mismatch channel
			BLADE_DETAIL_LOG(Warning, BTString("extra components will be dropped.") );
		}
		srcFormat = original->getFormat();

		size_t width = original->getImageWidth();
		size_t height = original->getImageHeight();
		size_t depth = original->getDepth();
		size_t faces = original->getFaceCount();

		size_t size = std::min(width,height);
		if( mipLODCount == IImage::MIPLOD_DEFAULT )
			mipLODCount = sourceImage->getMipmapCount();

		if( mipLODCount >= 1 || mipLODCount < 0 )
		{
			//max mip level count
			size_t mipLevelCount = Math::CalcMaxMipCount(size) - 2;	//min block: 4x4 pixel - 2 levels
			//size_t mipLevelCount = Math::CalcMaxMipCount(size);	//min block: 1x1 pixel
			mipLODCount = (int)std::min<size_t>( (size_t)mipLODCount, mipLevelCount);
		}
		this->generateMipmaps(original, mipLODCount);

#if HACK_BUMP2NORMAL
		//note: we need filter normal on each mipmap
		//if filter normal on mip 0 then downscale, the downscaling will get bad normal
		if( srcFormat == PF_R8 && (format == PF_BC5 || format == PF_RG_EAC) )
		{
			//RGBA
			PixelFormat fmt = FI_RGB;
			HIMAGE temp = this->createImage(original->getImageWidth(), original->getImageHeight(), IP_TEMPORARY, fmt, original->getOrientation(), original->getMipmapCount());

			srcFormat = original->getFormat();
			size_t srcUnitSize = original->getBytesPerUnit();
			size_t destUnitSize = temp->getBytesPerUnit();
			//handedness: use right handed normal map (bottom up) by default
			//scalar handedness = (original->getOrientation() == IMGO_TOP_DOWN) ? -1.0f : 1.0f;
			//note: most normal maps are right handed
			scalar handedness = (original->getOrientation() == IMGO_TOP_DOWN) ? 1.0f : -1.0f;

			for(uint8 i = 0; i < original->getMipmapCount(); ++i)
			{
				const IMG_MIPINFO* srcMip = original->getMipInfo(i, 0);
				const uint8* src = srcMip->data;
				uint8* dest = temp->getMipInfo(i, 0)->data;

				size_t rowCount = srcMip->rowPitch / srcUnitSize;
				size_t totalHeight = srcMip->height * original->getDepth();

				for(size_t row = 0; row < totalHeight; ++row )
				{
					for(size_t col = 0; col < rowCount; ++col)
					{
						Color lt,t,rt,l,r,lb,b,rb;
						//wrap around:
						size_t u0 = (col+rowCount-1)%rowCount;
						size_t u1 = col;
						size_t u2 = (col+rowCount+1)%rowCount;
						size_t v0 = (row+totalHeight-1)%totalHeight;
						size_t v1 = row;
						size_t v2 = (row+totalHeight+1)%totalHeight;

						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v0+u0), srcUnitSize, lt, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v0+u1), srcUnitSize, t, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v0+u2), srcUnitSize, rt, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v1+u0), srcUnitSize, l, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v1+u2), srcUnitSize, r, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v2+u0), srcUnitSize, lb, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v2+u1), srcUnitSize, b, srcFormat);
						ColorFormatter::readColor(src + srcUnitSize*(rowCount*v2+u2), srcUnitSize, rb, srcFormat);

						//SOBEL filter
						scalar dX = rt[Color::R] + 2 * r[Color::R] + rb[Color::R] - lt[Color::R] - 2 * l[Color::R] - lb[Color::R];
						scalar dY = lb[Color::R] + 2 * b[Color::R] + rb[Color::R] - lt[Color::R] - 2 * t[Color::R] - rt[Color::R];
						Vector3 normal(-dX, -dY*handedness, 0.8);
						normal.normalize();
						normal = (normal + Vector3::UNIT_ALL)*0.5f;
						Color c(normal.x, normal.y, normal.z, 1);
						ColorFormatter::writeColor(dest + destUnitSize*(rowCount*v1+u1), destUnitSize, c, fmt);
					}
				}
			}
			original = temp;
			srcFormat = fmt;
		}
#endif

		HIMAGE image = this->createImage(width, height, pool, format, original->getOrientation(), mipLODCount, (int)depth, (int)faces);

		for(int i = 0; i < mipLODCount; ++i )
		{
			//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
			for(size_t f = 0; f < faces; ++f)
			{
				const IMG_MIPINFO* dest = image->getMipInfo((size_t)i, f);
				const IMG_MIPINFO* src = original->getMipInfo((size_t)i, f);
				assert( dest != NULL );
				assert( dest->data != NULL );
				assert( dest->bytes == this->calcImageSize(width, height, image->getDepth(),format) );
				assert( src->width == width && src->height == height );

				size_t oriWidth = src->oriWidth;
				size_t oriHeight = src->oriHeight;
				//oriWidth = (oriWidth+3)/4*4;
				//oriHeight = (oriHeight+3)/4*4;

				size_t bytes = 0;
				switch(format)
				{
				case PF_BC1:
					bytes = BC::compressImageBC1(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_BC2:
					bytes = BC::compressImageBC2(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_BC3:
					bytes = BC::compressImageBC3(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_BC4:
					bytes = BC::compressImageBC4(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_BC5:
					bytes = BC::compressImageBC5(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat, true);
					break;
				case PF_RGB_ETC2:
					bytes = ETC2EAC::compressImageETC2(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_RGBA_ETC2EAC:
					bytes = ETC2EAC::compressImageETC2EAC(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_R_EAC:
					bytes = ETC2EAC::compressImageR11EAC(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat);
					break;
				case PF_RG_EAC:
					bytes = ETC2EAC::compressImageRG11EAC(dest->data, src->data, (int)oriWidth, (int)(oriHeight*depth), srcFormat, true);
					break;
				default:
					assert(false);
					break;
				}
				assert( dest->bytes == bytes );
			}

			width /= 2;
			height /= 2;
		}
		return image;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::decompressImage(const HIMAGE& sourceImage, PixelFormat format, IMAGE_POOL pool, int mipLODCount)
	{
		if( sourceImage == NULL  )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		const IMG_INFO& header = sourceImage->getInfo();

		if( !header.format.isCompressed() || format.isCompressed() )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		const IMG_MIPINFO* src0 = sourceImage->getMipInfo(0);
		if( src0 == NULL )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		//verify size
		size_t sizeBytes = this->calcImageSize( src0->oriWidth, src0->oriHeight, sourceImage->getDepth(), header.format );
		if( src0->bytes != sizeBytes )
			return HIMAGE::EMPTY;

		size_t width = src0->oriWidth;
		size_t height = src0->oriHeight;
		size_t depth = sourceImage->getDepth();
		size_t faces = sourceImage->getFaceCount();
		size_t mipMax = this->calcMaxMipCount(header.format,width, height);
		size_t mipCount = (size_t)mipLODCount;
		if( mipCount == IImage::MIPLOD_DEFAULT )
			mipCount = sourceImage->getMipmapCount();
		else if( mipCount == (size_t)IImage::MIPLOD_MAX )
			mipCount = mipMax;
		mipCount = std::min<size_t>(mipCount, mipMax);
		HIMAGE destImage = this->createImage(width, height, pool, format, sourceImage->getOrientation(), (int)mipCount, (int)depth, (int)faces );
		assert( destImage->getBytes() == this->calcImageSize(src0->oriWidth, src0->oriHeight, sourceImage->getDepth(), format) );

		//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
		for(size_t f = 0; f < faces; ++f)
		{
			for(size_t i = 0; i < mipCount; ++i)
			{
				const IMG_MIPINFO* src = sourceImage->getMipInfo(i, f);
				const IMG_MIPINFO* dest = destImage->getMipInfo(i, f);
				size_t oriWidth = src->oriWidth;
				size_t oriHeight = src->oriHeight;
				//oriWidth = (oriWidth+3)/4*4;
				//oriHeight = (oriHeight+3)/4*4;

				size_t bytes = 0;
				switch(header.format)
				{
				case PF_BC1:
					bytes = BC::decompressImageBC1(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_BC2:
					bytes = BC::decompressImageBC2(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_BC3:
					bytes = BC::decompressImageBC3(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_BC4:
					bytes = BC::decompressImageBC4(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_BC5:
					bytes = BC::decompressImageBC5(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_RGB_ETC2:
					bytes = ETC2EAC::decompressImageETC2(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_RGBA_ETC2EAC:
					bytes = ETC2EAC::decompressImageETC2EAC(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_R_EAC:
					bytes = ETC2EAC::decompressImageR11EAC(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				case PF_RG_EAC:
					bytes = ETC2EAC::decompressImageRG11EAC(dest->data, src->data, (int)oriWidth, (int)oriHeight*(int)depth, format);
					break;
				default:
					assert(false);
					break;
				}
				assert( dest->bytes == bytes );
			}
		}
		this->generateMipmaps(destImage, (int)mipCount);
		return destImage;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::convertToFormatNoCompressed(const HIMAGE& source, PixelFormat format, IMAGE_POOL pool, int mipLODCount/* = IImage::MIPLOD_DEFAULT*/)
	{
		HIMAGE original = source;

		if( original == NULL || original->isCompressed() )
		{
			assert(false);
			return HIMAGE::EMPTY;
		}

		if( mipLODCount == IImage::MIPLOD_DEFAULT )
			mipLODCount = original->getMipmapCount();
		else if( mipLODCount < 0 || mipLODCount > original->getMipmapCount() )
			mipLODCount = (int)this->calcMaxMipCount(original->getFormat(), original->getImageWidth(), original->getImageHeight() );

		if( original->getFormat() == format )
		{
			HIMAGE image = this->cloneImage(original, pool);
			if( mipLODCount > image->getMipmapCount() )
				this->generateMipmapsRGBA(image, mipLODCount, this->extractMipmap(image, image->getMipmapCount()-1, IP_TEMPORARY) );
			return image;
		}

		HIMAGE image = this->createImage(original->getWidth(), original->getHeight(), pool, format, 
		original->getOrientation(), mipLODCount, original->getDepth(), original->getFaceCount() );

		size_t srcUnitSize = original->getBytesPerUnit();
		size_t destUnitSize = image->getBytesPerUnit();

		PixelFormat srcFormat = original->getFormat();
		PixelFormat destFormat = image->getFormat();
		Color c = Color::WHITE;

		for(size_t f = 0; f < original->getFaceCount(); ++f)
		{
			for(size_t i = 0; i < original->getMipmapCount(); ++i)
			{
				const IMG_MIPINFO* srcMip = original->getMipInfo(i, f);
				const IMG_MIPINFO* destMip = image->getMipInfo(i, f);
				size_t count = srcMip->bytes / srcUnitSize;
				BLADE_UNREFERENCED(count);
				assert( count == destMip->bytes / destUnitSize );
				size_t rowCount = srcMip->rowPitch / srcUnitSize;
				assert( rowCount == destMip->rowPitch / destUnitSize );
				assert( srcMip->height == destMip->height && original->getDepth() == image->getDepth() );
				size_t totalHeight = srcMip->height * original->getDepth();
				
				const uint8* src = srcMip->data;
				uint8* dest = destMip->data;
				for(size_t row = 0; row < totalHeight; ++row )
				{
					for(size_t col = 0; col < rowCount; ++col)
					{
						ColorFormatter::readColor(src, srcUnitSize, c, srcFormat);
						ColorFormatter::writeColor(dest, destUnitSize, c, destFormat);
						assert( src - srcMip->data < (ptrdiff_t)srcMip->bytes );
						assert( dest - destMip->data < (ptrdiff_t)destMip->bytes );
						src += srcUnitSize;
						dest += destUnitSize;
					}
				}
				assert( src - srcMip->data == (ptrdiff_t)srcMip->bytes );
				assert( dest - destMip->data == (ptrdiff_t)destMip->bytes );
			}//for each mip map
		}//for each cube face

		if( mipLODCount > image->getMipmapCount() )
			this->generateMipmapsRGBA(image, mipLODCount, this->extractMipmap(image, image->getMipmapCount()-1, IP_TEMPORARY) );
		return image;
	}

	//////////////////////////////////////////////////////////////////////////
	HIMAGE			ImageManagerFI::scaleImageRGBA(const HIMAGE& original, const SIZE2I& targetSize, IMAGE_POOL pool, IImage::EScaleFilter filter/* = IImage::SF_BICUBIC*/)
	{
		if( original != NULL && FreeImageBridge::getSingleton().isSupportedRGB( original->getFormat() ) )
		{
			//assert( original->getBPP() == 32 || original->getBPP() == 24 );
			FreeImageMem* mem = FreeImageBridge::getSingleton().getPool( IP_TEMPORARY );
			ImageBase* img = ImageBase::createImage(pool);
			//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
			for(size_t i = 0; i < original->getMipmapCount(); ++i)
			{
				size_t faceCount = original->getFaceCount();
				assert(faceCount != 0);

				typedef TempVector<FIBITMAP*> BitmapList;
				BitmapList list;
				list.reserve(faceCount);

				for(size_t f = 0; f < faceCount; ++f)
				{
					FIBITMAP* dib = FreeImageBridge::getSingleton().createFIBitmap(original, (int)i, (int)f);
					assert( ::FreeImage_GetHeight(dib) == original->getHeight() * original->getDepth() );
					FIBITMAP* newDIB = ::FreeImage_Rescale(mem, dib, targetSize.x, targetSize.y * original->getDepth(), FreeImageBridge::getSingleton().convertFilter(filter) );
					FreeImageBridge::getSingleton().destroyFIBitmap(dib);
					list.push_back(newDIB);
				}
				HIMAGE mipfaces = this->createImageRGBA(&list[0], IP_TEMPORARY, original->getFormat(), original->getOrientation(), original->getDepth(), (int)faceCount);

				img->appendMipmap( mipfaces );
			}

			return HIMAGE(img);
		}
		assert(false);
		return HIMAGE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ImageManagerFI::generateMipmapsRGBA(const HIMAGE& image, int mipLODCount, const HIMAGE& minMipImage)
	{
		uint8 mipCount = image->getMipmapCount();
		HIMAGE mipImg = minMipImage;

		if( mipCount < mipLODCount && FreeImageBridge::getSingleton().isSupportedRGB( mipImg->getFormat() ) )
		{
			size_t width = mipImg->getWidth();
			size_t height = mipImg->getHeight();
			//must be the desired size
			assert( width == image->getMipInfo(mipCount-1u)->width && height == image->getMipInfo(mipCount-1u)->height );

			//note: 2D & 3D image have the same memory layout, so the memcpy is ok for all types
			for(int i = mipCount; i < mipLODCount; ++i)
			{
				SIZE2I size( (int)width/2, (int)height/2);
				mipImg = this->scaleImageRGBA(mipImg, size, IP_TEMPORARY, IImage::SF_BICUBIC);

				HIMAGE formattedImg;
				if( !FreeImageBridge::getSingleton().isSupportedRGB(image->getFormat()) )
					formattedImg = this->convertToFormat(mipImg, image->getFormat(), IP_TEMPORARY, 1);
				else
					formattedImg = mipImg;
				assert( formattedImg->getFaceCount() == image->getFaceCount() );

				//append mip image
				bool ret = static_cast<ImageBase*>(image)->appendMipmap( formattedImg );
				assert(ret);
				BLADE_UNREFERENCED(ret);

				width /= 2;
				height /= 2;
			}
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

}//namespace Blade
