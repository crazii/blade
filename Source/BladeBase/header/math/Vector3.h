/********************************************************************
	created:	2010/04/20
	filename: 	Vector3.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Vector3_h__
#define __Blade_Vector3_h__
#include <BladeException.h>
#include <math/BladeSIMD.h>

namespace Blade
{
	class Matrix44;
	class Matrix33;

	class Vector3
	{
	public:
		inline Vector3()
		{

		}

		inline Vector3(scalar x,scalar y,scalar z)
		{
			mData[0] = x;
			mData[1] = y;
			mData[2] = z;
		}

#if BLADE_ENABLE_SIMD
		BLADE_ALWAYS_INLINE Vector3(SIMD::vfp32x4 v)
		{
			*this = v;
		}

		BLADE_ALWAYS_INLINE operator SIMD::vfp32x4() const
		{
			return SIMD::VLOAD3(mData);
		}

		BLADE_ALWAYS_INLINE Vector3& operator=(SIMD::vfp32x4 v)
		{
			SIMD::VSTORE3(v, mData);
			return *this;
		}
#endif

		inline Vector3(const Vector3& v)
		{
			*this = v;
		}

		explicit inline Vector3(const scalar* pdata)
		{
			*this = *reinterpret_cast<const Vector3*>(pdata);
		}

		/** @brief  */
		inline scalar	operator[](size_t sub) const
		{
			assert( sub < 3 );
			if( sub < 3 )
				return mData[sub];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range.") );
		}

		/** @brief  */
		inline scalar&	operator[](size_t sub)
		{
			assert( sub < 3 );
			if( sub < 3 )
				return mData[sub];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range.") );
		}

		/** @brief  */
		inline const scalar	X() const
		{
			return mData[0];
		}

		/** @brief  */
		inline scalar&	X()
		{
			return mData[0];
		}


		/** @brief  */
		inline const scalar	Y() const
		{
			return mData[1];
		}

		/** @brief  */
		inline scalar&	Y()
		{
			return mData[1];
		}

		/** @brief  */
		inline const scalar	Z() const
		{
			return mData[2];
		}

		/** @brief  */
		inline scalar&	Z()
		{
			return mData[2];
		}

		/** @brief  */
		inline const scalar*	getData() const
		{
			return mData;
		}

		/** @brief  */
		inline scalar*			getData()
		{
			return mData;
		}

		/** @brief  */
		inline void				setData(const scalar* data)
		{
			*this = *reinterpret_cast<const Vector3*>(data);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator=(const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			SIMD::vfp32x4 v = (SIMD::vfp32x4)(rhs);
			*this = v;
#else
			std::memcpy(mData, rhs.mData, sizeof(mData));
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator==(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VEQUAL3((*this), rhs);
#else
			return mData[0] == rhs.mData[0] && mData[1] == rhs.mData[1] && mData[2] == rhs.mData[2];
#endif	
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator!=(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return !SIMD::VEQUAL3((*this), rhs);
#else
			return mData[0] != rhs.mData[0] || mData[1] != rhs.mData[1] || mData[2] != rhs.mData[2];
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator<(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLESS3((*this), rhs);
#else
			return mData[0] < rhs.mData[0] && mData[1] < rhs.mData[1] && mData[2] < rhs.mData[2];
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator>(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VGREATER3((*this), rhs);
#else
			return mData[0] > rhs.mData[0] && mData[1] > rhs.mData[1] && mData[2] > rhs.mData[2];
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator<=(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLESSEQUAL3((*this), rhs);
#else
			return mData[0] <= rhs.mData[0] && mData[1] <= rhs.mData[1] && mData[2] <= rhs.mData[2];
#endif	
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator>=(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VGREATEREQUAL3((*this), rhs);
#else
			return mData[0] >= rhs.mData[0] && mData[1] >= rhs.mData[1] && mData[2] >= rhs.mData[2];
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator+(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD((*this), rhs);
#else
			return Vector3(mData[0]+rhs.mData[0],mData[1]+rhs.mData[1],mData[2] + rhs.mData[2]);
#endif	
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator-(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB((*this), rhs);
#else
			return Vector3(mData[0]-rhs.mData[0],mData[1]-rhs.mData[1],mData[2] - rhs.mData[2]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator*(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL((*this), rhs);
#else
			return Vector3(mData[0]*rhs.mData[0],mData[1]*rhs.mData[1],mData[2] * rhs.mData[2]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator/(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV((*this), rhs);
#else
			return Vector3(mData[0]/rhs.mData[0],mData[1]/rhs.mData[1],mData[2] / rhs.mData[2]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator+(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD((*this), rhs);
#else
			return Vector3(mData[0]+rhs,mData[1]+rhs,mData[2]+rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator-(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB((*this), rhs);
#else
			return Vector3(mData[0]-rhs,mData[1]-rhs,mData[2]-rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator*(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL((*this), rhs);
#else
			return Vector3(mData[0]*rhs,mData[1]*rhs,mData[2]*rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator/(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV((*this), rhs);
#else
			scalar inv_rhs = 1.0f / rhs;
			return Vector3(mData[0]*inv_rhs,mData[1]*inv_rhs,mData[2]*inv_rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			operator-() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VNEG((*this));
#else
			return Vector3(-mData[0],-mData[1],-mData[2]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator+=(const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD((*this), rhs);
#else
			mData[0] += rhs.mData[0];
			mData[1] += rhs.mData[1];
			mData[2] += rhs.mData[2];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator-=(const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB(*this, rhs);
#else
			mData[0] -= rhs.mData[0];
			mData[1] -= rhs.mData[1];
			mData[2] -= rhs.mData[2];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator*=(const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VMUL(*this, rhs);
#else
			mData[0] *= rhs.mData[0];
			mData[1] *= rhs.mData[1];
			mData[2] *= rhs.mData[2];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator/=(const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV(*this, rhs);
#else
			mData[0] /= rhs.mData[0];
			mData[1] /= rhs.mData[1];
			mData[2] /= rhs.mData[2];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator+=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD(*this, rhs);
#else
			mData[0] += rhs;
			mData[1] += rhs;
			mData[2] += rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator-=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB(*this, rhs);
#else
			mData[0] -= rhs;
			mData[1] -= rhs;
			mData[2] -= rhs;
#endif
			return *this;

		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator*=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VMUL(*this, rhs);
#else
			mData[0] *= rhs;
			mData[1] *= rhs;
			mData[2] *= rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3&		operator/=(const scalar rhs)
		{
			
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			mData[0] *= inv_rhs;
			mData[1] *= inv_rhs;
			mData[2] *= inv_rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMaxComponents(const Vector3& vcompare)
		{
#if BLADE_ENABLE_SIMD
			*this = Vector3::getMaxVector(*this, vcompare);
#else
			if( vcompare.X() > this->X() )
				this->X() = vcompare.X();

			if( vcompare.Y() > this->Y() )
				this->Y() = vcompare.Y();

			if( vcompare.Z() > this->Z() )
				this->Z() = vcompare.Z();
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	void		setMinComponents(const Vector3& vcompare)
		{
#if BLADE_ENABLE_SIMD
			*this = Vector3::getMinVector(*this, vcompare);
#else
			if( vcompare.X() < this->X() )
				this->X() = vcompare.X();

			if( vcompare.Y() < this->Y() )
				this->Y() = vcompare.Y();

			if( vcompare.Z() < this->Z() )
				this->Z() = vcompare.Z();
#endif
		}

		/*
		@describe 
		@param 
		@return 
		*/
		static BLADE_ALWAYS_INLINE Vector3	getMaxVector(const Vector3& lhs, const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMAX(lhs, rhs);
#else
			Vector3 ret;
			if( lhs.x > rhs.x )
				ret.x = lhs.x;
			else
				ret.x = rhs.x;

			if( lhs.y > rhs.y )
				ret.y = lhs.y;
			else
				ret.y = rhs.y;

			if( lhs.z > rhs.z )
				ret.z = lhs.z;
			else
				ret.z = rhs.z;

			return ret;
#endif
		}


		/*
		@describe 
		@param 
		@return 
		*/
		static BLADE_ALWAYS_INLINE Vector3	getMinVector(const Vector3& lhs,const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMIN(lhs, rhs);
#else
			Vector3 ret;
			if( lhs.x < rhs.x )
				ret.x = lhs.x;
			else
				ret.x = rhs.x;

			if( lhs.y < rhs.y )
				ret.y = lhs.y;
			else
				ret.y = rhs.y;

			if( lhs.z < rhs.z )
				ret.z = lhs.z;
			else
				ret.z = rhs.z;

			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool		equal(const Vector3& rhs, scalar tolerance) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VEQUALTOLERANCE3(*this, rhs, tolerance);
#else
			return Math::Equal(mData[0], rhs.mData[0], tolerance)
				&& Math::Equal(mData[1], rhs.mData[1], tolerance)
				&& Math::Equal(mData[2], rhs.mData[2], tolerance);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	getLength() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLENGTH3(*this);
#else
			fp32 lengthSq = getSquaredLength();
			return std::sqrt(lengthSq);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getAbs() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VABS(*this);
#else
			return Vector3(std::fabs(x), std::fabs(y), std::fabs(z));
#endif
		}

		/** @brief  */
		inline scalar	getSquaredLength() const
		{
			return this->dotProduct(*this);
		}

		/** @brief  */
		inline scalar	getDistance(const Vector3& dest) const
		{
			return (*this - dest).getLength();
		}

		/** @brief  */
		inline scalar	getSquaredDistance(const Vector3& dest) const
		{
			return (*this - dest).getSquaredLength();
		}

		/** @brief  */
		inline scalar	getAngleTo(const Vector3& dest) const
		{
			scalar lenProduct = this->getLength()*dest.getLength();

			if( Math::Equal(lenProduct,scalar(0),scalar(Math::LOW_EPSILON) ) )
				return 0;

			scalar dot = this->dotProduct(dest);
			scalar angle =  ::acos( Math::Clamp(dot/lenProduct, -1.f, 1.f) );
			Math::Clamp(angle,scalar(-1.0)*Math::PI,scalar(1.0)*Math::PI );
			return angle;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar		dotProduct(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDOT3(*this, rhs);
#else
			return x*rhs.x + y*rhs.y + z*rhs.z;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3		crossProduct(const Vector3& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VCROSS3(*this, rhs);
#else
			Vector3 ret;
			const Vector3& lhs = *this;
			ret.x =  lhs.y * rhs.z - lhs.z * rhs.y;
			ret.y =  lhs.z * rhs.x - lhs.x * rhs.z;
			ret.z =  lhs.x * rhs.y - lhs.y * rhs.x;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar			normalize()
		{
#if BLADE_ENABLE_SIMD
			SIMD::vfp32x4 t = *this;
			scalar ret = SIMD::VNORMALIZE3(t);
			*this = t;
			return ret;
#else
			scalar lengthSq = this->getSquaredLength();
			if( !Math::Equal<scalar>(lengthSq, 1.0, Math::HIGH_EPSILON) )
			{
				scalar length = ::sqrt(lengthSq);
				x /= length;
				y /= length;
				z /= length;
				return length;
			}
			else
				return 1;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getNormalizedVector() const
		{
			Vector3 ret = *this;
			ret.normalize();
			return ret;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getPerpendicularVector() const
		{
			Vector3 perpendicular = this->crossProduct( UNIT_X );
			if( perpendicular.isZero() )
				perpendicular = this->crossProduct( UNIT_Y );
			return perpendicular;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getReflectVector(const Vector3& normal) const
		{
			return *this - normal*this->dotProduct(normal)*2;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getMidPoint(const Vector3& dest) const
		{
			return (*this + dest) * 0.5;
		}

		/** @brief  */
		bool			isZero() const
		{
			return this->equal(Vector3::ZERO, Math::HIGH_EPSILON);
		}

	public:

		union
		{
			struct
			{
				scalar x,y,z;
			};
			scalar	mData[3];
		};

		/** @brief  */
		BLADE_BASE_API Vector3&	operator*=(const Matrix44& rhs);

		/** @brief  */
		BLADE_BASE_API Vector3		operator*(const Matrix44& rhs) const;

		/** @brief  */
		BLADE_BASE_API Vector3&		operator*=(const Matrix33& rhs);

		/** @brief  */
		BLADE_BASE_API Vector3		operator*(const Matrix33& rhs) const;

		//add class keyword to avoid weird symbol conflicts
		BLADE_BASE_API static const class Vector3	ZERO;
		BLADE_BASE_API static const class Vector3	UNIT_X;
		BLADE_BASE_API static const class Vector3	UNIT_Y;
		BLADE_BASE_API static const class Vector3	UNIT_Z;
		BLADE_BASE_API static const class Vector3	UNIT_ALL;
		BLADE_BASE_API static const class Vector3	NEGATIVE_UNIT_X;
		BLADE_BASE_API static const class Vector3	NEGATIVE_UNIT_Y;
		BLADE_BASE_API static const class Vector3	NEGATIVE_UNIT_Z;
		BLADE_BASE_API static const class Vector3	NEGATIVE_UNIT_ALL;
	};//class Vector3

	typedef Vector3 POINT3;

	///commutative +
	BLADE_ALWAYS_INLINE static Vector3	operator+(scalar lhs, const Vector3& rhs)
	{
		return rhs + lhs;
	}
	///commutative *
	BLADE_ALWAYS_INLINE static Vector3	operator*(scalar lhs, const Vector3& rhs)
	{
		return rhs * lhs;
	}
	
}//namespace Blade

#endif //__Blade_Vector3_h__