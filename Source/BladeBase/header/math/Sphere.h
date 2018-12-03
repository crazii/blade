/********************************************************************
	created:	2010/04/21
	filename: 	Sphere.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Sphere_h__
#define __Blade_Sphere_h__
#include <math/Vector3.h>

namespace Blade
{
	class Plane;
	class Ray;
	class Box3;

	class Sphere
	{
	public:
		inline Sphere()
		{
			mCenter = Vector3::ZERO;
			mRadius = 1.0f;
		}

		inline Sphere(const Vector3& center,scalar radius)
		{
			mCenter = center;
			mRadius = radius;
		}

		/** @brief  */
		inline void	setCenter(const Vector3& center)
		{
			mCenter = center;
		}

		/** @brief  */
		inline void	setRadius(scalar radius)
		{
			mRadius = radius;
		}

		/** @brief  */
		inline Vector3	getCenter() const
		{
			return mCenter;
		}

		/** @brief  */
		inline scalar			getRadius() const
		{
			return mRadius;
		}

		/** @brief  */
		inline POS_VOL			intersect(const Vector3& point) const
		{
			scalar sql = (point - this->getCenter()).getSquaredLength();
			return (sql <= mRadius*mRadius) ? PV_INSIDE : PV_OUTSIDE;
		}

		/** @brief  */
		inline bool				contains(const Vector3& point) const
		{
			return this->intersect(point) == PV_INSIDE;
		}

		/** @brief  */
		BLADE_BASE_API POS_VOL	intersect(const Sphere& sphere) const;

		/** @brief  */
		BLADE_BASE_API POS_VOL	intersect(const Box3& box) const;

		/** @brief  */
		BLADE_BASE_API bool		intersect(const Plane& plane) const;

		/** @brief  */
		BLADE_BASE_API bool		intersect(const Ray& ray, scalar* d = NULL) const;

		/** @brief  */
		BLADE_BASE_API	bool	intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, POINT3* outPt = NULL) const;

	public:

		Vector3		mCenter;
		scalar		mRadius;
	};//class Sphere
	
}//namespace Blade


#endif //__Blade_Sphere_h__