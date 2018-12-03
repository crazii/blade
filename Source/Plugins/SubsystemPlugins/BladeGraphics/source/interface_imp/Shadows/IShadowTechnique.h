/********************************************************************
	created:	2017/06/21
	filename: 	ShadowTechinque.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShadowTechinque_h__
#define __Blade_ShadowTechinque_h__
#include <interface/ILight.h>
#include "../../Camera.h"

namespace Blade
{
	class IShadowTechnique
	{
	public:
		typedef TempVector<POINT3> PointList;
		static const int SHADOW_CULLING_DISTANCE = 2000;
		static const int SHADOW_ORTHO_NEAR = 0;

	public:
		virtual ~IShadowTechnique() {}

		/** @brief set culling matrix/frustum */
		virtual void setupCulling(Camera* lightCam, const ICamera* sceneCam, const ILight* light, const PointList& pointsWS) = 0;

		/** @brief update view/projection after culling */
		virtual void updateViewProjection(Camera* lightCam, Matrix44& depthMatrix, const ICamera* sceneCam,const ILight* light, const PointList& pointsWS) = 0;

		/** @brief  */
		static void clipBounding(const AABB& bounding, const Frustum* frustum, PointList& outPoints)
		{
			//clip bounds to convex by current frustum
			bool contained[8] = { false, };
			bool visible[8] = { false };
			POINT3 corners[AABB::BC_COUNT];
			bounding.getAllCorners(corners);

			for (size_t i = 0; i < 12; ++i)
			{
				{
					Frustum::CORNER fc0 = Frustum::LINES[i][0];
					Frustum::CORNER fc1 = Frustum::LINES[i][1];
					const POINT3& p0 = reinterpret_cast<const POINT3&>(frustum->getFrustumCorner(fc0));
					const POINT3& p1 = reinterpret_cast<const POINT3&>(frustum->getFrustumCorner(fc1));

					bool v0 = bounding.contains(p0);
					bool v1 = bounding.contains(p1);
					if (v0 && !contained[fc0])
					{
						contained[fc0] = true;
						outPoints.push_back(p0);
					}
					if (v1 && !contained[fc1])
					{
						contained[fc1] = true;
						outPoints.push_back(p1);
					}

					if (!(v0 && v1) )
					{
						Vector3 v = p1 - p0;
						scalar d = v.normalize();
						Ray r(p0, v);

						scalar t0 = 0, t1 = 0;
						if (r.intersect(bounding, t0, t1))
						{
							assert(t0 <= t1);
							if (t0 >= 0 && t0 <= d)
								outPoints.push_back(r.getRayPoint(t0));
							if (t1 >= 0 && t1 <= d)
								outPoints.push_back(r.getRayPoint(t1));
						}
					}
				}

				{
					AABB::CORNER c0 = AABB::LINES[i][0];
					AABB::CORNER c1 = AABB::LINES[i][1];
					POINT3 p0 = corners[c0];
					POINT3 p1 = corners[c1];
					bool v0 = frustum->isVisible(p0);
					bool v1 = frustum->isVisible(p1);
					if (v0 && !visible[c0])
					{
						visible[c0] = true;
						outPoints.push_back(p0);
					}
					if (v1 && !visible[c1])
					{
						visible[c1] = true;
						outPoints.push_back(p1);
					}

					if (!(v0 && v1))
					{
						scalar d = (p1 - p0).getLength();
						Ray r(p0, (p1 - p0) / d);
						//Ray r2(p1, (p0 - p1) / d);

						for (int j = 0; j < Frustum::FPI_COUNT; ++j)
						{
							scalar t0 = -1;
							if (r.intersect(frustum->getFrustumPlane(Frustum::PLANE(j)), t0) && t0 + Math::HIGH_EPSILON < d)
							{
								POINT3 p = r.getRayPoint(t0);
								if(frustum->isVisible(p))
									outPoints.push_back(p);
							}
							///current ray-plane intersection doesn't count in plane facing
							//if (r2.intersect(frustum->getFrustumPlane(Frustum::PLANE(j)), t0) && t0 < d)
							//	outPoints.push_back(r2.getRayPoint(t0));
						}
					}
				}
			}
		}


		/** @brief  */
		static inline Matrix44	unitCubeTransform(const AABB& bounds)
		{
			const Vector3 unitScale = Vector3::UNIT_ALL / bounds.getHalfSize();
			const Vector3 translate = -bounds.getCenter()*unitScale;
			Matrix44 unitCube = Matrix44::IDENTITY;
			unitCube.setScale(unitScale);
			unitCube.setTranslation(translate);
			return unitCube;
		}

		/** @brief  */
		static inline Matrix44	unitCubeTransform(const PointList& points, const Matrix44& LS)
		{
			AABB bounds;
			for (size_t i = 0; i < points.size(); ++i)
				bounds.merge(points[i] * LS);
			return unitCubeTransform(bounds);
		}
	};
	
}//namespace Blade

#endif//__Blade_ShadowTechinque_h__