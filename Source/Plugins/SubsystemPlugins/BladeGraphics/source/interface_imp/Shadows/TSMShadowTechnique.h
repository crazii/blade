/********************************************************************
	created:	2017/6/30
	filename: 	TSMShadowTechnique.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TSMShadowTechnique_h__
#define __Blade_TSMShadowTechnique_h__
#include "DefaultShadowTechnique.h"
#include <math/Matrix44.h>

namespace Blade
{
	//calculate TSM in post projected space of light
	//when set to 0, light view space is used.
#define USE_PPS 1				//setting to 0 will not work properly
#define USE_80PERCENT_RULE 1	//this must be enabled. set to 0 only for debugging
#define REMAP_DEPTH 1

	//distance ratio for 80% rule. TODO: config for each slice
	const float F_RATIO = 0.5f;

	//note that we use Z as the Y in TSM
	static const Matrix44 Trapezoid2Box(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 1,
		0, 0, 1, 0);

	static const Matrix44 LSYtoZ(
		1, 0, 0, 0,		// x
		0, 0, 1, 0,		// y
		0, -1, 0, 0,	// z	either -1 or 1 is OK because we don't care about Y
		0, 0, 0, 1);	// w

	class TSMShadowTechnique : public IShadowTechnique, public Allocatable
	{
	public:
		TSMShadowTechnique()	{}
		~TSMShadowTechnique()	{}

		/** @brief  */
		virtual void setupCulling(Camera* lightCam, const ICamera* sceneCam, const ILight* light, const PointList& pointsWS)
		{
			//apply default shadow mapping & culling frustum
			mDefault.setupCulling(lightCam, sceneCam, light, pointsWS);
		}

		/** @brief  */
		virtual void updateViewProjection(Camera* lightCam, Matrix44& depthMatrix, const ICamera* sceneCam, const ILight* light, const PointList& pointsWS)
		{
			//need depth matrix for default shadow
			mDefault.updateViewProjection(lightCam, depthMatrix, sceneCam, light, pointsWS);

			const POINT3& eyePos = sceneCam->getEyePosition();
			const Vector3& eyeDir = sceneCam->getLookAtDirection();
			const Vector3 lightDir = lightCam->getLookAtDirection();

			if (Math::Equal(std::abs(lightDir.dotProduct(eyeDir)), 1.0f, 1e-2f))
				return;

			PointList points = pointsWS;
			const AABB& visibleSceneBounding = lightCam->getVisibleBounding();
			if (!visibleSceneBounding.isNull())
				IShadowTechnique::clipBounding(visibleSceneBounding, lightCam->getFrustum(), points);

			//use lookat dir that orthogonal to light dir, so that up dir should be exactly -lightDir, without changes. (the light dir must no change)
			//the lookat dir is -eyeDir projected on light projection plane, so that the first 2 steps of setup TSM is not needed. (translate to origin, rotate)
			Vector3 lookat = (-lightDir).crossProduct((eyeDir).crossProduct(-lightDir));
			assert(lookat.dotProduct(eyeDir) > 0);
			Matrix44 lightView = Matrix44::generateViewMatrix(eyePos, eyePos - lookat, -lightDir);	//use lookat as +Z for convenience

			//use tight frustum
			//note: for large scenes this in not necessary because visible boundingVS usually totally encloses camera frustum and
			//scene camera usually fully falls in light frustum.
			//we still want to do this because camera may look a little bit vertical and visible near/far plane may change.
			PointList frustumPoints;
			{
				frustumPoints.reserve(8);
				AABB boundingVS;
				for (size_t i = 0; i < pointsWS.size(); ++i)
					boundingVS.merge(pointsWS[i] * sceneCam->getViewMatrix());
				Camera tightSceneCamera;
				tightSceneCamera.copyFrom(*sceneCam, ICamera::CF_DEFAULT | ICamera::CF_GEOMETRY);
				tightSceneCamera.setNearClipDistance(-boundingVS.getMaxPoint().z);
				tightSceneCamera.setFarClipDistance(-boundingVS.getMinPoint().z);
				tightSceneCamera.ensureCullingData(true);
				tightSceneCamera.updateMatrix();
				for (int i = 0; i < Frustum::FCI_COUNT; ++i)
					frustumPoints.push_back(tightSceneCamera.getFrustum()->getFrustumCorner(Frustum::CORNER(i)));

				AABB frustumboundingLS;
				for (size_t i = 0; i < frustumPoints.size(); ++i)
					frustumboundingLS.merge(frustumPoints[i] * lightView);

				//adjust lightView pos so that all frustum points are in front of lightView origin
				if (frustumboundingLS.getMinPoint().z <= 1e-2f)
				{
					//lightView *= Matrix44::generateTranslation(POINT3(0, 0, -frustumboundingLS.getMinPoint().z + sceneCam->getNearClipDistance()));
					//note: event offset is applied to origin, the projected origin is not view origin, need find projected origin.
					//TODO: calculate new origin using 80% line
					return;
				}
			}

			AABB boundingLS;
			for (size_t i = 0; i < points.size(); ++i)
				boundingLS.merge(points[i] * lightView);

			//http://www.comp.nus.edu.sg/~tants/tsm/TSM_recipe.html
			Matrix44 trapezoid = Matrix44::IDENTITY;
#if USE_PPS
			trapezoid *= IShadowTechnique::unitCubeTransform(boundingLS);
#endif

#if 1
			scalar oriZ = boundingLS.getMinPoint().z;
#else
			scalar oriZ = 0;	//this will fix/improve situations when camera looks from outside of the scene
#endif

			//calculate origin in light view using 80% rule
			scalar eta = oriZ;
#if USE_80PERCENT_RULE
			{
				//http://www.comp.nus.edu.sg/~tants/tsm/EGSR_TSM_presentation.ppt
				/*
				perspective projection along Z+:
				|F+zn,1|	*	|(zn+zf)/(zn-zf)    -1| = | -z*(F+zn), -(F+zn)|
								|-2*zn*zf/(zn-zf)    0|

				zn = eta, zf= eta+lambda
				(F+eta,1) * projection = (-z*(F+eta), -(F+eta))
				(F+eta)*(eta*2+lambda)/(-lambda) - 2*eta*(eta+lambda)/(-lambda) = -(F+eta)*z
				*/

				scalar z = -1 + 2 * 0.8f;
				scalar lambda = boundingLS.getHalfSize().z * 2;
				//scalar F = boundingLS.getCenter().z - boundingLS.getMinPoint().z;	//TODO: config F
				scalar F = lambda * F_RATIO;
				eta = F*lambda*(z - 1) / (F * 2 - lambda*(z + 1));
			}
#endif

			//step 3. translate view point to origin
			POINT3 eyeLS = POINT3(0, 0, oriZ - eta)*trapezoid;
			assert(!USE_PPS || eyeLS.z <= 0);	//look along +Z (not -Z)
			Matrix44 T_2 = Matrix44::generateTranslation(-eyeLS);
			trapezoid *= T_2;

			//step 4. shearing
			//find t2,t3 on near plane
			scalar zNear = FLT_MAX, zFar = -FLT_MAX;
			POINT3 t2 = Vector3::UNIT_ALL*FLT_MAX, t3 = Vector3::NEGATIVE_UNIT_ALL*FLT_MAX;
			POINT3 ty2 = Vector3::UNIT_ALL*FLT_MAX, ty3 = Vector3::NEGATIVE_UNIT_ALL*FLT_MAX;
			{
				Matrix44 LS = lightView * trapezoid;
				POINT3 view = POINT3::ZERO;
				POINT3 np = POINT3(0, 0, boundingLS.getMinPoint().z)*trapezoid;
				POINT3 fp = POINT3(0, 0, boundingLS.getMaxPoint().z)*trapezoid;
				zNear = np.z - view.z;	//zNear = eta in LS
				zFar = fp.z;

				for (size_t i = 0; i < frustumPoints.size(); ++i)
				{
					POINT3 p = frustumPoints[i] * LS;
					p.normalize();
					if (p.x < t2.x)
						t2 = p;
					if (p.x > t3.x)
						t3 = p;
				}

				//note: don't use frustum to get near y - shadow casters may be outside of frustum
				for (int i = 0; i < AABB::BC_COUNT; ++i)
				{
					POINT3 p = boundingLS.getCorner(AABB::CORNER(i)) * trapezoid;
					p.normalize();

					//if (i >= AABB::AABC_BACK_LEFT_BOTTOM || i <= AABB::AABC_BACK_LEFT_TOP)
					//{
					//	if (p.x < t2.x)
					//		t2 = p;
					//	if (p.x > t3.x)
					//		t3 = p;
					//}

					if (p.y < ty2.y)
						ty2 = p;
					if (p.y > ty3.y)
						ty3 = p;
				}

				t2 *= zNear / t2.z;
				t3 *= zNear / t3.z;
				ty2 *= zNear / ty2.z;
				ty3 *= zNear / ty3.z;
			}

			Matrix44 xForm = Matrix44::IDENTITY;
#if 1
			Matrix44 H = Matrix44::IDENTITY;
			{
				POINT3 tx = (t2 + t3) / 2;
				POINT3 ty = (ty2 + ty3) / 2;
				H[2][0] = -tx.x / tx.z;
#if REMAP_DEPTH
				H[2][1] = -ty.y / ty.z;
#endif
			}
			xForm *= H;

			//step 5. scale the trapezoid: near z to 1, and x,y to [1,1]
			t3 *= H;
			ty3 *= H;
			assert( Math::Equal(t3.z, zNear, Math::LOW_EPSILON) );
			scalar zScale = 1 / t3.z;
			scalar xScale = 1 / t3.x;
#if REMAP_DEPTH
			scalar yScale = 1 / ty3.y;
#else
			scalar yScale = 1.0f;
#endif
			Matrix44 S_1 = Matrix44::generateScale(Vector3(xScale, yScale, zScale));
			xForm *= S_1;

			//step 6. map x-z to rectangle
			xForm *= Trapezoid2Box;
#endif

			//step 7. translate to origin
			POINT3 nearPoint = Vector3(0, 0, zNear)*xForm;
			POINT3 farPoint = Vector3(0, 0, zFar)*xForm;
			xForm *= Matrix44::generateTranslation(POINT3(0,0,-(nearPoint.z+farPoint.z)/2));

			//step 8. scale to cube
#if USE_PPS
			//note: at this point far z may be smaller than near z, this scaling will inverse the perspective z to correct dir, so that far > near
			xForm *= Matrix44::generateScale(Vector3(1, 1, 2 / (farPoint.z - nearPoint.z)));
#else
			scalar w = boundingLS.getHalfSize().x;
			scalar h = boundingLS.getHalfSize().y;
			scalar z = boundingLS.getHalfSize().z;

			//xForm *= Matrix44::generateTranslation(boundingLS.getCenter());
			xForm *= Matrix44::generateScale(Vector3(w, h, z*2 / (farPoint.z - nearPoint.z)));
			xForm *= IShadowTechnique::unitCubeTransform(boundingLS);
#endif
			trapezoid *= xForm;

			//Y+(-lightDir) to Z+
			trapezoid *= LSYtoZ;

			//flip z. on right handed system projection matrix(either perspective or orthographic) will map [viewZMax, viewZMin] to [-1,1]
			//unit cube transform [viewZMax, viewZMin] to [1,-1] but does not do the flip
			Matrix44 flipMatrix = Matrix44::IDENTITY;
			flipMatrix[2][2] = -1;
			trapezoid *= flipMatrix;

#if BLADE_DEBUG && 0
			POINT3 p0 = boundingLS.getMinPoint()*trapezoid;
			POINT3 p1 = boundingLS.getMaxPoint()*trapezoid;
#endif

			lightCam->setViewMatrix(lightView, true);
			lightCam->setProjectionMatrix(trapezoid, true);
		}
		DefaultShadowTechnique	mDefault;
	};

	
}//namespace Blade

#endif//__Blade_TSMShadowTechnique_h__