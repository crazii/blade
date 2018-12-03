/********************************************************************
	created:	2010/04/21
	filename: 	Ray.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Ray_h__
#define __Blade_Ray_h__
#include <math/Vector3.h>
#include <math/BladeMath.h>

namespace Blade
{
	class Plane;
	class Sphere;
	class Box3;
	class Matrix44;

#define RAY_BOX_INTERSECT_BOOST 0

	class Ray
	{
	public:
		inline Ray()
		{
			mPoint = Vector3::ZERO;
			mVector = Vector3::UNIT_Z;
#if RAY_BOX_INTERSECT_BOOST
			mInvVector = Vector3::UNIT_ALL/mVector;

			mSign[0] = mVector.x < 0;
			mSign[1] = mVector.y < 0;
			mSign[2] = mVector.z < 0;
#endif
		}

		inline Ray(const Vector3& origin,const Vector3& dir)
		{
			mPoint = origin;
			mVector = dir;
#if RAY_BOX_INTERSECT_BOOST
			mInvVector = Vector3::UNIT_ALL/mVector;

			mSign[0] = mVector.x < 0;
			mSign[1] = mVector.y < 0;
			mSign[2] = mVector.z < 0;
#endif
		}

		/** @brief  */
		inline void	setOrigin(const Vector3& origin)
		{
			mPoint = origin;
		}

		/** @brief  */
		inline void setDirection(const Vector3& dir)
		{
			mVector = dir;
#if RAY_BOX_INTERSECT_BOOST
			mInvVector = Vector3::UNIT_ALL/mVector;

			mSign[0] = mVector.x < 0;
			mSign[1] = mVector.y < 0;
			mSign[2] = mVector.z < 0;
#endif
		}

		/** @brief  */
		inline const POINT3&	getOrigin() const
		{
			return mPoint;
		}

		/** @brief  */
		inline const Vector3&	getDirection() const
		{
			return mVector;
		}

		/** @brief  */
		inline POINT3			getRayPoint(scalar t) const
		{
			return this->getOrigin() + this->getDirection()*t;
		}

		/** @brief  */
		inline Ray&		operator*=(const Matrix44& transform)
		{
			*this = (*this) * transform;
			return *this;
		}

		/** @brief  */
		BLADE_BASE_API Ray		operator*(const Matrix44& transform) const;

		/** @brief  */
		BLADE_BASE_API bool		intersect(const Plane& plane, scalar& t) const;

		/** @brief  */
		BLADE_BASE_API bool		intersect(const Box3& aab, scalar& min_t, scalar& max_t) const;

		/** @brief  */
		BLADE_BASE_API bool		intersect(const Sphere& sphere, scalar& t) const;

		/** @brief triangle */
		/** @param [in] side : valid values are 
						PS_NEGATIVE,
						PS_POSITIVE,
						PS_BOTH
		*/
		BLADE_BASE_API bool		intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& t, POS_SIDE side = PS_POSITIVE, scalar limit = FLT_MAX) const;

		inline bool			intersect(const POINT3	triangle[3], scalar& t, POS_SIDE side = PS_POSITIVE, scalar limit = FLT_MAX) const
		{
			return this->intersect(triangle[0], triangle[1], triangle[2], t, side, limit);
		}

	protected:
		POINT3	mPoint;
		Vector3	mVector;
#if RAY_BOX_INTERSECT_BOOST
		//helper data
		Vector3	mInvVector;
		char	mSign[3];
#endif
	};//class Ray
	
}//namespace Blade


#endif //__Blade_Ray_h__