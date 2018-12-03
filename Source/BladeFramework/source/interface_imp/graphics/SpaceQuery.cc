/********************************************************************
	created:	2011/05/19
	filename: 	SpaceQueryData.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/SpaceQuery.h>
#include <interface/public/graphics/IGraphicsCamera.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/graphics/GraphicsUtility.h>

namespace Blade
{
	/************************************************************************/
	/* SpaceQuery                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				SpaceQuery::sort()
	{
		if(mResult != NULL)
			std::sort(mResult->begin(), mResult->end());
	}

	/************************************************************************/
	/* RaycastQuery                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	RaycastQuery::intersect(const GraphicsGeometry& geom, scalar& distance, scalar limit/* = FLT_MAX*/) const
	{
		scalar d = limit;
		bool ret = GraphicsUtility::rayIntersectTriangles(mRay, geom, d, mSide);
		if (ret)
			distance = d;
		return ret;
	}

	/************************************************************************/
	/* AABBQuery                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	AABBQuery::intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit/* = FLT_MAX*/) const
	{
		bool result = false;

		//usually when testing a triangle happens, 
		//it means the triangle's object bounding(i.e. bounding of sub mesh in model, or bounding of patch/block/chunk in terrain) is tested already
		//so we can drop precision to gain performance. this will get extra triangles 
		if (!this->isPrecise())
		{
			AABB b;
			b.merge(p0);
			b.merge(p1);
			b.merge(p2);

			if (mBounding.intersect(b))
			{
				Plane p(p0, p1, p2);
				result = mBounding.intersect(p);
			}
		}
		else	//precise test
			result = mBounding.intersect(p0, p1, p2);

		if (result)
		{
			//dir & distance are approximate value.
			Vector3 center = mBounding.getCenter();
			Vector3 dir0 = p0 - center;
			Vector3 dir1 = p1 - center;
			Vector3 dir2 = p2 - center;
			scalar dist0 = dir0.getSquaredLength();
			scalar dist1 = dir1.getSquaredLength();
			scalar dist2 = dir2.getSquaredLength();

			Vector3 dir = dir0;
			distance = std::min(limit*limit, dist0);
			if (dist1 < distance)
				dir = dir1;
			if (dist2 < distance)
				dir = dir2;

			distance = dir.getLength();
			mDirection = dir / distance;
		}
		return result;
	}

	/************************************************************************/
	/* SphereQuery                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	SphereQuery::intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2, scalar& distance, scalar limit/* = FLT_MAX*/) const
	{
		bool result = false;
		if (!this->isPrecise())
		{
			Plane p(p0, p1, p2);
			if (mSphere.intersect(p))
			{
				AABB b;
				b.merge(p0);
				b.merge(p1);
				b.merge(p2);
				result = mSphere.intersect(b) != PV_OUTSIDE;
			}

			if (result)
			{
				Vector3 center = mSphere.getCenter();
				Vector3 dir0 = p0 - center;
				Vector3 dir1 = p1 - center;
				Vector3 dir2 = p2 - center;

				scalar dist0 = dir0.getSquaredLength();
				scalar dist1 = dir1.getSquaredLength();
				scalar dist2 = dir2.getSquaredLength();

				Vector3 dir = dir0;
				distance = std::min(limit*limit, dist0);
				if (dist1 < distance)
					dir = dir1;
				if (dist2 < distance)
					dir = dir2;

				distance = dir.getLength();
				mDirection = dir / distance;
			}
		}
		else	//precise test
		{
			Vector3 contact = Vector3::ZERO;
			result = mSphere.intersect(p0, p1, p2, &contact);
			if (result)
			{
				mDirection = contact - mSphere.getCenter();
				distance = mDirection.getLength();
				mDirection /= distance;
			}
		}
		return result;
	}

	/************************************************************************/
	/* ScreenQuery                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	POS_VOL	ScreenQuery::intersect(const AABB& aab, scalar& distance) const
	{
		if (aab.isNull())
			return PV_INSIDE;
		else if (aab.isInfinite())
			return PV_INTERSECTED;

		if (mFilter == TF_2D)
		{
			//3d bounding: always pass
			if (aab.getHalfSize().z != 0)
				return PV_INTERSECTED;

			if (mView->getPixelWidth() == 0 || mView->getPixelHeight() == 0)
				return PV_OUTSIDE;

			Vector3 halfSize3 = aab.getHalfSize();
			POINT3 pos = mCamera->getProjectedPosition(aab.getCenter());
			if (pos.z <= -1.0f)
				return PV_OUTSIDE;

			//TODO: need pixel size
			scalar halfWidth = halfSize3.x / mView->getPixelWidth();
			scalar halfHeight = halfSize3.y / mView->getPixelHeight();
			Vector2 halfSize(halfWidth, halfHeight);
			Vector2 center(pos.x, pos.y);
			Box2 rect(center - halfSize, center + halfSize);
			distance = pos.z;

			return mRect.intersect(rect);
		}

		//3D: use view space bounding
		AABB viewAAB = aab * mCamera->getViewMatrix();
		Vector3 center3 = viewAAB.getCenter();
		Vector3 halfSize3 = viewAAB.getHalfSize();
		//project far point to clip
		POINT3 pos = mCamera->getProjectedPosition(center3 + halfSize3.z * Vector3::NEGATIVE_UNIT_Z);
		if (pos.z <= -1.0f)
			return PV_OUTSIDE;
		//use center
		pos = mCamera->getProjectedPosition(center3);

		//project view bounding size to screen
		if (mCamera->getProjectionType() == PT_PERSPECTIVE)
		{
			scalar dz = std::fabs(center3.z - mCamera->getEyePosition().z);
			if (Math::Equal(dz, 0.0f, Math::HIGH_EPSILON))
				dz = Math::LOW_EPSILON;
			halfSize3 /= dz;
		}

		Vector2 halfSize(halfSize3.x, halfSize3.y);
		Vector2 center(pos.x, pos.y);
		Box2 rect(center - halfSize, center + halfSize);
		return mRect.intersect(rect);
	}

}//namespace Blade
