/********************************************************************
	created:	2015/01/28
	filename: 	ETC2EACCommon.h
	author:		Crazii
	
	purpose:	wrapper/declaration of etcpack functions
*********************************************************************/
#ifndef __Blade_ETC2EACCommon_h__
#define __Blade_ETC2EACCommon_h__
#include <BladeTypes.h>
#include <interface/IPlatformManager.h>
#include <interface/public/graphics/Color.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include <cmath>

#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

typedef Blade::uint8 uint8;

bool readCompressParams(void);
void setupAlphaTableAndValtab();

// ETC1 compression 
double compressBlockDiffFlipFast(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockDiffFlipFastPerceptual(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockETC1ExhaustivePerceptual(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockETC1ExhaustivePerceptual(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   decompressBlockDiffFlipC(unsigned int block_part1, unsigned int block_part2, uint8 *img, int width, int height, int startx, int starty, int channels);

// ETC2 compression
void   compressBlockETC2Fast(uint8 *img, uint8* alphaimg, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockETC2RGBA1(uint8 *img, uint8* alphaimg, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockETC2Exhaustive(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockETC2FastPerceptual(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockETC2ExhaustivePerceptual(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2);
void   compressBlockAlphaSlow(uint8* data, int ix, int iy, int width, int height, uint8* returnData);
void   compressBlockAlphaFast(uint8 * data, int ix, int iy, int width, int height, uint8* returnData);
void   compressBlockAlpha16(uint8* data, int ix, int iy, int width, int height, uint8* returnData);
void   decompressBlockETC2c(unsigned int block_part1, unsigned int block_part2, uint8 *img, int width, int height, int startx, int starty, int channels);
void   decompressBlockETC21BitAlphaC(unsigned int block_part1, unsigned int block_part2, uint8 *img, uint8* alphaimg, int width, int height, int startx, int starty, int channelsRGB);
void   decompressBlockAlphaC(uint8* data, uint8* img, int width, int height, int ix, int iy, int channels);
void   decompressBlockAlpha16bitC(uint8* data, uint8* img, int width, int height, int ix, int iy, int channels);

namespace Blade
{

	/** @brief copy 4x4 color block into 16 linear RGB array */
	static BLADE_ALWAYS_INLINE void extractBlockRGB(Color::RGB* colorBlock, const uint8* colors, PixelFormat format, int width)
	{
		assert( !format.isCompressed() );

		Color::RGBA c;
		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			IPlatformManager::prefetch<PM_READ>(colors + width*bytes);
			for(size_t j = 0; j < 4; ++j )
			{
				ColorFormatter::readColorLDR(colors + j*bytes, c, format);
				*(colorBlock++) = c;
			}
			colors += width*bytes;
		}
	}

	/** @brief copy 4x4 color block into 16 linear RGB & Alpha array */
	static BLADE_ALWAYS_INLINE void extractBlockRGBA(Color::RGB* colorBlock, uint8* alphaBlock, const uint8* colors, PixelFormat format, int width)
	{
		assert( !format.isCompressed() );

		Color::RGBA c;
		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			IPlatformManager::prefetch<PM_READ>(colors + width*bytes);
			for(size_t j = 0; j < 4; ++j )
			{
				ColorFormatter::readColorLDR(colors + j*bytes, c, format);
				*(colorBlock++) = c;
				*(alphaBlock++) = c.a;
			}
			colors += width*bytes;
		}
	}

	/** @brief copy 4x4 color block into 16 linear red array */
	static BLADE_ALWAYS_INLINE void extractBlockR(uint8* redBlock, const uint8* colors, PixelFormat format, int width)
	{
		assert( !format.isCompressed() );

		Color::RGBA c;
		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			IPlatformManager::prefetch<PM_READ>(colors + width*bytes);
			for(size_t j = 0; j < 4; ++j )
			{
				ColorFormatter::readColorLDR(colors + j*bytes, c, format);
				*(redBlock++) = c.r;
			}
			colors += width*bytes;
		}
	}

	/** @brief copy 4x4 color block into 16 linear red & green array */
	static BLADE_ALWAYS_INLINE void extractBlockRG(uint8* redBlock, uint8* greenBlock, const uint8* colors, PixelFormat format, int width)
	{
		assert( !format.isCompressed() );

		Color::RGBA c;
		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			IPlatformManager::prefetch<PM_READ>(colors + width*bytes);
			for(size_t j = 0; j < 4; ++j )
			{
				ColorFormatter::readColorLDR(colors + j*bytes, c, format);
				*(redBlock++) = c.r;
				*(greenBlock++) = c.g;
			}
			colors += width*bytes;
		}
	}

	/** @brief */
	static BLADE_ALWAYS_INLINE void extractBlockRGNormalize(uint8* redBlock, uint8* greenBlock, const uint8* colors, PixelFormat format, int width)
	{
		assert( !format.isCompressed() );

		Color c;
		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			IPlatformManager::prefetch<PM_READ>(colors + width*bytes);
			for(size_t j = 0; j < 4; ++j )
			{
				ColorFormatter::readColor(colors + j*bytes, format.getSizeBytes(), c, format);
				c *= 2; c -= 1;
				scalar length = std::sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
				c /= length;
				c += 1;
				c /= 2;

				Color::RGBA rgba = c.getRGBA().color;
				*(redBlock++) = rgba.r;
				*(greenBlock++) = rgba.g;
			}
			colors += width*bytes;
		}
	}

	/** @brief */
	static BLADE_ALWAYS_INLINE void writeBigEndian(uint32 c, uint8* block)
	{
		block[0] = (c >> 24) & 0xFF;
		block[1] = (c >> 16) & 0xFF;
		block[2] = (c >> 8) & 0xFF;
		block[3] = (c >> 0) & 0xFF;
	}

	/** @brief */
	static BLADE_ALWAYS_INLINE void compressBlockETC2RGB(const Color::RGB* rgb, Color::RGBA* imgdec, int width,int height,int startx,int starty, uint8* block)
	{
		static bool perceptual = false;
		uint32 compressed1,compressed2;
		if(perceptual)
			compressBlockETC2FastPerceptual((uint8*)rgb, (uint8*)imgdec, width, height, startx, starty, compressed1, compressed2);
		else
			compressBlockETC2Fast((uint8*)rgb, NULL, (uint8*)imgdec, width, height, startx, starty, compressed1, compressed2);
		writeBigEndian(compressed1, block);
		writeBigEndian(compressed2, block+4);
	}

	/** @brief */
	static BLADE_ALWAYS_INLINE uint32 readBigEndian(const uint8* block)
	{
		uint32 c = uint32( (block[0] << 24) | (block[1] << 16) | (block[2] << 8) | block[3] );
		return c;
	}

	/** @brief */
	static BLADE_ALWAYS_INLINE void decompressBlockETC2RGB(const uint8* block, uint8 *img, int width, int height, int startx, int starty, int channels)
	{
		uint32 block_part1 = readBigEndian(block);
		uint32 block_part2 = readBigEndian(block+4);
		decompressBlockETC2c(block_part1, block_part2, img, width, height, startx, starty, channels);
	}

	/** @brief copy 16 color array into a 4x4 block  */
	static BLADE_ALWAYS_INLINE void insertBlock(uint8* color, const Color::RGBA* colors16, PixelFormat format, int width)
	{
		//convert to target format
		if( format == PF_R8G8B8A8 )
		{
			assert(format.getSizeBytes() == 4);
			for(size_t i = 0; i < 4; ++i)
			{
				std::memcpy(color, colors16, 4*4);
				color += width*4;
				colors16 += 4;
			}
			return;
		}

		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			for(size_t j = 0; j < 4; ++j)
				ColorFormatter::writeColorLDR(color + (i*width+j)*bytes, colors16[i*4+j], format);
		}
	}

}//namespace Blade

#endif//__Blade_ETC2EACCommon_h__