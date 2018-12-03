/********************************************************************
	created:	2010/04/21
	filename: 	Box3.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/AxisAlignedBox.h>

namespace Blade
{
	const Box3		Box3::EMPTY		= Box3(BT_NULL);
	const Box3		Box3::INFINITE	= Box3(BT_INFINITE);
	const Box3		Box3::UNIT		= Box3(Vector3(-0.5, -0.5, -0.5), Vector3(0.5, 0.5, 0.5));

	const Box3::CORNER	Box3::LINES[12][2] =
	{
		Box3::BC_BACK_LEFT_BOTTOM, Box3::BC_BACK_RIGHT_BOTTOM,
		Box3::BC_BACK_LEFT_BOTTOM, Box3::BC_BACK_LEFT_TOP,
		Box3::BC_BACK_RIGHT_BOTTOM, Box3::BC_BACK_RIGHT_TOP,
		Box3::BC_BACK_RIGHT_TOP, Box3::BC_BACK_LEFT_TOP,

		Box3::BC_FRONT_LEFT_BOTTOM, Box3::BC_FRONT_RIGHT_BOTTOM,
		Box3::BC_FRONT_LEFT_BOTTOM, Box3::BC_FRONT_LEFT_TOP,
		Box3::BC_FRONT_RIGHT_BOTTOM, Box3::BC_FRONT_RIGHT_TOP,
		Box3::BC_FRONT_RIGHT_TOP, Box3::BC_FRONT_LEFT_TOP,

		Box3::BC_BACK_LEFT_BOTTOM, Box3::BC_FRONT_LEFT_BOTTOM,
		Box3::BC_BACK_LEFT_TOP, Box3::BC_FRONT_LEFT_TOP,
		Box3::BC_BACK_RIGHT_BOTTOM, Box3::BC_FRONT_RIGHT_BOTTOM,
		Box3::BC_BACK_RIGHT_TOP, Box3::BC_FRONT_RIGHT_TOP,
	};

	//////////////////////////////////////////////////////////////////////////
	Box3::Box3(const Sphere& sphere)
	{
		mType = BT_FINITE;
		POINT3 center = sphere.getCenter();
		scalar radius = sphere.getRadius();
		mMinPoint = center - Vector3(radius, radius, radius);
		mMaxPoint = center + Vector3(radius, radius, radius);
	}

	//////////////////////////////////////////////////////////////////////////
	Box3&					Box3::operator*=(const Matrix44& mtransform)
	{
		if( mType != BT_FINITE )
			return *this;

		if( mtransform.isAffine() )
		{
			Vector4 xa = mtransform[0] * mMinPoint.x;
			Vector4 xb = mtransform[0] * mMaxPoint.x;

			Vector4 ya = mtransform[1] * mMinPoint.y;
			Vector4 yb = mtransform[1] * mMaxPoint.y;

			Vector4 za = mtransform[2] * mMinPoint.z;
			Vector4 zb = mtransform[2] * mMaxPoint.z;

			mMinPoint = Vector3::getMinVector(xa, xb) + Vector3::getMinVector(ya, yb) + Vector3::getMinVector(za, zb) + mtransform[3];
			mMaxPoint = Vector3::getMaxVector(xa, xb) + Vector3::getMaxVector(ya, yb) + Vector3::getMaxVector(za, zb) + mtransform[3];
			return *this;
		}
		else
		{
			/* @remark re-calculate the 8 corners after transform  */
			Vector3 oldMin, oldMax, currentCorner;


			oldMin = mMinPoint;
			oldMax = mMaxPoint;
			// reset
			this->setNull();
			// We sequentially compute the corners in the following order :
			// 0, 6, 5, 1, 2, 4 ,7 , 3
			// This sequence allows us to only change one member at a time to get at all corners.

			// For each one, we transform it using the matrix
			// Which gives the resulting point and merge the resulting point.

			// First corner 
			// min min min
			currentCorner = oldMin;
			this->merge( currentCorner*mtransform );

			// min,min,max
			currentCorner.z = oldMax.z;
			this->merge( currentCorner*mtransform );

			// min max max
			currentCorner.y = oldMax.y;
			this->merge( currentCorner*mtransform );

			// min max min
			currentCorner.z = oldMin.z;
			this->merge( currentCorner*mtransform );

			// max max min
			currentCorner.x = oldMax.x;
			this->merge( currentCorner*mtransform );

			// max max max
			currentCorner.z = oldMax.z;
			this->merge( currentCorner*mtransform );

			// max min max
			currentCorner.y = oldMin.y;
			this->merge( currentCorner*mtransform );

			// max min min
			currentCorner.z = oldMin.z;
			this->merge( currentCorner*mtransform );

			return *this;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Box3					Box3::operator*(const Matrix44& mtransform) const
	{
		if( mType != BT_FINITE )
			return *this;	//copy this

		Box3 ret(BT_NULL);

		if( mtransform.isAffine() )
		{
			Vector3 center	= this->getCenter();
			Vector3 halfSize= this->getHalfSize();

			center *= mtransform;
			Vector3 newHalfSize(
				::fabs(mtransform[0][0]) * halfSize.x + ::fabs(mtransform[0][1]) * halfSize.y + ::fabs(mtransform[0][2]) * halfSize.z, 
				::fabs(mtransform[1][0]) * halfSize.x + ::fabs(mtransform[1][1]) * halfSize.y + ::fabs(mtransform[1][2]) * halfSize.z,
				::fabs(mtransform[2][0]) * halfSize.x + ::fabs(mtransform[2][1]) * halfSize.y + ::fabs(mtransform[2][2]) * halfSize.z);

			ret.set(center - newHalfSize, center + newHalfSize);
			return ret;
		}
		else
		{
			/* @remark re-calculate the 8 corners after transform  */
			Vector3 oldMin, oldMax, currentCorner;


			oldMin = mMinPoint;
			oldMax = mMaxPoint;
			// min min min
			currentCorner = oldMin;
			ret.merge( currentCorner*mtransform );

			// min,min,max
			currentCorner.z = oldMax.z;
			ret.merge( currentCorner*mtransform );

			// min max max
			currentCorner.y = oldMax.y;
			ret.merge( currentCorner*mtransform );

			// min max min
			currentCorner.z = oldMin.z;
			ret.merge( currentCorner*mtransform );

			// max max min
			currentCorner.x = oldMax.x;
			ret.merge( currentCorner*mtransform );

			// max max max
			currentCorner.z = oldMax.z;
			ret.merge( currentCorner*mtransform );

			// max min max
			currentCorner.y = oldMin.y;
			ret.merge( currentCorner*mtransform );

			// max min min
			currentCorner.z = oldMin.z;
			ret.merge( currentCorner*mtransform );
			return ret;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void							Box3::scale(const Vector3& vscale)
	{
		if (mType != BT_FINITE)
			return;
		Matrix44 xform = Matrix44::generateScale(vscale);
		*this *= xform;
	}

	//////////////////////////////////////////////////////////////////////////
	bool							Box3::intersect(const Plane& plane) const
	{
		return plane.intersect(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	POS_VOL							Box3::intersect(const Sphere& sphere) const
	{
		if( this->isNull() )
			return PV_OUTSIDE;
		else if( this->isInfinite() )
			return PV_INSIDE;

		const Vector3& center = sphere.getCenter();
		scalar radius = sphere.getRadius();

		const Vector3& min = this->getMinPoint();
		const Vector3& max = this->getMaxPoint();

		Vector3 vradius = radius * Vector3::UNIT_ALL;
		Vector3 mindiff = center - min;
		Vector3 maxdiff = center - max;
		//if (center - vradius >= min && center + vradius <= max)
		if(mindiff >= vradius && maxdiff <= -vradius)
			return PV_INSIDE;

		scalar d = 0;
		for( size_t i = 0; i < 3 ; ++i)
		{
			scalar diff = mindiff[i];
			if( diff < 0 )
				d += diff*diff;

			diff = maxdiff[i];
			if( diff > 0 )
				d += diff*diff;
		}
		return d < radius*radius ? PV_INTERSECTED : PV_OUTSIDE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					Box3::intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2) const
	{
		//http://fileadmin.cs.lth.se/cs/personal/tomas_akenine-moller/code/tribox3.txt
		//https://gist.github.com/yomotsu/d845f21e2e1eb49f647f
		//https://stackoverflow.com/questions/17458562/efficient-aabb-triangle-intersection-in-c-sharp
		//they are all of the same idea, as noted in the first link
		/*    use separating axis theorem to test overlap between triangle and box */
		/*    need to test for overlap in these directions: */
		/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
		/*       we do not even need to test these) */
		/*    2) normal of the triangle */
		/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
		/*       this gives 3x3=9 more tests */

		//1. X,Y,Z as separate axis, use bounding directly
		{
			Box3 trib;
			trib.merge(p0);
			trib.merge(p1);
			trib.merge(p2);
			if (this->intersect(trib) == PV_OUTSIDE)
				return false;
		}

		POINT3 center = this->getCenter();
		Vector3 halfSize = this->getHalfSize();
		//3. triangle 3 edges cross x,y,z as separating axis
		{

			//move triangle to box origin for convenience
			POINT3 v0 = p0 - center;
			POINT3 v1 = p1 - center;
			POINT3 v2 = p2 - center;

			//edges
			Vector3 e0 = v1 - v0;
			Vector3 e1 = v2 - v1;
			Vector3 e2 = v0 - v2;

			//edges cross X axis (1, 0, 0)
			{
				//pre-calculate cross result since X,Y,Z axes are constant
				Vector2 crossProducts[3] =	//(0, -z, y)
				{
					Vector2(-e0.z, e0.y), Vector2(-e1.z, e1.y), Vector2(-e2.z, e2.y),
				};

				//use 2d dot product since x is 0
				Vector2 v02 = Vector2(v0.y, v0.z);
				Vector2 v12 = Vector2(v1.y, v1.z);
				Vector2 v22 = Vector2(v2.y, v2.z);
				Vector2 half2 = Vector2(halfSize.y, halfSize.z);

				for (int i = 0; i < 3; ++i)
				{
					scalar d0 = v02.dotProduct(crossProducts[i]);
					scalar d1 = v12.dotProduct(crossProducts[i]);
					scalar d2 = v22.dotProduct(crossProducts[i]);
					scalar d = half2.dotProduct(crossProducts[i].getAbs());
					scalar max = std::max(std::max(d0, d1), d2);
					scalar min = std::min(std::min(d0, d1), d2);
					if (d < std::max(-max, min))
						return false;
				}
			}

			//edges cross Y axis (0, 1, 0)
			{
				Vector2 crossProducts[3] =	//(z, 0, -x)
				{
					Vector2(e0.z, -e0.x), Vector2(e1.z, -e1.x), Vector2(e2.z, -e2.x),
				};

				Vector2 v02 = Vector2(v0.x, v0.z);
				Vector2 v12 = Vector2(v1.x, v1.z);
				Vector2 v22 = Vector2(v2.x, v2.z);
				Vector2 half2 = Vector2(halfSize.x, halfSize.z);

				for (int i = 0; i < 3; ++i)
				{
					scalar d0 = v02.dotProduct(crossProducts[i]);
					scalar d1 = v12.dotProduct(crossProducts[i]);
					scalar d2 = v22.dotProduct(crossProducts[i]);
					scalar d = half2.dotProduct(crossProducts[i].getAbs());
					scalar max = std::max(std::max(d0, d1), d2);
					scalar min = std::min(std::min(d0, d1), d2);
					if (d < std::max(-max, min))
						return false;
				}
			}

			//edges cross Z axis (0, 0, 1)
			{
				Vector2 crossProducts[3] = //(-y, x, 0)
				{
					Vector2(-e0.y, e0.x), Vector2(-e1.y, e1.x), Vector2(-e2.y, e2.x),
				};

				Vector2 v02 = Vector2(v0.x, v0.y);
				Vector2 v12 = Vector2(v1.x, v1.y);
				Vector2 v22 = Vector2(v2.x, v2.y);
				Vector2 half2 = Vector2(halfSize.x, halfSize.y);

				for (int i = 0; i < 3; ++i)
				{
					scalar d0 = v02.dotProduct(crossProducts[i]);
					scalar d1 = v12.dotProduct(crossProducts[i]);
					scalar d2 = v22.dotProduct(crossProducts[i]);
					scalar d = half2.dotProduct(crossProducts[i].getAbs());
					scalar max = std::max(std::max(d0, d1), d2);
					scalar min = std::min(std::min(d0, d1), d2);
					if (d < std::max(-max, min))
						return false;
				}
			}
		}

		//2. triangle normal as separate axis: use AABB-plane directly
		{
			Plane p(p0, p1, p2);
			if (p.getSide(center, halfSize) != PS_BOTH)
				return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	Box3					Box3::getIntersection(const Box3& box) const
	{
		//special types check
		if (this->isNull() || box.isNull())
		{
			return Box3::EMPTY;
		}
		else if (this->isInfinite())
		{
			return box;
		}
		else if (box.isInfinite())
		{
			return *this;
		}

		//two finite boxes

		Vector3 itsMin = Vector3::getMaxVector(mMinPoint,box.mMinPoint);
		Vector3 itsMax = Vector3::getMinVector(mMaxPoint,box.mMaxPoint);

		if( itsMin <= itsMax )
		{
			return Box3(itsMin, itsMax);
		}
		else
			return Box3::EMPTY;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool				Box3::getAllCorners(POINT3* corners) const
	{
		if (corners == NULL)
		{
			assert(false);
			return false;
		}

		corners[0] = mMinPoint;
		corners[1].x = mMaxPoint.x; corners[1].y = mMinPoint.y; corners[1].z = mMinPoint.z;
		corners[2].x = mMaxPoint.x; corners[2].y = mMaxPoint.y; corners[2].z = mMinPoint.z;
		corners[3].x = mMinPoint.x; corners[3].y = mMaxPoint.y; corners[3].z = mMinPoint.z;

		corners[4] = mMaxPoint;
		corners[5].x = mMinPoint.x; corners[5].y = mMaxPoint.y; corners[5].z = mMaxPoint.z;
		corners[6].x = mMinPoint.x; corners[6].y = mMinPoint.y; corners[6].z = mMaxPoint.z;
		corners[7].x = mMaxPoint.x; corners[7].y = mMinPoint.y; corners[7].z = mMaxPoint.z;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	POINT3							Box3::getCorner(CORNER corner_index) const
	{
		switch (corner_index)
		{
		case BC_BACK_LEFT_BOTTOM:
			return mMinPoint;
		case BC_BACK_RIGHT_BOTTOM:
			return Vector3(mMaxPoint.x, mMinPoint.y, mMinPoint.z);
		case BC_BACK_RIGHT_TOP:
			return Vector3(mMaxPoint.x, mMaxPoint.y, mMinPoint.z);
		case BC_BACK_LEFT_TOP:
			return Vector3(mMinPoint.x, mMaxPoint.y, mMinPoint.z);
		case BC_FRONT_RIGHT_TOP:
			return mMaxPoint;
		case BC_FRONT_LEFT_TOP:
			return Vector3(mMinPoint.x, mMaxPoint.y, mMaxPoint.z);
		case BC_FRONT_LEFT_BOTTOM:
			return Vector3(mMinPoint.x, mMinPoint.y, mMaxPoint.z);
		case BC_FRONT_RIGHT_BOTTOM:
			return Vector3(mMaxPoint.x, mMinPoint.y, mMaxPoint.z);
		default:
			assert(false);
			return Vector3::ZERO;
		}
	}

}//namespace Blade