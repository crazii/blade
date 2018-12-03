/********************************************************************
	created:	2017/06/17
	filename: 	ShadowManager.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_ShadowManager_h__
#define __Blade_ShadowManager_h__
#include <utility/Bindable.h>
#include <BladeGraphics.h>
#include <interface/ICameraManager.h>
#include <interface/ILightManager.h>
#include <GraphicsShaderShared.inl>
#include "../RenderScheme/RenderScheme.h"
#include "../RenderScheme/RenderOutput.h"
#include "IShadowTechnique.h"
#include "../../internal/GraphicsConfig.h"

namespace Blade
{
	static const fp32 SHAODW_AUTO_SPLIT_FACTOR = 0.95f;
	static const fp32 SHADOW_MANUAL_SPLIT_DISTANCE[MAX_SHADOW_CASCADE] =
	{
		5.0f,
		200.0f,
		700.0f,
		2000.0f,	//not used. always far plane
	};

	class ShadowManager : public IRenderSchemeOperation, public Allocatable
	{
	public:
		static const uint32 MAX_CASCADE_COUNT = MAX_SHADOW_CASCADE;
		static const TString SHADOW_MANAGER_TYPE;	//IRenderPipelineOperator type
	public:
		ShadowManager();
		~ShadowManager();

		/** @brief collect shadow outputs */
		virtual void initialize(ICameraManager* cameraManager, ILightManager* lightManager, IRenderPhase** phases, size_t phaseCount);

		/** @brief  */
		virtual void shutdown(ICameraManager* cameraManager, ILightManager* lightManager);

		/** @brief adjust buffer before creating buffers for scheme */
		virtual void adjustBuffer(const TString& name, size_t& width, size_t& height, PixelFormat& format, Sampler& sampler);

		/** @brief  */
		virtual void preCull(ICamera* sceneCamera, ICameraManager* /*cameraManager*/, ILightManager* lightManager)
		{
			this->setupShadowCulling(sceneCamera, lightManager->getMainLight());
		}

		/** @brief  */
		virtual void postCull(ICamera* sceneCamera, ICameraManager* /*cameraManager*/, ILightManager* lightManager)
		{
			this->updateShadowViewProjection(sceneCamera, lightManager->getMainLight());
		}

		/** @brief  */
		virtual void preRender(ICamera* /*sceneCamera*/);

		/** @brief  */
		virtual void postRender(ICamera* /*sceneCamera*/);

		/** @brief  */
		void setupShadowCulling(ICamera* sceneCamera, ILight* mainLight);

		/** @brief  */
		void updateShadowViewProjection(ICamera* sceneCamera, ILight* mainLight);

		/** @brief  */
		inline void setCascade(uint32 count)
		{
			count = std::min(4u, count);
			mCascadeCount = count;
			size_t size = mShadowPhase.size();

			for (uint32 i = 0; i < std::min(count, (uint32)size); ++i)
			{
				mShadowPhase[i]->setEnable(true);
			}

			for (uint32 i = count; i < (uint32)size; ++i)
			{
				mShadowPhase[i]->setEnable(false);
			}
		}

		/** @brief  */
		inline const ICamera*const* getShadowCameras() const
		{
			return mUpdated ? mShadowCamera : NULL;
		}

		/** @brief  */
		inline const HTEXTURE* getShadowTextures() const
		{
			return mUpdated ? mShadowTextures : NULL;
		}

		/** @brief  */
		inline const IGraphicsView*const* getShadowViews() const
		{
			return mShadowViews;
		}
		
		/** @brief  */
		inline const Matrix44* getShadowDepthMatrix() const
		{
			return mShadowDepthMatrix;
		}

		/** @brief  */
		inline const scalar* getShadowDistances() const
		{
			return mSplitDistance;
		}

	protected:
		typedef FixedVector<IRenderPhase*, MAX_CASCADE_COUNT> ShadowOutput;

		/** @brief  */
		void notifyConfigChanges(void* data);

		ShadowOutput mShadowPhase;
		TString		mCameraNames[MAX_CASCADE_COUNT];
		ICamera*	mShadowCamera[MAX_CASCADE_COUNT];
		HTEXTURE	mShadowTextures[MAX_CASCADE_COUNT];
		IGraphicsView*	mShadowViews[MAX_CASCADE_COUNT];
		Matrix44		mShadowDepthMatrix[MAX_CASCADE_COUNT];
		IShadowTechnique::PointList mPointCache[MAX_CASCADE_COUNT];
#if SHADOW_CULLING_OPTIMIZE
		typedef TempMap<const RenderType*, TempVector<RenderOperation> > SceneSliceCache;
		SceneSliceCache	mSceneSliceCache[MAX_CASCADE_COUNT];	//visible scene contents
#endif

		TString	mTechniqueName;
		IShadowTechnique* mTechnique;
		uint32	mCascadeCount;
		fp32	mAutoSplitFactor;
		fp32	mSplitDistance[MAX_CASCADE_COUNT];
		uint32		mQuality;
		LoopFlag	mUpdated;
		bool	mAutoDisance;

		friend class GraphicsSystemPlugin;
	};
	
}//namespace Blade

#endif//__Blade_ShadowManager_h__