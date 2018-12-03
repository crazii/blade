/********************************************************************
	created:	2010/04/20
	filename: 	Plane.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Plane_h__
#define __Blade_Plane_h__
#include <math/BladeMath.h>
#include <math/Vector3.h>

namespace Blade
{
	class Sphere;
	class Matrix44;
	class Box3;

	class Plane
	{
	public:
		inline Plane()
		{
			mNormal = Vector3::ZERO;
			mD = 0.0f;
		}

		inline Plane(const Plane& rhs)
		{
			mNormal = rhs.mNormal;
			mD = rhs.mD;
		}

		inline Plane(const Vector3& normal,scalar d)
		{
			mNormal = normal;
			mD = d;
		}

		inline Plane(scalar nx,scalar ny,scalar nz,scalar d)
		{
			mNormal.x	= nx;
			mNormal.y	= ny;
			mNormal.z	= nz;
			mD			= d;
		}

		inline Plane(const Vector3& p0,const Vector3& p1,const Vector3& p2)
		{
			this->set(p0,p1,p2);
		}

		inline Plane(const Vector3& normal,const Vector3& point)
		{
			this->set(normal,point);
		}

		inline	bool	operator==(const Plane& rhs) const
		{
			return mD == rhs.mD && mNormal == rhs.mNormal;
		}

		inline bool		operator!=(const Plane& rhs) const
		{
			return mD != rhs.mD || mNormal != rhs.mNormal;
		}

		inline Plane	operator-() const
		{
			return Plane(-mNormal, -mD);
		}

		/** @brief  */
		inline const Vector3&	getNormal() const
		{
			return mNormal;
		}

		/** @brief  */
		inline scalar	getPlaneDistance() const
		{
			return -mD;
		}

		/** @brief  */
		inline Plane&		operator*=(const Matrix44& xform)
		{
			*this = (*this)* xform;
			return *this;
		}

		/** @brief  */
		inline void		set(const Vector3& p0,const Vector3& p1,const Vector3& p2)
		{
			Vector3 v0 = p1 - p0;
			Vector3 v1 = p2 - p0;
			mNormal = v0.crossProduct(v1);
			mNormal.normalize();
			mD = -this->getNormal().dotProduct(p0);
		}

		/** @brief  */
		inline void		set(const Vector3& normal,const POINT3& point)
		{
			mNormal = normal;
			mD = -this->getNormal().dotProduct(point);
		}

		/** @brief  */
		inline scalar		getDistance(const POINT3& point) const
		{
			return this->getNormal().dotProduct(point) + mD;
		}

		/** @brief  */
		inline POINT3		getProjectedPoint(const POINT3& point) const
		{
			scalar d = this->getDistance(point);
			return point - mNormal * d;
		}

		/** @brief  */
		inline POS_SIDE	getSide(const POINT3& point, scalar tolerance = scalar(0.0) ) const
		{
			assert( tolerance >= 0 );
			scalar distance = this->getDistance(point);
			if( distance < -tolerance )
				return PS_NEGATIVE;
			else if( distance > tolerance )
				return PS_POSITIVE;
			else
				return PS_ON;
		}

		/** @brief  */
		inline bool		isPointOnPlane(const POINT3& point, scalar tolerance = Math::LOW_EPSILON ) const
		{
			return this->getSide(point,tolerance) == PS_ON;
		}

		/** @brief  multi plane - box optimization */
		inline POS_SIDE	getSide(const POINT3& boxCenter, const Vector3& boxHalfSize, scalar tolerance = 0) const
		{
			scalar dist = this->getDistance(boxCenter);

			// get the maximal allows absolute distance for center-plane the distance
			scalar maxAbsDist = std::fabs(mNormal.x*boxHalfSize.x) + std::fabs(mNormal.y*boxHalfSize.y) + std::fabs(mNormal.z*boxHalfSize.z);

			if (dist+maxAbsDist < -tolerance )
				return PS_NEGATIVE;

			if (dist-maxAbsDist > tolerance )
				return PS_POSITIVE;

			return PS_BOTH;
		}

		/** @brief  */
		inline scalar		normalize()
		{
			scalar length = this->getNormal().getLength();

			if (length > Math::HIGH_EPSILON )
			{
				scalar fInvLength = 1.0f / length;
				mNormal *= fInvLength;
				mD *= fInvLength;
			}

			return length;
		}

		/** @brief  */
		inline bool					intersect(const Box3& box) const
		{
			return this->getSide(box) == PS_BOTH;
		}

		/** @brief  */
		BLADE_BASE_API Plane		operator*(const Matrix44& xform) const;

		/** @brief  */
		BLADE_BASE_API POS_SIDE		getSide(const Box3& box, scalar tolerance = scalar(0.0)) const;

		/** @brief  */
		BLADE_BASE_API Vector3		getProjectedVector(const Vector3& v) const;

		/** @brief  */
		BLADE_BASE_API bool			intersect(const Sphere& sphere) const;

	public:
		Vector3		mNormal;
		scalar		mD;
	};//class Plane
	
}//namespace Blade


#endif //__Blade_Plane_h__