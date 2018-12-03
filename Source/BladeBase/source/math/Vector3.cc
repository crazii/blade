/********************************************************************
	created:	2010/04/20
	filename: 	Vector3.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	const Vector3	Vector3::ZERO = Vector3(0.0f,0.0f,0.0f);
	const Vector3	Vector3::UNIT_X = Vector3(1.0f,0.0f,0.0f);
	const Vector3	Vector3::UNIT_Y = Vector3(0.0f,1.0f,0.0f);
	const Vector3	Vector3::UNIT_Z = Vector3(0.0f,0.0f,1.0f);
	const Vector3	Vector3::UNIT_ALL = Vector3(1.0f,1.0f,1.0f);
	const Vector3	Vector3::NEGATIVE_UNIT_X = Vector3(-1.0f,0.0f,0.0f);
	const Vector3	Vector3::NEGATIVE_UNIT_Y = Vector3(0.0f,-1.0f,0.0f);
	const Vector3	Vector3::NEGATIVE_UNIT_Z = Vector3(0.0f,0.0f,-1.0f);
	const Vector3	Vector3::NEGATIVE_UNIT_ALL = Vector3(-1.0f,-1.0f,-1.0f);

	//////////////////////////////////////////////////////////////////////////
	Vector3&		Vector3::operator*=(const Matrix44& rhs)
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::vfp32x4 result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		result = _mm_mul_ps(result, m0);
		SIMD::vfp32x4 v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		v1 = _mm_mul_ps(v1, m1);
		result = _mm_add_ps(result, v1);
		v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v1 = _mm_mul_ps(v1, m2);
		result = _mm_add_ps(result, v1);
		result = _mm_add_ps(result, m3);
		//w
		v1 = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3,3,3,3));
		*this = _mm_div_ps(result, v1);
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::fp32x2 vl = vget_low_f32( v );
		SIMD::vfp32x4 result = vdupq_lane_f32( vl, 0 ); // X
		SIMD::vfp32x4 v1 = vdupq_lane_f32( vl, 1 ); // Y
		result = vmulq_f32( result, m0 );
		result = vmlaq_f32( result, v1, m1 );
		v1 = vdupq_lane_f32( vget_high_f32( v ), 0 ); // Z
		result = vmlaq_f32( result, v1, m2 );
		result = vaddq_f32( result, m3 );
		// (x y z)/w
		*this = vmulq_n_f32(result,  1/vgetq_lane_f32(result, 3) );
#else
		const Vector3& lhs = *this;
		scalar inv_W = 1.0f / ( lhs.X() * rhs[0][3] +  lhs.Y() * rhs[1][3] + lhs.Z() * rhs[2][3] + rhs[3][3] );

		scalar _x = ( lhs.X() * rhs[0][0] + lhs.Y() * rhs[1][0] + lhs.Z() * rhs[2][0] + rhs[3][0] ) * inv_W;
		scalar _y = ( lhs.X() * rhs[0][1] + lhs.Y() * rhs[1][1] + lhs.Z() * rhs[2][1] + rhs[3][1] ) * inv_W;
		scalar _z = ( lhs.X() * rhs[0][2] + lhs.Y() * rhs[1][2] + lhs.Z() * rhs[2][2] + rhs[3][2] ) * inv_W;
		x = _x;
		y = _y;
		z = _z;
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Vector3	Vector3::operator*(const Matrix44& rhs) const
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::vfp32x4 result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		result = _mm_mul_ps(result, m0);
		SIMD::vfp32x4 v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		v1 = _mm_mul_ps(v1, m1);
		result = _mm_add_ps(result, v1);
		v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v1 = _mm_mul_ps(v1, m2);
		result = _mm_add_ps(result, v1);
		result = _mm_add_ps(result, m3);
		// /w
		v1 = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3,3,3,3));
		return _mm_div_ps(result, v1);
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::fp32x2 vl = vget_low_f32( v );
		SIMD::vfp32x4 result = vdupq_lane_f32( vl, 0 ); // X
		SIMD::vfp32x4 v1 = vdupq_lane_f32( vl, 1 ); // Y
		result = vmulq_f32( result, m0 );
		result = vmlaq_f32( result, v1, m1 );
		v1 = vdupq_lane_f32( vget_high_f32( v ), 0 ); // Z
		result = vmlaq_f32( result, v1, m2 );
		result = vaddq_f32( result, m3 );
		// (x y z)/w
		return vmulq_n_f32(result,  1/vgetq_lane_f32(result, 3) );
#else
		Vector3 ret;
		const Vector3& lhs = *this;
		scalar inv_W = 1.0f / ( lhs.X() * rhs[0][3] +  lhs.Y() * rhs[1][3] + lhs.Z() * rhs[2][3] + rhs[3][3] );

		ret.X() = ( lhs.X() * rhs[0][0] + lhs.Y() * rhs[1][0] + lhs.Z() * rhs[2][0] + rhs[3][0] ) * inv_W;
		ret.Y() = ( lhs.X() * rhs[0][1] + lhs.Y() * rhs[1][1] + lhs.Z() * rhs[2][1] + rhs[3][1] ) * inv_W;
		ret.Z() = ( lhs.X() * rhs[0][2] + lhs.Y() * rhs[1][2] + lhs.Z() * rhs[2][2] + rhs[3][2] ) * inv_W;
		return ret;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	Vector3&		Vector3::operator*=(const Matrix33& rhs)
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];

		SIMD::vfp32x4 result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		result = _mm_mul_ps(result, m0);
		SIMD::vfp32x4 v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		v1 = _mm_mul_ps(v1, m1);
		result = _mm_add_ps(result, v1);
		v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v1 = _mm_mul_ps(v1, m2);
		*this = _mm_add_ps(result, v1);
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];

		SIMD::fp32x2 vl = vget_low_f32( v );
		SIMD::vfp32x4 result = vdupq_lane_f32( vl, 0 ); // X
		SIMD::vfp32x4 v1 = vdupq_lane_f32( vl, 1 ); // Y
		result = vmulq_f32( result, m0 );
		result = vmlaq_f32( result, v1, m1 );
		v1 = vdupq_lane_f32( vget_high_f32( v ), 0 ); // Z
		*this = vmlaq_f32( result, v1, m2 );
#else
		const Vector3& lhs = *this;
		scalar _x = ( lhs.X() * rhs[0][0] + lhs.Y() * rhs[1][0] + lhs.Z() * rhs[2][0] );
		scalar _y = ( lhs.X() * rhs[0][1] + lhs.Y() * rhs[1][1] + lhs.Z() * rhs[2][1] );
		scalar _z = ( lhs.X() * rhs[0][2] + lhs.Y() * rhs[1][2] + lhs.Z() * rhs[2][2] );
		x = _x;
		y = _y;
		z = _z;
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	Vector3			Vector3::operator*(const Matrix33& rhs) const
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];

		SIMD::vfp32x4 result = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		result = _mm_mul_ps(result, m0);
		SIMD::vfp32x4 v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		v1 = _mm_mul_ps(v1, m1);
		result = _mm_add_ps(result, v1);
		v1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v1 = _mm_mul_ps(v1, m2);
		return _mm_add_ps(result, v1);
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];

		SIMD::fp32x2 vl = vget_low_f32( v );
		SIMD::vfp32x4 result = vdupq_lane_f32( vl, 0 ); // X
		SIMD::vfp32x4 v1 = vdupq_lane_f32( vl, 1 ); // Y
		result = vmulq_f32( result, m0 );
		result = vmlaq_f32( result, v1, m1 );
		v1 = vdupq_lane_f32( vget_high_f32( v ), 0 ); // Z
		return vmlaq_f32( result, v1, m2 );
#else
		Vector3 ret;
		const Vector3& lhs = *this;
		ret.X() = ( lhs.X() * rhs[0][0] + lhs.Y() * rhs[1][0] + lhs.Z() * rhs[2][0] );
		ret.Y() = ( lhs.X() * rhs[0][1] + lhs.Y() * rhs[1][1] + lhs.Z() * rhs[2][1] );
		ret.Z() = ( lhs.X() * rhs[0][2] + lhs.Y() * rhs[1][2] + lhs.Z() * rhs[2][2] );
		return ret;
#endif
	}
	
}//namespace Blade