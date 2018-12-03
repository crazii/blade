/********************************************************************
	created:	2010/04/21
	filename: 	Ray.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "MathAlgorithm.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Ray				Ray::operator*(const Matrix44& transform) const
	{
		Vector3 origin = mPoint * transform;
		Vector3 dir = mVector * (Matrix33)transform;
		dir.normalize();
		return Ray(origin, dir);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Ray::intersect(const Plane& plane, scalar& t) const
	{
		scalar cos_theta = plane.getNormal().dotProduct(mVector);

		//parallel
		if ( std::fabs(cos_theta) < std::numeric_limits<scalar>::epsilon() )
		{
			return false;
		}
		else
		{
			scalar	distance = plane.getDistance( mPoint );
			t = -(distance/cos_theta);
			return t >= 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	struct BOX
	{
		POINT3 mPoints[2];
	};
	bool			Ray::intersect(const Box3& aab, scalar& min_t, scalar& max_t) const
	{
		// ref: http://www.cs.utah.edu/~awilliam/box/
		BOX box;
		box.mPoints[0] = aab.getMinPoint();
		box.mPoints[1] = aab.getMaxPoint();

#if !RAY_BOX_INTERSECT_BOOST
		Vector3 mInvVector = Vector3::UNIT_ALL / mVector;
		int mSign[3];
		mSign[0] = mVector.x < 0;
		mSign[1] = mVector.y < 0;
		mSign[2] = mVector.z < 0;
#endif

		scalar tymin,tymax,tzmin,tzmax;
		min_t = (box.mPoints[(int)mSign[0]].x - mPoint.x)* mInvVector.x;
		max_t = (box.mPoints[ 1 - (int)mSign[0] ].x - mPoint.x)* mInvVector.x;

		tymin = (box.mPoints[(int)mSign[1]].y - mPoint.y)* mInvVector.y;
		tymax = (box.mPoints[ 1 - (int)mSign[1] ].y - mPoint.y)* mInvVector.y;
		if( min_t > tymax || max_t < tymin )
			return false;
		if( tymin > min_t )
			min_t = tymin;
		if( tymax < max_t )
			max_t = tymax;

		tzmin = (box.mPoints[(int)mSign[2]].z - mPoint.z)*mInvVector.z;
		tzmax = (box.mPoints[ 1 - (int)mSign[2] ].z - mPoint.z)*mInvVector.z;
		if( min_t > tzmax || max_t < tzmin )
			return false;

		if( max_t > tzmax )
			max_t = tzmax;
		if (min_t < tzmin)
			min_t = tzmin;

		return min_t <= max_t && max_t >= 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Ray::intersect(const Sphere& sphere, scalar& t) const
	{
		Vector3 relativePoint = sphere.getCenter() - mPoint;
		scalar B = relativePoint.dotProduct(mVector);
		scalar C = relativePoint.getSquaredLength() - sphere.getRadius()*sphere.getRadius();
		scalar d = B*B - C;
		if( d < 0 )
			return false;
		else
		{
			scalar sqrtd = std::sqrt(d);
			t = B - sqrtd;
			scalar tfar = B + sqrtd;
			if( t < 0 && tfar > 0)
			{
				//inside
				t = scalar(0);
				return true;
			}
			return t >= 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Ray::intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& t, POS_SIDE side/* = PS_POSITIVE*/, scalar limit/* = FLT_MAX*/) const
	{
		//invalid side parameter
		if( side == PS_ON || side == PS_NONE )
		{
			assert(false);
			return false;
		}

		bool front = side == PS_POSITIVE || side == PS_BOTH;
		bool back = side == PS_NEGATIVE || side == PS_BOTH;

		Vector3 normal = (p1 - p0).crossProduct(p2- p0);
		normal.normalize();

		//line vector and normal must be normalized
		scalar cos_theta = mVector.dotProduct(normal);

		//check parallel
		if( cos_theta > Math::HIGH_EPSILON )
		{
			if( !back )
				return false;
		}
		else if( cos_theta < -Math::HIGH_EPSILON )
		{
			if( !front)
				return false;
		}
		else
			//parallel
			return false;

		Vector3 PA = p0 - mPoint;
		scalar dist = normal.dotProduct(PA)/cos_theta;

		//behind origin
		if(dist < 0 || dist > limit)
			return false;

		if (Math::isPointInTriangle(normal, p0, p1, p2, mPoint + mVector*dist))
		{
			t = dist;
			return true;
		}
		return false;
	}
	
}//namespace Blade