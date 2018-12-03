/********************************************************************
	created:	2014/05/16
	filename: 	BCIntrinsics.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePlatform.h>
#include <math/BladeSIMD.h>
#include <interface/public/graphics/Color.h>

namespace Blade
{

#define C565_5_MASK 0xF8 // 0xFF minus last three bits
#define C565_6_MASK 0xFC // 0xFF minus last two bits
#define INSET_SHIFT 4u // inset the bounding box with ( range >> shift )

#if BLADE_SIMD == BLADE_SIMD_SSE
#	define BC_INTRINSICS_SSE2 1
#	include <emmintrin.h>  // sse2

#	define R_SHUFFLE_D( x, y, z, w ) (( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))
	static BLADE_ALIGNED(16) uint8 SIMD_SSE2_byte_0[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static BLADE_ALIGNED(16) uint16 SIMD_SSE2_word_0[8] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
	static BLADE_ALIGNED(16) uint16 SIMD_SSE2_word_1[8] = { 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001 };
	static BLADE_ALIGNED(16) uint16 SIMD_SSE2_word_2[8] = { 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002 };
	static BLADE_ALIGNED(16) uint16 SIMD_SSE2_word_div_by_3[8] = { (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1, (1<<16)/3+1 };
	static BLADE_ALIGNED(16) uint8 SIMD_SSE2_byte_colorMask[16] = { C565_5_MASK, C565_6_MASK, C565_5_MASK, 0x00, 0x00, 0x00, 0x00, 0x00, C565_5_MASK, C565_6_MASK, C565_5_MASK, 0x00, 0x00, 0x00, 0x00, 0x00 };

	//////////////////////////////////////////////////////////////////////////
	void GetMinMaxColors(uint8 minColor[16], uint8 maxColor[16], const uint8 *colorBlock)
	{
		__m128i t0, t1, t3, t4, t6, t7;

		// get bounding box
		// ----------------

		// load the first row
		t0 = _mm_load_si128 ( (__m128i*) colorBlock );
		t1 = _mm_load_si128 ( (__m128i*) colorBlock );

		__m128i t16 = _mm_load_si128 ( (__m128i*) (colorBlock+16) );
		// Minimum of Packed Unsigned Byte Integers
		t0 = _mm_min_epu8 ( t0, t16);
		// Maximum of Packed Unsigned Byte Integers
		t1 = _mm_max_epu8 ( t1, t16);

		__m128i t32 = _mm_load_si128 ( (__m128i*) (colorBlock+32) );
		t0 = _mm_min_epu8 ( t0, t32);
		t1 = _mm_max_epu8 ( t1, t32);

		__m128i t48 = _mm_load_si128 ( (__m128i*) (colorBlock+48) );
		t0 = _mm_min_epu8 ( t0, t48);
		t1 = _mm_max_epu8 ( t1, t48);

		// Shuffle Packed Doublewords
		t3 = _mm_shuffle_epi32( t0, R_SHUFFLE_D( 2, 3, 2, 3 ) );
		t4 = _mm_shuffle_epi32( t1, R_SHUFFLE_D( 2, 3, 2, 3 ) );

		t0 = _mm_min_epu8 ( t0, t3);
		t1 = _mm_max_epu8 ( t1, t4);

		// Shuffle Packed Low Words
		t6 = _mm_shufflelo_epi16( t0, R_SHUFFLE_D( 2, 3, 2, 3 ) );
		t7 = _mm_shufflelo_epi16( t1, R_SHUFFLE_D( 2, 3, 2, 3 ) );

		t0 = _mm_min_epu8 ( t0, t6);
		t1 = _mm_max_epu8 ( t1, t7);

		// inset the bounding box
		// ----------------------

		// Unpack Low Data
		//__m128i t66 = _mm_set1_epi8( 0 );
		__m128i t66 = _mm_load_si128 ( (__m128i*) SIMD_SSE2_byte_0 );
		t0 = _mm_unpacklo_epi8(t0, t66);
		t1 = _mm_unpacklo_epi8(t1, t66);

		// copy (movdqa)
		//__m128i t2 = _mm_load_si128 ( &t1 );
		__m128i t2 = t1;

		// Subtract Packed Integers
		t2 = _mm_sub_epi16(t2, t0);

		// Shift Packed Data Right Logical 
		t2 = _mm_srli_epi16(t2, INSET_SHIFT);

		// Add Packed Integers
		t0 = _mm_add_epi16(t0, t2);

		t1 = _mm_sub_epi16(t1, t2);

		// Pack with Unsigned Saturation
		t0 = _mm_packus_epi16(t0, t0);
		t1 = _mm_packus_epi16(t1, t1);

		// store bounding box extents
		// --------------------------
		_mm_store_si128 ( (__m128i*) minColor, t0 );
		_mm_store_si128 ( (__m128i*) maxColor, t1 );
	}
#	define GetMinMaxColorsAlpha GetMinMaxColors

	//////////////////////////////////////////////////////////////////////////
	void EmitColorIndices(uint8*& outData, const uint8* colorBlock, const uint8 minColor[16], const uint8 maxColor[16])
	{
		uint8 BLADE_ALIGNED(16) color0[16];
		uint8 BLADE_ALIGNED(16) color1[16];
		uint8 BLADE_ALIGNED(16) color2[16];
		uint8 BLADE_ALIGNED(16) color3[16];
		uint8 BLADE_ALIGNED(16) result[16];

		__m128i t0, t1, t2, t3, t4, t5, t6, t7;

		t7 = _mm_setzero_si128();

		_mm_store_si128 ( (__m128i*) &result, t7 );

		t0 = _mm_cvtsi32_si128( *(int*)maxColor);

		// Bitwise AND
		__m128i tt = _mm_load_si128 ( (__m128i*) SIMD_SSE2_byte_colorMask );
		t0 = _mm_and_si128(t0, tt);

		t0 = _mm_unpacklo_epi8(t0, t7);

		t4 = _mm_shufflelo_epi16( t0, R_SHUFFLE_D( 0, 3, 2, 3 ));
		t5 = _mm_shufflelo_epi16( t0, R_SHUFFLE_D( 3, 1, 3, 3 ));

		t4 = _mm_srli_epi16(t4, 5);
		t5 = _mm_srli_epi16(t5, 6);

		// Bitwise Logical OR
		t0 = _mm_or_si128(t0, t4);
		t0 = _mm_or_si128(t0, t5);   // t0 contains color0 in 565

		t1 = _mm_cvtsi32_si128( *(int*)minColor);

		t1 = _mm_and_si128(t1, tt);

		t1 = _mm_unpacklo_epi8(t1, t7);

		t4 = _mm_shufflelo_epi16( t1, R_SHUFFLE_D( 0, 3, 2, 3 ));
		t5 = _mm_shufflelo_epi16( t1, R_SHUFFLE_D( 3, 1, 3, 3 ));

		t4 = _mm_srli_epi16(t4, 5);
		t5 = _mm_srli_epi16(t5, 6);

		t1 = _mm_or_si128(t1, t4);
		t1 = _mm_or_si128(t1, t5);  // t1 contains color1 in 565

		t2 = t0;

		t2 = _mm_packus_epi16(t2, t7);

		t2 = _mm_shuffle_epi32( t2, R_SHUFFLE_D( 0, 1, 0, 1 ));

		_mm_store_si128 ( (__m128i*) &color0, t2 );

		t6 = t0;
		t6 = _mm_add_epi16(t6, t0);
		t6 = _mm_add_epi16(t6, t1);

		// Multiply Packed Signed Integers and Store High Result
		__m128i tw3 = _mm_load_si128 ( (__m128i*) SIMD_SSE2_word_div_by_3 );
		t6 = _mm_mulhi_epi16(t6, tw3);
		t6 = _mm_packus_epi16(t6, t7);

		t6 = _mm_shuffle_epi32( t6, R_SHUFFLE_D( 0, 1, 0, 1 ));

		_mm_store_si128 ( (__m128i*) &color2, t6 );

		t3 = t1;
		t3 = _mm_packus_epi16(t3, t7);
		t3 = _mm_shuffle_epi32( t3, R_SHUFFLE_D( 0, 1, 0, 1 ));

		_mm_store_si128 ( (__m128i*) &color1, t3 );

		t1 = _mm_add_epi16(t1, t1);
		t0 = _mm_add_epi16(t0, t1);

		t0 = _mm_mulhi_epi16(t0, tw3);
		t0 = _mm_packus_epi16(t0, t7);

		t0 = _mm_shuffle_epi32( t0, R_SHUFFLE_D( 0, 1, 0, 1 ));
		_mm_store_si128 ( (__m128i*) &color3, t0 );

		__m128i w0 = _mm_load_si128 ( (__m128i*) SIMD_SSE2_word_0);
		__m128i w1 = _mm_load_si128 ( (__m128i*) SIMD_SSE2_word_1);
		__m128i w2 = _mm_load_si128 ( (__m128i*) SIMD_SSE2_word_2);

		int x = 32;
		while (x >= 0)
		{
			t3 = _mm_loadl_epi64( (__m128i*) (colorBlock+x+0));
			t3 = _mm_shuffle_epi32( t3, R_SHUFFLE_D( 0, 2, 1, 3 ));

			t5 = _mm_loadl_epi64( (__m128i*) (colorBlock+x+8));
			t5 = _mm_shuffle_epi32( t5, R_SHUFFLE_D( 0, 2, 1, 3 ));

			t0 = t3;
			t6 = t5;
			// Compute Sum of Absolute Difference
			__m128i c0 = _mm_load_si128 ( (__m128i*)  color0 );
			t0 = _mm_sad_epu8(t0, c0);
			t6 = _mm_sad_epu8(t6, c0);
			// Pack with Signed Saturation 
			t0 = _mm_packs_epi32 (t0, t6);

			t1 = t3;
			t6 = t5;
			__m128i c1 = _mm_load_si128 ( (__m128i*)  color1 );
			t1 = _mm_sad_epu8(t1, c1);
			t6 = _mm_sad_epu8(t6, c1);
			t1 = _mm_packs_epi32 (t1, t6);

			t2 = t3;
			t6 = t5;
			__m128i c2 = _mm_load_si128 ( (__m128i*)  color2 );
			t2 = _mm_sad_epu8(t2, c2);
			t6 = _mm_sad_epu8(t6, c2);
			t2 = _mm_packs_epi32 (t2, t6);

			__m128i c3 = _mm_load_si128 ( (__m128i*)  color3 );
			t3 = _mm_sad_epu8(t3, c3);
			t5 = _mm_sad_epu8(t5, c3);
			t3 = _mm_packs_epi32 (t3, t5);


			t4 = _mm_loadl_epi64( (__m128i*) (colorBlock+x+16));
			t4 = _mm_shuffle_epi32( t4, R_SHUFFLE_D( 0, 2, 1, 3 ));

			t5 = _mm_loadl_epi64( (__m128i*) (colorBlock+x+24));
			t5 = _mm_shuffle_epi32( t5, R_SHUFFLE_D( 0, 2, 1, 3 ));

			t6 = t4;
			t7 = t5;
			t6 = _mm_sad_epu8(t6, c0);
			t7 = _mm_sad_epu8(t7, c0);
			t6 = _mm_packs_epi32 (t6, t7);
			t0 = _mm_packs_epi32 (t0, t6);  // d0

			t6 = t4;
			t7 = t5;
			t6 = _mm_sad_epu8(t6, c1);
			t7 = _mm_sad_epu8(t7, c1);
			t6 = _mm_packs_epi32 (t6, t7);
			t1 = _mm_packs_epi32 (t1, t6);  // d1

			t6 = t4;
			t7 = t5;
			t6 = _mm_sad_epu8(t6, c2);
			t7 = _mm_sad_epu8(t7, c2);
			t6 = _mm_packs_epi32 (t6, t7);
			t2 = _mm_packs_epi32 (t2, t6);  // d2

			t4 = _mm_sad_epu8(t4, c3);
			t5 = _mm_sad_epu8(t5, c3);
			t4 = _mm_packs_epi32 (t4, t5);
			t3 = _mm_packs_epi32 (t3, t4);  // d3

			t7 = _mm_load_si128 ( (__m128i*) result );

			t7 = _mm_slli_epi32( t7, 16);

			t4 = t0;
			t5 = t1;
			// Compare Packed Signed Integers for Greater Than
			t0 = _mm_cmpgt_epi16(t0, t3); // b0
			t1 = _mm_cmpgt_epi16(t1, t2); // b1
			t4 = _mm_cmpgt_epi16(t4, t2); // b2
			t5 = _mm_cmpgt_epi16(t5, t3); // b3
			t2 = _mm_cmpgt_epi16(t2, t3); // b4

			t4 = _mm_and_si128(t4, t1); // x0
			t5 = _mm_and_si128(t5, t0); // x1
			t2 = _mm_and_si128(t2, t0); // x2

			t4 = _mm_or_si128(t4, t5);
			t2 = _mm_and_si128(t2, w1);
			t4 = _mm_and_si128(t4, w2);
			t2 = _mm_or_si128(t2, t4);

			t5 = _mm_shuffle_epi32( t2, R_SHUFFLE_D( 2, 3, 0, 1 ));

			// Unpack Low Data
			t2 = _mm_unpacklo_epi16 ( t2, w0);
			t5 = _mm_unpacklo_epi16 ( t5, w0);

			//t5 = _mm_slli_si128 ( t5, 8);
			t5 = _mm_slli_epi32( t5, 8);

			t7 = _mm_or_si128(t7, t5);
			t7 = _mm_or_si128(t7, t2);

			_mm_store_si128 ( (__m128i*) &result, t7 );

			x -=32;
		}

		t4 = _mm_shuffle_epi32( t7, R_SHUFFLE_D( 1, 2, 3, 0 ));
		t5 = _mm_shuffle_epi32( t7, R_SHUFFLE_D( 2, 3, 0, 1 ));
		t6 = _mm_shuffle_epi32( t7, R_SHUFFLE_D( 3, 0, 1, 2 ));

		t4 = _mm_slli_epi32 ( t4, 2);
		t5 = _mm_slli_epi32 ( t5, 4);
		t6 = _mm_slli_epi32 ( t6, 6);

		t7 = _mm_or_si128(t7, t4);
		t7 = _mm_or_si128(t7, t5);
		t7 = _mm_or_si128(t7, t6);

		int r = _mm_cvtsi128_si32 (t7);
		std::memcpy(outData, &r, 4);   // Anything better ?

		outData += 4;
	}
#elif (BLADE_MATH_SIMD == BLADE_MATH_SIMD_NEON)

#endif
	
}//namespace Blade