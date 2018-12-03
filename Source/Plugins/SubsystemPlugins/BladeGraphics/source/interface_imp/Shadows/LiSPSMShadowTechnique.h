/********************************************************************
	created:	2017/06/21
	filename: 	LiSPSMShadowTechnique.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LiSPSMShadowTechnique_h__
#define __Blade_LiSPSMShadowTechnique_h__
#include "IShadowTechnique.h"
#include <math/Matrix44.h>
#include <math/AxisAlignedBox.h>
#include "DefaultShadowTechnique.h"

//use general formula to calc nopt: this method gives overall better quality, but causes very low quality on some scenarios
#define CALC_NOPT_GENERAL 1

namespace Blade
{
	static const Matrix44 LSTransform(
		1, 0, 0, 0,		// x
		0, 0, 1, 0,		// y
		0, -1, 0, 0,	// z
		0, 0, 0, 1);	// w

	static const Matrix44 InvLSTransform(
		1, 0, 0, 0,		// x
		0, 0, -1, 0,	// y
		0, 1, 0, 0,		// z
		0, 0, 0, 1);	// w

	class LiSPSMShadowTechnique : public IShadowTechnique, public Allocatable
	{
	public:
		LiSPSMShadowTechnique() {}
		~LiSPSMShadowTechnique() {}

		/** @brief  */
		virtual void setupCulling(Camera* lightCam, const ICamera* sceneCam, const ILight* light, const PointList& pointsWS)
		{
			//apply default shadow mapping & culling frustum
			mDefault.setupCulling(lightCam, sceneCam, light, pointsWS);
		}

		/** @brief  */
		virtual void updateViewProjection(Camera* lightCam, Matrix44& depthMatrix, const ICamera* sceneCam, const ILight* light, const PointList& pointsWS)
		{
			mDefault.updateViewProjection(lightCam, depthMatrix, sceneCam, light, pointsWS);

			const Vector3& lightDir = light->getDirection();

			//body B points: expand points along light -direction to visible scene bounding
			//this makes Perspective Y (shadow Z) reaches min/max to the caster and doesn't need depth clamp.
			//(depth clamp now cause errors, figure out why): Y dir perspective depth may cause position large changes, including Y position,
			//even using fragment depth cannot prevent it.
			PointList points = pointsWS;

			const AABB& visibleSceneBounding = lightCam->getVisibleBounding();
			if (!visibleSceneBounding.isNull())
			{
#if 0
				//don't direct clip it, that will expand shadow X,Y besides Z, we only want Z (along -light dir) expanded 
				//so use -light ray to cast top points.
				IShadowTechnique::clipBounding(visibleSceneBounding, lightCam->getFrustum(), points);
#else

				//note: use ray cast may cause missing points on extreme cases. 
				//TODO: use tall(height expanded) view frustum to clip the bounding, and even to cull out invisible casters
				/*
				C
				+---------
				|
				+ B
				|\
				| \
				|  \
				when far plane point out side of scene(clipped), corner C will be missed because near plane ray point B doesn't reach to C, but light volumes covers C.
				*/
				AABB lightBounding;
				{
					PointList lightPoints;
					IShadowTechnique::clipBounding(visibleSceneBounding, lightCam->getFrustum(), lightPoints);
					for (size_t i = 0; i < lightPoints.size(); ++i)
						lightBounding.merge(lightPoints[i]);
				}
				for (size_t i = 0; i < pointsWS.size(); ++i)
				{
					const POINT3& p = pointsWS[i];
					Ray r(p, -lightDir);
					scalar t0, t1;
					if (r.intersect(lightBounding, t0, t1))
					{
						if (t1 > Math::HIGH_EPSILON)
							points.push_back(r.getRayPoint(t1));
						else if (t0 > Math::HIGH_EPSILON)
							points.push_back(r.getRayPoint(t0));
					}
				}
#endif
			}

			Matrix44 lightView = Matrix44::generateViewMatrix(sceneCam->getEyePosition(), sceneCam->getEyePosition() + lightDir, sceneCam->getLookAtDirection());
			Matrix44 lightProj = Matrix44::IDENTITY;

			/*
			Z+ (-light / -view)
			^
			|
			+------>Y-
			*/

			//y => z, z => y
			lightProj *= LSTransform;

			/*
			Y+
			^
			|
			+------>Z- (view)
			*/
			Vector3 dir = this->getLSProjectedViewDir(lightView * lightProj, POINT3::ZERO, sceneCam->getLookAtDirection());
			lightProj *= Matrix44::generateViewMatrix(POINT3::ZERO, POINT3::ZERO + dir);

			POINT3 nearestPointWS = this->getNearesetPoint(sceneCam->getViewMatrix(), pointsWS);			
			AABB boundingLS;
			{
				Matrix44 LS = lightView * lightProj;
				for (size_t i = 0; i < points.size(); ++i)
					boundingLS.merge(points[i] * LS);
			}

			assert(!isnan(lightProj[0][0]));
			Matrix44 LiSPSM;
			if (this->calculateLiSPSM(LiSPSM, sceneCam, lightView * lightProj, boundingLS, nearestPointWS))
			{
				lightProj *= LiSPSM;
				assert(!isnan(lightProj[0][0]));
			}
			else //default shadow mapping
				return;

			//transform to unit cube (off center orthographic projection)
			lightProj *= IShadowTechnique::unitCubeTransform(points, lightView * lightProj);

			lightProj *= InvLSTransform;

			//projection: flip z. on right handed system projection matrix(either perspective or orthographic) will map [viewZMax, viewZMin] to [-1,1]
			//unit cube transform [viewZMax, viewZMin] to [1,-1] but does not do the flip
			Matrix44 flipMatrix = Matrix44::IDENTITY;
			flipMatrix[2][2] = -1;
			lightProj *= flipMatrix;

#if BLADE_DEBUG && 0
			{
				AABB bounds;
				Matrix44 ls = lightView*lightProj;
				for (size_t i = 0; i < points.size(); ++i)
				{
					POINT3 p = points[i] * ls;
					assert(p.x >= -1 - Math::LOW_EPSILON && p.x <= 1 + Math::LOW_EPSILON 
						&& p.y >= -1 - Math::LOW_EPSILON && p.y <= 1 + Math::LOW_EPSILON 
						&& p.z >= -1 - Math::LOW_EPSILON && p.z <= 1 + Math::LOW_EPSILON);
					bounds.merge(p);
				}
				assert(bounds.getHalfSize().equal(Vector3::UNIT_ALL, Math::LOW_EPSILON));
				assert(bounds.getCenter().equal(Vector3::ZERO, Math::LOW_EPSILON));
			}
#endif

			//set shadow matrix
			lightCam->setViewMatrix(lightView, true);
			lightCam->setProjectionMatrix(lightProj, true);
		}

		/** @brief  */
		Vector3 getLSProjectedViewDir(const Matrix44& LS, const POINT3& eyeWS, const Vector3& eyeDirWS) const
		{
			// goal is to construct a view direction
			// because parallel lines are not parallel any more after perspective projection we have to transform
			// a ray to point us the viewing direction

			// plus the direction results in a second point
			const POINT3 lookatWS = eyeWS + eyeDirWS;

			// transformation into light space
			const POINT3 eyeLS = eyeWS * LS;
			const POINT3 lookatLS = lookatWS * LS;

			// calculate the projection direction, which is the subtraction of
			// b_ls from e_ls. The y component is set to 0 to project the view
			// direction into the shadow map plane.
			Vector3 projectionDir(lookatLS - eyeLS);
			projectionDir.y = 0;

			// deal with Y-only vectors
			return Math::Equal(projectionDir.getLength(), 0.0f, Math::LOW_EPSILON) ?
				Vector3::UNIT_Z : projectionDir.getNormalizedVector();
		}

		/** @brief  */
		bool		calculateLiSPSM(Matrix44& LiSPSM, const ICamera* cam, const Matrix44& LS, const AABB& boundingLS, const POINT3& nearestPointWS)
		{
			scalar Nopt = this->calculateNOpt(cam, nearestPointWS, LS, boundingLS);

			if (Nopt >= FLT_MAX || Nopt < 0)
				return false;	// apply default shadow mapping

			const POINT3 nearestLS = nearestPointWS * LS;
			const Vector3 centerNearLS(nearestLS.x, nearestLS.y, boundingLS.getMaxPoint().z);

			// calculate the projection center C which is n units behind the near plane of P
			// we look into the negative z direction so add n
			const Vector3 eyeLS(centerNearLS + Nopt * Vector3::UNIT_Z);

			// set up a transformation matrix to transform the light space to its new origin
			Matrix44 projectionCenter = Matrix44::generateTranslation(-eyeLS);

			// range from bMin to bMax; d = |B_z_far - B_z_near|
			scalar d = Math::Abs(boundingLS.getMaxPoint().z - boundingLS.getMinPoint().z);

			// set up the LiSPSM perspective transformation
			// build up frustum to map Proj onto the unit cube with (-1/-1/-1) and (+1/+1/+1)
			Matrix44 Proj = Matrix44::generatePerspectiveProjectionMatrix(2, 2, Nopt, Nopt + d);

			//generatePerspectiveProjectionMatrix will map [Nopt,Nopt+d] to [-1,1] (flipped)
			//but perspective Z will be Y axis in light space, which don't need flip, so revert flipping
			//this is optional because we don't care about Y in light space, only need it map to [-1,1]
			Matrix44 flipMatrix = Matrix44::IDENTITY;
			flipMatrix[2][2] = -1;

			LiSPSM = projectionCenter*Proj*flipMatrix;
			return true;
		}

		/** @brief  */
		POINT3 getNearesetPoint(const Matrix44& viewMatrix, const PointList& pointsWS)
		{
			size_t index = 0;
			POINT3 nearsetVS = pointsWS[0] * viewMatrix;

			for (size_t i = 1; i < pointsWS.size(); ++i)
			{
				POINT3 vp = pointsWS[i] * viewMatrix;
				assert(vp.z <= 0);
				if (nearsetVS.z + Math::HIGH_EPSILON < vp.z)
				{
					nearsetVS = vp;
					index = i;
				}
			}

			return pointsWS[index];
		}

		/** @brief  */
		scalar calculateNOpt(const ICamera* cam, const POINT3& eyeWS, const Matrix44& LS, const AABB& boundingLS)
		{
#if CALC_NOPT_GENERAL
			Matrix44 invLS = LS.getInverse();

			const Matrix44& viewMatrix = cam->getViewMatrix();

			// calculate z0LS
			const Vector3 z0LS = this->calculateZ0LS(eyeWS, cam->getLookAtDirection(), LS, boundingLS.getMaxPoint());

			// z1LS has the same x and y values as z0_ls and the minimum z values of bodyABB_ls
			const Vector3 z1LS = Vector3(z0LS.x, z0LS.y, boundingLS.getMinPoint().z);

			// world space
			const Vector3 z0WS = z0LS * invLS;
			const Vector3 z1WS = z1LS * invLS;

			// view space
			const Vector3 z0VS =  z0WS * viewMatrix;
			const Vector3 z1VS = z1WS * viewMatrix;

			const scalar z0 = z0VS.z;
			const scalar z1 = z1VS.z;

			// check if we have to do uniform shadow mapping
			if ((z0 < 0 && z1 > 0) ||
				(z1 < 0 && z0 > 0))
			{
				// apply default shadow mapping
				return FLT_MAX;
			}

			scalar d = Math::Abs(boundingLS.getMaxPoint().z - boundingLS.getMinPoint().z);
			return d / (std::sqrt(z1 / z0) - 1.0f);
#else
			BLADE_UNREFERENCED(LS);
			BLADE_UNREFERENCED(boundingLS);

			const Matrix44& viewMatrix = cam->getViewMatrix();
			const Vector3 eyeVS = eyeWS * viewMatrix;
			scalar n = cam->getNearClipDistance();
			scalar f = cam->getFarClipDistance();
			
			Matrix33 LSRot = static_cast<const Matrix33&>(LS);
			Vector3 eyeDirLS = cam->getLookAtDirection() * LSRot;
			Vector3 lightDirLS = Vector3::UNIT_Y * LSRot;
			scalar sinGamma = eyeDirLS.crossProduct(lightDirLS).getLength();
			if (sinGamma < Math::LOW_EPSILON)
				return FLT_MAX;
			scalar nOpt = (Math::Abs(eyeVS.z) + std::sqrt(n * f));
			return nOpt / sinGamma;
			//return (n + std::sqrt(n*(n + (f-n)*sinGamma))) / sinGamma;
#endif
		}

		/** @brief  */
		POINT3 calculateZ0LS(const Vector3& eyeWS, const Vector3& eyeDirWS, const Matrix44& LS, const POINT3& maxLS) const
		{
			//to calculate the parallel plane to the near plane through e we 
			//calculate the plane A with the three points
			Plane plane(eyeDirWS, eyeWS);
			//to transform plane A into lightSpace calculate transposed inverted lightSpace	
			//and transform the plane equation with it
			plane *= LS;
			//get the parameters of A from the plane equation n dot d = 0
			const scalar d = plane.getPlaneDistance();
			const Vector3& normal = plane.getNormal();
			//transform to light space
			const Vector3 eyeLS = eyeWS * LS;
			//z_0 has the x coordinate of e, the y coord of B.max() 
			//and the z coord of the plane intersection
			return POINT3(eyeLS.x, (d - normal.z*maxLS.z - normal.x*eyeLS.x) / normal.y, maxLS.z);
		}

		DefaultShadowTechnique	mDefault;
	};
	
}//namespace Blade

#endif//__Blade_LiSPSMShadowTechnique_h__