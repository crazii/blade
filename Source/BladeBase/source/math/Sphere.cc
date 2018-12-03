/********************************************************************
	created:	2010/04/21
	filename: 	Sphere.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Sphere.h>
#include <math/Plane.h>
#include <math/AxisAlignedBox.h>
#include <math/Ray.h>
#include "MathAlgorithm.h"

namespace Blade
{
	using namespace  Math;
	//////////////////////////////////////////////////////////////////////////
	POS_VOL			Sphere::intersect(const Sphere& sphere) const
	{
		scalar sql = (this->getCenter() - sphere.getCenter()).getSquaredLength();
		scalar diffr = this->getRadius() - sphere.getRadius();
		scalar sumr = this->getRadius() + sphere.getRadius();

		if (sql <= diffr*diffr)
		{
			if (diffr > 0)
				return PV_INSIDE;
			else
				return PV_INTERSECTED;
		}
		else if (sql <= sumr*sumr)
			return PV_INTERSECTED;
		else
			return PV_OUTSIDE;
	}

	//////////////////////////////////////////////////////////////////////////
	POS_VOL			Sphere::intersect(const Box3& box) const
	{
		//TODO: FIXME! orders now matters, intersect not interchangeable
		//http://web.archive.org/web/20100323053111/http://www.ics.uci.edu/~arvo/code/BoxSphereIntersect.c
		if (box.isNull())
			return PV_INSIDE;
		else if (box.isInfinite())
			return PV_INTERSECTED;

		const Vector3& center = this->getCenter();
		scalar radius = this->getRadius();

		const Vector3& min = box.getMinPoint();
		const Vector3& max = box.getMaxPoint();

		{
			//enclosing sphere of box
			Vector3 center2 = (min + max) * 0.5f;
			scalar radius2 = min.getDistance(max) * 0.5f;

			scalar sql = (center - center2).getSquaredLength();
			scalar diffr = radius - radius2;
			if (sql <= diffr*diffr && diffr > 0)
				return PV_INSIDE;
		}

		//Vector3 vradius = radius * Vector3::UNIT_ALL;
		Vector3 mindiff = center - min;
		Vector3 maxdiff = center - max;

		scalar d = 0;
		for (size_t i = 0; i < 3; ++i)
		{
			scalar diff = mindiff[i];
			if (diff < 0)
				d += diff*diff;

			diff = maxdiff[i];
			if (diff > 0)
				d += diff*diff;
		}
		//if sphere is inside box, treat it as intersected
		return d < radius*radius ? PV_INTERSECTED : PV_OUTSIDE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Sphere::intersect(const Plane& plane) const
	{
		return plane.intersect( *this );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Sphere::intersect(const Ray& ray, scalar* d/* = NULL*/) const
	{
		scalar t;
		scalar& r = (d != NULL) ? *d : t;
		return ray.intersect(*this, r);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			Sphere::intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, POINT3* outPt/* = NULL*/) const
	{
		//http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.49.9172 "ERIT - ACollection of Efficient and Reliable Intersection Tests"
		//or https://gist.github.com/yomotsu/d845f21e2e1eb49f647f
#if 0
		//move to sphere center
		const Vector3& center = this->getCenter();
		Vector3 v0 = p0 - center;
		Vector3 v1 = p1 - center;
		Vector3 v2 = p2 - center;
		Sphere sphere(Vector3::ZERO, this->getRadius());

		//plane test
		Plane p(v0, v1, v2);
		if (!sphere.intersect(p))
			return false;


		//triangle vertices test
		scalar sqrR = this->getRadius()*this->getRadius();
		scalar d0 = v0.getSquaredLength();
		scalar d1 = v1.getSquaredLength();
		scalar d2 = v2.getSquaredLength();
		scalar d01 = v0.dotProduct(v1);
		scalar d02 = v0.dotProduct(v2);
		scalar d12 = v1.dotProduct(v2);
		if (d0 > sqrR && d01 > d0 && d02 > d0
			|| d1 > sqrR && d01 > d1 && d12 > d1
			|| d2 > sqrR && d02 > d2 && d12 > d2)
			return false;
#else
		//plane test
		Plane p(p0, p1, p2);
		if (!this->intersect(p))
			return false;

		//edge test
		Vector3 edges[3] = 
		{
			p1 - p0,
			p2 - p1,
			p0 - p2,
		};
		Vector3 points[3] = { p0, p1, p2 };
		for (int i = 0; i < 3; ++i)
		{
			scalar len = edges[i].getLength();
			edges[i] /= len;

			Ray ray(points[i], edges[i]);
			scalar d;
			if (ray.intersect(*this, d) && d <= len)
			{
				if (outPt != NULL)
					*outPt = ray.getRayPoint(d);
				return true;
			}
		}

		//normal projection against triangle test
		POINT3 Q = p.getProjectedPoint(this->getCenter());
		if (Math::isPointInTriangle(p.getNormal(), p0, p1, p2, Q))
		{
			if (outPt != NULL)
				*outPt = Q;
			return true;
		}
		return false;
#endif
	}
	
}//namespace Blade