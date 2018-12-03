/********************************************************************
	created:	2010/04/20
	filename: 	Matrix44.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	Matrix44		Matrix44::IDENTITY = Matrix44(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1);

	Matrix44		Matrix44::ZERO = Matrix44(
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0);

	//////////////////////////////////////////////////////////////////////////
	void				Matrix44::set(const Quaternion& rotation, const Vector3& translation)
	{
		Matrix33 m33;
		rotation.toRotationMatrix(m33);
		this->set(m33);
		this->setTranslation(translation);
	}

	//////////////////////////////////////////////////////////////////////////
	Matrix44			Matrix44::concatenate(const Matrix44 &rhs) const
	{
		Matrix44 ret;
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 l0 = (*this)[0];
		SIMD::vfp32x4 l1 = (*this)[1];
		SIMD::vfp32x4 l2 = (*this)[2];
		SIMD::vfp32x4 l3 = (*this)[3];
		SIMD::vfp32x4 r0 = rhs[0];
		SIMD::vfp32x4 r1 = rhs[1];
		SIMD::vfp32x4 r2 = rhs[2];
		SIMD::vfp32x4 r3 = rhs[3];

		// Use vW to hold the original row
		SIMD::vfp32x4 v = l0;
		// Splat the component X,Y,Z then W
		SIMD::vfp32x4 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		SIMD::vfp32x4 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		SIMD::vfp32x4 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
		// Perform the operation on the first row
		x = _mm_mul_ps(x, r0);
		y = _mm_mul_ps(y, r1);
		z = _mm_mul_ps(z, r2);
		v = _mm_mul_ps(v, r3);
		// Perform a binary add to reduce cumulative errors
		x = _mm_add_ps(x,z);
		y = _mm_add_ps(y,v);
		x = _mm_add_ps(x,y);
		ret[0] = x;
		// Repeat for the other 3 rows
		v = l1;
		x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
		x = _mm_mul_ps(x,r0);
		y = _mm_mul_ps(y,r1);
		z = _mm_mul_ps(z,r2);
		v = _mm_mul_ps(v,r3);
		x = _mm_add_ps(x,z);
		y = _mm_add_ps(y,v);
		x = _mm_add_ps(x,y);
		ret[1] = x;
		v = l2;
		x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
		x = _mm_mul_ps(x,r0);
		y = _mm_mul_ps(y,r1);
		z = _mm_mul_ps(z,r2);
		v = _mm_mul_ps(v,r3);
		x = _mm_add_ps(x,z);
		y = _mm_add_ps(y,v);
		x = _mm_add_ps(x,y);
		ret[2] = x;
		v = l3;
		x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		v = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));
		x = _mm_mul_ps(x,r0);
		y = _mm_mul_ps(y,r1);
		z = _mm_mul_ps(z,r2);
		v = _mm_mul_ps(v,r3);
		x = _mm_add_ps(x,z);
		y = _mm_add_ps(y,v);
		x = _mm_add_ps(x,y);
		ret[3] = x;
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 l0 = (*this)[0];
		SIMD::vfp32x4 l1 = (*this)[1];
		SIMD::vfp32x4 l2 = (*this)[2];
		SIMD::vfp32x4 l3 = (*this)[3];
		SIMD::vfp32x4 r0 = rhs[0];
		SIMD::vfp32x4 r1 = rhs[1];
		SIMD::vfp32x4 r2 = rhs[2];
		SIMD::vfp32x4 r3 = rhs[3];

		SIMD::fp32x2 vl = vget_low_f32( l0 );
		SIMD::fp32x2 vh = vget_high_f32( l0 );
		// Splat the component X,Y,Z then W
		SIMD::vfp32x4 x = vdupq_lane_f32(vl, 0);
		SIMD::vfp32x4 y = vdupq_lane_f32(vl, 1);
		SIMD::vfp32x4 z = vdupq_lane_f32(vh, 0);
		SIMD::vfp32x4 w = vdupq_lane_f32(vh, 1);
		// Perform the operation on the first row
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		w = vmlaq_f32(y,w,r3);
		ret[0] = vaddq_f32( z, w );
		// Repeat for the other 3 rows
		vl = vget_low_f32( l1 );
		vh = vget_high_f32( l1 );
		x = vdupq_lane_f32(vl, 0);
		y = vdupq_lane_f32(vl, 1);
		z = vdupq_lane_f32(vh, 0);
		w = vdupq_lane_f32(vh, 1);
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		w = vmlaq_f32(y,w,r3);
		ret[1] = vaddq_f32( z, w );
		vl = vget_low_f32( l2 );
		vh = vget_high_f32( l2 );
		x = vdupq_lane_f32(vl, 0);
		y = vdupq_lane_f32(vl, 1);
		z = vdupq_lane_f32(vh, 0);
		w = vdupq_lane_f32(vh, 1);
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		w = vmlaq_f32(y,w,r3);
		ret[2] = vaddq_f32( z, w );
		vl = vget_low_f32( l3 );
		vh = vget_high_f32( l3 );
		x = vdupq_lane_f32(vl, 0);
		y = vdupq_lane_f32(vl, 1);
		z = vdupq_lane_f32(vh, 0);
		w = vdupq_lane_f32(vh, 1);
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		w = vmlaq_f32(y,w,r3);
		ret[3] = vaddq_f32( z, w );
#else
		ret.mV[0][0] = mV[0][0] * rhs.mV[0][0] + mV[0][1] * rhs.mV[1][0] + mV[0][2] * rhs.mV[2][0] + mV[0][3] * rhs.mV[3][0];
		ret.mV[0][1] = mV[0][0] * rhs.mV[0][1] + mV[0][1] * rhs.mV[1][1] + mV[0][2] * rhs.mV[2][1] + mV[0][3] * rhs.mV[3][1];
		ret.mV[0][2] = mV[0][0] * rhs.mV[0][2] + mV[0][1] * rhs.mV[1][2] + mV[0][2] * rhs.mV[2][2] + mV[0][3] * rhs.mV[3][2];
		ret.mV[0][3] = mV[0][0] * rhs.mV[0][3] + mV[0][1] * rhs.mV[1][3] + mV[0][2] * rhs.mV[2][3] + mV[0][3] * rhs.mV[3][3];

		ret.mV[1][0] = mV[1][0] * rhs.mV[0][0] + mV[1][1] * rhs.mV[1][0] + mV[1][2] * rhs.mV[2][0] + mV[1][3] * rhs.mV[3][0];
		ret.mV[1][1] = mV[1][0] * rhs.mV[0][1] + mV[1][1] * rhs.mV[1][1] + mV[1][2] * rhs.mV[2][1] + mV[1][3] * rhs.mV[3][1];
		ret.mV[1][2] = mV[1][0] * rhs.mV[0][2] + mV[1][1] * rhs.mV[1][2] + mV[1][2] * rhs.mV[2][2] + mV[1][3] * rhs.mV[3][2];
		ret.mV[1][3] = mV[1][0] * rhs.mV[0][3] + mV[1][1] * rhs.mV[1][3] + mV[1][2] * rhs.mV[2][3] + mV[1][3] * rhs.mV[3][3];

		ret.mV[2][0] = mV[2][0] * rhs.mV[0][0] + mV[2][1] * rhs.mV[1][0] + mV[2][2] * rhs.mV[2][0] + mV[2][3] * rhs.mV[3][0];
		ret.mV[2][1] = mV[2][0] * rhs.mV[0][1] + mV[2][1] * rhs.mV[1][1] + mV[2][2] * rhs.mV[2][1] + mV[2][3] * rhs.mV[3][1];
		ret.mV[2][2] = mV[2][0] * rhs.mV[0][2] + mV[2][1] * rhs.mV[1][2] + mV[2][2] * rhs.mV[2][2] + mV[2][3] * rhs.mV[3][2];
		ret.mV[2][3] = mV[2][0] * rhs.mV[0][3] + mV[2][1] * rhs.mV[1][3] + mV[2][2] * rhs.mV[2][3] + mV[2][3] * rhs.mV[3][3];

		ret.mV[3][0] = mV[3][0] * rhs.mV[0][0] + mV[3][1] * rhs.mV[1][0] + mV[3][2] * rhs.mV[2][0] + mV[3][3] * rhs.mV[3][0];
		ret.mV[3][1] = mV[3][0] * rhs.mV[0][1] + mV[3][1] * rhs.mV[1][1] + mV[3][2] * rhs.mV[2][1] + mV[3][3] * rhs.mV[3][1];
		ret.mV[3][2] = mV[3][0] * rhs.mV[0][2] + mV[3][1] * rhs.mV[1][2] + mV[3][2] * rhs.mV[2][2] + mV[3][3] * rhs.mV[3][2];
		ret.mV[3][3] = mV[3][0] * rhs.mV[0][3] + mV[3][1] * rhs.mV[1][3] + mV[3][2] * rhs.mV[2][3] + mV[3][3] * rhs.mV[3][3];
#endif
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar				Matrix44::getDeterminant() const
	{
		return mV[0][0]*getMinor(0,0) - mV[0][1]*getMinor(0,1) + mV[0][1]*getMinor(0,2) - mV[0][1]*getMinor(0,3);
	}

	//////////////////////////////////////////////////////////////////////////
	Matrix44			Matrix44::getAdjoint() const
	{
		//return Matrix44(
		//	getCofactor(0,0),getCofactor(1,0),getCofactor(2,0),getCofactor(3,0),
		//	getCofactor(0,1),getCofactor(1,1),getCofactor(2,1),getCofactor(3,1),
		//	getCofactor(0,2),getCofactor(1,2),getCofactor(2,2),getCofactor(3,2),
		//	getCofactor(0,3),getCofactor(1,3),getCofactor(2,3),getCofactor(3,3),
		//	);

		//return Matrix44(
		//	+getMinor(0,0),-getMinor(1,0),+getMinor(2,0),-getMinor(3,0),
		//	-getMinor(0,1),+getMinor(1,1),-getMinor(2,1),+getMinor(3,1),
		//	+getMinor(0,2),-getMinor(1,2),+getMinor(2,2),-getMinor(3,2),
		//	-getMinor(0,3),+getMinor(1,3),-getMinor(2,3),+getMinor(3,3)
		//	);

		scalar m00 = mV[0][0], m01 = mV[0][1], m02 = mV[0][2], m03 = mV[0][3];
		scalar m10 = mV[1][0], m11 = mV[1][1], m12 = mV[1][2], m13 = mV[1][3];
		scalar m20 = mV[2][0], m21 = mV[2][1], m22 = mV[2][2], m23 = mV[2][3];
		scalar m30 = mV[3][0], m31 = mV[3][1], m32 = mV[3][2], m33 = mV[3][3];

		//get 2x2 minor in column 2,3
		scalar v0 = getMinor2x2(0,1,2,3);
		scalar v1 = getMinor2x2(0,2,2,3);
		scalar v2 = getMinor2x2(0,3,2,3);
		scalar v3 = getMinor2x2(1,2,2,3);
		scalar v4 = getMinor2x2(1,3,2,3);
		scalar v5 = getMinor2x2(2,3,2,3);

		//adj(aij)n = (Aij)n T
		scalar d00 = + (v5 * m11 - v4 * m21 + v3 * m31);
		scalar d01 = - (v5 * m01 - v2 * m21 + v1 * m31);
		scalar d02 = + (v4 * m01 - v2 * m11 + v0 * m31);
		scalar d03 = - (v3 * m01 - v1 * m11 + v0 * m21);

		scalar d10 = - (v5 * m10 - v4 * m20 + v3 * m30);
		scalar d11 = + (v5 * m00 - v2 * m20 + v1 * m30);
		scalar d12 = - (v4 * m00 - v2 * m10 + v0 * m30);
		scalar d13 = + (v3 * m00 - v1 * m10 + v0 * m20);

		//get 2x2 minor in column 0,1
		v0 = getMinor2x2(0,1,0,1);
		v1 = getMinor2x2(0,2,0,1);
		v2 = getMinor2x2(0,3,0,1);
		v3 = getMinor2x2(1,2,0,1);
		v4 = getMinor2x2(1,3,0,1);
		v5 = getMinor2x2(2,3,0,1);

		scalar d20 = + (v5 * m13 - v4 * m23 + v3 * m33);
		scalar d21 = - (v5 * m03 - v2 * m23 + v1 * m33);
		scalar d22 = + (v4 * m03 - v2 * m13 + v0 * m33);
		scalar d23 = - (v3 * m03 - v1 * m13 + v0 * m32);

		scalar d30 = - (v5 * m12 - v4 * m22 + v4 * m32);
		scalar d31 = + (v5 * m02 - v2 * m22 + v1 * m32);
		scalar d32 = - (v4 * m02 - v2 * m12 + v0 * m32);
		scalar d33 = + (v3 * m02 - v1 * m12 + v0 * m22);

		return Matrix44(
			d00, d01, d02, d03,
			d10, d11, d12, d13,
			d20, d21, d22, d23,
			d30, d31, d32, d33);
	}

	//////////////////////////////////////////////////////////////////////////
	Matrix44			Matrix44::getInverse() const
	{

#if BLADE_SIMD == BLADE_SIMD_SSE
		Matrix44 ret = *this;
		ret.transpose();
		SIMD::vfp32x4 r0 = ret[0];
		SIMD::vfp32x4 r1 = ret[1];
		SIMD::vfp32x4 r2 = ret[2];
		SIMD::vfp32x4 r3 = ret[3];

		SIMD::vfp32x4 V00 = _mm_shuffle_ps(r2,r2,_MM_SHUFFLE(1,1,0,0));
		SIMD::vfp32x4 V10 = _mm_shuffle_ps(r3,r3,_MM_SHUFFLE(3,2,3,2));
		SIMD::vfp32x4 V01 = _mm_shuffle_ps(r0,r0,_MM_SHUFFLE(1,1,0,0));
		SIMD::vfp32x4 V11 = _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,2,3,2));
		SIMD::vfp32x4 V02 = _mm_shuffle_ps(r2, r0,_MM_SHUFFLE(2,0,2,0));
		SIMD::vfp32x4 V12 = _mm_shuffle_ps(r3, r1,_MM_SHUFFLE(3,1,3,1));

		SIMD::vfp32x4 D0 = _mm_mul_ps(V00,V10);
		SIMD::vfp32x4 D1 = _mm_mul_ps(V01,V11);
		SIMD::vfp32x4 D2 = _mm_mul_ps(V02,V12);

		V00 = _mm_shuffle_ps(r2,r2,_MM_SHUFFLE(3,2,3,2));
		V10 = _mm_shuffle_ps(r3,r3,_MM_SHUFFLE(1,1,0,0));
		V01 = _mm_shuffle_ps(r0,r0,_MM_SHUFFLE(3,2,3,2));
		V11 = _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,0,0));
		V02 = _mm_shuffle_ps(r2,r0,_MM_SHUFFLE(3,1,3,1));
		V12 = _mm_shuffle_ps(r3,r1,_MM_SHUFFLE(2,0,2,0));

		V00 = _mm_mul_ps(V00,V10);
		V01 = _mm_mul_ps(V01,V11);
		V02 = _mm_mul_ps(V02,V12);
		D0 = _mm_sub_ps(D0,V00);
		D1 = _mm_sub_ps(D1,V01);
		D2 = _mm_sub_ps(D2,V02);
		// V11 = D0Y,D0W,D2Y,D2Y
		V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,1,3,1));
		V00 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(1,0,2,1));
		V10 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(0,3,0,2));
		V01 = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0,1,0,2));
		V11 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(2,1,2,1));
		// V13 = D1Y,D1W,D2W,D2W
		SIMD::vfp32x4 V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,3,3,1));
		V02 = _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(1,0,2,1));
		V12 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(0,3,0,2));
		SIMD::vfp32x4 V03 = _mm_shuffle_ps(r2,r2,_MM_SHUFFLE(0,1,0,2));
		V13 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(2,1,2,1));

		SIMD::vfp32x4 C0 = _mm_mul_ps(V00,V10);
		SIMD::vfp32x4 C2 = _mm_mul_ps(V01,V11);
		SIMD::vfp32x4 C4 = _mm_mul_ps(V02,V12);
		SIMD::vfp32x4 C6 = _mm_mul_ps(V03,V13);

		// V11 = D0X,D0Y,D2X,D2X
		V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(0,0,1,0));
		V00 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2,1,3,2));
		V10 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(2,1,0,3));
		V01 = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(1,3,2,3));
		V11 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(0,2,1,2));
		// V13 = D1X,D1Y,D2Z,D2Z
		V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(2,2,1,0));
		V02 = _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(2,1,3,2));
		V12 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(2,1,0,3));
		V03 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(1,3,2,3));
		V13 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(0,2,1,2));

		V00 = _mm_mul_ps(V00,V10);
		V01 = _mm_mul_ps(V01,V11);
		V02 = _mm_mul_ps(V02,V12);
		V03 = _mm_mul_ps(V03,V13);
		C0 = _mm_sub_ps(C0,V00);
		C2 = _mm_sub_ps(C2,V01);
		C4 = _mm_sub_ps(C4,V02);
		C6 = _mm_sub_ps(C6,V03);

		V00 = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0,3,0,3));
		// V10 = D0Z,D0Z,D2X,D2Y
		V10 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,2,2));
		V10 = _mm_shuffle_ps(V10, V10, _MM_SHUFFLE(0,2,3,0));
		V01 = _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2,0,3,1));
		// V11 = D0X,D0W,D2X,D2Y
		V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,3,0));
		V11 = _mm_shuffle_ps(V11, V11, _MM_SHUFFLE(2,1,0,3));
		V02 = _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(0,3,0,3));
		// V12 = D1Z,D1Z,D2Z,D2W
		V12 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,2,2));
		V12 = _mm_shuffle_ps(V12, V12, _MM_SHUFFLE(0,2,3,0));
		V03 = _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(2,0,3,1));
		// V13 = D1X,D1W,D2Z,D2W
		V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,3,0));
		V13 = _mm_shuffle_ps(V13, V13, _MM_SHUFFLE(2,1,0,3));

		V00 = _mm_mul_ps(V00,V10);
		V01 = _mm_mul_ps(V01,V11);
		V02 = _mm_mul_ps(V02,V12);
		V03 = _mm_mul_ps(V03,V13);
		SIMD::vfp32x4 C1 = _mm_sub_ps(C0,V00);
		C0 = _mm_add_ps(C0,V00);
		SIMD::vfp32x4 C3 = _mm_add_ps(C2,V01);
		C2 = _mm_sub_ps(C2,V01);
		SIMD::vfp32x4 C5 = _mm_sub_ps(C4,V02);
		C4 = _mm_add_ps(C4,V02);
		SIMD::vfp32x4 C7 = _mm_add_ps(C6,V03);
		C6 = _mm_sub_ps(C6,V03);

		C0 = _mm_shuffle_ps(C0,C1,_MM_SHUFFLE(3,1,2,0));
		C2 = _mm_shuffle_ps(C2,C3,_MM_SHUFFLE(3,1,2,0));
		C4 = _mm_shuffle_ps(C4,C5,_MM_SHUFFLE(3,1,2,0));
		C6 = _mm_shuffle_ps(C6,C7,_MM_SHUFFLE(3,1,2,0));
		C0 = _mm_shuffle_ps(C0, C0, _MM_SHUFFLE(3,1,2,0));
		C2 = _mm_shuffle_ps(C2, C2, _MM_SHUFFLE(3,1,2,0));
		C4 = _mm_shuffle_ps(C4, C4, _MM_SHUFFLE(3,1,2,0));
		C6 = _mm_shuffle_ps(C6, C6, _MM_SHUFFLE(3,1,2,0));
		// get the determinate
		//dot 4: use quaternion as helper
		SIMD::vfp32x4 vTemp = _mm_set_ps1( Quaternion(C0).dotProduct(r0) );
		vTemp = _mm_div_ps(_mm_set_ps1(1), vTemp);

		ret[0] = _mm_mul_ps(C0,vTemp);
		ret[1] = _mm_mul_ps(C2,vTemp);
		ret[2] = _mm_mul_ps(C4,vTemp);
		ret[3] = _mm_mul_ps(C6,vTemp);
		return ret;

//#elif BLADE_SIMD == BLADE_SIMD_NEON

#else
		scalar m00 = mV[0][0], m01 = mV[0][1], m02 = mV[0][2], m03 = mV[0][3];
		scalar m10 = mV[1][0], m11 = mV[1][1], m12 = mV[1][2], m13 = mV[1][3];
		scalar m20 = mV[2][0], m21 = mV[2][1], m22 = mV[2][2], m23 = mV[2][3];
		scalar m30 = mV[3][0], m31 = mV[3][1], m32 = mV[3][2], m33 = mV[3][3];

		scalar v0 = getMinor2x2(0,1,2,3);
		scalar v1 = getMinor2x2(0,2,2,3);
		scalar v2 = getMinor2x2(0,3,2,3);
		scalar v3 = getMinor2x2(1,2,2,3);
		scalar v4 = getMinor2x2(1,3,2,3);
		scalar v5 = getMinor2x2(2,3,2,3);

		scalar t00 = + (v5 * m11 - v4 * m21 + v3 * m31);
		scalar t01 = - (v5 * m01 - v2 * m21 + v1 * m31);
		scalar t02 = + (v4 * m01 - v2 * m11 + v0 * m31);
		scalar t03 = - (v3 * m01 - v1 * m11 + v0 * m21);

		scalar invDet = 1 / (t00 * m00 + t01 * m10 + t02 * m20 + t03 * m30);

		scalar d00 = t00 * invDet;
		scalar d01 = t01 * invDet;
		scalar d02 = t02 * invDet;
		scalar d03 = t03 * invDet;

		scalar d10 = - (v5 * m10 - v4 * m20 + v3 * m30) * invDet;
		scalar d11 = + (v5 * m00 - v2 * m20 + v1 * m30) * invDet;
		scalar d12 = - (v4 * m00 - v2 * m10 + v0 * m30) * invDet;
		scalar d13 = + (v3 * m00 - v1 * m10 + v0 * m20) * invDet;

		v0 = getMinor2x2(0,1,0,1);
		v1 = getMinor2x2(0,2,0,1);
		v2 = getMinor2x2(0,3,0,1);
		v3 = getMinor2x2(1,2,0,1);
		v4 = getMinor2x2(1,3,0,1);
		v5 = getMinor2x2(2,3,0,1);

		scalar d20 = + (v5 * m13 - v4 * m23 + v3 * m33) * invDet;
		scalar d21 = - (v5 * m03 - v2 * m23 + v1 * m33) * invDet;
		scalar d22 = + (v4 * m03 - v2 * m13 + v0 * m33) * invDet;
		scalar d23 = - (v3 * m03 - v1 * m13 + v0 * m23) * invDet;

		scalar d30 = - (v5 * m12 - v4 * m22 + v3 * m32) * invDet;
		scalar d31 = + (v5 * m02 - v2 * m22 + v1 * m32) * invDet;
		scalar d32 = - (v4 * m02 - v2 * m12 + v0 * m32) * invDet;
		scalar d33 = + (v3 * m02 - v1 * m12 + v0 * m22) * invDet;

		return Matrix44(
			d00, d01, d02, d03,
			d10, d11, d12, d13,
			d20, d21, d22, d23,
			d30, d31, d32, d33);
#endif
	}

	/************************************************************************/
	/* static member function                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		Matrix44::generateTransform(Matrix44& out,const POINT3& pos, const Vector3& vscale,const Quaternion& rotation)
	{
		Matrix33 rotationMatrix;
		rotation.toRotationMatrix(rotationMatrix);
		Matrix33 scaleMatrix = Matrix33::IDENTITY;
		scaleMatrix.setScale(vscale);

		//scale first, then rotate
		out = scaleMatrix*rotationMatrix;
		out.setTranslation(pos);
	}

	//////////////////////////////////////////////////////////////////////////
	void		Matrix44::generateViewMatrix(Matrix44& out,const POINT3& eye,const POINT3& lookatPoint, const Vector3& _upVec/* = Vector3::UNIT_Y*/)
	{
		Vector3	right,up,backward;	//x+,y+,z+
		backward = eye - lookatPoint;
		backward.normalize();
		Vector3 upVec = _upVec;

		if(Math::Equal(std::abs(backward.dotProduct(upVec)), 1.0f, Math::LOW_EPSILON ) )	//check parallel
			upVec = backward.getPerpendicularVector();

		right = upVec.crossProduct(backward);
		right.normalize();
		up = backward.crossProduct(right);

		out[0] = Vector4(right,	-right.dotProduct(eye) );
		out[1] = Vector4(up,		-up.dotProduct(eye) );
		out[2] = Vector4(backward,	-backward.dotProduct(eye) );
		out[3] = Vector4(0,0,0,1);
		out.transpose();
	}

	//////////////////////////////////////////////////////////////////////////
	void		Matrix44::generateViewMatrix(Matrix44& out,const POINT3& eye,const Quaternion& rotation)
	{
		Matrix33 rotationMatrix;
		rotation.toRotationMatrix(rotationMatrix);
		rotationMatrix.transpose();	//orthogonal: transpose == inverse

		out = rotationMatrix;
		Vector3	trans = -(eye*rotationMatrix);
		out[3] = Vector4( trans, 1 );
	}

	//////////////////////////////////////////////////////////////////////////
	void		Matrix44::generateOrthoProjectionMatrix(Matrix44& out,scalar w,scalar h,scalar zn,scalar zf)
	{
		//generate a right hand orthographic projection matrix
		//projected depth range:[-1,1]
		scalar inv_w = scalar(1.0)/w;
		scalar inv_h = scalar(1.0)/h;
		out = Matrix44::ZERO;
		scalar inv_distance = scalar(1.0f)/(zn - zf);
		scalar addition = zn + zf;

		out[0][0] = scalar(2.0)*inv_w;
		out[1][1] = scalar(2.0)*inv_h;
		out[2][2] = scalar(2.0)*inv_distance;
		out[3][2] = addition*inv_distance;
		out[3][3] = scalar(1.0);
	}

	//////////////////////////////////////////////////////////////////////////
	void		Matrix44::generatePerspectiveProjectionMatrix(Matrix44& out,scalar w,scalar h,scalar zn,scalar zf)
	{
		//generate a right hand perspective projection matrix
		//projected depth range:[-1,1]
		out = Matrix44::ZERO;
		scalar inv_w = scalar(1.0)/w;
		scalar inv_h = scalar(1.0)/h;
		scalar inv_distance = scalar(1.0f)/(zn - zf);

		scalar addation = zn + zf;
		scalar multipli = zn*zf;

		out[0][0] = zn*scalar(2.0)*inv_w;
		out[1][1] = zn*scalar(2.0)*inv_h;
		out[2][2] = addation*inv_distance;
		out[2][3] = scalar(-1.0);
		out[3][2] = 2*multipli*inv_distance;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Matrix44::generatePerspectiveFovProjectionMatrix(Matrix44& out,scalar fFOVx,scalar fAspect,scalar zn,scalar zf)
	{
		//generate a right hand perspective projection matrix
		//projected depth range:[-1,1]
		out = Matrix44::ZERO;
		//scalar yscale = Math::Cotan( fFovY2 );
		//scalar xscale = yscale/fAspect;

		scalar xscale = Math::Cotan( fFOVx/2 );
		scalar yscale = xscale*fAspect;
		scalar inv_distance = scalar(1.0f)/(zn - zf);

		scalar addation = zn + zf;
		scalar multipli = zn*zf;

		out[0][0] = xscale;
		out[1][1] = yscale;
		out[2][2] = addation*inv_distance;
		out[2][3] = scalar(-1.0);
		out[3][2] = 2*multipli*inv_distance;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar				Matrix44::getMinor(size_t i,size_t j) const
	{
		assert( i < 4 && j < 4 );
		Matrix33 subMatrix;
		size_t sub_i = 0,sub_j;
		for( size_t x = 0; x < 4; ++x)
		{
			if( x == i )
				continue;

			sub_j = 0;
			for( size_t y = 0; y < 4; ++y)
			{
				if( y == j )
					continue;
				subMatrix[sub_i][sub_j++] = mV[x][y];
			}
			++sub_i;
		}
		return subMatrix.getDeterminant();
	}

	//////////////////////////////////////////////////////////////////////////
	scalar				Matrix44::getCofactor(size_t i ,size_t j) const
	{
		assert( i < 4 && j < 4 );
		if( (i+j)%2 == 0 )
			return this->getMinor(i,j);
		else
			return -this->getMinor(i,j);
	}

	scalar				Matrix44::getMinor2x2(size_t row0,size_t row1,size_t col0,size_t col1) const
	{
		assert( row0 < 4 && row1 < 4 && col0 < 4 && col1 < 4 );
		//			col0	col1
		// row0	|a(i0,j0) a(a0,j1)|
		// row1	|a(i1,j0) a(i1,j1)|
		return mV[row0][col0]*mV[row1][col1] - mV[row0][col1]*mV[row1][col0];
	}
	
}//namespace Blade