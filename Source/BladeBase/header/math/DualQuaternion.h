/********************************************************************
	created:	2014/08/15
	filename: 	DualQuaternion.h
	author:		Crazii
	
	purpose:	dual quaternion for skinning
				ref: http://www.seas.upenn.edu/~ladislav/kavan08geometric/kavan08geometric.html
				Dual quaternion Linear Blending(DLB) is suitable(not 100% accurate though) for multiple transforms,
				but only suitable for small changes.
				Dual quaternion Iterative Blending(DIB) can be used to increasing accuracy.

				operations:http://www.xbdev.net/misc_demos/demos/dual_quaternions_beyond/paper.pdf
*********************************************************************/
#ifndef __Blade_DualQuaternion_h__
#define __Blade_DualQuaternion_h__
#include "Quaternion.h"
#include "Vector4.h"
#if BLADE_DEBUG
#	include <math/BladeMath.h>
#endif

namespace Blade
{
	class Matrix44;
	class Matrix33;

	class DualQuaternion
	{
	public:
		BLADE_ALWAYS_INLINE DualQuaternion() {}

		BLADE_ALWAYS_INLINE DualQuaternion(const Quaternion& _p, const Quaternion& _q):real(_p),dual(_q)	{}

		BLADE_ALWAYS_INLINE DualQuaternion(const Quaternion& r, const Vector3& t)	{this->set(r, t);}

		BLADE_ALWAYS_INLINE DualQuaternion(const Matrix44& m)						{this->set(m);}

		BLADE_ALWAYS_INLINE DualQuaternion(const Matrix33& m, const Vector3& t)		{this->set(m, t);}

		/** @brief  */
		inline void set(const Quaternion& r, const Vector3& t)
		{
			real = r;
#if BLADE_SIMD != BLADE_SIMD_NONE
			Vector4 v = Vector4(t,0);
			dual = reinterpret_cast<const Quaternion&>(v).multiply(r) * 0.5;
#else
			dual.x = 0.5f * ( t[0] * r.w + t[1] * r.z - t[2] * r.y ) ; 
			dual.y = 0.5f * (-t[0] * r.z + t[1] * r.w + t[2] * r.x ) ; 
			dual.z = 0.5f * ( t[0] * r.y - t[1] * r.x + t[2] * r.w ) ; 
			dual.w = -0.5f * (t[0] * r.x + t[1] * r.y + t[2] * r.z ) ; 
#endif
		}

		BLADE_ALWAYS_INLINE void		setTranslation(const Vector3& t)
		{
			const Quaternion& r = real;
#if BLADE_SIMD != BLADE_SIMD_NONE
			Vector4 v = Vector4(t,0);
			dual = reinterpret_cast<const Quaternion&>(v).multiply(r) * 0.5;
#else
			dual.x = 0.5f * ( t[0] * r.w + t[1] * r.z - t[2] * r.y ) ; 
			dual.y = 0.5f * (-t[0] * r.z + t[1] * r.w + t[2] * r.x ) ; 
			dual.z = 0.5f * ( t[0] * r.y - t[1] * r.x + t[2] * r.w ) ; 
			dual.w = -0.5f * (t[0] * r.x + t[1] * r.y + t[2] * r.z ) ; 
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	normalize()
		{
			scalar length = real.normalize();
			dual /= length;
			return length;
		}

		BLADE_ALWAYS_INLINE DualQuaternion	getNormalizedDQ() const
		{
			scalar length = real.getLength();
			return DualQuaternion( real/length, dual/length );
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar		inverse()
		{
			scalar normSQ = real.inverse();
			dual = dual.getConjugate()/normSQ;
			return normSQ;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE DualQuaternion	getInversedDQ() const
		{
			Quaternion ir = real;
			scalar normSQ = ir.inverse();
			return DualQuaternion(ir, dual.getConjugate()/normSQ);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE DualQuaternion getConjugate()  const
		{ 
			return DualQuaternion( real.getConjugate(), dual.getConjugate() );
		} 

		/** @brief  */
		BLADE_ALWAYS_INLINE DualQuaternion	operator-() const
		{
			return DualQuaternion(-real, -dual);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE DualQuaternion	operator+(const DualQuaternion& rhs) const
		{
			return DualQuaternion(real + rhs.real, dual+rhs.dual );
		}

		BLADE_ALWAYS_INLINE DualQuaternion&	operator+=(const DualQuaternion& rhs)
		{
			real += rhs.real;
			dual += rhs.dual;
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE DualQuaternion		operator*(scalar t) const
		{
			return DualQuaternion(real*t, dual*t);
		}

		BLADE_ALWAYS_INLINE DualQuaternion&		operator*=(scalar t)
		{
			real *= t;
			dual *= t;
			return *this;
		}

		/** @note: multiplication: *this * rhs */
		BLADE_ALWAYS_INLINE DualQuaternion		multiply(const DualQuaternion& rhs) const
		{
#if 0
			const DualQuaternion nl = this->getNormalizedDQ();
			const DualQuaternion nr = rhs.getNormalizedDQ();
#else
			//must be normalized
			assert( Math::Equal<scalar>(real.getLength(), 1.0f, Math::LOW_EPSILON) );
			assert( Math::Equal<scalar>(rhs.real.getLength(), 1.0f, Math::LOW_EPSILON) );
			const DualQuaternion& nl = *this;
			const DualQuaternion& nr = rhs;
#endif
			return DualQuaternion(nl.real.multiply(nr.real), nl.real.multiply(nr.dual) + nl.dual.multiply(nr.real) );
		}

		/**
		@describe transform concatenation
		@param
		@return
		if you need a "dual quaternion multiplication" you should use DualQuaternion::multiply()
		if you are doing a transform concatenation, use this function.
		transform order of dq1, dq2 are represented as dq1*dq2
		*/
		BLADE_ALWAYS_INLINE DualQuaternion	operator*(const DualQuaternion& rhs) const
		{
			return rhs.multiply(*this);
		}

		BLADE_ALWAYS_INLINE DualQuaternion&	operator*=(const DualQuaternion& rhs)
		{
			*this = *this * rhs;
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE const Quaternion& getRotation() const
		{
			return real;
		}

		/** @brief translation vector */
		BLADE_ALWAYS_INLINE Vector3			getTranslation() const
		{
#if 0
			// As the dual quaternions may be the results from a
			// linear blending we have to normalize it :
			this->normalize();
#else
			assert( Math::Equal<scalar>(real.getLength(), 1.0f, Math::LOW_EPSILON) );
#endif
			// translation from the normalized dual quaternion equals:
			// 2.f * dual * conjugate(real)
#if BLADE_ENABLE_SIMD
			Quaternion trans = dual.multiply(real.getConjugate()) * 2;
			return reinterpret_cast<const Vector4&>(trans);
#else
			const Vector3& v0 = reinterpret_cast<const Vector3&>(real);
			const Vector3& ve = reinterpret_cast<const Vector3&>(dual);
			return (ve*real.w - v0*dual.w + v0.crossProduct(ve) ) * 2.f;
#endif
		}

		BLADE_ALWAYS_INLINE Vector3	transform(const Vector3& v) const
		{
#if 0
			this->normalize();
#else
			assert( Math::Equal<scalar>(real.getLength(), 1.0f, Math::LOW_EPSILON) );
#endif
			// rotate + translation
			return v * real + this->getTranslation();
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			rotate(const Vector3& v) const
		{
			return v * real;
		}

		/** @brief normalized linear interpolation */
		BLADE_ALWAYS_INLINE DualQuaternion&	nlerpWith(const DualQuaternion& rhs, scalar t, bool shortPath = false)
		{
#if 0
			// As the dual quaternions may be the results from a
			// linear blending we have to normalize it :
			this->normalize();
#else
			assert( Math::Equal<scalar>(real.getLength(), 1.0f, Math::LOW_EPSILON) );
			assert( Math::Equal<scalar>(rhs.real.getLength(), 1.0f, Math::LOW_EPSILON) );
#endif

			scalar length = 0;
			shortPath = real.nlerpWith(rhs.real, t, shortPath, &length);
			dual *= (1-t);

			if( shortPath )
				dual -= rhs.dual*t;
			else
				dual += rhs.dual*t;

			dual /= length;
			return *this;
		}

		/** @brief  */
		BLADE_BASE_API void		set(const Matrix44& matrix);

		/** @brief  */
		BLADE_BASE_API void		set(const Matrix33& rotation, const Vector3& translation);

		/** @brief to transform matrix */
		BLADE_BASE_API void		toMatrix(Matrix44& outMatrix) const;

		/** @brief ScLERP(Screw Linear Interpolation) */
		BLADE_BASE_API DualQuaternion&	sclerpWith(const DualQuaternion& rhs, scalar t, bool shortPath = false);


		//add class keyword to avoid weird symbol conflicts
		BLADE_BASE_API static const class DualQuaternion IDENTITY;
		BLADE_BASE_API static const class DualQuaternion ZERO;

		Quaternion real;	///real part: usually for rotation
		Quaternion dual;	///dual part: usually for translation
	};

	BLADE_ALWAYS_INLINE static Vector3	operator*(const Vector3& v, const DualQuaternion& dq)
	{
		return dq.transform(v);
	}

	BLADE_ALWAYS_INLINE static Vector3& operator*=(Vector3& v, const DualQuaternion& dq)
	{
		return (v = v*dq);
	}

	static_assert(sizeof(DualQuaternion) == 32, "size error");

}//namespace Blade


#endif // __Blade_DualQuaternion_h__