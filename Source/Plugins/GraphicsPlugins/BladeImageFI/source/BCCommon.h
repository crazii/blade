/********************************************************************
	created:	2014/05/16
	filename: 	BCCommon.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BCCommon_h__
#define __Blade_BCCommon_h__
#include <BladeTypes.h>
#include <math/BladeMath.h>
#include <interface/public/graphics/PixelColorFormatter.h>
#include "BCIntrinsics.h"

namespace Blade
{
	/** @brief copy 4x4 color block into 16 linear color array */
	BLADE_ALWAYS_INLINE void ExtractBlock(Color::RGBA* colorBlock, const uint8* colors, PixelFormat format, int width)
	{
		assert( !format.isCompressed() );

		if( format == PF_R8G8B8A8 )
		{
			assert(format.getSizeBytes() == 4);
			for(size_t i = 0; i < 4; ++i)
			{
				IPlatformManager::prefetch<PM_READ>(colors + width*4);
				std::memcpy(colorBlock, colors, 4*4);
				colors += width*4;
				colorBlock += 4;
			}
			return;
		}

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

	/** @brief  */
	BLADE_ALWAYS_INLINE void ExtractBlockNormalize(Color::RGBA* colorBlock, const uint8* colors, PixelFormat format, int width)
	{
		Color c;
		size_t bytes = format.getSizeBytes();
		for(size_t i = 0; i < 4; ++i)
		{
			IPlatformManager::prefetch<PM_READ>(colors + width*bytes);
			for(size_t j = 0; j < 4; ++j )
			{
				ColorFormatter::readColor(colors + j*bytes, bytes, c, format);
				c *= 2; c -= 1;
				scalar length = std::sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
				c /= length;
				c += 1;
				c /= 2;
				(*(colorBlock++)).color = c.getRGBA();
			}
			colors += width*bytes;
		}
	}

	/** @brief copy 16 color array into a 4x4 block  */
	BLADE_ALWAYS_INLINE void InsertBlock(uint8* color, const Color::RGBA* colors16, PixelFormat format, int width)
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

	/** @brief  */
	BLADE_ALWAYS_INLINE uint16 ColorTo565(const uint8 *color)
	{
		int val = ( ( color[ 0 ] >> 3 ) << 11 ) |
			( ( color[ 1 ] >> 2 ) <<  5 ) |
			(   color[ 2 ] >> 3 );
		return (uint16)val;
	}

	/** @brief  */
	BLADE_ALWAYS_INLINE void EmitDoubleWord(uint32 i, uint8 *&outData)
	{
		outData[0] = ( i >> 0 ) & 255;
		outData[1] = ( i >> 8 ) & 255;
		outData[2] = ( i >> 16 ) & 255;
		outData[3] = ( i >> 24 ) & 255;
		outData += 4;
	}

	/** @brief  */
	BLADE_ALWAYS_INLINE void EmitWord(uint8*& outData, uint16 s)
	{
		outData[0] = ( s >> 0 ) & 255u;
		outData[1] = ( s >> 8 ) & 255u;
		outData += 2;
	}

	/** @brief  */
	BLADE_ALWAYS_INLINE void EmitByte(uint8*& outData, uint8 b)
	{
		outData[0] = b;
		outData += 1;
	}

	/** @brief  */
	BLADE_ALWAYS_INLINE void EmitAlpha4Bitx16(uint8*& outData, const Color::RGBA* colorBlock)
	{
		for(size_t i = 0; i < 4; ++i)
		{
			//http://msdn.microsoft.com/en-us/library/windows/desktop/bb694531(v=vs.85).aspx#BC2
			*( ((uint16*&)outData)++ ) = CF_MAKEU16(CF_PACK8TO4(colorBlock[3].a), CF_PACK8TO4(colorBlock[2].a), CF_PACK8TO4(colorBlock[1].a), CF_PACK8TO4(colorBlock[0].a));	//4Bitx4
			colorBlock += 4;
		}
	}

	/** @brief  */
	BLADE_ALWAYS_INLINE void UnpackAlpha4Bitx16(Color::RGBA* color, const uint8* block)
	{
		const uint16* alphaTable = (const uint16*)block;
		for( int i = 0; i < 4; ++i )
		{
			color[4*i+0].a = CF_UNPACK4TO8(CF_U16MSB3(alphaTable[i]));
			color[4*i+1].a = CF_UNPACK4TO8(CF_U16MSB2(alphaTable[i]));
			color[4*i+2].a = CF_UNPACK4TO8(CF_U16MSB1(alphaTable[i]));
			color[4*i+3].a = CF_UNPACK4TO8(CF_U16MSB0(alphaTable[i]));
		}
	}

	/** @brief  */
	void EmitChannelIndices(uint8*& outData, const uint8* colorBlock, Color::EChannel eChannel, const uint8 min, const uint8 max)
	{
		assert(eChannel <= Color::A);
		uint8 indices[16];
		uint8 mid = ( max - min ) / ( 2u * 7u );
		uint8 ab1 = uint8(min + mid);
		uint8 ab2 = ( 6u * max + 1u * min ) / 7u + mid;
		uint8 ab3 = ( 5u * max + 2u * min ) / 7u + mid;
		uint8 ab4 = ( 4u * max + 3u * min ) / 7u + mid;
		uint8 ab5 = ( 3u * max + 4u * min ) / 7u + mid;
		uint8 ab6 = ( 2u * max + 5u * min ) / 7u + mid;
		uint8 ab7 = ( 1u * max + 6u * min ) / 7u + mid;

		colorBlock += eChannel;

		for ( int i = 0; i < 16; i++ )
		{

			uint8 a = colorBlock[i*4];

			int b1 = ( a <= ab1 );
			int b2 = ( a <= ab2 );
			int b3 = ( a <= ab3 );
			int b4 = ( a <= ab4 );
			int b5 = ( a <= ab5 );
			int b6 = ( a <= ab6 );
			int b7 = ( a <= ab7 );

			int index = ( b1 + b2 + b3 + b4 + b5 + b6 + b7 + 1 ) & 7;

			indices[i] = uint8(index ^ ( 2 > index ));
		}

		EmitByte(outData, uint8( (indices[ 0] >> 0) | (indices[ 1] << 3) | (indices[ 2] << 6) ));
		EmitByte(outData, uint8( (indices[ 2] >> 2) | (indices[ 3] << 1) | (indices[ 4] << 4) | (indices[ 5] << 7) ));
		EmitByte(outData, uint8( (indices[ 5] >> 1) | (indices[ 6] << 2) | (indices[ 7] << 5) ));
		EmitByte(outData, uint8( (indices[ 8] >> 0) | (indices[ 9] << 3) | (indices[10] << 6) ));
		EmitByte(outData, uint8( (indices[10] >> 2) | (indices[11] << 1) | (indices[12] << 4) | (indices[13] << 7) ));
		EmitByte(outData, uint8( (indices[13] >> 1) | (indices[14] << 2) | (indices[15] << 5) ));
	}

	/** @brief  */
	void	UnPackChannel(Color::RGBA* color, const uint8* block, Color::EChannel eChannel)
	{
		assert(eChannel <= Color::A);
		uint8 channel[8];
		//get the two alpha values
		channel[0] = block[0];
		channel[1] = block[1];

		//build the alpha table
		if( channel[0] <= channel[1] )
		{
			//5 entry
			for( int i = 1; i < 5; ++i )
				channel[1 + i] = ( uint8 )( ( ( 5 - i )*channel[0] + i*channel[1] )/5 );
			channel[6] = 0;
			channel[7] = 255;
		}
		else
		{
			//7 entry
			for( int i = 1; i < 7; ++i )
				channel[1 + i] = ( uint8 )( ( ( 7 - i )*channel[0] + i*channel[1] )/7 );
		}

		//decode the indices
		uint8 indices[16];
		uint8 const* src = block + 2;
		uint8* dest = indices;
		for( int i = 0; i < 2; ++i )
		{
			// grab 3 bytes
			uint value = 0;
			for( int j = 0; j < 3; ++j )
			{
				uint byte = *src++;
				value |= ( byte << 8*j );
			}

			// unpack 8 3-bit values from it
			for( int j = 0; j < 8; ++j )
			{
				uint index = ( value >> 3*j ) & 0x7;
				*dest++ = ( uint8 )index;
			}
		}

		//decode channel
		for( int i = 0; i < 16; ++i )
			color[i].component[eChannel] = channel[ indices[i] ];
	}

#if !defined(BC_INTRINSICS_SSE2)
	/** @brief  */
	void GetMinMaxColors(uint8* minColor, uint8* maxColor, const uint8 *colorBlock)
	{
		int i;
		uint8 inset[3];
		minColor[0] = minColor[1] = minColor[2] = 255;
		maxColor[0] = maxColor[1] = maxColor[2] = 0;
		for ( i = 0; i < 16; i++ ) {
			if ( colorBlock[i*4+0] < minColor[0] ) { minColor[0] = colorBlock[i*4+0]; }
			if ( colorBlock[i*4+1] < minColor[1] ) { minColor[1] = colorBlock[i*4+1]; }
			if ( colorBlock[i*4+2] < minColor[2] ) { minColor[2] = colorBlock[i*4+2]; }
			if ( colorBlock[i*4+0] > maxColor[0] ) { maxColor[0] = colorBlock[i*4+0]; }
			if ( colorBlock[i*4+1] > maxColor[1] ) { maxColor[1] = colorBlock[i*4+1]; }
			if ( colorBlock[i*4+2] > maxColor[2] ) { maxColor[2] = colorBlock[i*4+2]; }
		}
		inset[0] = (uint8)(( maxColor[0] - minColor[0] ) >> INSET_SHIFT);
		inset[1] = (uint8)(( maxColor[1] - minColor[1] ) >> INSET_SHIFT);
		inset[2] = (uint8)(( maxColor[2] - minColor[2] ) >> INSET_SHIFT);
		minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255u;
		minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255u;
		minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255u;
		maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0u;
		maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0u;
		maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0u;
	}


	/** @brief  */
	void GetMinMaxColorsAlpha(uint8 minColor[16], uint8 maxColor[16], const uint8* colorBlock)
	{
		int i;
		uint8 inset[4];
#if defined(YCOCG_BC3)
		uint8 y,cg, co, r, g, b;
#endif

		std::memset(minColor, 0xFF, 16*sizeof(uint8));
		std::memset(maxColor, 0, 16 * sizeof(uint8));
		BLADE_ALIGNED(64) uint8 cachBlock[64];
		std::memcpy(cachBlock, colorBlock, sizeof(cachBlock));

		for ( i = 0; i < 16; i++ ) {
#if defined(YCOCG_BC3)	//YCoCg
			r = cachBlock[i * 4 + 0];
			g = cachBlock[i * 4 + 1];
			b = cachBlock[i * 4 + 2];
			y = (uint8)((g >> 1) + ((r + b) >> 2));
			cg = (uint8)(g - ((r + b) >> 1));
			co = (uint8)(r - b);

			cachBlock[i * 4 + 0] = co;
			cachBlock[i * 4 + 1] = cg;
			cachBlock[i * 4 + 2] = 0;
			cachBlock[i * 4 + 3] = y;
#endif

			if ( cachBlock[i*4+0] < minColor[0] ) { minColor[0] = cachBlock[i*4+0]; }
			if ( cachBlock[i*4+1] < minColor[1] ) { minColor[1] = cachBlock[i*4+1]; }
			if ( cachBlock[i*4+2] < minColor[2] ) { minColor[2] = cachBlock[i*4+2]; }
			if ( cachBlock[i*4+3] < minColor[3] ) { minColor[3] = cachBlock[i*4+3]; }
			if ( cachBlock[i*4+0] > maxColor[0] ) { maxColor[0] = cachBlock[i*4+0]; }
			if ( cachBlock[i*4+1] > maxColor[1] ) { maxColor[1] = cachBlock[i*4+1]; }
			if ( cachBlock[i*4+2] > maxColor[2] ) { maxColor[2] = cachBlock[i*4+2]; }
			if ( cachBlock[i*4+3] > maxColor[3] ) { maxColor[3] = cachBlock[i*4+3]; }
		}

		inset[0] = (uint8)(( maxColor[0] - minColor[0] ) >> INSET_SHIFT);
		inset[1] = (uint8)(( maxColor[1] - minColor[1] ) >> INSET_SHIFT);
		inset[2] = (uint8)(( maxColor[2] - minColor[2] ) >> INSET_SHIFT);
		inset[3] = (uint8)(( maxColor[3] - minColor[3] ) >> INSET_SHIFT);

		minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255u;
		minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255u;
		minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255u;
		minColor[3] = ( minColor[3] + inset[3] <= 255 ) ? minColor[3] + inset[3] : 255u;

		maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0u;
		maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0u;
		maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0u;
		maxColor[3] = ( maxColor[3] >= inset[3] ) ? maxColor[3] - inset[3] : 0u;
	}
	
	/** @brief  */
	void EmitColorIndices(uint8*& outData, const uint8 *colorBlock, const uint8* minColor, const uint8* maxColor)
	{
		uint16 colors[4][4];
		uint32 result = 0;

		colors[0][0] = (uint16)(( maxColor[0] & C565_5_MASK ) | ( maxColor[0] >> 5 ));
		colors[0][1] = (uint16)(( maxColor[1] & C565_6_MASK ) | ( maxColor[1] >> 6 ));
		colors[0][2] = (uint16)(( maxColor[2] & C565_5_MASK ) | ( maxColor[2] >> 5 ));
		colors[1][0] = (uint16)(( minColor[0] & C565_5_MASK ) | ( minColor[0] >> 5 ));
		colors[1][1] = (uint16)(( minColor[1] & C565_6_MASK ) | ( minColor[1] >> 6 ));
		colors[1][2] = (uint16)(( minColor[2] & C565_5_MASK ) | ( minColor[2] >> 5 ));
		colors[2][0] = (uint16)(( 2 * colors[0][0] + 1 * colors[1][0] ) / 3);
		colors[2][1] = (uint16)(( 2 * colors[0][1] + 1 * colors[1][1] ) / 3);
		colors[2][2] = (uint16)(( 2 * colors[0][2] + 1 * colors[1][2] ) / 3);
		colors[3][0] = (uint16)(( 1 * colors[0][0] + 2 * colors[1][0] ) / 3);
		colors[3][1] = (uint16)(( 1 * colors[0][1] + 2 * colors[1][1] ) / 3);
		colors[3][2] = (uint16)(( 1 * colors[0][2] + 2 * colors[1][2] ) / 3);

		for ( int i = 15; i >= 0; i-- ) {
			int c0 = colorBlock[i*4+0];
			int c1 = colorBlock[i*4+1];
			int c2 = colorBlock[i*4+2];
			int d0 = std::abs( colors[0][0] - c0 ) + std::abs( colors[0][1] - c1 ) + std::abs( colors[0][2] - c2 );
			int d1 = std::abs( colors[1][0] - c0 ) + std::abs( colors[1][1] - c1 ) + std::abs( colors[1][2] - c2 );
			int d2 = std::abs( colors[2][0] - c0 ) + std::abs( colors[2][1] - c1 ) + std::abs( colors[2][2] - c2 );
			int d3 = std::abs( colors[3][0] - c0 ) + std::abs( colors[3][1] - c1 ) + std::abs( colors[3][2] - c2 );

			int b0 = d0 > d3;
			int b1 = d1 > d2;
			int b2 = d0 > d2;
			int b3 = d1 > d3;
			int b4 = d2 > d3;

			int x0 = b1 & b2;
			int x1 = b0 & b3;
			int x2 = b0 & b4;
			result |= ( x2 | ( ( x0 | x1 ) << 1 ) ) << ( i << 1 );
		}
		EmitDoubleWord(result, outData);
	}

#endif
	
}//namespace Blade


#endif // __Blade_BCCommon_h__