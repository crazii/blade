/********************************************************************
	created:	2017/02/06
	filename: 	GlobalShaderVariableUpdaters.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GlobalShaderVariableUpdaters_h__
#define __Blade_GlobalShaderVariableUpdaters_h__
#include "ShaderVariableUpdaters.h"
#include <interface/ITimeService.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <GraphicsShaderShared.inl>
#include "ShaderVariableSource.h"
#include "../Shadows/ShadowManager.h"

namespace Blade
{
	class TimeUpdater : public GlobalVariableUpdater
	{
	public:
		TimeUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		virtual const void*	updateData() const
		{
			//accumulated time
			mTime[0] = ITimeService::getSingleton().getTimeSource().getTime();
			//delta time
			mTime[1] = ITimeService::getSingleton().getTimeSource().getTimeThisLoop();
			reinterpret_cast<Vector2&>(mTime[2]) = Vector2::UNIT_ALL / reinterpret_cast<const Vector2&>(mTime);
			return &mTime;
		}
	protected:
		mutable Vector4 mTime;
	};

	class SinCosTimeUpdater : public GlobalVariableUpdater
	{
	public:
		SinCosTimeUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, 1)
		{

		}

		virtual const void*	updateData() const
		{
			//accumulated time
			scalar time = ITimeService::getSingleton().getTimeSource().getTime();
			mSinCosTime[0] = std::sin(time);
			//delta time
			mSinCosTime[1] = std::cos(time);
			reinterpret_cast<Vector2&>(mSinCosTime[2]) = Vector2::UNIT_ALL / reinterpret_cast<const Vector2&>(mSinCosTime);
			return &mSinCosTime;
		}
	protected:
		mutable Vector4 mSinCosTime;
	};

	class ShadowMatrixUpdater : public GlobalVariableUpdater
	{
	public:
		ShadowMatrixUpdater()
			:GlobalVariableUpdater(SCT_MATRIX44, MAX_SHADOW_CASCADE)
		{

		}

		virtual const void*	updateData() const
		{
			const ShadowManager* shadowManager = ShaderVariableSource::getSingleton().getShadowManager();
			if (shadowManager == NULL)
			{
				assert(false);
				return &mShadowMatrices;
			}
			const ICamera*const* shadowCameras = shadowManager->getShadowCameras();
			const HTEXTURE* shadowTextues = shadowManager->getShadowTextures();

			for (size_t i = 0; i < MAX_SHADOW_CASCADE; ++i)
			{
				if (shadowCameras != NULL && shadowTextues != NULL 
					&&  shadowCameras[i] != NULL && shadowTextues[i] != NULL)
				{
					scalar bias = 1e-3f * shadowTextues[i]->getWidth() / 2048.0f;
					Matrix44 proj = shadowCameras[i]->getProjectionMatrix();
					IGraphicsResourceManager::getSingleton().convertProjectionMatrix(proj);

					Matrix44 biasMatrix = Matrix44::IDENTITY;
					biasMatrix[3].z = bias;
					mShadowMatrices[i] = shadowCameras[i]->getViewMatrix() * biasMatrix * proj /** biasMatrix*/;	//use view space bias (constant for perspective shadow mapping)
				}
				else
					mShadowMatrices[i] = Matrix44::IDENTITY;
			}
			return mShadowMatrices;
		}

	protected:
		mutable Matrix44 mShadowMatrices[MAX_SHADOW_CASCADE];
		friend class ShadowDepthMatrixUpdater;
	};

	class ShadowTextureCoordinateUpdate : public GlobalVariableUpdater
	{
	public:
		ShadowTextureCoordinateUpdate()
			:GlobalVariableUpdater(SCT_FLOAT4, MAX_SHADOW_CASCADE)
		{

		}

		virtual const void*	updateData() const
		{
			const ShadowManager* shadowManager = ShaderVariableSource::getSingleton().getShadowManager();
			if (shadowManager == NULL)
			{
				assert(false);
				return NULL;
			}

			const ICamera*const* shadowCameras = shadowManager->getShadowCameras();
			const HTEXTURE* shadowTextues = shadowManager->getShadowTextures();
			const IGraphicsView*const* shadowViews = shadowManager->getShadowViews();

			for (size_t i = 0; i < MAX_SHADOW_CASCADE; ++i)
			{
				if (shadowCameras != NULL && shadowTextues != NULL && shadowViews != NULL
					&& shadowCameras[i] != NULL && shadowTextues[i] != NULL && shadowViews[i] != NULL)
				{
					scalar halfWidth = 0.5f / shadowTextues[i]->getWidth();
					scalar halfHeight = 0.5f / shadowTextues[i]->getHeight();
					scalar l = shadowViews[i]->getRelativeLeft();
					scalar t = shadowViews[i]->getRelativeTop();
					scalar w = shadowViews[i]->getRelativeWidth();
					scalar h = shadowViews[i]->getRelativeHeight();

					mShadowCoordTransform[i].x = w*(1 - halfWidth * 2);
					mShadowCoordTransform[i].y = h*(1 - halfHeight * 2);
					//always apply half pixel offset because of sub view
					mShadowCoordTransform[i].z = (halfWidth + 1e-9f)  + l;
					mShadowCoordTransform[i].w = (halfHeight + 1e-9f) + t;
				}
			}
			return mShadowCoordTransform;
		}
	protected:
		mutable Vector4 mShadowCoordTransform[MAX_SHADOW_CASCADE];
	};

	///depth matrix for some shadow techniques altering fragment depth.
	///i.e. TSM use two matrices, one to project shadow (distribution), one to write standard shadow depth
	///in such case shadowing pass need two matrices too.
	class ShadowDepthMatrixUpdater : public GlobalVariableUpdater
	{
	public:
		ShadowDepthMatrixUpdater()
			:GlobalVariableUpdater(SCT_MATRIX44, MAX_SHADOW_CASCADE)
		{

		}

		virtual const void*	updateData() const
		{
			const ShadowManager* shadowManager = ShaderVariableSource::getSingleton().getShadowManager();
			if (shadowManager == NULL)
			{
				assert(false);
				return NULL;
			}
			const Matrix44* depthMatrices = shadowManager->getShadowDepthMatrix();
			const ICamera*const* shadowCameras = shadowManager->getShadowCameras();
			const HTEXTURE* shadowTextues = shadowManager->getShadowTextures();

			bool applyBias = true;
			if (applyBias)
			{
				//don't apply bias when rendering shadow depth
				const ICamera* currentCamera = IShaderVariableSource::getSingleton().getCamera();
				for (size_t i = 0; i < MAX_SHADOW_CASCADE; ++i)
				{
					if (currentCamera == shadowCameras[i])
					{
						applyBias = false;
						break;
					}
				}
			}

			for (size_t i = 0; i < MAX_SHADOW_CASCADE; ++i)
			{
				if (depthMatrices != NULL)
					mShadowDepthMatrices[i] = depthMatrices[i];
				else
					mShadowDepthMatrices[i] = Matrix44::IDENTITY;
				if (applyBias && shadowTextues != NULL && shadowTextues[i] != NULL)
				{
					scalar bias = -5e-3f * (shadowTextues[i]->getWidth() / 2048.0f);
					Matrix44 biasMatrix = Matrix44::IDENTITY;
					biasMatrix[3].z = bias;
					mShadowDepthMatrices[i] *= biasMatrix;
				}
			}
			return mShadowDepthMatrices;
		}

	protected:
		mutable Matrix44 mShadowDepthMatrices[MAX_SHADOW_CASCADE];
	};

	class CurrentShadowDepthMatrixUpdater : public GlobalVariableUpdater, public NonAssignable
	{
	public:
		CurrentShadowDepthMatrixUpdater(ShadowDepthMatrixUpdater& updater)
			:GlobalVariableUpdater(SCT_MATRIX44,1)
			,mUpdater(updater)
		{}
		~CurrentShadowDepthMatrixUpdater() {}

		virtual const void*	updateData() const
		{
			const ShadowManager* shadowManager = ShaderVariableSource::getSingleton().getShadowManager();
			if (shadowManager == NULL)
			{
				assert(false);
				return NULL;
			}

			const Matrix44* depthMatrices = (const Matrix44*)mUpdater.getVariable()->getData();
			const ICamera* currentCamera = IShaderVariableSource::getSingleton().getCamera();
			const ICamera*const* shadowCameras = shadowManager->getShadowCameras();
			for (size_t i = 0; i < MAX_SHADOW_CASCADE; ++i)
			{
				if (currentCamera == shadowCameras[i])
					return &depthMatrices[i];
			}
			return &Matrix44::IDENTITY;
		}

	protected:
		ShadowDepthMatrixUpdater& mUpdater;
	};

	class ShadowDistancesUpdater : public GlobalVariableUpdater
	{
	public:
		ShadowDistancesUpdater()
			:GlobalVariableUpdater(SCT_FLOAT4, MAX_SHADOW_CASCADE)
		{

		}

		virtual const void*	updateData() const
		{
			const ShadowManager* shadowManager = ShaderVariableSource::getSingleton().getShadowManager();
			if (shadowManager == NULL)
				std::memset(mShadowDistances, 0, sizeof(mShadowDistances));
			else
			{
				const scalar* distances = shadowManager->getShadowDistances();
				for (int i = 0; i < MAX_SHADOW_CASCADE; ++i)
					mShadowDistances[i][0] = distances[i];
			}
			return mShadowDistances;
		}
	protected:
		mutable Vector4 mShadowDistances[MAX_SHADOW_CASCADE];
	};
	
}//namespace Blade


#endif // __Blade_GlobalShaderVariableUpdaters_h__