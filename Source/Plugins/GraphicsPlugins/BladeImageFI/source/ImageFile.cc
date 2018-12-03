/********************************************************************
	created:	2015/01/29
	filename: 	ImageFile.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/ISerializer.h>
#include "ImageFile.h"
#include "Image.h"
#include "fileformat/KTX.h"
#include "fileformat/DDS.h"

namespace Blade
{
	template class Factory<IImageFile>;
	const TString ImageFileDDS::IMAGEFILE_DDS = BTString("DDS");
	const TString ImageFileKTX::IMAGEFILE_KTX = BTString("KTX");

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ImageFileDDS::loadImage(const HSTREAM& stream, ImageBase* image)
	{
		if( stream == NULL || !stream->isValid() )
			return false;

		IOBuffer buffer(stream);
		const DDSUtil::DDS_HEADER_DXT10* headerDX10;
		uint8* data;
		size_t bytes;
		size_t faceCount = 0;
		DDSUtil::DDS_HEADER header;
		if( DDSUtil::loadTextureData(buffer, &header, &faceCount, &headerDX10, &data, &bytes) == false )
			return false;

		// We could support a subset of 'DX10' extended header DDS files, but we'll assume here we are only
		// supporting legacy DDS files for a Direct3D9 device
		PixelFormat fmt = DDSUtil::getPixelFormat( header.ddspf, headerDX10);

		//some DDS just are RGBA8888 uncompressed format
		if ( fmt == PF_UNDEFINED || fmt.getSizeBytes() == 0 /*|| !fmt.isCompressed()*/ )
			return false;

		image->setOrientation(IMGO_TOP_DOWN);

		//header.dwWidth = (header.dwWidth+3)/4*4;
		//header.dwHeight = (header.dwHeight+3)/4*4;

		size_t iWidth = header.dwWidth;
		size_t iHeight = header.dwHeight;
		size_t iMipCount = header.dwMipMapCount;
		size_t iDepth = header.dwDepth;
		assert( iMipCount >= 1 && (faceCount == 1||faceCount == 6) && iDepth >= 1);

		image->initialize(iWidth, iHeight, iMipCount, fmt, iDepth, faceCount);

		// Lock, fill, unlock
		size_t NumBytes, RowBytes, NumRows;
		const uint8* pSrcBits = data;
		const uint8 *pEndBits = data + bytes;

		for( size_t f = 0; f < faceCount; ++f)
		{
			iWidth = header.dwWidth;
			iHeight = header.dwHeight;

			for( size_t i = 0; i < iMipCount; ++i )
			{
				const IMG_MIPINFO* infoHeader = image->getMipInfo(i, f);
				DDSUtil::getSurfaceInfo( iWidth, iHeight, fmt, &NumBytes, &RowBytes, &NumRows );
				//image's memory layout is compact as stored in file
				assert( infoHeader->rowPitch == RowBytes && infoHeader->slicePitch == NumBytes && infoHeader->bytes == NumBytes*iDepth );

				//overflow check
				if ( ( pSrcBits + (NumBytes*iDepth) ) > pEndBits )
				{
					assert(false);
					image->clear();
					return false;
				}

				if( infoHeader != NULL )
				{
					uint8* pDestBits = infoHeader->data;
					std::memcpy(pDestBits, pSrcBits, infoHeader->bytes);
					pSrcBits += NumBytes;
				}
				else
					assert(false);

				iWidth = iWidth >> 1;
				iHeight = iHeight >> 1;
				iDepth = iDepth >> 1;
				if( iWidth == 0 )
					iWidth = 1;
				if( iHeight == 0 )
					iHeight = 1;
				if( iDepth == 0 )
					iDepth = 1;
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ImageFileDDS::saveImage(const HSTREAM& stream, ImageBase* image)
	{
		DDSUtil::DDS_HEADER header;
		DDSUtil::DDS_HEADER_DXT10 headerDX10;

		std::memset(&header, 0 , sizeof(header));
		std::memset(&headerDX10, 0 , sizeof(headerDX10));
		header.dwSize = sizeof(header);
		header.dwFlags = DDS_HEADER_FLAGS_TEXTURE| ((image->getMipmapCount()>1) ? DDS_SURFACE_FLAGS_MIPMAP : 0u);
		if( image->getDepth() > 1 )
			header.dwFlags |= DDS_HEADER_FLAGS_VOLUME;

		if( image->isCompressed() )
		{
			header.dwFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
			header.dwPitchOrLinearSize = (DWORD)image->getBytes();
		}
		else
		{
			header.dwPitchOrLinearSize = image->getPitch();
			header.dwFlags |= DDS_HEADER_FLAGS_PITCH;
			header.ddspf.dwRGBBitCount = (DWORD)image->getFormat().getSizeBytes() * 8;
		}

		header.dwWidth = image->getImageWidth();
		header.dwHeight = image->getImageHeight();	
		header.dwDepth = image->getDepth();
		header.dwMipMapCount = image->getMipmapCount();
		header.ddspf = DDSUtil::getDDSFormat(image->getFormat());
		header.dwCaps = DDS_SURFACE_FLAGS_TEXTURE|DDS_SURFACE_FLAGS_COMPLEX | ((image->getMipmapCount()>1)? DDS_SURFACE_FLAGS_MIPMAP:0u);

		if( image->getFaceCount() == 6 )
			header.dwCaps2 = DDS_CUBEMAP_ALLFACES;
		else if( image->getDepth() > 1 )
			header.dwCaps2 = DDS_FLAGS_VOLUME;

		bool USE_DX10_HEADER = false;
		if( USE_DX10_HEADER )
		{
			headerDX10.reserved = 0;
			headerDX10.arraySize = 1;
			headerDX10.dxgiFormat = DDSUtil::getDXGIFormat(image->getFormat());
			headerDX10.resourceDimension = image->getDepth() == 1 ? DDSUtil::DDS_DIMENSION_TEXTURE2D : DDSUtil::DDS_DIMENSION_TEXTURE3D;
			headerDX10.miscFlag = image->getFaceCount() == 6u ? (DWORD)DDSUtil::DDS_RESOURCE_MISC_TEXTURECUBE : 0;

			header.ddspf.dwFourCC = MAKEFOURCC( 'D', 'X', '1', '0' );
			header.ddspf.dwFlags |= DDS_FOURCC;
		}

		IOBuffer buffer;
		IOBuffer contentBuffer;

		for(size_t f = 0; f < image->getFaceCount(); ++f)
		{
			for(size_t i = 0; i < image->getMipmapCount(); ++i)
			{
				const IMG_MIPINFO* src = image->getMipInfo(i,f);
				contentBuffer.writeData(src->data, (IStream::Size)src->bytes);
			}
		}
		DDSUtil::saveTexture(buffer, header, &headerDX10, (uint8*)contentBuffer.getData(), (size_t)contentBuffer.tell() );
		return buffer.saveToStream(stream);
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ImageFileKTX::loadImage(const HSTREAM& stream, ImageBase* image)
	{
		KTXUtil::KTX_header header;
		IOBuffer content;
		{
			IOBuffer buffer;
			buffer.loadFromStream(stream);
			if( KTXUtil::loadTextureKTX(buffer, header, content) != KTXUtil::KTX_SUCCESS )
				return false;
		}

		image->setOrientation(IMGO_BOTTOM_UP);

		PixelFormat format = KTXUtil::getPixelFormat(header);

		if( header.pixelDepth == 0 )	//KTX format => blade depth param
			header.pixelDepth = 1;

		image->initialize(header.pixelWidth, header.pixelHeight, header.numberOfMipmapLevels, format, header.pixelDepth, header.numberOfFaces);

		for(uint32 i = 0; i < header.numberOfMipmapLevels; ++i)
		{
			for(uint32 j = 0; j < header.numberOfFaces; ++j)
			{
				if( content.eof() )
				{
					image->clear();
					return false;
				}
				const IMG_MIPINFO* info = image->getMipInfo(i, j);

				uint32 blockMark = 0;
				uint32 blockSize = 0;
				content.read(&blockMark);
				content.read(&blockSize);

				if( blockMark != KTX_BLOCK_MARK || blockSize > info->bytes)
				{
					image->clear();
					return false;
				}

				std::memcpy(info->data, content.getCurrentData(), info->bytes);
				content.seekForward((IStream::Off)info->bytes);
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ImageFileKTX::saveImage(const HSTREAM& stream, ImageBase* image)
	{
		//https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/
		KTXUtil::KTX_header header = KTX_IDENTIFIER_REF;
		header.endianness = KTX_ENDIAN_REF;
		header.pixelWidth = image->getImageWidth();
		header.pixelHeight = image->getImageHeight();
		header.numberOfMipmapLevels = image->getMipmapCount();
		header.numberOfFaces = image->getFaceCount();
		header.numberOfArrayElements = 0;

		header.pixelDepth = image->getDepth();
		if( header.pixelDepth == 1 )
			header.pixelDepth = 0;
		header.bytesOfKeyValueData = 0;

		PixelFormat format = image->getFormat();
		KTXUtil::setPixelFormat(format, header);

		IOBuffer buffer;
		IOBuffer contentBuffer;
		uint8 padding[KTX_GL_UNPACK_ALIGNMENT] = {0};

		for(size_t i = 0; i < image->getMipmapCount(); ++i)
		{
			const IMG_MIPINFO* face0 = image->getMipInfo(i);
			size_t rowPadding = 0;
			if( !format.isCompressed() && format.getSizeBytes() < KTX_GL_UNPACK_ALIGNMENT )
				rowPadding = KTX_GL_UNPACK_ALIGNMENT_PADDING(face0->rowPitch);

			uint32 size32 = (uint32)KTX_GL_UNPACK_ALIGNMENT_PITCH( KTX_GL_UNPACK_ALIGNMENT_PITCH(face0->rowPitch)*face0->height );
			contentBuffer.write(&size32);
			uint32* row = (uint32*)BLADE_TMP_ALIGN_ALLOC(face0->rowPitch, IImage::IMAGE_ALIGNMENT);

			for(size_t f = 0; f < image->getFaceCount(); ++f)
			{
				const IMG_MIPINFO* src = image->getMipInfo(i,f);
				const uint8* data = src->data;
				assert( src->rowPitch == face0->rowPitch );

				for(size_t j = 0; j < src->height; ++j)
				{
					std::memcpy(row, data, src->rowPitch);

					//set big endian
					//note: endian is set on compression/decompressoin
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE && 0
					if( format == PF_RGB_ETC2 )
						KTXUtil::swapEndian32(row, src->rowPitch/4);
					else if(format == PF_RGBA_ETC2EAC)
					{
						//skip alpha: int32x2
						//swap color: int32x2
						for(size_t k = 2; k < src->rowPitch/4; k += 2)
							KTXUtil::swapEndian32(row + k, 2);
					}
#endif
					contentBuffer.writeData(row, src->rowPitch);
					data += src->rowPitch;
					contentBuffer.writeData(padding, (IStream::Size)rowPadding);
				}
				assert( size32 - uint32(src->bytes) <= KTX_GL_UNPACK_ALIGNMENT-1 );
				contentBuffer.write(padding, size32 - uint32(src->bytes) );
			}

			BLADE_TMP_ALIGN_FREE(row);
		}
		KTXUtil::saveTextureKTX(buffer, header, (uint8*)contentBuffer.getData(), (size_t)contentBuffer.tell() );
		return buffer.saveToStream(stream);
	}
}//namespace Blade
