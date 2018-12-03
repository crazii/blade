/********************************************************************
	created:	2010/04/21
	filename: 	Vector4.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	const Vector4	Vector4::ZERO = Vector4(0.0f,0.0f,0.0f,0.0f);
	const Vector4	Vector4::UNIT_X = Vector4(1.0f,0.0f,0.0f,0.0f);
	const Vector4	Vector4::UNIT_Y = Vector4(0.0f,1.0f,0.0f,0.0f);
	const Vector4	Vector4::UNIT_Z = Vector4(0.0f,0.0f,1.0f,0.0f);
	const Vector4	Vector4::UNIT_XYZ = Vector4(1.0f,1.0f,1.0f,0.0f);
	const Vector4	Vector4::UNIT_ALL = Vector4(1.0f,1.0f,1.0f,1.0f);
	const Vector4	Vector4::NEGATIVE_UNIT_X = Vector4(-1.0f,0.0f,0.0f,0.0f);
	const Vector4	Vector4::NEGATIVE_UNIT_Y = Vector4(0.0f,-1.0f,0.0f,0.0f);
	const Vector4	Vector4::NEGATIVE_UNIT_Z = Vector4(0.0f,0.0f,-1.0f,0.0f);
	const Vector4	Vector4::NEGATIVE_UNIT_ALL = Vector4(-1.0f,-1.0f,-1.0f,-1.0f);


	//////////////////////////////////////////////////////////////////////////
	Vector4				Vector4::operator*(const Matrix44& rhs) const
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::vfp32x4 _x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		SIMD::vfp32x4 _y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		SIMD::vfp32x4 _z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		SIMD::vfp32x4 _w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
		// Mul by the matrix
		_x = _mm_mul_ps(_x, m0);
		_y = _mm_mul_ps(_y, m1);
		_z = _mm_mul_ps(_z, m2);
		_w = _mm_mul_ps(_w, m3);
		// Add them all together
		_x = _mm_add_ps(_x, _y);
		_z = _mm_add_ps(_z, _w);
		_x = _mm_add_ps(_x, _z);
		return _x;
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::fp32x2 vl = vget_low_f32( v );
		SIMD::vfp32x4 v1 = vdupq_lane_f32( vl, 0 ); // X
		SIMD::vfp32x4 v2 = vdupq_lane_f32( vl, 1 ); // Y
		SIMD::vfp32x4 result = vmulq_f32( v1, m0 );
		result = vmlaq_f32( result, v2, m1 );
		SIMD::fp32x2 vh = vget_high_f32( v );
		v1 = vdupq_lane_f32( vh, 0 ); // Z
		v2 = vdupq_lane_f32( vh, 1 ); // W
		result = vmlaq_f32( result, v1, m2 );
		return vmlaq_f32( result, v2, m3 );
#else
		const Vector4& lhs = *this;
		return Vector4(
			lhs.X()*rhs[0][0] + lhs.Y()*rhs[1][0] + lhs.Z()*rhs[2][0] + lhs.W()*rhs[3][0],
			lhs.X()*rhs[0][1] + lhs.Y()*rhs[1][1] + lhs.Z()*rhs[2][1] + lhs.W()*rhs[3][1],
			lhs.X()*rhs[0][2] + lhs.Y()*rhs[1][2] + lhs.Z()*rhs[2][2] + lhs.W()*rhs[3][2],
			lhs.X()*rhs[0][3] + lhs.Y()*rhs[1][3] + lhs.Z()*rhs[2][3] + lhs.W()*rhs[3][3]
		);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	Vector4&			Vector4::operator*=(const Matrix44& rhs)
	{
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::vfp32x4 _x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		SIMD::vfp32x4 _y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		SIMD::vfp32x4 _z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		SIMD::vfp32x4 _w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
		// Mul by the matrix
		_x = _mm_mul_ps(_x, m0);
		_y = _mm_mul_ps(_y, m1);
		_z = _mm_mul_ps(_z, m2);
		_w = _mm_mul_ps(_w, m3);
		// Add them all together
		_x = _mm_add_ps(_x, _y);
		_z = _mm_add_ps(_z, _w);
		_x = _mm_add_ps(_x, _z);
		*this = _x;
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 v = *this;
		SIMD::vfp32x4 m0 = rhs[0];
		SIMD::vfp32x4 m1 = rhs[1];
		SIMD::vfp32x4 m2 = rhs[2];
		SIMD::vfp32x4 m3 = rhs[3];

		SIMD::fp32x2 vl = vget_low_f32( v );
		SIMD::vfp32x4 v1 = vdupq_lane_f32( vl, 0 ); // X
		SIMD::vfp32x4 v2 = vdupq_lane_f32( vl, 1 ); // Y
		SIMD::vfp32x4 result = vmulq_f32( v1, m0 );
		result = vmlaq_f32( result, v2, m1 );
		SIMD::fp32x2 vh = vget_high_f32( v );
		v1 = vdupq_lane_f32( vh, 0 ); // Z
		v2 = vdupq_lane_f32( vh, 1 ); // W
		result = vmlaq_f32( result, v1, m2 );
		*this = vmlaq_f32( result, v2, m3 );
#else
		const Vector4& lhs = *this;
		scalar _x = lhs.X()*rhs[0][0] + lhs.Y()*rhs[1][0] + lhs.Z()*rhs[2][0] + lhs.W()*rhs[3][0];
		scalar _y = lhs.X()*rhs[0][1] + lhs.Y()*rhs[1][1] + lhs.Z()*rhs[2][1] + lhs.W()*rhs[3][1];
		scalar _z = lhs.X()*rhs[0][2] + lhs.Y()*rhs[1][2] + lhs.Z()*rhs[2][2] + lhs.W()*rhs[3][2];
		scalar _w = lhs.X()*rhs[0][3] + lhs.Y()*rhs[1][3] + lhs.Z()*rhs[2][3] + lhs.W()*rhs[3][3];

		x = _x;
		y = _y;
		z = _z;
		w = _w;
#endif
		return *this;
	}

	
}//namespace Blade