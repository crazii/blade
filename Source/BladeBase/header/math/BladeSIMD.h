/********************************************************************
	created:	2014/09/16
	filename: 	BladeSIMD.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeSIMD_h__
#define __Blade_BladeSIMD_h__
#include <BladeTypes.h>
#include <math/BladeMath.h>

#define BLADE_ENABLE_SIMD 1

#define BLADE_SIMD_NONE 0
#define BLADE_SIMD_SSE 1
#define BLADE_SIMD_NEON 2

#if !BLADE_ENABLE_SIMD
#	define BLADE_SIMD BLADE_SIMD_NONE
#endif

#if !defined(BLADE_SIMD)
#	if BLADE_PROCESSOR == BLADE_PROCESSOR_X86
#		define BLADE_SIMD BLADE_SIMD_SSE
#	elif BLADE_PROCESSOR == BLADE_PROCESSOR_ARM
#		define BLADE_SIMD BLADE_SIMD_NEON
#else
#		define BLADE_SIMD BLADE_SIMD_NONE
#		undef BLADE_ENABLE_SIMD
#		define BLADE_ENABLE_SIMD 0
#	endif
#endif


#if BLADE_SIMD == BLADE_SIMD_SSE
#		include <xmmintrin.h>
#		include <emmintrin.h>
#elif BLADE_SIMD == BLADE_SIMD_NEON
#		include <arm_neon.h>
#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		define vcale_f32 vacle_f32
#		define vcaleq_f32 vacleq_f32
#	endif
#else
#endif

#if BLADE_ENABLE_SIMD

#ifdef VMIN
#undef VMIN
#endif

namespace Blade
{
	namespace SIMD
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		typedef BLADE_ALIGNED(8) __m64 fp32x2;
		typedef BLADE_ALIGNED(16) __m128 fp32x4;
#elif BLADE_SIMD == BLADE_SIMD_NEON
		typedef BLADE_ALIGNED(8) float32x2_t fp32x2;
		typedef BLADE_ALIGNED(16) float32x4_t fp32x4;
#else
#endif


		/************************************************************************/
		/* helper: uint32x4 not frequently used                                                                    */
		/************************************************************************/
		struct BLADE_ALIGNED(16) uint32x4
		{
			union
			{
				struct
				{
					uint32 x;
					uint32 y;
					uint32 z;
					uint32 w;
				};

#	if BLADE_SIMD == BLADE_SIMD_SSE
				__m128		data;
#	elif BLADE_SIMD == BLADE_SIMD_NEON
				uint32x4_t	data;
#	else
				uint32	data[4];
#	endif
			};
		};

		typedef fp32x4 vfp32x4;
#	if BLADE_SIMD == BLADE_SIMD_NEON
		typedef fp32x2 vfp32x2;
#	else
		typedef fp32x4 vfp32x2;
#	endif

		/************************************************************************/
		/* basic math function wrapper                                                                     */
		/************************************************************************/
		BLADE_ALWAYS_INLINE static vfp32x4 VSETX(vfp32x4 v, const fp32* x)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 xxxx = _mm_load_ss(x);
			return  _mm_move_ss(v, xxxx);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vld1q_lane_f32(x, v, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VSETY(vfp32x4 v, const fp32* y)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 2, 0, 1));	//yxzw
			vfp32x4 yyyy = _mm_load_ss(y);
			v = _mm_move_ss(v, yyyy);
			return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 2, 0, 1));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vld1q_lane_f32(y, v, 1);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VSETZ(vfp32x4 v, const fp32* z)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 1, 2));	//zyxw
			vfp32x4 zzzz = _mm_load_ss(z);
			v = _mm_move_ss(v, zzzz);
			return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 1, 2));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vld1q_lane_f32(z, v, 2);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VSETW(vfp32x4 v, const fp32* w)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 2, 1, 3));	//wyzx
			vfp32x4 wwww = _mm_load_ss(w);
			v = _mm_move_ss(v, wwww);
			return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 2, 1, 3));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vld1q_lane_f32(w, v, 3);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VGETX(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_cvtss_f32(v);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vgetq_lane_f32(v, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VGETY(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vgetq_lane_f32(v, 1);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VGETZ(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vgetq_lane_f32(v, 2);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VGETW(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vgetq_lane_f32(v, 3);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VADD(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_add_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vaddq_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VSUB(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_sub_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vsubq_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VMUL(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_mul_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmulq_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VDIV(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_div_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 reciprocal = vrecpeq_f32(r);
			//newton-raphson iteration
			reciprocal = vmulq_f32(vrecpsq_f32(r, reciprocal), reciprocal);
			reciprocal = vmulq_f32(vrecpsq_f32(r, reciprocal), reciprocal);
			return vmulq_f32(l, reciprocal);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VADD(vfp32x4 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_add_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vaddq_f32(l, vdupq_n_f32(r));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VSUB(vfp32x4 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_sub_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vsubq_f32(l, vdupq_n_f32(r));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VMUL(vfp32x4 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_mul_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmulq_f32(l, vdupq_n_f32(r));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VDIV(vfp32x4 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_div_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 r4 = vdupq_n_f32(r);
			vfp32x4 reciprocal = vrecpeq_f32(r4);
			//newton-raphson iteration
			reciprocal = vmulq_f32(vrecpsq_f32(r4, reciprocal), reciprocal);
			reciprocal = vmulq_f32(vrecpsq_f32(r4, reciprocal), reciprocal);
			return vmulq_f32(l, reciprocal);;
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VDIV(fp32 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_div_ps(_mm_set_ps1(l), r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 reciprocal = vrecpeq_f32(r);
			//newton-raphson iteration
			reciprocal = vmulq_f32(vrecpsq_f32(r, reciprocal), reciprocal);
			reciprocal = vmulq_f32(vrecpsq_f32(r, reciprocal), reciprocal);
			return vmulq_f32(vdupq_n_f32(l), reciprocal);
#else
#	error not implemented.
#endif
		}

		//get min values on components basis
		BLADE_ALWAYS_INLINE static vfp32x4 VMIN(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_min_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vminq_f32(l, r);
#else
#	error not implemented.
#endif
		}

		//get max values on components basis
		BLADE_ALWAYS_INLINE static vfp32x4 VMAX(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_max_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmaxq_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VABS(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//'sign_mask' : construction of local static object is not thread-safe
#endif

			//mask off IEEE sign bit
			static const vfp32x4 sign_mask = _mm_set1_ps(-0.f); // -0.f = 1 << 31
			vfp32x4 ret = _mm_andnot_ps(sign_mask, v);
			return ret;
#elif BLADE_SIMD == BLADE_SIMD_NEON
			//mask off IEEE sign bit
			static const vfp32x4 sign_mask = vdupq_n_f32(-0.f); // -0.f = 1 << 31
			vfp32x4 ret = vbicq_s32(sign_mask, v);
			return ret;
#else
#	error not implemented.
#endif

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

		}

		BLADE_ALWAYS_INLINE static vfp32x4 VNEG(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_sub_ps(_mm_setzero_ps(), v);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vnegq_f32(v);
#else
#	error not implemented.
#endif
		}



		/************************************************************************/
		/* 3d vector function wrapper                                                                     */
		/************************************************************************/
		BLADE_ALWAYS_INLINE static vfp32x4 VLOAD3(const fp32* f32x3)
		{
#	if BLADE_SIMD == BLADE_SIMD_SSE
#if 1
			vfp32x4 xy = _mm_loadl_pi(_mm_setzero_ps(), (const fp32x2*)f32x3);
			vfp32x4 z = _mm_load_ss(&f32x3[2]);
			return _mm_movelh_ps(xy, z);
#else
			return _mm_loadu_ps(f32x3);	//this will cause undefined w when directly convert to vfp32x4 as used as 4 component vector
#endif
#	elif BLADE_SIMD == BLADE_SIMD_NEON
#if 1
			fp32x2 xy = vld1_f32(f32x3);
			fp32x2 zw = vdup_n_f32(1);
			zw = vld1_lane_f32(f32x3 + 2, zw, 0);
			return vcombine_f32(xy, zw);
#else
			return vld1q_f32(f32x3);
#endif
#	else
#	error not implemented.
#	endif
		}

		BLADE_ALWAYS_INLINE static void VSTORE3(vfp32x4 v, fp32* f32x3)
		{
#	if BLADE_SIMD == BLADE_SIMD_SSE
			_mm_storel_pi((fp32x2*)f32x3, v);					//[0],[1]
			v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));	//[2][2][2][2]
			_mm_store_ss(&f32x3[2], v);	//z
#	elif BLADE_SIMD == BLADE_SIMD_NEON
			fp32x2 xy = vget_low_f32(v);
			vst1_f32(f32x3, xy);
			vst1q_lane_f32(f32x3 + 2, v, 2);
#	else
#	error not implemented.
#	endif
		}

		//compare
		BLADE_ALWAYS_INLINE static bool VEQUAL3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 m = _mm_cmpeq_ps(l, r);
			return (((_mm_movemask_ps(m) & 7) == 7) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vceqq_f32(l, r);
			/*uint8x8x2_t m = */vzip_u8(vget_low_u8((uint8x16_t)result), vget_high_u8((uint8x16_t)result));
			uint16x4x2_t m16x4u = vzip_u16(m16x4u.val[0], m16x4u.val[1]);
			return ((vget_lane_u32(m16x4u.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
#	error not implemented.
#endif	
		}

		BLADE_ALWAYS_INLINE static bool VLESS3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 m = _mm_cmplt_ps(l, r);
			return (((_mm_movemask_ps(m) & 7) == 7) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcltq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m16x4u = vzip_u16(m.val[0], m.val[1]);
			return ((vget_lane_u32(m16x4u.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VLESSEQUAL3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 m = _mm_cmple_ps(l, r);
			return (((_mm_movemask_ps(m) & 7) == 7) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcleq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m16x4u = vzip_u16(m.val[0], m.val[1]);
			return ((vget_lane_u32(m16x4u.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
#	error not implemented.
#endif	
		}

		BLADE_ALWAYS_INLINE static bool VGREATER3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 m = _mm_cmpgt_ps(l, r);
			return (((_mm_movemask_ps(m) & 7) == 7) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcgtq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m16x4u = vzip_u16(m.val[0], m.val[1]);
			return ((vget_lane_u32(m16x4u.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VGREATEREQUAL3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 m = _mm_cmpge_ps(l, r);
			return (((_mm_movemask_ps(m) & 7) == 7) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcgeq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m16x4u = vzip_u16(m.val[0], m.val[1]);
			return ((vget_lane_u32(m16x4u.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
#	error not implemented.
#endif	
		}

		BLADE_ALWAYS_INLINE static bool VEQUALTOLERANCE3(vfp32x4 l, vfp32x4 r, fp32 tolerance)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// Get the difference
			vfp32x4 diff = _mm_sub_ps(l, r);
			// Get the absolute value of the difference
			vfp32x4 result = _mm_setzero_ps();
			result = _mm_sub_ps(result, diff);
			result = _mm_max_ps(result, diff);
			result = _mm_cmple_ps(result, _mm_set_ps1(tolerance));
			// w is don't care
			return (((_mm_movemask_ps(result) & 7) == 0x7) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 diff = vsubq_f32(l, r);
			uint32x4_t result = vcaleq_f32(diff, vdupq_n_f32(tolerance));
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m16x4u = vzip_u16(m.val[0], m.val[1]);
			return ((vget_lane_u32(m16x4u.val[1], 1) & 0xFFFFFFU) == 0xFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VLENGTH3(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// dot3
			vfp32x4 dot = _mm_mul_ps(v, v);
			// m has z and y
			vfp32x4 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 2, 1, 2));
			// x+z, y
			dot = _mm_add_ss(dot, m);
			// y,y,y,y
			m = _mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 1, 1, 1));
			// x+z+y,??,??,??
			dot = _mm_add_ss(dot, m);
			// splat the length squared
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
			// get the length
			dot = _mm_sqrt_ps(dot);
			return _mm_cvtss_f32(dot);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			// dot3
			vfp32x4 m = vmulq_f32(v, v);
			fp32x2 v1 = vget_low_f32(m);
			fp32x2 v2 = vget_high_f32(m);
			v1 = vpadd_f32(v1, v1);
			v2 = vdup_lane_f32(v2, 0);
			v1 = vadd_f32(v1, v2);
			const fp32x2 zero = vdup_n_f32(0);
			uint32x2_t equalZero = vceq_f32(v1, zero);
			// sqrt
			fp32x2 S0 = vrsqrte_f32(v1);
			fp32x2 P0 = vmul_f32(v1, S0);
			fp32x2 R0 = vrsqrts_f32(P0, S0);
			fp32x2 S1 = vmul_f32(S0, R0);
			fp32x2 P1 = vmul_f32(v1, S1);
			fp32x2 R1 = vrsqrts_f32(P1, S1);
			fp32x2 result = vmul_f32(S1, R1);
			result = vmul_f32(v1, result);
			result = vbsl_f32(equalZero, zero, result);
			return vget_lane_f32(result, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VDOT3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// dot3
			vfp32x4 dot = _mm_mul_ps(l, r);
			// x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
			vfp32x4 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2, 1, 2, 1));
			// Result.vector4_f32[0] = x+y
			dot = _mm_add_ss(dot, m);
			// x=Dot.vector4_f32[2]
			m = _mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 1, 1, 1));
			// Result.vector4_f32[0] = (x+y)+z
			dot = _mm_add_ss(dot, m);
			// Splat x
			return _mm_cvtss_f32(dot);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 m = vmulq_f32(l, r);
			fp32x2 v1 = vget_low_f32(m);
			fp32x2 v2 = vget_high_f32(m);
			v1 = vpadd_f32(v1, v1);
			v2 = vdup_lane_f32(v2, 0);
			v1 = vadd_f32(v1, v2);
			return vget_lane_f32(v1, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 VCROSS3(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// y1,z1,x1,w1
			vfp32x4 m1 = _mm_shuffle_ps(l, l, _MM_SHUFFLE(3, 0, 2, 1));
			// z2,x2,y2,w2
			vfp32x4 m2 = _mm_shuffle_ps(r, r, _MM_SHUFFLE(3, 1, 0, 2));
			// Perform the left operation
			vfp32x4 result = _mm_mul_ps(m1, m2);
			// z1,x1,y1,w1
			m1 = _mm_shuffle_ps(m1, m1, _MM_SHUFFLE(3, 0, 2, 1));
			// y2,z2,x2,w2
			m2 = _mm_shuffle_ps(m2, m2, _MM_SHUFFLE(3, 1, 0, 2));
			// perform the right operation
			m1 = _mm_mul_ps(m1, m2);
			// subtract the right from left, and return answer
			return _mm_sub_ps(result, m1);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			fp32x2 v1xy = vget_low_f32(l);
			fp32x2 v2xy = vget_low_f32(r);

			fp32x2 v1yx = vrev64_f32(v1xy);
			fp32x2 v2yx = vrev64_f32(v2xy);

			fp32x2 v1zz = vdup_lane_f32(vget_high_f32(l), 0);
			fp32x2 v2zz = vdup_lane_f32(vget_high_f32(r), 0);

			vfp32x4 result = vmulq_f32(vcombine_f32(v1yx, v1xy), vcombine_f32(v2zz, v2yx));
			result = vmlsq_f32(result, vcombine_f32(v1zz, v1yx), vcombine_f32(v2yx, v2xy));

			//XOR sign bit of y in fp32
			static uint32x4 invertSignY = { 0, 0x80000000, 0, 0 };
			return (vfp32x4)veorq_u32(reinterpret_cast<uint32x4_t&>(result), reinterpret_cast<uint32x4_t&>(invertSignY));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VNORMALIZE3(vfp32x4& v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			static const uint32x4 infinity = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
			static const uint32x4 qnan = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };

			// dot3
			vfp32x4 dot = _mm_mul_ps(v, v);
			vfp32x4 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2, 1, 2, 1));
			dot = _mm_add_ss(dot, m);
			m = _mm_shuffle_ps(m, m, _MM_SHUFFLE(1, 1, 1, 1));
			dot = _mm_add_ss(dot, m);
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
			// prepare for the division
			vfp32x4 result = _mm_sqrt_ps(dot);
			// create zero with a single instruction
			vfp32x4 zeroMask = _mm_setzero_ps();
			// test for a divide by zero (Must be FP to detect -0.0)
			zeroMask = _mm_cmpneq_ps(zeroMask, result);
			// failsafe on zero/ epsilon length planes
			// if the length is infinity, set the elements to zero
			dot = _mm_cmpneq_ps(dot, infinity.data);
			//get single fp32 length value
			fp32 ret = _mm_cvtss_f32(result);
			// divide to perform the normalization
			result = _mm_div_ps(v, result);
			// any that are infinity, set to zero
			result = _mm_and_ps(result, zeroMask);
			// select qnan or result based on infinite length
			vfp32x4 m1 = _mm_andnot_ps(dot, qnan.data);
			vfp32x4 m2 = _mm_and_ps(result, dot);
			result = _mm_or_ps(m1, m2);
			v = result;
			return ret;
#elif BLADE_SIMD == BLADE_SIMD_NEON
			static const uint32x4 infitiy = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
			static const uint32x4 qnan = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };

			// dot3
			vfp32x4 m = vmulq_f32(v, v);
			fp32x2 v1 = vget_low_f32(m);
			fp32x2 v2 = vget_high_f32(m);
			v1 = vpadd_f32(v1, v1);
			v2 = vdup_lane_f32(v2, 0);
			v1 = vadd_f32(v1, v2);
			uint32x2_t equalZero = vceq_f32(v1, vdup_n_f32(0));
			uint32x2_t equalInf = vceq_f32(v1, reinterpret_cast<const fp32x2&>(infitiy));
			// Reciprocal sqrt (2 iterations of Newton-Raphson)
			fp32x2 S0 = vrsqrte_f32(v1);
			fp32x2 P0 = vmul_f32(v1, S0);
			fp32x2 R0 = vrsqrts_f32(P0, S0);
			fp32x2 S1 = vmul_f32(S0, R0);
			fp32x2 P1 = vmul_f32(v1, S1);
			fp32x2 R1 = vrsqrts_f32(P1, S1);
			v2 = vmul_f32(S1, R1);
			// Normalize
			vfp32x4 result = vmulq_f32(v, vcombine_f32(v2, v2));
			result = vbslq_f32(vcombine_u32(equalZero, equalZero), vdupq_n_f32(0), result);
			result = vbslq_f32(vcombine_u32(equalInf, equalInf), reinterpret_cast<const vfp32x4&>(qnan), result);
			v = result;
			return vget_lane_f32(P1, 0);
#else
#	error not implemented.
#endif
		}

		/************************************************************************/
		/* 4d vector function wrapper                                                                     */
		/************************************************************************/
		BLADE_ALWAYS_INLINE static vfp32x4 VLOAD4(const fp32* f32x4)
		{
#	if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_loadu_ps(f32x4);
#	elif BLADE_SIMD == BLADE_SIMD_NEON
			return vld1q_f32(f32x4);
#	else
#	error not implemented.
#	endif
		}

		BLADE_ALWAYS_INLINE static void VSTORE4(vfp32x4 v, fp32* f32x4)
		{
#	if BLADE_SIMD == BLADE_SIMD_SSE
			_mm_storeu_ps(f32x4, v);
#	elif BLADE_SIMD == BLADE_SIMD_NEON
			vst1q_f32(f32x4, v);
#	else
#	error not implemented.
#	endif
		}

		BLADE_ALWAYS_INLINE static bool VEQUAL4(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 cmp = _mm_cmpeq_ps(l, r);
			return ((_mm_movemask_ps(cmp) == 0x0f) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vceqq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m64x2 = vzip_u16(m.val[0], m.val[1]);
			return (vget_lane_u32(m64x2.val[1], 1) == 0xFFFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VLESS4(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 cmp = _mm_cmplt_ps(l, r);
			return ((_mm_movemask_ps(cmp) == 0x0f) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcltq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m64x2 = vzip_u16(m.val[0], m.val[1]);
			return (vget_lane_u32(m64x2.val[1], 1) == 0xFFFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VLESSEQUAL4(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 cmp = _mm_cmple_ps(l, r);
			return ((_mm_movemask_ps(cmp) == 0x0f) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcleq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m64x2 = vzip_u16(m.val[0], m.val[1]);
			return (vget_lane_u32(m64x2.val[1], 1) == 0xFFFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VGREATER4(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 cmp = _mm_cmpgt_ps(l, r);
			return ((_mm_movemask_ps(cmp) == 0x0f) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcgtq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m64x2 = vzip_u16(m.val[0], m.val[1]);
			return (vget_lane_u32(m64x2.val[1], 1) == 0xFFFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VGREATEREQUAL4(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 cmp = _mm_cmpge_ps(l, r);
			return ((_mm_movemask_ps(cmp) == 0x0f) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			uint32x4_t result = vcgeq_f32(l, r);
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m64x2 = vzip_u16(m.val[0], m.val[1]);
			return (vget_lane_u32(m64x2.val[1], 1) == 0xFFFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VEQUALTOLERANCE4(vfp32x4 l, vfp32x4 r, fp32 tolerance)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// Get the difference
			vfp32x4 diff = _mm_sub_ps(l, r);
			// Get the absolute value of the difference
			vfp32x4 result = _mm_setzero_ps();
			result = _mm_sub_ps(result, diff);
			result = _mm_max_ps(result, diff);
			result = _mm_cmple_ps(result, _mm_set_ps1(tolerance));

			return ((_mm_movemask_ps(result) == 0x0f) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 diff = vsubq_f32(l, r);
			uint32x4_t result = vcaleq_f32(diff, vdupq_n_f32(tolerance));
			uint8x8x2_t m = vzip_u8(vget_low_u8(result), vget_high_u8(result));
			uint16x4x2_t m64x2 = vzip_u16(m.val[0], m.val[1]);
			return (vget_lane_u32(m64x2.val[1], 1) == 0xFFFFFFFFU);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VLENGTH4(vfp32x4 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// Perform the dot product on x,y,z and w
			vfp32x4 dot = _mm_mul_ps(v, v);
			// m has z and w
			vfp32x4 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(3, 2, 3, 2));
			// x+z, y+w
			dot = _mm_add_ps(dot, m);
			// x+z,x+z,x+z,y+w
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 0, 0, 0));
			// ??,??,y+w,y+w
			m = _mm_shuffle_ps(m, dot, _MM_SHUFFLE(3, 3, 0, 0));
			// ??,??,x+z+y+w,??
			dot = _mm_add_ps(dot, m);
			// Splat the length
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2, 2, 2, 2));
			// Prepare for the division
			dot = _mm_sqrt_ps(dot);
			return _mm_cvtss_f32(dot);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			// dot4
			vfp32x4 m = vmulq_f32(v, v);
			fp32x2 v1 = vget_low_f32(m);
			fp32x2 v2 = vget_high_f32(m);
			v1 = vpadd_f32(v1, v1);
			v2 = vpadd_f32(v2, v2);
			v1 = vadd_f32(v1, v2);
			const fp32x2 zero = vdup_n_f32(0);
			uint32x2_t equalZero = vceq_f32(v1, zero);
			// sqrt
			fp32x2 S0 = vrsqrte_f32(v1);
			fp32x2 P0 = vmul_f32(v1, S0);
			fp32x2 R0 = vrsqrts_f32(P0, S0);
			fp32x2 S1 = vmul_f32(S0, R0);
			fp32x2 P1 = vmul_f32(v1, S1);
			fp32x2 R1 = vrsqrts_f32(P1, S1);
			fp32x2 result = vmul_f32(S1, R1);
			result = vmul_f32(v1, result);
			result = vbsl_f32(equalZero, zero, result);
			return vget_lane_f32(result, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VDOT4(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x4 r2 = r;
			vfp32x4 r1 = _mm_mul_ps(l, r2);
			r2 = _mm_shuffle_ps(r2, r1, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
			r2 = _mm_add_ps(r2, r1);          // Add Z = X+Z; W = Y+W;
			r1 = _mm_shuffle_ps(r1, r2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
			r1 = _mm_add_ps(r1, r2);           // Add Z and W together
			return _mm_cvtss_f32(_mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2)));    // Splat Z and return
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x4 m = vmulq_f32(l, r);
			fp32x2 v1 = vget_low_f32(m);
			fp32x2 v2 = vget_high_f32(m);
			v1 = vpadd_f32(v1, v1);
			v2 = vpadd_f32(v2, v2);
			v1 = vadd_f32(v1, v2);
			return vget_lane_f32(v1, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VNORMALIZE4(vfp32x4& v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			static const uint32x4 infinity = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
			static const uint32x4 qnan = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };
			vfp32x4 r = v;
			// perform the dot product on x,y,z and w
			vfp32x4 dot = _mm_mul_ps(r, r);
			// m has z and w
			vfp32x4 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(3, 2, 3, 2));
			// x+z, y+w
			dot = _mm_add_ps(dot, m);
			// x+z,x+z,x+z,y+w
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 0, 0, 0));
			// ??,??,y+w,y+w
			m = _mm_shuffle_ps(m, dot, _MM_SHUFFLE(3, 3, 0, 0));
			// ??,??,x+z+y+w,??
			dot = _mm_add_ps(dot, m);
			// splat the length
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(2, 2, 2, 2));
			// prepare for the division
			vfp32x4 result = _mm_sqrt_ps(dot);
			// create zero with a single instruction
			vfp32x4 zeroMask = _mm_setzero_ps();
			// test for a divide by zero (Must be FP to detect -0.0)
			zeroMask = _mm_cmpneq_ps(zeroMask, result);
			// failsafe on zero (Or epsilon) length planes
			// if the length is infinity, set the elements to zero
			dot = _mm_cmpneq_ps(dot, infinity.data);
			// get return single float result
			fp32 ret = _mm_cvtss_f32(result);
			// Divide to perform the normalization
			result = _mm_div_ps(r, result);
			// Any that are infinity, set to zero
			result = _mm_and_ps(result, zeroMask);
			// Select qnan or result based on infinite length
			vfp32x4 m1 = _mm_andnot_ps(dot, qnan.data);
			vfp32x4 m2 = _mm_and_ps(result, dot);
			v = _mm_or_ps(m1, m2);
			return ret;
#elif BLADE_SIMD == BLADE_SIMD_NEON
			static const uint32x4 infinity = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
			static const uint32x4 qnan = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };

			vfp32x4 r = v;
			// dot4
			vfp32x4 dot = vmulq_f32(r, r);
			fp32x2 v1 = vget_low_f32(dot);
			fp32x2 v2 = vget_high_f32(dot);
			v1 = vpadd_f32(v1, v1);
			v2 = vpadd_f32(v2, v2);
			v1 = vadd_f32(v1, v2);
			uint32x2_t equalZero = vceq_f32(v1, vdup_n_f32(0));
			uint32x2_t equalInf = vceq_f32(v1, vget_low_f32(reinterpret_cast<const vfp32x4&>(infinity)));
			// Reciprocal sqrt (2 iterations of Newton-Raphson)
			fp32x2 S0 = vrsqrte_f32(v1);
			fp32x2 P0 = vmul_f32(v1, S0);
			fp32x2 R0 = vrsqrts_f32(P0, S0);
			fp32x2 S1 = vmul_f32(S0, R0);
			fp32x2 P1 = vmul_f32(v1, S1);
			fp32x2 R1 = vrsqrts_f32(P1, S1);
			v2 = vmul_f32(S1, R1);
			// Normalize
			vfp32x4 result = vmulq_f32(r, vcombine_f32(v2, v2));
			result = vbslq_f32(vcombine_u32(equalZero, equalZero), vdupq_n_f32(0), result);
			v = vbslq_f32(vcombine_u32(equalInf, equalInf), reinterpret_cast<const vfp32x4&>(qnan), result);
			return vget_lane_f32(P1, 0);
#else
#	error not implemented.
#endif
		}

		/************************************************************************/
		/* 2d vector function wrapper                                                                     */
		/************************************************************************/
		BLADE_ALWAYS_INLINE static vfp32x2 VADD2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_add_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vadd_f32(l, r);
#else

#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VSUB2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_sub_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vsub_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VMUL2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_mul_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmul_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VDIV2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_div_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x2 reciprocal = vrecpe_f32(r);
			//newton-raphson iteration
			reciprocal = vmul_f32(vrecps_f32(r, reciprocal), reciprocal);
			reciprocal = vmul_f32(vrecps_f32(r, reciprocal), reciprocal);
			return vmul_f32(l, reciprocal);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VADD2(vfp32x2 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_add_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vadd_f32(l, vdup_n_f32(r));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VSUB2(vfp32x2 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_sub_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vsub_f32(l, vdup_n_f32(r));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VMUL2(vfp32x2 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_mul_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmul_f32(l, vdup_n_f32(r));
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VDIV2(vfp32x2 l, fp32 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_div_ps(l, _mm_set_ps1(r));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x2 r2 = vdup_n_f32(r);
			vfp32x2 reciprocal = vrecpe_f32(r2);
			//newton-raphson iteration
			reciprocal = vmul_f32(vrecps_f32(r2, reciprocal), reciprocal);
			reciprocal = vmul_f32(vrecps_f32(r2, reciprocal), reciprocal);
			return vmul_f32(l, reciprocal);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VDIV2(fp32 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_div_ps(_mm_set_ps1(l), r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x2 reciprocal = vrecpe_f32(r);
			//newton-raphson iteration
			reciprocal = vmul_f32(vrecps_f32(r, reciprocal), reciprocal);
			reciprocal = vmul_f32(vrecps_f32(r, reciprocal), reciprocal);
			return vmul_f32(vdup_n_f32(l), reciprocal);
#else
#	error not implemented.
#endif
		}

		//get min values on components basis
		BLADE_ALWAYS_INLINE static vfp32x2 VMIN2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_min_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmin_f32(l, r);
#else
#	error not implemented.
#endif
		}

		//get max values on components basis
		BLADE_ALWAYS_INLINE static vfp32x2 VMAX2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_max_ps(l, r);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vmax_f32(l, r);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VABS2(vfp32x2 v)
		{

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//'sign_mask' : construction of local static object is not thread-safe
#endif

#if BLADE_SIMD == BLADE_SIMD_SSE
			//mask off IEEE sign bit
			static const vfp32x2 sign_mask = _mm_set1_ps(-0.f); // -0.f = 1 << 31
			vfp32x2 ret = _mm_andnot_ps(sign_mask, v);
			return ret;
#elif BLADE_SIMD == BLADE_SIMD_NEON
			//mask off IEEE sign bit
			static const vfp32x2 sign_mask = vdup_n_f32(-0.f); // -0.f = 1 << 31
			vfp32x2 ret = vbic_s32(sign_mask, v);
			return ret;
#else
#	error not implemented.
#endif

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VNEG2(vfp32x2 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_sub_ps(_mm_setzero_ps(), v);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return vneg_f32(v);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x2 VLOAD2(const fp32* f32x2)
		{
#	if BLADE_SIMD == BLADE_SIMD_SSE
			return _mm_loadl_pi(_mm_setzero_ps(), (const fp32x2*)f32x2);
			//vfp32x2 _x = _mm_load_ss(&f32x2[0]);
			//vfp32x2 _y = _mm_load_ss(&f32x2[1]);
			//return _mm_unpacklo_ps(_x, _y);
#	elif BLADE_SIMD == BLADE_SIMD_NEON
			return vld1_f32(f32x2);
#	else
#	error not implemented.
#	endif
		}

		BLADE_ALWAYS_INLINE static void VSTORE2(vfp32x2 v, fp32* f32x2)
		{
#	if BLADE_SIMD == BLADE_SIMD_SSE
			//vfp32x2 t = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
			//_mm_store_ss(&f32x2[0], v);
			//_mm_store_ss(&f32x2[1], t);
			_mm_storel_pi((fp32x2*)f32x2, v);					//[0],[1]
#	elif BLADE_SIMD == BLADE_SIMD_NEON
			vst1_f32(f32x2, v);
#	else
#	error not implemented.
#	endif
		}

		//compare
		BLADE_ALWAYS_INLINE static bool VEQUAL2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 m = _mm_cmpeq_ps(l, r);
			// z and w are not cared about
			return (((_mm_movemask_ps(m) & 3) == 3) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return (uint64)vceq_f32(l, r) == 0xFFFFFFFFFFFFFFFFU;
#else
#	error not implemented.
#endif	
		}

		BLADE_ALWAYS_INLINE static bool VLESS2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 m = _mm_cmplt_ps(l, r);
			return (((_mm_movemask_ps(m) & 3) == 3) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return (uint64)vclt_f32(l, r) == 0xFFFFFFFFFFFFFFFFU;
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VLESSEQUAL2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 m = _mm_cmple_ps(l, r);
			return (((_mm_movemask_ps(m) & 3) == 3) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return (uint64)vcle_f32(l, r) == 0xFFFFFFFFFFFFFFFFU;
#else
#	error not implemented.
#endif	
		}

		BLADE_ALWAYS_INLINE static bool VGREATER2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 m = _mm_cmpgt_ps(l, r);
			return (((_mm_movemask_ps(m) & 3) == 3) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return (uint64)vcgt_f32(l, r) == 0xFFFFFFFFFFFFFFFFU;
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static bool VGREATEREQUAL2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 m = _mm_cmpge_ps(l, r);
			return (((_mm_movemask_ps(m) & 3) == 3) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			return (uint64)vcge_f32(l, r) == 0xFFFFFFFFFFFFFFFFU;
#else
#	error not implemented.
#endif	
		}

		BLADE_ALWAYS_INLINE static bool VEQUALTOLERANCE2(vfp32x2 l, vfp32x2 r, fp32 tolerance)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			// get the difference
			vfp32x2 diff = _mm_sub_ps(l, r);
			// get the absolute value of the difference
			vfp32x2 m = _mm_setzero_ps();
			m = _mm_sub_ps(m, diff);
			m = _mm_max_ps(m, diff);
			m = _mm_cmple_ps(m, _mm_set_ps1(tolerance));
			// z and w are not cared about
			return (((_mm_movemask_ps(m) & 3) == 0x3) != 0);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x2 diff = vsub_f32(l, r);
			uint32x2_t m = vcale_f32(diff, vdup_n_f32(tolerance));
			uint64_t m64u = vget_lane_u64((uint64x1_t)m, 0);
			return m64u == 0xFFFFFFFFFFFFFFFFU;
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VLENGTH2(vfp32x2 v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 dot = _mm_mul_ps(v, v);
			// m has y splatted
			vfp32x2 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 1, 1, 1));
			// x+y
			dot = _mm_add_ss(dot, m);
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
			dot = _mm_sqrt_ps(dot);
			return _mm_cvtss_f32(dot);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x2 m = vmul_f32(v, v);
			m = vpadd_f32(m, m);
			const vfp32x2 zero = vdup_n_f32(0);
			uint32x2_t equalsZero = vceq_f32(m, zero);
			// Sqrt
			vfp32x2 S0 = vrsqrte_f32(m);
			vfp32x2 P0 = vmul_f32(m, S0);
			vfp32x2 R0 = vrsqrts_f32(P0, S0);
			vfp32x2 S1 = vmul_f32(S0, R0);
			vfp32x2 P1 = vmul_f32(m, S1);
			vfp32x2 R1 = vrsqrts_f32(P1, S1);
			vfp32x2 result = vmul_f32(S1, R1);
			result = vmul_f32(m, result);
			result = vbsl_f32(equalsZero, zero, result);
			return vget_lane_f32(result, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VDOT2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 dot = _mm_mul_ps(l, r);
			// m has y splatted
			vfp32x2 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 1, 1, 1));
			// x+y
			dot = _mm_add_ss(dot, m);
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
			return _mm_cvtss_f32(dot);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			vfp32x2 dot2 = vmul_f32(l, r);
			dot2 = vpadd_f32(dot2, dot2);
			return vget_lane_f32(dot2, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VCROSS2(vfp32x2 l, vfp32x2 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			vfp32x2 result = _mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 1, 0, 1));
			// Perform the mul
			result = _mm_mul_ps(result, l);
			// Splat y
			vfp32x2 m = _mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 1, 1, 1));
			// Sub the values
			result = _mm_sub_ss(result, m);
			// Splat the cross product
			result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(0, 0, 0, 0));
			return _mm_cvtss_f32(result);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			static const vfp32x2 _negate = { 1.f, -1.f };
			vfp32x2 m = vmul_f32(l, r);
			m = vmul_f32(m, _negate);
			m = vpadd_f32(m, m);
			return vget_lane_f32(m, 0);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static fp32 VNORMALIZE2(vfp32x2& v)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			static const vfp32x2 infinite = { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
			static const vfp32x2 qnan = { 0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000 };
			// Perform the dot product on x and y only
			vfp32x2 dot = _mm_mul_ps(v, v);
			vfp32x2 m = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(1, 1, 1, 1));
			dot = _mm_add_ss(dot, m);
			dot = _mm_shuffle_ps(dot, dot, _MM_SHUFFLE(0, 0, 0, 0));
			// Prepare for the division
			vfp32x2 result = _mm_sqrt_ps(dot);
			// Create zero with a single instruction
			vfp32x2 zeroMask = _mm_setzero_ps();
			// Test for a divide by zero (Must be FP to detect -0.0)
			zeroMask = _mm_cmpneq_ps(zeroMask, result);
			// Failsafe on zero (Or epsilon) length planes
			// If the length is infinity, set the elements to zero
			dot = _mm_cmpneq_ps(dot, infinite);
			fp32 ret = _mm_cvtss_f32(result);
			// Reciprocal mul to perform the normalization
			result = _mm_div_ps(v, result);
			// Any that are infinity, set to zero
			result = _mm_and_ps(result, zeroMask);
			// Select qnan or result based on infinite length
			vfp32x2 m1 = _mm_andnot_ps(dot, qnan);
			vfp32x2 m2 = _mm_and_ps(result, dot);
			result = _mm_or_ps(m1, m2);
			v = result;
			return ret;
#elif BLADE_SIMD == BLADE_SIMD_NEON
			//static const vfp32x2 infinite = { 0x7F800000, 0x7F800000 };
			//static const vfp32x2 qnan = { 0x7FC00000, 0x7FC00000 };
			// dot2
			vfp32x2 m = vmul_f32(v, v);
			m = vpadd_f32(m, m);
			uint32x2_t equalZero = vceq_f32(m, vdup_n_f32(0));
			//uint32x2_t equalInf = vceq_f32(m, infinite);
			// reciprocal sqrt (2 iterations of Newton-Raphson)
			vfp32x2 S0 = vrsqrte_f32(m);
			vfp32x2 P0 = vmul_f32(m, S0);
			vfp32x2 R0 = vrsqrts_f32(P0, S0);
			vfp32x2 S1 = vmul_f32(S0, R0);
			vfp32x2 P1 = vmul_f32(m, S1);
			vfp32x2 R1 = vrsqrts_f32(P1, S1);
			m = vmul_f32(S1, R1);
			fp32 ret = 1 / vget_lane_f32(m, 0);
			// normalize
			vfp32x2 result = vmul_f32(v, m);
			result = vbsl_f32(equalZero, vdup_n_f32(0), result);
			return ret;
#else
#	error not implemented.
#endif
		}

		/************************************************************************/
		/* quaternion function helpers                                                                     */
		/************************************************************************/
		BLADE_ALWAYS_INLINE static vfp32x4 QMUL(vfp32x4 l, vfp32x4 r)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			static const vfp32x4 ControlWZYX = { 1.0f,-1.0f, 1.0f,-1.0f };
			static const vfp32x4 ControlZWXY = { 1.0f, 1.0f,-1.0f,-1.0f };
			static const vfp32x4 ControlYXWZ = { -1.0f, 1.0f, 1.0f,-1.0f };
			// note: the code copied from DirectXMath which reversed the multiplication order

			// copy to SSE registers and use as few as possible for x86
			vfp32x4 Q2X = l;
			vfp32x4 Q2Y = l;
			vfp32x4 Q2Z = l;
			vfp32x4 result = l;
			// Splat with one instruction
			result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 3, 3, 3));
			Q2X = _mm_shuffle_ps(Q2X, Q2X, _MM_SHUFFLE(0, 0, 0, 0));
			Q2Y = _mm_shuffle_ps(Q2Y, Q2Y, _MM_SHUFFLE(1, 1, 1, 1));
			Q2Z = _mm_shuffle_ps(Q2Z, Q2Z, _MM_SHUFFLE(2, 2, 2, 2));
			// Retire Q1 and perform Q1*Q2W
			result = _mm_mul_ps(result, r);
			vfp32x4 Q1Shuffle = r;
			// Shuffle the copies of Q1
			Q1Shuffle = _mm_shuffle_ps(Q1Shuffle, Q1Shuffle, _MM_SHUFFLE(0, 1, 2, 3));
			// Mul by Q1WZYX
			Q2X = _mm_mul_ps(Q2X, Q1Shuffle);
			Q1Shuffle = _mm_shuffle_ps(Q1Shuffle, Q1Shuffle, _MM_SHUFFLE(2, 3, 0, 1));
			// Flip the signs on y and z
			Q2X = _mm_mul_ps(Q2X, ControlWZYX);
			// Mul by Q1ZWXY
			Q2Y = _mm_mul_ps(Q2Y, Q1Shuffle);
			Q1Shuffle = _mm_shuffle_ps(Q1Shuffle, Q1Shuffle, _MM_SHUFFLE(0, 1, 2, 3));
			// Flip the signs on z and w
			Q2Y = _mm_mul_ps(Q2Y, ControlZWXY);
			// Mul by Q1YXWZ
			Q2Z = _mm_mul_ps(Q2Z, Q1Shuffle);
			result = _mm_add_ps(result, Q2X);
			// Flip the signs on x and w
			Q2Z = _mm_mul_ps(Q2Z, ControlYXWZ);
			Q2Y = _mm_add_ps(Q2Y, Q2Z);
			result = _mm_add_ps(result, Q2Y);
			return result;
#elif BLADE_SIMD == BLADE_SIMD_NEON
			static const vfp32x4 ControlWZYX = { 1.0f,-1.0f, 1.0f,-1.0f };
			static const vfp32x4 ControlZWXY = { 1.0f, 1.0f,-1.0f,-1.0f };
			static const vfp32x4 ControlYXWZ = { -1.0f, 1.0f, 1.0f,-1.0f };

			fp32x2 Q2L = vget_low_f32(l);
			fp32x2 Q2H = vget_high_f32(l);

			vfp32x4 Q2X = vdupq_lane_f32(Q2L, 0);
			vfp32x4 Q2Y = vdupq_lane_f32(Q2L, 1);
			vfp32x4 Q2Z = vdupq_lane_f32(Q2H, 0);
			vfp32x4 result = vdupq_lane_f32(Q2H, 1);
			result = vmulq_f32(result, r);

			// Mul by Q1WZYX
			vfp32x4 m = vrev64q_f32(r);
			m = vcombine_f32(vget_high_f32(m), vget_low_f32(m));
			Q2X = vmulq_f32(Q2X, m);
			result = vmlaq_f32(result, Q2X, ControlWZYX);

			// Mul by Q1ZWXY
			m = vrev64q_f32(m);
			Q2Y = vmulq_f32(Q2Y, m);
			result = vmlaq_f32(result, Q2Y, ControlZWXY);

			// Mul by Q1YXWZ
			m = vrev64q_f32(m);
			m = vcombine_f32(vget_high_f32(m), vget_low_f32(m));
			Q2Z = vmulq_f32(Q2Z, m);
			result = vmlaq_f32(result, Q2Z, ControlYXWZ);
			return result;
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 QCONJ(vfp32x4 q)
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			static const vfp32x4 vmul = { -1, -1, -1, 1 };
			return _mm_mul_ps(q, vmul);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			static const vfp32x4 vmul = { -1, -1, -1, 1 };
			return vmulq_f32(q, vmul);
#else
#	error not implemented.
#endif
		}

		BLADE_ALWAYS_INLINE static vfp32x4 QLN(vfp32x4 q)
		{
			scalar A = std::acos(Math::Clamp(VGETW(q), -1.0f, 1.0f));
#if BLADE_SIMD == BLADE_SIMD_SSE
			static const vfp32x4 vmul = { 1, 1, 1, 0 };
			return _mm_mul_ps(_mm_set_ps1(A), vmul);
#elif BLADE_SIMD == BLADE_SIMD_NEON
			static const vfp32x4 vmul = { 1, 1, 1, 0 };
			return vmulq_f32(vdupq_n_f32(A), vmul);
#else
#	error not implemented.
#endif
		}

	}//namespace SIMD 

}//namespace Blade

#endif//BLADE_ENABLE_SIMD

#endif // __Blade_BladeSIMD_h__