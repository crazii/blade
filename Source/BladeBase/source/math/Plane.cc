/********************************************************************
	created:	2010/04/21
	filename: 	Plane.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Plane		Plane::operator*(const Matrix44& xform) const
	{
		Plane plane;
		Matrix44 itXform = xform.getInverse().getTranspose();
		Vector4 v = reinterpret_cast<const Vector4&>(*this);
		v *= itXform;
		plane.mNormal = static_cast<Vector3&>(v);
		plane.mD = v.w / plane.mNormal.normalize();
		return plane;
	}

	//////////////////////////////////////////////////////////////////////////
	POS_SIDE	Plane::getSide(const Box3& box, scalar tolerance/* = scalar(0.0) */) const
	{
		if( box.isNull() )
			return PS_NONE;
		else if( box.isInfinite() )
			return PS_BOTH;

		const POINT3 center = box.getCenter();
		const Vector3 half = box.getHalfSize();
		return this->getSide(center, half, tolerance);
	}

	//////////////////////////////////////////////////////////////////////////
	Vector3		Plane::getProjectedVector(const Vector3& v) const
	{
		Matrix33 xform;
		xform[0][0] = 1.0f - mNormal.x * mNormal.x;
		xform[1][0] = -mNormal.x * mNormal.y;
		xform[2][0] = -mNormal.x * mNormal.z;
		xform[0][1] = -mNormal.y * mNormal.x;
		xform[1][1] = 1.0f - mNormal.y * mNormal.y;
		xform[2][1] = -mNormal.y * mNormal.z;
		xform[0][2] = -mNormal.z * mNormal.x;
		xform[1][2] = -mNormal.z * mNormal.y;
		xform[2][2] = 1.0f - mNormal.z * mNormal.z;
		return v*xform;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Plane::intersect(const Sphere& sphere) const
	{
		return std::fabs( this->getDistance(sphere.getCenter()) ) <= sphere.getRadius();
	}
	
}//namespace Blade