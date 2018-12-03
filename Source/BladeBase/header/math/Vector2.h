/********************************************************************
	created:	2010/04/20
	filename: 	Vector2.h
	author:		Crazii
	
	purpose:	
	NEON doc:	http://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/ARM-NEON-Intrinsics.html
*********************************************************************/
#ifndef __Blade_Vector2_h__
#define __Blade_Vector2_h__
#include <BladeException.h>
#include <math/BladeSIMD.h>

namespace Blade
{
	class Vector2
	{
	public:

		inline Vector2()
		{

		}

		inline Vector2(scalar x,scalar y)
		{
			mData[0] = x;
			mData[1] = y;
		}

		inline Vector2(const Vector2& v)
		{
			*this = v;
		}

#if BLADE_ENABLE_SIMD

		BLADE_ALWAYS_INLINE Vector2(SIMD::vfp32x2 v)
		{
			*this = v;
		}

		BLADE_ALWAYS_INLINE operator SIMD::vfp32x2() const
		{
			return SIMD::VLOAD2(mData);
		}

		BLADE_ALWAYS_INLINE Vector2& operator=(SIMD::vfp32x2 v)
		{
			SIMD::VSTORE2(v, mData);
			return *this;
		}

		BLADE_ALWAYS_INLINE Vector2&		operator=(const Vector2& rhs)
		{
#	if BLADE_ENABLE_SIMD
			*this = (SIMD::vfp32x2)rhs;
#	else
			mData[0] = rhs.mData[0];
			mData[1] = rhs.mData[1];
#	endif
			return *this;
		}
#endif
		
		explicit inline Vector2(const scalar* pdata)
		{
#if BLADE_ENABLE_SIMD
			SIMD::vfp32x2 t = SIMD::VLOAD2(pdata);
			*this = t;
#else
			mData[0] = pdata[0];
			mData[1] = pdata[1];
#endif
		}

		/** @brief  */
		inline scalar	operator[](size_t sub) const
		{
			assert( sub < 2 );
			if( sub < 2 )
				return mData[sub];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range.") );
		}

		/** @brief  */
		inline scalar&	operator[](size_t sub)
		{
			assert( sub < 2 );
			if( sub < 2 )
				return mData[sub];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range.") );			
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
		BLADE_ALWAYS_INLINE bool			operator==(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VEQUAL2(*this, rhs);
#else
			return x == rhs.x && y == rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator!=(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return !SIMD::VEQUAL2(*this, rhs);
#else
			return x != rhs.x || y != rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator<(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLESS2(*this, rhs);
#else
			return x < rhs.x && y < rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator>(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VGREATER2(*this, rhs);
#else
			return x > rhs.x && y > rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator<=(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLESSEQUAL2(*this, rhs);
#else
			return x <= rhs.x && y <= rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator>=(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VGREATEREQUAL2(*this, rhs);
#else
			return x >= rhs.x && y >= rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator+(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD2(*this, rhs);
#else
			Vector2 ret;
			ret.x = x + rhs.x;
			ret.y = y + rhs.y;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator-(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB2(*this, rhs);
#else
			Vector2 ret;
			ret.x = x - rhs.x;
			ret.y = y - rhs.y;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator*(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL2(*this, rhs);
#else
			Vector2 ret;
			ret.x = x * rhs.x;
			ret.y = y * rhs.y;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator/(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV2(*this, rhs);
#else
			Vector2 ret;
			ret.mData[0] = mData[0] / rhs.x;
			ret.mData[1] = mData[1] / rhs.y;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator+(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD2(*this, rhs);
#else
			Vector2 ret;
			ret.mData[0] = mData[0] + rhs;
			ret.mData[1] = mData[1] + rhs;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator-(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB2(*this, rhs);
#else
			Vector2 ret;
			ret.mData[0] = mData[0] - rhs;
			ret.mData[1] = mData[1] - rhs;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator*(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL2(*this, rhs);
#else
			Vector2 ret;
			ret.mData[0] = mData[0] * rhs;
			ret.mData[1] = mData[1] * rhs;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator/(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV2(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			Vector2 ret;
			ret.mData[0] = mData[0] * inv_rhs;
			ret.mData[1] = mData[1] * inv_rhs;
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2		operator-() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VNEG2(*this);
#else
			Vector2 ret;
			ret.mData[0] = -mData[0];
			ret.mData[1] = -mData[1];
			return ret;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator+=(const Vector2& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD2(*this, rhs);
#else
			mData[0] += rhs.mData[0];
			mData[1] += rhs.mData[1];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator-=(const Vector2& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB2(*this, rhs);
#else
			mData[0] -= rhs.mData[0];
			mData[1] -= rhs.mData[1];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator*=(const Vector2& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VMUL2(*this, rhs);
#else
			mData[0] *= rhs.mData[0];
			mData[1] *= rhs.mData[1];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator/=(const Vector2& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV2(*this, rhs);
#else
			mData[0] /= rhs.mData[0];
			mData[1] /= rhs.mData[1];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator+=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD2(*this, rhs);
#else
			mData[0] += rhs;
			mData[1] += rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator-=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB2(*this, rhs);
#else
			mData[0] -= rhs;
			mData[1] -= rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator*=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD2(*this, rhs);
#else
			mData[0] *= rhs;
			mData[1] *= rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2&		operator/=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV2(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			mData[0] *= inv_rhs;
			mData[1] *= inv_rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	bool	equal(const Vector2& rhs, scalar tolerance) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VEQUALTOLERANCE2(*this, rhs, tolerance);
#else
			return Math::Equal(mData[0], rhs.mData[0], tolerance)
				&& Math::Equal(mData[1], rhs.mData[1], tolerance);
#endif
		}

		/** @brief  */
		scalar			getSquaredLength() const
		{
			return this->dotProduct(*this);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	scalar	getLength() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLENGTH2(*this);
#else
			fp32 lengthSq = getSquaredLength();
			return std::sqrt(lengthSq);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector2	getAbs() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VABS2(*this);
#else
			return Vector2(std::fabs(x), std::fabs(y));
#endif
		}

		/** @brief  */
		scalar			getDistance(const Vector2& dest) const
		{
			return (*this-dest).getLength();
		}

		/** @brief  */
		scalar			getSquaredDistance(const Vector2& dest) const
		{
			return (*this-dest).getSquaredLength();
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	scalar	dotProduct(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDOT2(*this, rhs);
#else
			return x*rhs.x + y*rhs.y;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	scalar	crossProduct(const Vector2& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VCROSS2(*this, rhs);
#else
			return x*rhs.y - y*rhs.x;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	scalar	normalize()
		{
#if BLADE_ENABLE_SIMD
			SIMD::vfp32x2 t = *this;
			fp32 ret = SIMD::VNORMALIZE2(t);
			*this = t;
			return ret;
#else
			scalar lengthSq = this->getSquaredLength();
			if( !Math::Equal<scalar>(lengthSq, 1.0, Math::HIGH_EPSILON) )
			{
				scalar length = ::sqrt(lengthSq);
				x /= length;
				y /= length;
				return length;
			}
			else
				return 1;
#endif
		}

		/** @brief  */
		Vector2			getNormalizedVector() const
		{
			Vector2 r = *this;
			r.normalize();
			return r;
		}

		/** @brief  */
		Vector2			getPerpendicularVector() const
		{
			return Vector2(y, -x);
		}

		/** @brief  */
		Vector2			getReflectVector(const Vector2& normal) const
		{
			return *this - normal*this->dotProduct(normal)*2;
		}

		/** @brief  */
		Vector2			getMidPoint(const Vector2& dest) const
		{
			return (*this + dest)*0.5;
		}

		/** @brief  */
		bool			isZero() const
		{
			return this->equal(Vector2::ZERO, Math::HIGH_EPSILON);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static inline void	getMaxVector(Vector2& out,const Vector2& lhs,const Vector2& rhs)
		{
			out = Vector2::getMaxVector(lhs, rhs);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static inline void	getMinVector(Vector2& out,const Vector2& lhs,const Vector2& rhs)
		{
			out = Vector2::getMinVector(lhs, rhs);
		}


		/**
		@describe 
		@param 
		@return 
		*/
		static BLADE_ALWAYS_INLINE Vector2	getMaxVector(const Vector2& lhs,const Vector2& rhs)
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMAX2(lhs, rhs);
#else
			Vector2 out;
			if( lhs.x > rhs.x )
				out.x = lhs.x;
			else
				out.x = rhs.x;
			if( lhs.y > rhs.y )
				out.y = lhs.y;
			else
				out.y = rhs.y;
			return out;
#endif
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static BLADE_ALWAYS_INLINE Vector2	getMinVector(const Vector2& lhs, const Vector2& rhs)
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMIN2(lhs, rhs);
#else
			Vector2 out;
			if( lhs.x < rhs.x )
				out.x = lhs.x;
			else
				out.x = rhs.x;
			if( lhs.y < rhs.y )
				out.y = lhs.y;
			else
				out.y = rhs.y;
			return out;
#endif
		}

	public:
		union
		{
			struct
			{
				scalar x,y;
			};
			scalar	mData[2];
		};

		//add class keyword to avoid weird symbol conflicts(with 3ds max sdk)
		BLADE_BASE_API static const class Vector2	ZERO;
		BLADE_BASE_API static const class Vector2	UNIT_X;
		BLADE_BASE_API static const class Vector2	UNIT_Y;
		BLADE_BASE_API static const class Vector2	UNIT_ALL;
		BLADE_BASE_API static const class Vector2	NEGATIVE_UNIT_X;
		BLADE_BASE_API static const class Vector2	NEGATIVE_UNIT_Y;
		BLADE_BASE_API static const class Vector2	NEGATIVE_UNIT_ALL;

	};//class Vector2

	typedef Vector2 POINT2;

	///commutative +
	BLADE_ALWAYS_INLINE static Vector2	operator+(scalar lhs, const Vector2& rhs)
	{
		return rhs + lhs;
	}

	///commutative *
	BLADE_ALWAYS_INLINE static Vector2	operator*(scalar lhs, const Vector2& rhs)
	{
		return rhs * lhs;
	}
	
}//namespace Blade


#endif //__Blade_Vector2_h__