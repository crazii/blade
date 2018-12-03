/********************************************************************
	created:	2017/06/17
	filename: 	ShadowManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ShadowManager.h"
#include <utility/StringHelper.h>
#include <interface/ITimeService.h>
#include <interface/IRenderTypeManager.h>
#include <interface/ISpace.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/IRenderTarget.h>
#include "../RenderScheme/RenderSchemeConfig.h"
#include "../ShaderVariables/ShaderVariableSource.h"
#include "../RenderScheme/DefaultRenderQueue.h"

#define PRECISE_BOUNDING 0

namespace Blade
{
	static const scalar FRUSTUM_OVERLAP_SCALE = 1.005f;
	static const scalar FRUSTUM_OVERLAP = 0.0f;

	const TString ShadowManager::SHADOW_MANAGER_TYPE = BTString("Shadows");

	//////////////////////////////////////////////////////////////////////////
	ShadowManager::ShadowManager()
	{
		mTechniqueName = Factory<IShadowTechnique>::getSingleton().getRegisteredClass(0);
		mTechnique = NULL;
		mCascadeCount = MAX_CASCADE_COUNT;
		mAutoSplitFactor = SHAODW_AUTO_SPLIT_FACTOR;

		for (size_t i = 0; i < countOf(mShadowCamera); ++i)
		{
			mShadowCamera[i] = NULL;
			mShadowViews[i] = NULL;
			mSplitDistance[i] = 0;
			mShadowDepthMatrix[i] = Matrix44::IDENTITY;
		}
		mQuality = 1;
		mUpdated = false;
		mAutoDisance = true;
	}

	//////////////////////////////////////////////////////////////////////////
	ShadowManager::~ShadowManager()
	{
		
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::initialize(ICameraManager* cameraManager, ILightManager* /*lightManager*/, IRenderPhase** phases, size_t phaseCount)
	{
		assert(cameraManager != NULL);
		mCascadeCount = 0;
		for (size_t i = 0; i < phaseCount; ++i)
		{
			IRenderPhase* phase = phases[i];
			const TString& cameraName = phase->getCameraName();
			assert(TStringHelper::isStartWith(phase->getName(), this->getTargetPrefix()));

			if (!cameraName.empty())
			{
				assert(phase->getView() != NULL);
				mShadowPhase.push_back(phase);
				size_t index = mShadowPhase.size() - 1;
				Camera* cam = BLADE_NEW Camera();

				cam->ensureCullingData(SHADOW_CULLING_OPTIMIZE ? false : true); //additional culling planes for SHADOW_CULLING_OPTIMIZE, no need for far clipping
				cam->setContentMask(CSF_SHADOWCASTER);
				//still need to update visible contents:
				//invisible contents form sceneCamera may be visible in shadow camera
				//cam->setUpdateMask(CUF_NONE);
				cam->getCameraRenderBuffer().setUsage(IRenderQueue::RQU_SHADOW);
				cam->getCameraRenderBuffer().setIndex(index);

				cameraManager->addCamera(cameraName, cam);

				assert(mShadowCamera[index] == NULL);
				mCameraNames[index] = cameraName;
				mShadowCamera[index] = cam;
				mShadowViews[index] = phase->getView();
				++mCascadeCount;
			}
			else
				phase->setEnable(false);
		}

		mTechnique = BLADE_FACTORY_CREATE(IShadowTechnique, mTechniqueName);

		this->setCascade(mCascadeCount);
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::shutdown(ICameraManager* cameraManager, ILightManager* /*lightManager*/)
	{
		for (size_t i = 0; i < countOf(mShadowCamera); ++i)
		{
			if (mShadowCamera[i] != NULL)
			{
				if(cameraManager != NULL)
					cameraManager->removeCamera(mCameraNames[i]);
				BLADE_DELETE mShadowCamera[i];
				mShadowCamera[i] = NULL;
			}
			mShadowTextures[i] = HTEXTURE::EMPTY;
			mShadowViews[i] = NULL;
		}
		mShadowPhase.clear();
		BLADE_DELETE mTechnique;
		mTechnique = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::adjustBuffer(const TString& name, size_t& width, size_t& height, PixelFormat& /*format*/, Sampler& sampler)
	{
		assert(TStringHelper::isStartWith(name, this->getTargetPrefix()));
		BLADE_UNREFERENCED(name);
		index_t quality = mQuality;
		quality = Math::Clamp<index_t>(quality, 0, 2u);
		scalar scales[] = { 0.5f, 1.0f, 2.0f };
		width = (size_t)((scalar)width * scales[quality]);
		height = (size_t)((scalar)height * scales[quality]);

		//shadow compare & sampler for GLES3/DX11
		sampler.setTestFunc(TF_LESS);
		sampler.setShadowSampler(true);
		//HW PCF
		sampler.setFilterMode(TFM_LINEAR, TFM_LINEAR, TFM_POINT);
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::preRender(ICamera* sceneCamera)
	{
		BLADE_UNREFERENCED(sceneCamera);
		ShaderVariableSource::getSingleton().setShadowManager(this);
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::postRender(ICamera* /*sceneCamera*/)
	{
		ShaderVariableSource::getSingleton().setShadowManager(NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::setupShadowCulling(ICamera* sceneCamera, ILight* mainLight)
	{
		assert(mainLight == NULL || mainLight->getType() == LT_DIRECTIONAL);
		if (mCascadeCount == 0 || mShadowPhase.size() == 0)
			return;

		if (mainLight == NULL)
		{
			for (uint32 i = 0; i < mCascadeCount; ++i)
			{
				mShadowPhase[i]->setEnable(false);
				mShadowTextures[i] = HTEXTURE::EMPTY;
				mShadowDepthMatrix[i] = Matrix44::IDENTITY;
			}
			return;
		}

		scalar near = sceneCamera->getNearClipDistance();
		scalar far = sceneCamera->getFarClipDistance();

		if (mAutoDisance)
		{
			float lambda = mAutoSplitFactor;
			float ratio = far / near;

			for (uint32 i = 0; i < mCascadeCount - 1; i++)
			{
				scalar si = (i+1) / (scalar)mCascadeCount;
				mSplitDistance[i] = lambda*(near*std::pow(ratio, si)) + (1 - lambda)*(near + (far - near)*si);
			}
			mSplitDistance[mCascadeCount - 1] = far;
		}
		else
		{
			scalar _far = far + 1e-3f;
			for (uint32 i = 0; i < mCascadeCount - 1; i++)
				mSplitDistance[mCascadeCount - 1] = std::min(mSplitDistance[mCascadeCount - 1], _far);
			mSplitDistance[mCascadeCount - 1] = _far;
		}

		Camera slice;
		slice.copyFrom(*sceneCamera, ICamera::CF_DEFAULT|ICamera::CF_GEOMETRY);
		Frustum f;
		f.enableNearFarPlane(true);
		//LoopID id = ITimeService::getSingleton().getTimeSource().getLoopID();

		for (uint32 i = 0; i < mCascadeCount; ++i)
		{
			IRenderPhase* phase = mShadowPhase[i];
			phase->setEnable(true);
			//phase->setEnable(i < 2 ? true : (i%2)==id%2);
			//if(!phase->isEnabled())
			//	continue;
			mShadowTextures[i] = phase->getTarget()->getDepthBuffer();
			assert(mShadowCamera[i] != NULL);

			scalar sliceNear = (i == 0) ? near : mSplitDistance[i - 1];
			scalar sliceFar = mSplitDistance[i] * FRUSTUM_OVERLAP_SCALE + FRUSTUM_OVERLAP;
			slice.setNearClipDistance(sliceNear);
			slice.setFarClipDistance(sliceFar);
			slice.getFrustum(f);

			AABB maxBounds;
			for (size_t j = 0; j < Frustum::FCI_COUNT; ++j)
				maxBounds.merge(f.getFrustumCorner((Frustum::CORNER)j));

			AABB bounds;
			bool isMax = false;
			bool preciseBounding = PRECISE_BOUNDING && (i == 0);

			mPointCache[i].clear();

			if (sliceNear < far)
			{
				mPointCache[i].reserve((2 * 32)*PRECISE_BOUNDING + (!PRECISE_BOUNDING) * 32);

				for (size_t j = 0; !isMax && j < phase->getStepCount(); ++j)
				{
					const RenderType* type = phase->getStepRenderType(j);
					IRenderQueue* queue = sceneCamera->getRenderQueue(type);
					if (queue == NULL)
						continue;

					//iterate culling results. note: shadow camera still need cull against scene because
					//invisible objects from scene camera may still cast shadows
					for (size_t g = 0; !isMax && g < queue->getGroupCount(); ++g)
					{
						IRenderGroup* group = queue->getRenderGroup(g);
						RenderOperation* rops = NULL;
						size_t count = group->getROPArray(rops);
						for (size_t r = 0; r < count; ++r)
						{
							const RenderOperation& rop = rops[r];
							const AABB& rbounds = rop.renderable->getWorldBounding();
							ICamera::VISIBILITY vis;
#if 0
							if (bounds.contains(rbounds))
								continue;
							vis = f.getVisibility(rbounds);
#else
							scalar minDist = rop.viewDistance - rop.radius;
							scalar maxDist = rop.viewDistance + rop.radius;
							bool minIn = (minDist >= sliceNear && minDist <= sliceFar);
							bool maxIn = (maxDist >= sliceNear && maxDist <= sliceFar);
							bool containing = minDist < sliceNear && maxDist > sliceFar;	//whole slice is within radius
							vis = (minIn && maxIn) ? ICamera::CV_ALL : ((minIn || maxIn || containing) ? ICamera::CV_PARTIAL : ICamera::CV_NULL);
#endif
							if (vis == ICamera::CV_NULL)
								continue;

#if SHADOW_CULLING_OPTIMIZE
							//add scene contents to light camera later
							mSceneSliceCache[i][type].push_back(rop);
#endif

							if (preciseBounding)
							{
								if (vis == ICamera::CV_PARTIAL)
									IShadowTechnique::clipBounding(rbounds, &f, mPointCache[i]);
								else
								{
									for (size_t k = 0; k < AABB::BC_COUNT; ++k)
										mPointCache[i].push_back(rbounds.getCorner(AABB::CORNER(k)));
								}
							}

							bounds.merge(rbounds);
							if (bounds.contains(maxBounds))
							{
								bounds = maxBounds;
								isMax = true;
								break;
							}
						}
					}	//for each group in queue
				}//for each render type queue
			}

			if (bounds.isNull() || (preciseBounding && mPointCache[i].empty()))
			{
				phase->setEnable(false);
				continue;
			}

			//build cascade camera
			Camera* cam = static_cast<Camera*>(mShadowCamera[i]);
			cam->setMainCamera(sceneCamera);

			if (isMax)
			{
				mPointCache[i].clear();
				for (size_t j = 0; j < Frustum::FCI_COUNT; ++j)
					mPointCache[i].push_back(f.getFrustumCorner((Frustum::CORNER)j));
			}
			else if (!preciseBounding)
			{
				IShadowTechnique::clipBounding(bounds, &f, mPointCache[i]);
				if (mPointCache[i].empty())
				{
					phase->setEnable(false);
					continue;
				}
			}

#if SHADOW_CULLING_OPTIMIZE
			{
				const Plane* p = sceneCamera->getFrustum()->getFrustumPlanes();
				const POINT3 virtualLightPos = sceneCamera->getEyePosition() - mainLight->getDirection() * (scalar)10000.0f;
				Plane cullingPlane[6];
				size_t cullingPlaneCount = 0;
				for (size_t j = 0; j < 6; ++j)
				{
					if (p[j].getSide(virtualLightPos, Math::LOW_EPSILON) == PS_NEGATIVE)
					{
						assert(cullingPlaneCount < 5);
						cullingPlane[cullingPlaneCount++] = -p[j];
					}
				}
				cam->enableFrustumExtraPlane(cullingPlane, cullingPlaneCount);
			}
#endif

			mTechnique->setupCulling(cam, &slice, mainLight, mPointCache[i]);
		}
		mUpdated = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::updateShadowViewProjection(ICamera* sceneCamera, ILight* mainLight)
	{
		if (mCascadeCount == 0 || mainLight == NULL || mShadowPhase.size() == 0)
			return;

		Camera slice;
		slice.copyFrom(*sceneCamera, ICamera::CF_DEFAULT | ICamera::CF_GEOMETRY);

		for (uint32 i = 0; i < mCascadeCount; ++i)
		{
			IRenderPhase* phase = mShadowPhase[i];
			if (phase->isEnabled() && !mPointCache[i].empty())
			{
				slice.setNearClipDistance(i == 0 ? sceneCamera->getNearClipDistance() : mSplitDistance[i - 1]);
				slice.setFarClipDistance(mSplitDistance[i] * FRUSTUM_OVERLAP_SCALE + FRUSTUM_OVERLAP);
				slice.updateMatrix();
				Camera* cam = static_cast<Camera*>(mShadowCamera[i]);

#if SHADOW_CULLING_OPTIMIZE
				//add scene visible result directly to light camera
				for (SceneSliceCache::const_iterator it = mSceneSliceCache[i].begin(); it != mSceneSliceCache[i].end(); ++it)
				{
					IRenderQueue* queue = cam->getCameraRenderBuffer().getRenderQueue(it->first);
					for (size_t j = 0; j < it->second.size(); ++j)
						static_cast<DefaultRenderQueue*>(queue)->addRenderOP(it->second[j]);
				}
				mSceneSliceCache[i].clear();
#endif

				mTechnique->updateViewProjection(cam, mShadowDepthMatrix[i], &slice, mainLight, mPointCache[i]);
			}
		}

		for (uint32 i = 0; i < MAX_CASCADE_COUNT; ++i)
		{
#if SHADOW_CULLING_OPTIMIZE
			mSceneSliceCache[i].clear();
#endif
			mPointCache[i].clear();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void ShadowManager::notifyConfigChanges(void* data)
	{
		this->setCascade(mCascadeCount);
		if (data == &mTechniqueName)
		{
			BLADE_DELETE mTechnique;
			mTechnique = BLADE_FACTORY_CREATE(IShadowTechnique, mTechniqueName);
		}
	}
	
}//namespace Blade