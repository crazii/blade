/********************************************************************
	created:	2015/01/28
	filename: 	KTX.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "KTX.h"
//Note: additional dependency(BladeDevice) for this single file
#define BLADE_USE_GLES 1
#include <graphics/GLES/public/GLESPixelFormat.h>
#include <interface/public/ISerializer.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/*
 * @private
 * @~English
 * @name Supported Sized Format Macros
 *
 * These macros describe values that may be used with the sizedFormats
 * variable.
 */
/**@{*/
#define _NON_LEGACY_FORMATS 0x1 /*< @private @internal non-legacy sized formats are supported. */
#define _LEGACY_FORMATS 0x2  /*< @private @internal legacy sized formats are supported. */
/*
 * @private
 * @~English
 * @brief all sized formats are supported
 */
#define _ALL_SIZED_FORMATS (_NON_LEGACY_FORMATS | _LEGACY_FORMATS)
#define _NO_SIZED_FORMATS 0 /*< @private @internal no sized formats are supported. */

static int sizedFormats = _ALL_SIZED_FORMATS;
#if SUPPORT_LEGACY_FORMAT_CONVERSION
static bool supportsSwizzle = true;
#endif

namespace Blade
{
	namespace KTXUtil
	{
		//////////////////////////////////////////////////////////////////////////
		void swapEndian16(uint16* pData16, int count)
		{
			int i;
			for (i = 0; i < count; ++i)
			{
				uint16 x = *pData16;
				*pData16++ = uint16((x << 8) | (x >> 8));
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void swapEndian32(uint32* pData32, int count)
		{
			int i;
			for (i = 0; i < count; ++i)
			{
				uint32 x = *pData32;
				*pData32++ = (x << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | (x >> 24);
			}
		}

		typedef struct KTXtexinfo_t {
			/* Data filled in by checkHeaderKTX() */
			uint32 textureDimensions;
			uint32 glTarget;
			uint32 compressed;
			uint32 generateMipmaps;
		} KTX_texinfo;

		//////////////////////////////////////////////////////////////////////////
		KTX_error_code checkHeaderKTX(KTX_header* header, KTX_texinfo* texinfo)
		{
			uint8 identifier_reference[12] = KTX_IDENTIFIER_REF;
			uint32 max_dim;

			/* Compare identifier, is this a KTX file? */
			if (::memcmp(header->identifier, identifier_reference, 12) != 0)
			{
				return KTX_UNKNOWN_FILE_FORMAT;
			}

			if (header->endianness == KTX_ENDIAN_REF_REV)
			{
				/* Convert endianness of header fields if necessary */
				swapEndian32(&header->glType, 12);

				if (header->glTypeSize != 1 ||
					header->glTypeSize != 2 ||
					header->glTypeSize != 4)
				{
					/* Only 8, 16, and 32-bit types supported so far */
					return KTX_INVALID_VALUE;
				}
			}
			else if (header->endianness != KTX_ENDIAN_REF)
			{
				return KTX_INVALID_VALUE;
			}

			/* Check glType and glFormat */
			texinfo->compressed = 0;
			if (header->glType == 0 || header->glFormat == 0)
			{
				if (header->glType + header->glFormat != 0)
				{
					/* either both or none of glType, glFormat must be zero */
					return KTX_INVALID_VALUE;
				}
				texinfo->compressed = 1;
			}

			/* Check texture dimensions. KTX files can store 8 types of textures:
			   1D, 2D, 3D, cube, and array variants of these. There is currently
			   no GL extension that would accept 3D array or cube array textures. */
			if ((header->pixelWidth == 0) ||
				(header->pixelDepth > 0 && header->pixelHeight == 0))
			{
				/* texture must have width */
				/* texture must have height if it has depth */
				return KTX_INVALID_VALUE; 
			}

			texinfo->textureDimensions = 1;
			texinfo->glTarget = GL_TEXTURE_1D;
			texinfo->generateMipmaps = 0;
			if (header->pixelHeight > 0)
			{
				texinfo->textureDimensions = 2;
				texinfo->glTarget = GL_TEXTURE_2D;
			}
			if (header->pixelDepth > 0)
			{
				texinfo->textureDimensions = 3;
				texinfo->glTarget = GL_TEXTURE_3D;
			}

			if (header->numberOfFaces == 6)
			{
				if (texinfo->textureDimensions == 2)
				{
					texinfo->glTarget = GL_TEXTURE_CUBE_MAP;
				}
				else
				{
					/* cube map needs 2D faces */
					return KTX_INVALID_VALUE;
				}
			}
			else if (header->numberOfFaces != 1)
			{
				/* numberOfFaces must be either 1 or 6 */
				return KTX_INVALID_VALUE;
			}

			/* load as 2D texture if 1D textures are not supported */
			if (texinfo->textureDimensions == 1 )
			{
				texinfo->textureDimensions = 2;
				texinfo->glTarget = GL_TEXTURE_2D;
				header->pixelHeight = 1;
			}

			if (header->numberOfArrayElements > 0)
			{
				if (texinfo->glTarget == GL_TEXTURE_1D)
				{
					texinfo->glTarget = GL_TEXTURE_1D_ARRAY_EXT;
				}
				else if (texinfo->glTarget == GL_TEXTURE_2D)
				{
					texinfo->glTarget = GL_TEXTURE_2D_ARRAY_EXT;
				}
				else
				{
					/* No API for 3D and cube arrays yet */
					return KTX_UNSUPPORTEDEXTUREYPE;
				}
				texinfo->textureDimensions++;
			}

			/* Check number of mipmap levels */
			if (header->numberOfMipmapLevels == 0)
			{
				texinfo->generateMipmaps = 1;
				header->numberOfMipmapLevels = 1;
			}
			max_dim = MAX(MAX(header->pixelWidth, header->pixelHeight), header->pixelDepth);
			if (max_dim < ((uint32)1 << (header->numberOfMipmapLevels - 1)))
			{
				/* Can't have more mip levels than 1 + log2(max(width, height, depth)) */
				return KTX_INVALID_VALUE;
			}

			return KTX_SUCCESS;
		}

		//////////////////////////////////////////////////////////////////////////
		PixelFormat		getPixelFormat(const KTX_header& header)
		{
			GLESPixelFormat format = {(GLint)header.glInternalFormat, header.glFormat, header.glType};
			return GLESPixelFormatConverter(format);
		}

		//////////////////////////////////////////////////////////////////////////
		bool			setPixelFormat(PixelFormat format, KTX_header& header)
		{
			GLESPixelFormat glFormat = GLESPixelFormatConverter(format);
			header.glInternalFormat = (uint32)glFormat.internalFormat;
			header.glFormat = glFormat.format;
			header.glType = glFormat.type;
			header.glBaseInternalFormat = glFormat.format;
			if( header.glType != 0 )
				header.glTypeSize = (uint32)format.getSizeBytes();
			else
				header.glTypeSize = 1;

			if( format.isCompressed() )
				header.glFormat = 0;
			return glFormat.internalFormat != 0 || glFormat.format != 0 || glFormat.type != 0;
		}

		//////////////////////////////////////////////////////////////////////////
		KTX_error_code loadTextureKTX(IOBuffer& buffer, KTX_header& header, IOBuffer& outputContent)
		{
			KTX_error_code errorCode = KTX_SUCCESS;
			KTX_texinfo texinfo;

			buffer.read(&header);
			errorCode = checkHeaderKTX(&header, &texinfo);

			if (errorCode != KTX_SUCCESS) {
				return errorCode;
			}

			//skip key-value meta data
			buffer.seekForward(header.bytesOfKeyValueData);

			if (texinfo.glTarget == GL_TEXTURE_CUBE_MAP) {
				texinfo.glTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			}

			uint glInternalFormat = header.glInternalFormat;
			//uint glFormat = header.glFormat;
			if(!texinfo.compressed)
			{
#if SUPPORT_LEGACY_FORMAT_CONVERSION
				// If sized legacy formats are supported there is no need to convert.
				// If only unsized formats are supported, there is no point in converting
				// as the modern formats aren't supported either.
				if (sizedFormats == _NON_LEGACY_FORMATS && supportsSwizzle) {
					#error convertFormat unimplemented
					convertFormat(texinfo.glTarget, &glFormat, &glInternalFormat);
				} else if (sizedFormats == _NO_SIZED_FORMATS)
					glInternalFormat = header.glBaseInternalFormat;
#else
				// When no sized formats are supported, or legacy sized formats are not
				// supported, must change internal format.
				if (sizedFormats == _NO_SIZED_FORMATS
					|| (!(sizedFormats & _LEGACY_FORMATS) &&
					(header.glBaseInternalFormat == GL_ALPHA	
					|| header.glBaseInternalFormat == GL_LUMINANCE
					|| header.glBaseInternalFormat == GL_LUMINANCE_ALPHA
					|| header.glBaseInternalFormat == GL_INTENSITY))) {
						glInternalFormat = header.glBaseInternalFormat;
				}
#endif
			}
			BLADE_UNREFERENCED(glInternalFormat);

			TempBuffer data;
			for (uint32 level = 0; level < header.numberOfMipmapLevels; ++level)
			{
				//size_t pixelWidth  = MAX(1, header.pixelWidth  >> level);
				//size_t pixelHeight = MAX(1, header.pixelHeight >> level);
				//size_t pixelDepth  = MAX(1, header.pixelDepth  >> level);

				uint32 faceLodSize = 0;
				uint32 faceLodSizeRounded = 0;
				if (!buffer.read(&faceLodSize)) {
					errorCode = KTX_UNEXPECTED_END_OF_FILE;
					return errorCode;
				}
				if (header.endianness == KTX_ENDIAN_REF_REV) {
					swapEndian32(&faceLodSize, 1);
				}
				faceLodSizeRounded = (faceLodSize + 3) & ~(uint32)3;		
				if (data.getCapacity() == 0) {
					/* allocate memory sufficient for the first level */
					data.reserve(faceLodSizeRounded);
					if (!data.getData()) {
						errorCode = KTX_OUT_OF_MEMORY;
						return errorCode;
					}
				}
				else if (data.getCapacity() < faceLodSizeRounded) {
					/* subsequent levels cannot be larger than the first level */
					errorCode = KTX_INVALID_VALUE;
					return errorCode;
				}

				for (uint32 face = 0; face < header.numberOfFaces; ++face)
				{
					if (!buffer.readData(data.getData(), faceLodSizeRounded)) {
						errorCode = KTX_UNEXPECTED_END_OF_FILE;
						return errorCode;
					}

					/* Perform endianness conversion on texture data */
					if (header.endianness == KTX_ENDIAN_REF_REV && header.glTypeSize == 2) {
						swapEndian16((uint16*)data.getData(), (int)faceLodSize / 2);
					}
					else if (header.endianness == KTX_ENDIAN_REF_REV && header.glTypeSize == 4) {
						swapEndian32((uint32*)data.getData(), (int)faceLodSize / 4);
					}

					static const uint32 BLOCK_MARK = KTX_BLOCK_MARK;
					outputContent.write(&BLOCK_MARK);
					outputContent.write(&faceLodSizeRounded);
					outputContent.writeData(data.getData(), faceLodSizeRounded);
				}//for each face

			}//for each mip level
			outputContent.rewind();
			return errorCode;
		}

		//////////////////////////////////////////////////////////////////////////
		KTX_error_code	saveTextureKTX(IOBuffer& buffer, const KTX_header& header, const uint8* content, size_t contenSize)
		{
			KTX_error_code errorCode;
			if( content == NULL || contenSize == 0 )
				return KTX_INVALID_VALUE;

			KTX_header check = header;
			KTX_texinfo info;
			errorCode = checkHeaderKTX(&check, &info);
			if( errorCode != KTX_SUCCESS )
				return errorCode;
			
			buffer.write(&header);
			buffer.write(content, contenSize);
			return KTX_SUCCESS;
		}
		
	}//namespace KTXUtil
	
}//namespace Blade