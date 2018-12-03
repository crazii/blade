/********************************************************************
	created:	2014/05/15
	filename: 	BC.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wattributes"
#endif

#include "BC.h"
#include "BCCommon.h"

namespace Blade
{

	namespace BC
	{

		/************************************************************************/
		/* compression                                                                     */
		/************************************************************************/

		//////////////////////////////////////////////////////////////////////////
		void compressBlockBC1(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			uint8 BLADE_ALIGNED(64) *outData = outBlock;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];

			ExtractBlock(block, colors, format, 4);

			GetMinMaxColors(minColor, maxColor, (const uint8*)block);

			EmitWord(outData, ColorTo565(maxColor));
			EmitWord(outData, ColorTo565(minColor));
			EmitColorIndices(outData, (const uint8*)block, minColor, maxColor);
		}

		//////////////////////////////////////////////////////////////////////////
		void compressBlockBC2(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			uint8 BLADE_ALIGNED(64) *outData = outBlock;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];

			ExtractBlock(block, colors, format, 4);
			GetMinMaxColors(minColor, maxColor, (const uint8*)block);

			EmitAlpha4Bitx16(outData, block);

			EmitWord(outData, ColorTo565(maxColor));
			EmitWord(outData, ColorTo565(minColor));
			EmitColorIndices(outData, (const uint8*)block, minColor, maxColor);
		}

		//////////////////////////////////////////////////////////////////////////
		void compressBlockBC3(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			uint8 BLADE_ALIGNED(64) *outData = outBlock;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];

			ExtractBlock(block, colors, format, 4);

			GetMinMaxColorsAlpha(minColor, maxColor, (uint8*)block);

			EmitByte(outData, maxColor[Color::A]);
			EmitByte(outData, minColor[Color::A]);
			EmitChannelIndices(outData, (const uint8*)block, Color::A, minColor[Color::A], maxColor[Color::A]);

			EmitWord(outData, ColorTo565(minColor));
			EmitWord(outData, ColorTo565(maxColor));
			EmitColorIndices(outData, (const uint8*)block, minColor, maxColor);
		}

		//////////////////////////////////////////////////////////////////////////
		void compressBlockBC4(uint8 *outBlock, const uint8* colors, PixelFormat format)
		{
			uint8 BLADE_ALIGNED(64) *outData = outBlock;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];

			ExtractBlock(block, colors, format, 4);

			GetMinMaxColors(minColor, maxColor, (const uint8*)block);

			EmitByte(outData, minColor[Color::R]);
			EmitByte(outData, minColor[Color::R]);
			EmitChannelIndices(outData, (const uint8*)block, Color::R, minColor[Color::R], maxColor[Color::R]);
		}

		//////////////////////////////////////////////////////////////////////////
		void compressBlockBC5(uint8 *outBlock, const uint8* colors, PixelFormat format, bool normalize)
		{
			uint8 BLADE_ALIGNED(64) *outData = outBlock;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];

			if( normalize )
				ExtractBlockNormalize(block, colors, format, 4);
			else
				ExtractBlock(block, colors, format, 4);

			GetMinMaxColors(minColor, maxColor, (const uint8*)block);

			EmitByte(outData, maxColor[Color::R]);
			EmitByte(outData, minColor[Color::R]);
			EmitChannelIndices(outData, (const uint8*)block, Color::R, minColor[Color::R], maxColor[Color::R]);

			EmitByte(outData, maxColor[Color::G]);
			EmitByte(outData, minColor[Color::G]);
			EmitChannelIndices(outData, (const uint8*)block, Color::G, minColor[Color::G], maxColor[Color::G]);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t compressImageBC1(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];
			size_t pixelBytes = format.getSizeBytes();
			size_t compressedBytes = PixelFormat(PF_BC1).getSizeBytes();

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + (i+4)*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + compressedBytes);
					ExtractBlock(block, colors + i*pixelBytes, format, width);

					GetMinMaxColors(minColor, maxColor, (const uint8*)block);

					EmitWord(outData, ColorTo565(maxColor));
					EmitWord(outData, ColorTo565(minColor));
					EmitColorIndices(outData, (const uint8*)block, minColor, maxColor);
				}
				colors += pixelBytes*width*4;
			}
			return size_t(outData - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t compressImageBC2(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];
			size_t pixelBytes = format.getSizeBytes();
			size_t compressedBytes = PixelFormat(PF_BC2).getSizeBytes();

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + (i+4)*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + compressedBytes);

					ExtractBlock(block, colors + i*pixelBytes, format, width);

					GetMinMaxColors(minColor, maxColor, (const uint8*)colors + i*pixelBytes);

					EmitAlpha4Bitx16(outData, block);

					EmitWord(outData, ColorTo565(maxColor));
					EmitWord(outData, ColorTo565(minColor));
					EmitColorIndices(outData, (const uint8*)block, minColor, maxColor);
				}
				 colors += pixelBytes*width*4;
			}
			return size_t(outData - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t compressImageBC3(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];
			size_t pixelBytes = format.getSizeBytes();
			size_t compressedBytes = PixelFormat(PF_BC3).getSizeBytes();

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + (i+4)*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + compressedBytes);

					ExtractBlock(block, colors + i*pixelBytes, format, width);

					GetMinMaxColorsAlpha(minColor, maxColor, (uint8*)block);

					EmitByte(outData, maxColor[Color::A]);
					EmitByte(outData, minColor[Color::A]);
					EmitChannelIndices(outData, (const uint8*)block, Color::A, minColor[Color::A], maxColor[Color::A]);

					EmitWord(outData, ColorTo565(maxColor));
					EmitWord(outData, ColorTo565(minColor));
					EmitColorIndices(outData, (const uint8*)block, minColor, maxColor);
				}
				 colors += pixelBytes*width*4;
			}
			return size_t(outData - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t compressImageBC4(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];
			size_t pixelBytes = format.getSizeBytes();
			size_t compressedBytes = PixelFormat(PF_BC4).getSizeBytes();

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + (i+4)*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + compressedBytes);

					ExtractBlock(block, colors + i*pixelBytes, format, width);

					GetMinMaxColors(minColor, maxColor, (const uint8*)block);

					EmitByte(outData, minColor[Color::R]);
					EmitByte(outData, minColor[Color::R]);
					EmitChannelIndices(outData, (const uint8*)block, Color::R, minColor[Color::R], maxColor[Color::R]);
				}
				colors += pixelBytes*width*4;
			}
			return size_t(outData - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t compressImageBC5(uint8 *outBuffer, const uint8* colors, int width, int height, PixelFormat format, bool normalize)
		{
			assert( width % 4 == 0 );
			assert( height % 4 == 0 );
			uint8 BLADE_ALIGNED(64) *outData = outBuffer;
			Color::RGBA BLADE_ALIGNED(64) block[16];
			uint8 BLADE_ALIGNED(64) minColor[16];
			uint8 BLADE_ALIGNED(64) maxColor[16];
			size_t pixelBytes = format.getSizeBytes();
			size_t compressedBytes = PixelFormat(PF_BC5).getSizeBytes();

			for (int j = 0; j < height; j += 4)
			{
				for (int i = 0; i < width; i += 4)
				{
					IPlatformManager::prefetch<PM_READ>(colors + (i+4)*pixelBytes);
					IPlatformManager::prefetch<PM_WRITE>(outData + compressedBytes);

					if( normalize )
						ExtractBlockNormalize(block, colors + i*pixelBytes, format, width);
					else
						ExtractBlock(block, colors + i*pixelBytes, format, width);

					GetMinMaxColors(minColor, maxColor, (const uint8*)block);
					
					EmitByte(outData, maxColor[Color::R]);
					EmitByte(outData, minColor[Color::R]);
					EmitChannelIndices(outData, (const uint8*)block, Color::R, minColor[Color::R], maxColor[Color::R]);

					EmitByte(outData, maxColor[Color::G]);
					EmitByte(outData, minColor[Color::G]);
					EmitChannelIndices(outData, (const uint8*)block, Color::G, minColor[Color::G], maxColor[Color::G]);
				}
				colors += pixelBytes*width*4;
			}
			return size_t(outData - outBuffer);
		}


		/************************************************************************/
		/* decompression                                                                     */
		/************************************************************************/
		//////////////////////////////////////////////////////////////////////////
		static inline void decompressBlockBC1Internal(Color::RGBA* outColors, const uint8 *block, bool isBC1 = false)
		{
			BLADE_ALIGNED(64) Color::RGBA c[4];
			const uint16* color = (const uint16*)block;
			block += 4;
			//min,max
			ColorFormatter::readColorLDR(&color[0], c[0], PF_B5G6R5);
			ColorFormatter::readColorLDR(&color[1], c[1], PF_B5G6R5);

			//mid point
			for( int i = 0; i < 3; ++i )
			{
				int c0 = c[0].component[i];
				int c1 = c[1].component[i];

				if( isBC1 && color[0] <= color[1] )
				{
					c[2].component[i] = (uint8)( ( c0 + c1 )/2 );
					c[3].component[i] = 0;
				}
				else
				{
					c[2].component[i] = (uint8)( ( 2*c0 + c1 )/3 );
					c[3].component[i] = (uint8)( ( c0 + 2*c1 )/3 );
				}
			}

			//alpha
			c[2].a = 255u;
			c[3].a = (isBC1 && color[0] <= color[1] ) ? 0 : 255u;

			//indices
			uint8 indices[16];
			for( int i = 0; i < 4; ++i )
			{
				uint8* index = indices + 4*i;
				uint8 packed = block[i];

				index[0] = packed & 0x3u;
				index[1] = ( packed >> 2u ) & 0x3u;
				index[2] = ( packed >> 4u ) & 0x3u;
				index[3] = ( packed >> 6u ) & 0x3u;
			}

			for( int i = 0; i < 16; ++i )
				std::memcpy(&outColors[i], &c[ indices[i] ], sizeof(Color::RGBA) );
		}

		//////////////////////////////////////////////////////////////////////////
		void decompressBlockBC1(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			BLADE_ALIGNED(64) Color::RGBA color[16];
			std::memset(color, 0, sizeof(color));

			decompressBlockBC1Internal(color, block, true);

			InsertBlock(outColors, color, format, 4);
		}

		//////////////////////////////////////////////////////////////////////////
		void decompressBlockBC2(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			BLADE_ALIGNED(64) Color::RGBA color[16];
			std::memset(color, 0, sizeof(color));

			BC::decompressBlockBC1Internal(color, block+8);
			//unpack alpha values later, or it will get overwritten
			UnpackAlpha4Bitx16(color, block);

			InsertBlock(outColors, color, format, 4);
		}

		//////////////////////////////////////////////////////////////////////////
		void decompressBlockBC3(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			BLADE_ALIGNED(64) Color::RGBA color[16];
			std::memset(color, 0, sizeof(color));

			decompressBlockBC1Internal(color, block + 8);

			UnPackChannel(color, block, Color::A);

			InsertBlock(outColors, color, format, 4);
		}

		//////////////////////////////////////////////////////////////////////////
		void decompressBlockBC4(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			BLADE_ALIGNED(64) Color::RGBA color[16];
			std::memset(color, 0, sizeof(color));
			UnPackChannel(color, block, Color::R);
			InsertBlock(outColors, color, format, 4);
		}

		//////////////////////////////////////////////////////////////////////////
		void decompressBlockBC5(uint8* outColors, const uint8 *block, PixelFormat format)
		{
			BLADE_ALIGNED(64) Color::RGBA color[16];
			std::memset(color, 0, sizeof(color));
			UnPackChannel(color, block, Color::R);
			UnPackChannel(color, block+8, Color::G);
			//make b channel have valid value, in case target 'format' contains B channel
			for(size_t i = 0; i < 16; ++i)
			{
				Color c = color[i];
				c *= scalar(255.0/128.0);
				c -= 1;

				c[Color::B] = std::sqrt(1 - c[Color::R]*c[Color::R] - c[Color::G]*c[Color::G]);

				c += 1;
				c /= scalar(255.0/128.0);
				color[i] = c;
			}
			InsertBlock(outColors, color, format, 4);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t decompressImageBC1(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			BLADE_ALIGNED(64) Color::RGBA color[16];
			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			const size_t blockBytes = PixelFormat(PF_BC1).getSizeBytes();

			size_t blockWidth = size_t((width+3)/4);
			size_t blockHeight = size_t((height+3)/4);

			std::memset(color, 0, sizeof(color));
			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + blockBytes);
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					decompressBlockBC1Internal(color, blocks, true);
					blocks += blockBytes;
					InsertBlock(buffer, color, format, width);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return size_t(buffer - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t decompressImageBC2(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			BLADE_ALIGNED(64) Color::RGBA color[16];
			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			const size_t blockBytes = PixelFormat(PF_BC2).getSizeBytes();

			size_t blockWidth = size_t((width+3)/4);
			size_t blockHeight = size_t((height+3)/4);

			std::memset(color, 0, sizeof(color));
			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + blockBytes);
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					BC::decompressBlockBC1Internal(color, blocks+8);
					UnpackAlpha4Bitx16(color, blocks);	//note: bc1 block will extra Alpha, extra alpha first and it will get overwritten
					blocks += blockBytes;

					InsertBlock(buffer, color, format, width);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return size_t(buffer - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t decompressImageBC3(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			BLADE_ALIGNED(64) Color::RGBA color[16];
			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			const size_t blockBytes = PixelFormat(PF_BC3).getSizeBytes();

			size_t blockWidth = size_t((width+3)/4);
			size_t blockHeight = size_t((height+3)/4);

			std::memset(color, 0, sizeof(color));
			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + blockBytes);
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					decompressBlockBC1Internal(color, blocks + 8);
					UnPackChannel(color, blocks, Color::A);	//note: bc1 block will extra Alpha, extra alpha first and it will get overwritten
					blocks += blockBytes;

					InsertBlock(buffer, color, format, width);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return size_t(buffer - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t decompressImageBC4(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			BLADE_ALIGNED(64) Color::RGBA color[16];
			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			const size_t blockBytes = PixelFormat(PF_BC4).getSizeBytes();

			size_t blockWidth = size_t((width+3)/4);
			size_t blockHeight = size_t((height+3)/4);

			std::memset(color, 0, sizeof(color));
			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + blockBytes);
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					UnPackChannel(color, blocks, Color::R);
					blocks += blockBytes;

					InsertBlock(buffer, color, format, width);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return size_t(buffer - outBuffer);
		}

		//////////////////////////////////////////////////////////////////////////
		size_t decompressImageBC5(uint8 *outBuffer, const uint8* blocks, int width, int height, PixelFormat format)
		{
			assert(width % 4 == 0);
			assert(height % 4 == 0);
			BLADE_ALIGNED(64) Color::RGBA color[16];
			BLADE_ALIGNED(64) uint8* buffer = outBuffer;
			size_t pixelBytes = format.getSizeBytes();
			const size_t blockBytes = PixelFormat(PF_BC5).getSizeBytes();

			size_t blockWidth = size_t((width+3)/4);
			size_t blockHeight = size_t((height+3)/4);

			std::memset(color, 0, sizeof(color));
			for (size_t j = 0; j < blockHeight; ++j)
			{
				for (size_t i = 0; i < blockWidth; ++i)
				{
					IPlatformManager::prefetch<PM_READ>(blocks + blockBytes);
					IPlatformManager::prefetch<PM_WRITE>(buffer + 4*pixelBytes);

					UnPackChannel(color, blocks, Color::R);
					UnPackChannel(color, blocks, Color::G);
					blocks += blockBytes;

					InsertBlock(buffer, color, format, width);
					buffer += 4*pixelBytes;
				}
				buffer += width*pixelBytes*3;
			}
			return size_t(buffer - outBuffer);
		}

		
	}//namespace BC

}//namespace Blade