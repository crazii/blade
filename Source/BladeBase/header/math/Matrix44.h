/********************************************************************
	created:	2010/04/20
	filename: 	Matrix44.h
	author:		Crazii
	
	purpose:	row major matrix4x4
*********************************************************************/
#ifndef __Blade_Matrix44_h__
#define __Blade_Matrix44_h__
#include <math/Vector4.h>
#include <math/Matrix33.h>

namespace Blade
{
	class Quaternion;

	class Matrix44
	{
	public:
		inline Matrix44()	{}

		inline Matrix44(scalar m00,scalar m01,scalar m02,scalar m03,
							scalar m10,scalar m11,scalar m12,scalar m13,
							scalar m20,scalar m21,scalar m22,scalar m23,
							scalar m30,scalar m31,scalar m32,scalar m33)
		{
			mV[0][0] = m00;
			mV[0][1] = m01;
			mV[0][2] = m02;
			mV[0][3] = m03;
			mV[1][0] = m10;
			mV[1][1] = m11;
			mV[1][2] = m12;
			mV[1][3] = m13;
			mV[2][0] = m20;
			mV[2][1] = m21;
			mV[2][2] = m22;
			mV[2][3] = m23;
			mV[3][0] = m30;
			mV[3][1] = m31;
			mV[3][2] = m32;
			mV[3][3] = m33;
		}

		inline Matrix44(const Vector4& row0,const Vector4& row1,const Vector4& row2,const Vector4& row3)
		{
			(*this)[0] = row0;
			(*this)[1] = row1;
			(*this)[2] = row2;
			(*this)[3] = row3;
		}

		inline Matrix44(const Matrix44& src)
		{
			*this = src;
		}

		inline Matrix44(const Matrix33& m33)
		{
			this->set(m33);
		}

		inline Matrix44(const Matrix33& m33, const Vector3& translation)
		{
			this->set(m33, translation);
		}

		inline Matrix44(const Quaternion& rotation, const Vector3& translation)
		{
			this->set(rotation, translation);
		}

		BLADE_ALWAYS_INLINE void set(const Matrix33& src)
		{
#if BLADE_ENABLE_SIMD
			(*this)[0].set(src[0], 0);
			(*this)[1].set(src[1], 0);
			(*this)[2].set(src[2], 0);
			(*this)[3].set(Vector3::ZERO, 1);
#else
			mV[0][0] = src[0][0];
			mV[0][1] = src[0][1];
			mV[0][2] = src[0][2];
			mV[0][3] = 0.0f;
			mV[1][0] = src[1][0];
			mV[1][1] = src[1][1];
			mV[1][2] = src[1][2];
			mV[1][3] = 0.0f;
			mV[2][0] = src[2][0];
			mV[2][1] = src[2][1];
			mV[2][2] = src[2][2];
			mV[2][3] = 0.0f;

			mV[3][0] = 0.0f;
			mV[3][1] = 0.0f;
			mV[3][2] = 0.0f;
			mV[3][3] = 1.0f;
#endif
		}

		BLADE_ALWAYS_INLINE void set(const Matrix33& src, const Vector3& translation)
		{
#if BLADE_ENABLE_SIMD
			(*this)[0].set(src[0], 0);
			(*this)[1].set(src[1], 0);
			(*this)[2].set(src[2], 0);
			(*this)[3].set(translation, 1);
#else
			mV[0][0] = src[0][0];
			mV[0][1] = src[0][1];
			mV[0][2] = src[0][2];
			mV[0][3] = 0.0f;
			mV[1][0] = src[1][0];
			mV[1][1] = src[1][1];
			mV[1][2] = src[1][2];
			mV[1][3] = 0.0f;
			mV[2][0] = src[2][0];
			mV[2][1] = src[2][1];
			mV[2][2] = src[2][2];
			mV[2][3] = 0.0f;

			mV[3][0] = translation.x;
			mV[3][1] = translation.y;
			mV[3][2] = translation.z;
			mV[3][3] = 1.0f;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Matrix44&			operator=(const Matrix44& rhs)
		{
#if BLADE_ENABLE_SIMD
			(*this)[0] = rhs[0];
			(*this)[1] = rhs[1];
			(*this)[2] = rhs[2];
			(*this)[3] = rhs[3];
#else
			std::memcpy(mM, rhs.mM, sizeof(*this));
#endif
			return *this;
		}

		/** @brief  */
		inline Matrix44&			operator=(const Matrix33& rhs)
		{
			this->set(rhs);
			return *this;
		}

		/** @brief  */
		inline const Vector4&		operator[](size_t row_index) const
		{
			if( row_index < 4 )
				return reinterpret_cast<const Vector4&>(mV[row_index]);
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("row index out of range."));			
		}

		/** @brief  */
		inline Vector4&			operator[](size_t row_index)
		{
			if( row_index < 4 )
				return reinterpret_cast<Vector4&>(mV[row_index]);
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("row index out of range."));			
		}

		/** @brief  */
		inline bool				operator==(const Matrix44& rhs) const
		{
			return (*this)[0] == rhs[0] && (*this)[1] == rhs[1] && (*this)[2] == rhs[2] && (*this)[3] == rhs[3];
		}

		/** @brief  */
		inline bool				operator!=(const Matrix44& rhs) const
		{
			return (*this)[0] != rhs[0] || (*this)[1] != rhs[1] || (*this)[2] != rhs[2] || (*this)[3] != rhs[3];
		}

		/** @brief  */
		inline operator			Matrix33() const
		{
			Matrix33 m33;
			m33[0] = static_cast<const Vector3&>( (*this)[0] );
			m33[1] = static_cast<const Vector3&>( (*this)[1] );
			m33[2] = static_cast<const Vector3&>( (*this)[2] );
			return m33;
		}

		/** @brief  */
		inline Matrix44			operator+(const Matrix44& rhs) const
		{
			Matrix44 ret;
			ret[0] = (*this)[0] + rhs[0];
			ret[1] = (*this)[1] + rhs[1];
			ret[2] = (*this)[2] + rhs[2];
			ret[3] = (*this)[3] + rhs[3];
			return ret;
		}

		/** @brief  */
		inline Matrix44			operator-(const Matrix44& rhs) const
		{
			Matrix44 ret;
			ret[0] = (*this)[0] - rhs[0];
			ret[1] = (*this)[1] - rhs[1];
			ret[2] = (*this)[2] - rhs[2];
			ret[3] = (*this)[3] - rhs[3];
			return ret;
		}

		/** @brief  */
		inline Matrix44			operator*(const Matrix44& rhs) const
		{
			return this->concatenate(rhs);
		}

		/** @brief  */
		inline Matrix44			operator/(const Matrix44& rhs) const
		{
			Matrix44 ret;
			ret[0] = (*this)[0] / rhs[0];
			ret[1] = (*this)[1] / rhs[1];
			ret[2] = (*this)[2] / rhs[2];
			ret[3] = (*this)[3] / rhs[3];
			return ret;
		}

		/** @brief  */
		inline Matrix44			operator*(scalar rhs) const
		{
			Matrix44 ret;
			ret[0] = (*this)[0] * rhs;
			ret[1] = (*this)[1] * rhs;
			ret[2] = (*this)[2] * rhs;
			ret[3] = (*this)[3] * rhs;
			return ret;
		}

		/** @brief  */
		inline Matrix44			operator/(scalar rhs) const
		{
			Matrix44 ret;
			scalar inv_rhs = 1/rhs;
			ret[0] = (*this)[0] * inv_rhs;
			ret[1] = (*this)[1] * inv_rhs;
			ret[2] = (*this)[2] * inv_rhs;
			ret[3] = (*this)[3] * inv_rhs;
			return ret;
		}

		/** @brief  */
		inline Matrix44&			operator+=(const Matrix44& rhs)
		{
			(*this)[0] += rhs[0];
			(*this)[1] += rhs[1];
			(*this)[2] += rhs[2];
			(*this)[3] += rhs[3];
			return *this;
		}

		/** @brief  */
		inline Matrix44&			operator-=(const Matrix44& rhs)
		{
			(*this)[0] -= rhs[0];
			(*this)[1] -= rhs[1];
			(*this)[2] -= rhs[2];
			(*this)[3] -= rhs[3];
			return *this;
		}

		/** @brief  */
		inline Matrix44&			operator*=(const Matrix44& rhs)
		{
			*this = this->concatenate(rhs);
			return *this;
		}

		/** @brief  */
		inline Matrix44&			operator/=(const Matrix44& rhs)
		{
			(*this)[0] /= rhs[0];
			(*this)[1] /= rhs[1];
			(*this)[2] /= rhs[2];
			(*this)[3] /= rhs[3];
			return *this;
		}

		/** @brief  */
		inline Matrix44&			operator*=(scalar rhs)
		{
			(*this)[0] *= rhs;
			(*this)[1] *= rhs;
			(*this)[2] *= rhs;
			(*this)[3] *= rhs;
			return *this;
		}

		/** @brief  */
		inline Matrix44&			operator/=(scalar rhs)
		{
			scalar inv_rhs = 1/rhs;
			(*this)[0] *= inv_rhs;
			(*this)[1] *= inv_rhs;
			(*this)[2] *= inv_rhs;
			(*this)[3] *= inv_rhs;
			return *this;

		}

		/** @brief  */
		inline bool			isAffine() const
		{
			return mV[0][3] == 0 && mV[1][3] == 0 && mV[2][3] == 0 && mV[3][3] == 1;
		}

		/** @brief  */
		inline void			translate(const Vector3& trans)
		{
			static_cast<Vector3&>( (*this)[3] ) += trans;
		}

		/** @brief  */
		inline void				setTranslation(const Vector3& trans)
		{
			static_cast<Vector3&>( (*this)[3] ) = trans;
		}

		/** @brief  */
		inline void				setTranslation(const Vector4& trans)
		{
			(*this)[3] = static_cast<const Vector3&>( trans );
		}

		/** @brief  */
		inline const Vector4&		getTranslation() const
		{
			return (*this)[3];
		}

		/** @brief  */
		inline	void		scale(const Vector3& vscale)
		{
			Vector4 v4scale(vscale, 1.0f);
			(*this)[0] *= v4scale;
			(*this)[1] *= v4scale;
			(*this)[2] *= v4scale;
			//(*this)[3] *= v4scale;
		}

		/** @brief  */
		inline void			setScale(const Vector3& vscale)
		{
			Vector4 v4scale(vscale, 1.0f);
			(*this)[0] *= vscale[0];
			(*this)[1] *= vscale[1];
			(*this)[2] *= vscale[2];
			//(*this)[3] *= v4scale;
		}

		/** @brief  */
		inline Vector3	getScale() const
		{
			//SRT de-composition
			return Vector3(
				reinterpret_cast<const Vector3&>((*this)[0]).getLength(), 
				reinterpret_cast<const Vector3&>((*this)[1]).getLength(), 
				reinterpret_cast<const Vector3&>((*this)[2]).getLength()
			);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void	transpose()
		{
#if BLADE_SIMD == BLADE_SIMD_SSE
			SIMD::vfp32x4 r0 = (*this)[0];
			SIMD::vfp32x4 r1 = (*this)[1];
			SIMD::vfp32x4 r2 = (*this)[2];
			SIMD::vfp32x4 r3 = (*this)[3];

			// x.x,x.y,y.x,y.y
			SIMD::vfp32x4 m1 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1,0,1,0));
			// x.z,x.w,y.z,y.w
			SIMD::vfp32x4 m3 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(3,2,3,2));
			// z.x,z.y,w.x,w.y
			SIMD::vfp32x4 m2 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(1,0,1,0));
			// z.z,z.w,w.z,w.w
			SIMD::vfp32x4 m4 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(3,2,3,2));

			// x.x,y.x,z.x,w.x
			(*this)[0] = _mm_shuffle_ps(m1, m2,_MM_SHUFFLE(2,0,2,0));
			// x.y,y.y,z.y,w.y
			(*this)[1] = _mm_shuffle_ps(m1, m2,_MM_SHUFFLE(3,1,3,1));
			// x.z,y.z,z.z,w.z
			(*this)[2] = _mm_shuffle_ps(m3, m4,_MM_SHUFFLE(2,0,2,0));
			// x.w,y.w,z.w,w.w
			(*this)[3] = _mm_shuffle_ps(m3, m4,_MM_SHUFFLE(3,1,3,1));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			SIMD::vfp32x4 r0 = (*this)[0];
			SIMD::vfp32x4 r1 = (*this)[1];
			SIMD::vfp32x4 r2 = (*this)[2];
			SIMD::vfp32x4 r3 = (*this)[3];

			float32x4x2_t P0 = vzipq_f32( r0, r2 );
			float32x4x2_t P1 = vzipq_f32( r1, r3 );

			float32x4x2_t T0 = vzipq_f32( P0.val[0], P1.val[0] );
			float32x4x2_t T1 = vzipq_f32( P0.val[1], P1.val[1] );

			(*this)[0] = T0.val[0];
			(*this)[1] = T0.val[1];
			(*this)[2] = T1.val[0];
			(*this)[3] = T1.val[1];
#else
			std::swap(mV[0][1],mV[1][0]);
			std::swap(mV[0][2],mV[2][0]);
			std::swap(mV[0][3],mV[3][0]);
			std::swap(mV[1][2],mV[2][1]);
			std::swap(mV[1][3],mV[3][1]);
			std::swap(mV[2][3],mV[3][2]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Matrix44			getTranspose() const
		{
			Matrix44 ret;
#if BLADE_SIMD == BLADE_SIMD_SSE
			SIMD::vfp32x4 r0 = (*this)[0];
			SIMD::vfp32x4 r1 = (*this)[1];
			SIMD::vfp32x4 r2 = (*this)[2];
			SIMD::vfp32x4 r3 = (*this)[3];

			// x.x,x.y,y.x,y.y
			SIMD::vfp32x4 m1 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1,0,1,0));
			// x.z,x.w,y.z,y.w
			SIMD::vfp32x4 m3 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(3,2,3,2));
			// z.x,z.y,w.x,w.y
			SIMD::vfp32x4 m2 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(1,0,1,0));
			// z.z,z.w,w.z,w.w
			SIMD::vfp32x4 m4 = _mm_shuffle_ps(r2, r3, _MM_SHUFFLE(3,2,3,2));

			// x.x,y.x,z.x,w.x
			ret[0] = _mm_shuffle_ps(m1, m2,_MM_SHUFFLE(2,0,2,0));
			// x.y,y.y,z.y,w.y
			ret[1] = _mm_shuffle_ps(m1, m2,_MM_SHUFFLE(3,1,3,1));
			// x.z,y.z,z.z,w.z
			ret[2] = _mm_shuffle_ps(m3, m4,_MM_SHUFFLE(2,0,2,0));
			// x.w,y.w,z.w,w.w
			ret[3] = _mm_shuffle_ps(m3, m4,_MM_SHUFFLE(3,1,3,1));
#elif BLADE_SIMD == BLADE_SIMD_NEON
			SIMD::vfp32x4 r0 = (*this)[0];
			SIMD::vfp32x4 r1 = (*this)[1];
			SIMD::vfp32x4 r2 = (*this)[2];
			SIMD::vfp32x4 r3 = (*this)[3];

			float32x4x2_t P0 = vzipq_f32( r0, r2 );
			float32x4x2_t P1 = vzipq_f32( r1, r3 );

			float32x4x2_t T0 = vzipq_f32( P0.val[0], P1.val[0] );
			float32x4x2_t T1 = vzipq_f32( P0.val[1], P1.val[1] );

			ret[0] = T0.val[0];
			ret[1] = T0.val[1];
			ret[2] = T1.val[0];
			ret[3] = T1.val[1];
#else
			ret = *this;
			ret.transpose();
#endif
			return ret;
		}

		/** @brief  */
		inline static	void		generateScale(Matrix44& out,const Vector3& vscale)
		{
			out = IDENTITY;
			out[0][0] = vscale.X();
			out[1][1] = vscale.Y();
			out[2][2] = vscale.Z();
		}

		/** @brief  */
		inline static	void		generateTranslation(Matrix44& out,const POINT3& trans)
		{
			out = IDENTITY;
			out[3].set(trans,1);
		}

		/** @brief  */
		inline static	Matrix44	generateScale(const Vector3& vscale)
		{
			Matrix44 ret;
			Matrix44::generateScale(ret, vscale);
			return ret;
		}

		/** @brief  */
		inline static	Matrix44	generateTranslation(const POINT3& trans)
		{
			Matrix44 ret;
			Matrix44::generateTranslation(ret, trans);
			return ret;
		}

		/** @brief generate SRT transform */
		inline static	Matrix44	generateTransform(const POINT3& pos,const POINT3& vscale,const Quaternion& rotation)
		{
			Matrix44 ret;
			Matrix44::generateTransform(ret, pos, vscale, rotation);
			return ret;
		}

		/** @brief RIGHT-HANDED */
		inline static Matrix44		generateViewMatrix(const POINT3& eye,const POINT3& lookatPoint, const Vector3& upVec = Vector3::UNIT_Y)
		{
			Matrix44 ret;
			Matrix44::generateViewMatrix(ret, eye, lookatPoint, upVec);
			return ret;
		}

		/** @brief RIGHT-HANDED */
		inline static Matrix44		generateViewMatrix(const POINT3& eye,const Quaternion& rotation)
		{
			Matrix44 ret;
			Matrix44::generateViewMatrix(ret, eye, rotation);
			return ret;
		}

		/** @brief RIGHT-HANDED */
		inline static Matrix44		generateOrthoProjectionMatrix(scalar w,scalar h,scalar zn,scalar zf)
		{
			Matrix44 ret;
			Matrix44::generateOrthoProjectionMatrix(ret, w, h ,zn, zf);
			return ret;
		}

		/** @brief RIGHT-HANDED */
		inline static Matrix44		generatePerspectiveProjectionMatrix(scalar w,scalar h,scalar zn,scalar zf)
		{
			Matrix44 ret;
			Matrix44::generatePerspectiveProjectionMatrix(ret, w, h ,zn, zf);
			return ret;
		}

		/** @brief RIGHT-HANDED */
		inline static Matrix44		generatePerspectiveFovProjectionMatrix(scalar fFOVx,scalar fAspect,scalar zNear,scalar zFar)
		{
			//generate a right hand perspective projection matrix
			Matrix44 ret;
			Matrix44::generatePerspectiveFovProjectionMatrix(ret, fFOVx, fAspect, zNear, zFar);
			return ret;
		}

		/** @brief  */
		BLADE_BASE_API void				set(const Quaternion& rotation, const Vector3& translation);

		/** @brief  */
		BLADE_BASE_API Matrix44			concatenate(const Matrix44 &rhs) const;

		/** @brief  */
		BLADE_BASE_API scalar			getDeterminant() const;

		/** @brief  */
		BLADE_BASE_API Matrix44			getAdjoint() const;

		/** @brief  */
		BLADE_BASE_API Matrix44			getInverse() const;

		/** @brief  */
		BLADE_BASE_API static	void	generateTransform(Matrix44& out,const POINT3& pos,const Vector3& vscale,const Quaternion& rotation);

		/** @brief  */
		BLADE_BASE_API static void		generateViewMatrix(Matrix44& out,const POINT3& eye,const POINT3& lookatPoint, const Vector3& upVec = Vector3::UNIT_Y);

		/** @brief  */
		BLADE_BASE_API static void		generateViewMatrix(Matrix44& out,const POINT3& eye,const Quaternion& rotation);

		/** @brief  */
		BLADE_BASE_API static void		generateOrthoProjectionMatrix(Matrix44& out,scalar w,scalar h,scalar zn,scalar zf);

		/** @brief  */
		BLADE_BASE_API static void		generatePerspectiveProjectionMatrix(Matrix44& out,scalar w,scalar h,scalar zn,scalar zf);

		/** @brief  */
		BLADE_BASE_API static void		generatePerspectiveFovProjectionMatrix(Matrix44& out,scalar fFOVx,scalar fAspect,scalar zNear,scalar zFar);

		BLADE_BASE_API static Matrix44		IDENTITY;
		BLADE_BASE_API static Matrix44		ZERO;
	protected:
		/** @brief  */
		BLADE_BASE_API scalar			getMinor(size_t i,size_t j) const;

		/** @brief  */
		BLADE_BASE_API scalar			getCofactor(size_t i ,size_t j) const;

		/** @brief  */
		BLADE_BASE_API scalar			getMinor2x2(size_t row0,size_t row1,size_t col0,size_t col1) const;

		union
		{
			scalar	mV[4][4];
			scalar	mM[16];
		};
	};//class Matrix44
	
}//namespace Blade



#endif //__Blade_Matrix44_h__