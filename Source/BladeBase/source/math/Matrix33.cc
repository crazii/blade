/********************************************************************
	created:	2010/04/20
	filename: 	Matrix33.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Matrix33.h>

namespace Blade
{
	const Matrix33	Matrix33::ZERO = Matrix33(
		0,0,0,
		0,0,0,
		0,0,0);

	const Matrix33	Matrix33::IDENTITY = Matrix33(
		1,0,0,
		0,1,0,
		0,0,1);

	//////////////////////////////////////////////////////////////////////////
	bool			Matrix33::inverse(scalar tolerance/* = Math::LOW_EPSILON*/)
	{
		Matrix33 _inverse;

		_inverse[0][0] = mV[1][1]*mV[2][2] -
			mV[1][2]*mV[2][1];
		_inverse[0][1] = mV[0][2]*mV[2][1] -
			mV[0][1]*mV[2][2];
		_inverse[0][2] = mV[0][1]*mV[1][2] -
			mV[0][2]*mV[1][1];
		_inverse[1][0] = mV[1][2]*mV[2][0] -
			mV[1][0]*mV[2][2];
		_inverse[1][1] = mV[0][0]*mV[2][2] -
			mV[0][2]*mV[2][0];
		_inverse[1][2] = mV[0][2]*mV[1][0] -
			mV[0][0]*mV[1][2];
		_inverse[2][0] = mV[1][0]*mV[2][1] -
			mV[1][1]*mV[2][0];
		_inverse[2][1] = mV[0][1]*mV[2][0] -
			mV[0][0]*mV[2][1];
		_inverse[2][2] = mV[0][0]*mV[1][1] -
			mV[0][1]*mV[1][0];

		scalar fDet =
			mV[0][0]*_inverse[0][0] +
			mV[0][1]*_inverse[1][0] +
			mV[0][2]*_inverse[2][0];

		if ( ::fabs(fDet) <= tolerance )
			return false;

		scalar fInvDet = 1.0f/fDet;
		for (size_t iRow = 0; iRow < 3; iRow++)
			(*this)[iRow] *= _inverse[iRow]*fInvDet;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar			Matrix33::getDeterminant() const
	{
		scalar fCofactor00 = mV[1][1]*mV[2][2] -
			mV[1][2]*mV[2][1];
		scalar fCofactor10 = mV[1][2]*mV[2][0] -
			mV[1][0]*mV[2][2];
		scalar fCofactor20 = mV[1][0]*mV[2][1] -
			mV[1][1]*mV[2][0];

		scalar fDet =
			mV[0][0]*fCofactor00 +
			mV[0][1]*fCofactor10 +
			mV[0][2]*fCofactor20;

		return fDet;
	}

	//////////////////////////////////////////////////////////////////////////
	Matrix33		Matrix33::concatenate(const Matrix33 &rhs) const
	{
		Matrix33 ret;
#if BLADE_SIMD == BLADE_SIMD_SSE
		SIMD::vfp32x4 l0 = (*this)[0];
		SIMD::vfp32x4 l1 = (*this)[1];
		SIMD::vfp32x4 l2 = (*this)[2];
		SIMD::vfp32x4 r0 = rhs[0];
		SIMD::vfp32x4 r1 = rhs[1];
		SIMD::vfp32x4 r2 = rhs[2];

		// use vW to hold the original row
		SIMD::vfp32x4 v = l0;
		// splat the component X,Y,Z then W
		SIMD::vfp32x4 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		SIMD::vfp32x4 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		SIMD::vfp32x4 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));

		// perform the operation on the first row
		x = _mm_mul_ps(x, r0);
		y = _mm_mul_ps(y, r1);
		z = _mm_mul_ps(z, r2);
		// perform a binary add to reduce cumulative errors
		x = _mm_add_ps(x,z);
		x = _mm_add_ps(x,y);
		ret[0] = x;
		// repeat for the other 3 rows
		v = l1;
		x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		x = _mm_mul_ps(x,r0);
		y = _mm_mul_ps(y,r1);
		z = _mm_mul_ps(z,r2);
		x = _mm_add_ps(x,z);
		x = _mm_add_ps(x,y);
		ret[1] = x;
		v = l2;
		x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
		y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
		z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
		x = _mm_mul_ps(x,r0);
		y = _mm_mul_ps(y,r1);
		z = _mm_mul_ps(z,r2);
		x = _mm_add_ps(x,z);
		x = _mm_add_ps(x,y);
		ret[2] = x;
#elif BLADE_SIMD == BLADE_SIMD_NEON
		SIMD::vfp32x4 l0 = (*this)[0];
		SIMD::vfp32x4 l1 = (*this)[1];
		SIMD::vfp32x4 l2 = (*this)[2];
		SIMD::vfp32x4 r0 = rhs[0];
		SIMD::vfp32x4 r1 = rhs[1];
		SIMD::vfp32x4 r2 = rhs[2];

		SIMD::fp32x2 vl = vget_low_f32( l0 );
		SIMD::fp32x2 vh = vget_high_f32( l0 );
		// Splat the component X,Y,Z then W
		SIMD::vfp32x4 x = vdupq_lane_f32(vl, 0);
		SIMD::vfp32x4 y = vdupq_lane_f32(vl, 1);
		SIMD::vfp32x4 z = vdupq_lane_f32(vh, 0);
		// Perform the operation on the first row
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		ret[0] = vaddq_f32( z, y );
		// Repeat for the other 3 rows
		vl = vget_low_f32( l1 );
		vh = vget_high_f32( l1 );
		x = vdupq_lane_f32(vl, 0);
		y = vdupq_lane_f32(vl, 1);
		z = vdupq_lane_f32(vh, 0);
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		ret[1] = vaddq_f32( z, y );
		vl = vget_low_f32( l2 );
		vh = vget_high_f32( l2 );
		x = vdupq_lane_f32(vl, 0);
		y = vdupq_lane_f32(vl, 1);
		z = vdupq_lane_f32(vh, 0);
		x = vmulq_f32(x,r0);
		y = vmulq_f32(y,r1);
		z = vmlaq_f32(x,z,r2);
		ret[2] = vaddq_f32( z, y );
#else
		for (size_t iRow = 0; iRow < 3; iRow++)
		{
			for (size_t iCol = 0; iCol < 3; iCol++)
			{
				ret.mV[iRow][iCol] =
					mV[iRow][0]*rhs.mV[0][iCol] +
					mV[iRow][1]*rhs.mV[1][iCol] +
					mV[iRow][2]*rhs.mV[2][iCol];
			}
		}
#endif
		return ret;
	}
	
}//namespace Blade