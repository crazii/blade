/********************************************************************
	created:	2014/05/09
	filename: 	Image.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "Image.h"
#include <utility/Profiling.h>
#include <interface/public/graphics/PixelColorFormatter.h>

namespace Blade
{
	template class ImageImpl<IP_DEFAULT>;
	template class ImageImpl<IP_TEMPORARY>;

	//////////////////////////////////////////////////////////////////////////
	bool			ImageBase::setOrientation(IMG_ORIENT layout)
	{
		if( mLayout == layout )
			return true;
		else
		{
			if( mInfo.format != PF_UNDEFINED && (mInfo.depth > 0 || mInfo.faces > 0) && mInfo.mipmap != 0 )
			{
				//upside down - v flip
				HIMAGE _this(this);
				HIMAGE img = IImageManager::getSingleton().flipImage(_this, true, false, IP_TEMPORARY);
				_this.unbind();

				//copy data
				for(size_t f = 0; f < img->getFaceCount(); ++f)
				{
					for(size_t i = 0; i < img->getMipmapCount(); ++i)
					{
						const IMG_MIPINFO* src = img->getMipInfo(i,f);
						const IMG_MIPINFO* dest = this->getMipInfo(i,f);
						assert( src->width == dest->width && src->height == dest->height );
						assert( src->oriWidth == dest->oriWidth && src->oriHeight == dest->oriHeight );
						assert( src->bytes == dest->bytes );
						std::memcpy(dest->data, src->data, dest->bytes);
					}
				}
			}
			mLayout = layout;
			return true;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	bool			ImageBase::setByteOrder(PIXEL_ORDER order, bool reformat/* = false*/)
	{
		if( !mInfo.format.hasByteOrder() )
		{
			assert(false);
			return false;
		}

		//keep original order
		if( order == PO_NONE || order == mInfo.format.getByteOrder() )
			return true;

		if( reformat )
		{
			RGBAConverter converter = order;

			for(size_t f = 0; f < this->getFaceCount(); ++f)
			{
				for(size_t i = 0; i < this->getMipmapCount(); ++i)
				{
					const IMG_MIPINFO* mip = this->getMipInfo(i, f);
					Color::COLOR* color = reinterpret_cast<Color::COLOR*>( mip->data );
					for(uint32 j = 0; j < mip->height; ++j)
					{
						for(uint32 k = 0; k < mip->height; ++k)
							color[j].color = converter.convertFrom(mInfo.format.getByteOrder(), color[k].color);
						color += mip->rowPitch / sizeof(Color::COLOR);
					}
				}
			}
		}
		mInfo.format.setByteOrder(order);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			ImageBase::getPixelColor(index_t x, index_t y, Color::RGBA& outColor) const
	{
		//TODO: decompress  block and return color
		if( !this->isCompressed() )
		{
			uint8* data = this->getBits();
			data += y * this->getPitch() + x * this->getBytesPerUnit();
			return ColorFormatter::readColorLDR(data, outColor, this->getFormat() );
		}
		assert(false);
		return false;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ImageBase::load(const HSTREAM&	stream)
	{
		if( stream == NULL || !stream->isValid() )
			return false;

		FreeImageMem* mem = FreeImageBridge::getSingleton().getPool( IP_TEMPORARY );
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

		//check the file signature and deduce its format
		const TString& name = stream->getName();
#if BLADE_UNICODE
		fif = ::FreeImage_GetFIFFromFilenameU(mem, name.c_str());
#else
		fif = ::FreeImage_GetFIFFromFilename(mem, name.c_str() );
#endif

		//if still unknown, try to guess the file format from the file extension
		if(fif == FIF_UNKNOWN) 
		{
			fif = ::FreeImage_GetFileTypeFromHandle(FreeImageBridge::getSingleton().getIO(), static_cast<fi_handle>( stream.ptr() ),0);
			stream->rewind();
		}

		//if still unkown, return failure
		if(fif == FIF_UNKNOWN || fif == FIF_DDS)
			return false;

		FIBITMAP* dib;
		//check that the plugin has reading capabilities and load the file
		if( ::FreeImage_FIFSupportsReading(fif) )
		{
			//BLADE_LW_PROFILING(FREE_IMAGE_LOAD);
			dib = ::FreeImage_LoadFromHandle(fif, FreeImageBridge::getSingleton().getIO(), mem, static_cast<fi_handle>( stream.ptr() ) );
		}
		//if the image failed to load, return failure
		else
			return false;
			
		//fill data
		PixelFormat format;
		if( dib != NULL )
		{
			//BITMAPINFOHEADER* header = ::FreeImage_GetInfoHeader(dib);
			//BITMAPINFO* info = ::FreeImage_GetInfo(dib);
			format = FreeImageBridge::getSingleton().getPixelFormatFromDIB(dib);

			size_t depth = 1;
			size_t mipmap = 1;
			size_t width = (size_t)::FreeImage_GetWidth(dib);
			size_t height = (size_t)::FreeImage_GetHeight(dib);
			this->initialize(width, height, mipmap, format, depth);
			assert( this->getBytes() == width*height*::FreeImage_GetBPP(dib)/8 );

			size_t destPitch = (size_t)this->getPitch();
			size_t sourcePitch = (size_t)::FreeImage_GetPitch(dib);

			if( destPitch == sourcePitch )
				std::memcpy(this->getBits(), ::FreeImage_GetBits(dib), this->getBytes() );
			else
			{
				uint8* dest = this->getBits();
				uint8* source = (uint8*)::FreeImage_GetBits(dib);
				size_t bytes = std::min<size_t>( destPitch, sourcePitch );
				for(size_t i = 0; i < this->getHeight(); ++i)
				{
					std::memcpy(dest, source, bytes);
					dest += destPitch;
					source += sourcePitch;
				}
				assert( dest - this->getBits() == (intptr_t)this->getBytes() );
			}

			::FreeImage_Unload(mem, dib);
		}

		//FreeImage 's DIB is bottom up
		mLayout = IMGO_BOTTOM_UP;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ImageBase::create(size_t width, size_t height, PixelFormat format, IMG_ORIENT layout, int LOD, int depth, int faces)
	{
		if( mInfo.depth <= 1 && mInfo.faces == 0 && mInfo.mipmap == 0 && this->getMipInfo(0) == NULL )
		{
			mLayout = layout;
			this->initialize(width, height, (size_t)LOD, format, (size_t)depth, (size_t)faces);
			return this->getMipInfo(0) != NULL;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	ImageBase*		ImageBase::clone(IMAGE_POOL pool) const
	{
		ImageBase* image = ImageBase::createImage(pool);
		assert( image != NULL );

		//initialize memory
		image->initialize( this->getImageWidth(), this->getImageHeight(), this->getMipmapCount(), this->getFormat(), this->getDepth(), this->getFaceCount() );
		image->mLayout = mLayout;

		//copy data
		for(size_t f = 0; f < image->getFaceCount(); ++f)
		{
			for(size_t i = 0; i < image->getMipmapCount(); ++i)
			{
				const IMG_MIPINFO* src = this->getMipInfo(i,f);
				const IMG_MIPINFO* dest = image->getMipInfo(i,f);
				assert( src->width == dest->width && src->height == dest->height );
				assert( src->oriWidth == dest->oriWidth && src->oriHeight == dest->oriHeight );
				assert( src->bytes == dest->bytes );
				std::memcpy(dest->data, src->data, dest->bytes);
			}
		}
		return image;
	}
	
}//namespace Blade