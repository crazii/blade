/********************************************************************
	created:	2015/01/28
	filename: 	ETC2EAC.cc
	author:		Crazii
	
	purpose:	ETC2/EAC block compression
	reference:	
*********************************************************************/
#include <BladePCH.h>
#include "ETC2EAC.h"
#include "ETC2EACCommon.h"

namespace Blade
{
	namespace ETC2EAC
	{

		/************************************************************************/
		/* compression                                                                     */
		/************************************************************************/

		/** @brief compress input 4x4 colors in given format, to ETC2 - RGB  */
		void compressBlockETC2(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			Color::RGBA BLADE_ALIGNED(64) decodeBuffer[4*4];
			Color::RGB BLADE_ALIGNED(64) colorBuffer[4*4];

			extractBlockRGB(colorBuffer, colors, format, 4);
			uint32 c[2];
			compressBlockETC2RGB(colorBuffer, decodeBuffer, 4, 4, 0, 0, (uint8*)&c[0]);
			std::memcpy(outBlock, c, sizeof(c) );
		}

		/** @brief compress input 4x4 colors in given format, to ETC2 - RGBA  */
		void compressBlockETC2EAC(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			Color::RGBA BLADE_ALIGNED(64) decodeBuffer[4*4];
			Color::RGB BLADE_ALIGNED(64) colorBuffer[4*4];
			uint8 BLADE_ALIGNED(64) alphaBuffer[4*4];

			extractBlockRGBA(colorBuffer, alphaBuffer, colors, format, 4);
			uint32 c[4];
			compressBlockAlphaFast(alphaBuffer, 0, 0, 4, 4, (uint8*)&c[0]);
			compressBlockETC2RGB(colorBuffer, decodeBuffer, 4, 4, 0, 0, (uint8*)&c[2]);
			std::memcpy(outBlock, c, sizeof(c) );
		}

		/** @brief compress input 4x4 colors in given format, to EAC, using only the R channel  */
		void compressBlockR11EAC(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			uint8 BLADE_ALIGNED(64) redBuffer[4*4];

			extractBlockR(redBuffer, colors, format, 4);
			uint32 c[2];
			compressBlockAlphaFast(redBuffer, 0, 0, 4, 4, (uint8*)&c[0]);
			std::memcpy(outBlock, c, sizeof(c) );
		}

		/** @brief compress input 4x4 colors in given format, to EAC, using only the R,G channel  */
		void compressBlockRG11EAC(uint8 *outBlock, const uint8* colors, PixelFormat format, bool normalize)
		{
			uint8 BLADE_ALIGNED(64) redBuffer[4*4];
			uint8 BLADE_ALIGNED(64) greenBuffer[4*4];

			if( normalize )
				extractBlockRGNormalize(redBuffer, greenBuffer, colors, format, 4);
			else
				extractBlockRG(redBuffer, greenBuffer, colors, format, 4);
			uint32 c[4];
			compressBlockAlphaFast(redBuffer, 0, 0, 4, 4, (uint8*)&c[0]);
			compressBlockAlphaFast(greenBuffer, 0, 0, 4, 4, (uint8*)&c[2]);
			std::memcpy(outBlock, c, sizeof(c) );
		}

		/** @brief  */
		size_t compressImageETC2(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;

			Color::RGBA BLADE_ALIGNED(64) decodeBuffer[4*4];
			Color::RGB BLADE_ALIGNED(64) colorBuffer[4*4];
			uint32 c[2];

			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_RGB_ETC2).getSizeBytes() == sizeof(c) );

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + pixelBytes*4);
					IPlatformManager::prefetch<PM_WRITE>(outData + sizeof(c));

					extractBlockRGB(colorBuffer, colors, format, width);
					compressBlockETC2RGB(colorBuffer, decodeBuffer, 4, 4, 0, 0, (uint8*)&c[0]);
					std::memcpy(outData, c, sizeof(c) );

					colors += pixelBytes*4;
					outData += sizeof(c);
				}
				colors += pixelBytes*width*3;
			}
			return (size_t)(outData - outBuffer);
		}

		/** @brief  */
		size_t compressImageETC2EAC(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;

			Color::RGBA BLADE_ALIGNED(64) decodeBuffer[4*4];
			Color::RGB BLADE_ALIGNED(64) colorBuffer[4*4];
			uint8 BLADE_ALIGNED(64) alphaBuffer[4*4];
			uint32 c[4];

			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_RGBA_ETC2EAC).getSizeBytes() == sizeof(c) );

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + pixelBytes*4);
					IPlatformManager::prefetch<PM_WRITE>(outData + sizeof(c));

					extractBlockRGBA(colorBuffer, alphaBuffer, colors, format, width);
					compressBlockAlphaFast(alphaBuffer, 0, 0, 4, 4, (uint8*)&c[0]);
					compressBlockETC2RGB(colorBuffer, decodeBuffer, 4, 4, 0, 0, (uint8*)&c[2]);
					std::memcpy(outData, c, sizeof(c) );

					colors += pixelBytes*4;
					outData += sizeof(c);
				}
				colors += pixelBytes*width*3;
			}
			return (size_t)(outData - outBuffer);
		}

		/** @brief  */
		size_t compressImageR11EAC(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;

			uint8 BLADE_ALIGNED(64) redBuffer[4*4];
			uint32 c[2];

			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_R_EAC).getSizeBytes() == sizeof(c) );

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + 4*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + sizeof(c));

					extractBlockR(redBuffer, colors, format, width);
					
					compressBlockAlphaFast(redBuffer, 0, 0, 4, 4, (uint8*)&c[0]);
					std::memcpy(outData, c, sizeof(c) );

					colors += pixelBytes*4;
					outData += sizeof(c);
				}
				colors += pixelBytes*width*3;
			}
			return (size_t)(outData - outBuffer);
		}

		/** @brief  */
		size_t compressImageRG11EAC(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format, bool normalize)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;

			uint8 BLADE_ALIGNED(64) redBuffer[4*4];
			uint8 BLADE_ALIGNED(64) greenBuffer[4*4];
			uint32 c[4];

			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_RG_EAC).getSizeBytes() == sizeof(c) );

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + 4*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + sizeof(c));

					if( normalize )
						extractBlockRGNormalize(redBuffer, greenBuffer, colors, format, width);
					else
						extractBlockRG(redBuffer, greenBuffer, colors, format, width);
					compressBlockAlphaFast(redBuffer, 0, 0, 4, 4, (uint8*)&c[0]);
					compressBlockAlphaFast(greenBuffer, 0, 0, 4, 4, (uint8*)&c[2]);
					std::memcpy(outData, c, sizeof(c) );

					colors += pixelBytes*4;
					outData += sizeof(c);
				}
				colors += pixelBytes*width*3;
			}
			return (size_t)(outData - outBuffer);
		}


		/************************************************************************/
		/* decompression                                                                     */
		/************************************************************************/
		/** @brief  */
		void decompressBlockETC2(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );

			uint32 c[2];
			std::memcpy(c, block, sizeof(c));
			decompressBlockETC2RGB((uint8*)&c[0], (uint8*)colorBuffer, 4, 4, 0, 0, 4);

			insertBlock(outColors, colorBuffer, format, 4);
		}

		/** @brief  */
		void decompressBlockETC2EAC(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );

			uint32 c[4];
			std::memcpy(c, block, sizeof(c));
			
			decompressBlockAlphaC((uint8*)&c[0], (uint8*)&colorBuffer[0].a, 4, 4, 0, 0, 4);
			decompressBlockETC2RGB((uint8*)&c[2], (uint8*)colorBuffer, 4, 4, 0, 0, 4);

			insertBlock(outColors, colorBuffer, format, 4);
		}

		/** @brief  */
		void decompressBlockR11EAC(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );

			uint32 c[2];
			std::memcpy(c, block, sizeof(c));

			decompressBlockAlphaC((uint8*)&c[0], (uint8*)&(colorBuffer[0].r), 4, 4, 0, 0, 4);

			insertBlock(outColors, colorBuffer, format, 4);
		}

		/** @brief  */
		void decompressBlockRG11EAC(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );

			uint32 c[4];
			std::memcpy(c, block, sizeof(c));

			decompressBlockAlphaC((uint8*)&c[0], (uint8*)&(colorBuffer[0].r), 4, 4, 0, 0, 4);
			decompressBlockAlphaC((uint8*)&c[2], (uint8*)&(colorBuffer[0].g), 4, 4, 0, 0, 4);

			insertBlock(outColors, colorBuffer, format, 4);
		}


		/** @brief return output bytes */
		/* @note width & height are in original source image dimensions */
		size_t decompressImageETC2(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );
			uint32 c[2];

			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_RGB_ETC2).getSizeBytes() == sizeof(c));

			size_t blockWidth = size_t((width+3)/4);
			size_t blockHeight = size_t((height+3)/4);

			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + sizeof(c));
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					std::memcpy(c, blocks, sizeof(c));
					decompressBlockETC2RGB((uint8*)&c[0], (uint8*)colorBuffer, 4, 4, 0, 0, 4);
					insertBlock(buffer, colorBuffer, format, width);

					blocks += sizeof(c);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return (size_t)(buffer - outBuffer);
		}

		/** @brief  */
		size_t decompressImageETC2EAC(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );
			uint32 c[4];

			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_RGBA_ETC2EAC).getSizeBytes() == sizeof(c));

			size_t blockWidth = (size_t)((width+3)/4);
			size_t blockHeight = (size_t)((height+3)/4);

			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + sizeof(c));
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					std::memcpy(c, blocks, sizeof(c));
					decompressBlockAlphaC((uint8*)&c[0], (uint8*)&colorBuffer[0].a, 4, 4, 0, 0, 4);
					decompressBlockETC2RGB((uint8*)&c[2], (uint8*)colorBuffer, 4, 4, 0, 0, 4);
					insertBlock(buffer, colorBuffer, format, width);

					blocks += sizeof(c);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return (size_t)(buffer - outBuffer);
		}

		/** @brief  */
		size_t decompressImageR11EAC(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );
			uint32 c[2];

			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_R_EAC).getSizeBytes() == sizeof(c));

			size_t blockWidth = (size_t)((width+3)/4);
			size_t blockHeight = (size_t)((height+3)/4);

			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + sizeof(c));
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					std::memcpy(c, blocks, sizeof(c));
					decompressBlockAlphaC((uint8*)&c[0], (uint8*)&(colorBuffer[0].r), 4, 4, 0, 0, 4);
					insertBlock(buffer, colorBuffer, format, width);

					blocks += sizeof(c);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return (size_t)(buffer - outBuffer);
		}

		/** @brief  */
		size_t decompressImageRG11EAC(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			Color::RGBA BLADE_ALIGNED(64) colorBuffer[4*4];
			std::memset(colorBuffer, 0, sizeof(colorBuffer) );
			uint32 c[4];

			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			assert( PixelFormat(PF_RG_EAC).getSizeBytes() == sizeof(c));

			size_t blockWidth = (size_t)((width+3)/4);
			size_t blockHeight = (size_t)((height+3)/4);

			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + sizeof(c));
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					std::memcpy(c, blocks, sizeof(c));
					decompressBlockAlphaC((uint8*)&c[0], (uint8*)&(colorBuffer[0].r), 4, 4, 0, 0, 4);
					decompressBlockAlphaC((uint8*)&c[2], (uint8*)&(colorBuffer[0].g), 4, 4, 0, 0, 4);
					insertBlock(buffer, colorBuffer, format, width);

					blocks += sizeof(c);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return (size_t)(buffer - outBuffer);
		}



	}//namespace ETC2EAC

}//namespace Blade