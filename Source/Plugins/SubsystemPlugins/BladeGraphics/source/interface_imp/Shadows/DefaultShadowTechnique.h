/********************************************************************
	created:	2017/06/21
	filename: 	DefaultShadowTechnique.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultShadowTechnique_h__
#define __Blade_DefaultShadowTechnique_h__
#include "IShadowTechnique.h"
#include <math/Matrix44.h>
#include <math/AxisAlignedBox.h>

namespace Blade
{

	class DefaultShadowTechnique : public IShadowTechnique, public Allocatable
	{
	public:
		DefaultShadowTechnique() {}
		~DefaultShadowTechnique() {}

		/** @brief  */
		virtual void setupCulling(Camera* lightCam, const ICamera* sceneCam, const ILight* light, const PointList& pointsWS)
		{
			const Vector3& dir = light->getDirection();
			//position doesn't matter since it used to calc bounding range, which is relative value
			Matrix44 lightView = Matrix44::generateViewMatrix(POINT3::ZERO, POINT3::ZERO + dir /*make point explict, not a dir*/, sceneCam->getLookAtDirection());

			AABB boundingLS;
			for (size_t j = 0; j < pointsWS.size(); ++j)
				boundingLS.merge(pointsWS[j] * lightView);

			scalar w = boundingLS.getHalfSize().x * 2;
			scalar h = boundingLS.getHalfSize().y * 2;
			scalar z = boundingLS.getHalfSize().z * 2;

			Matrix44 invLightView = lightView.getInverse();
			const POINT3 worldPos = boundingLS.getCenter()*invLightView;
			//clear fixed view projection
			lightCam->setViewMatrix(Matrix44::IDENTITY, false);
			lightCam->setProjectionMatrix(Matrix44::IDENTITY, false);

			//set view/projection matrix for culling: note that there are object behind the tight frustum casting shadows, so use a longer culling frustum
			lightCam->setOrthoMode(w, h);
			lightCam->setNearClipDistance((scalar)SHADOW_ORTHO_NEAR);
			lightCam->setFarClipDistance((scalar)SHADOW_CULLING_DISTANCE + z*0.5f + (scalar)SHADOW_ORTHO_NEAR);
			lightCam->setEyePosition(worldPos - dir*(scalar)SHADOW_CULLING_DISTANCE);
			lightCam->setRotation(static_cast<const Matrix33&>(invLightView));
			lightCam->updateMatrix();

			//override view projection matrix for rendering
			//lightCam->setViewMatrix(Matrix44::generateViewMatrix(worldPos - dir * (z*0.5f + (scalar)SHADOW_ORTHO_NEAR), static_cast<const Matrix33&>(invLightView)), true);
			//lightCam->setProjectionMatrix(Matrix44::generateOrthoProjectionMatrix(w, h, (scalar)SHADOW_ORTHO_NEAR, (scalar)SHADOW_ORTHO_NEAR + z), true);
		}

		/** @brief  */
		virtual void updateViewProjection(Camera* lightCam, Matrix44& depthMatrix, const ICamera* /*sceneCam*/, const ILight* light, const PointList& pointsWS)
		{
			////view projection updated in setup culling, nothing to do.

			//update projection depth range so that depth clamp is not needed
			PointList points = pointsWS;
			const AABB& visibleSceneBounding = lightCam->getVisibleBounding();
			if (!visibleSceneBounding.isNull())
				IShadowTechnique::clipBounding(visibleSceneBounding, lightCam->getFrustum(), points);

			const Matrix44& lightView = lightCam->getViewMatrix();

			AABB boundingLS;
			for (size_t j = 0; j < points.size(); ++j)
				boundingLS.merge(points[j] * lightView);

			scalar w = boundingLS.getHalfSize().x * 2;
			scalar h = boundingLS.getHalfSize().y * 2;
			scalar z = boundingLS.getHalfSize().z * 2;

			POINT3 pos = boundingLS.getCenter()*lightView.getInverse();
			lightCam->setViewMatrix(Matrix44::generateViewMatrix(pos - light->getDirection() * (z*0.5f + (scalar)SHADOW_ORTHO_NEAR), lightCam->getRotation()), true);
			lightCam->setProjectionMatrix(Matrix44::generateOrthoProjectionMatrix(w, h, (scalar)SHADOW_ORTHO_NEAR, (scalar)SHADOW_ORTHO_NEAR + z), true);
			Matrix44 proj = lightCam->getProjectionMatrix();
			depthMatrix = lightCam->getViewMatrix()*IGraphicsResourceManager::getSingleton().convertProjectionMatrix(proj);;
		}
	};
	
}//namespace Blade

#endif//__Blade_DefaultShadowTechnique_h__