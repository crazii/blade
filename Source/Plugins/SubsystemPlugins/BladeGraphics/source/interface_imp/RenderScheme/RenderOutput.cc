/********************************************************************
	created:	2011/12/18
	filename: 	RenderOutput.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderOutput.h"
#include "RenderStepInstance.h"
#include "RenderSchemeManager.h"
#include "RenderSortHelper.h"

#include <utility/Profiling.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/IPlatformManager.h>
#include <interface/public/graphics/IGraphicsService.h>

#include <interface/ICamera.h>
#include <interface/IShaderVariableSource.h>
#include <Technique.h>
#include <Pass.h>
#include <MaterialInstance.h>
#include <utility/Profiling.h>


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	RenderOutput::RenderOutput(const OUTPUT_DESC* info)
		:mDesc(info)
		,mTarget(NULL)
		,mView(NULL)
		,mCustomCamera(NULL)
		,mInputCount(0)
	{
		mEnabled = mDesc->mEnable;
		mLDR = false;
		mSorter = RenderSchemeManager::getSingleton().createRenderSorter(mDesc->mSorters);
	}

	//////////////////////////////////////////////////////////////////////////
	RenderOutput::~RenderOutput()
	{
		if( mDesc->mViewDesc.mViewRef != VR_FINAL )
			IGraphicsService::getSingleton().destroyView(mView);

		for(size_t i = 0; i < mStepList.size(); ++i)
		{
			RenderStep* step = mStepList[i];
			BLADE_DELETE step;
		}
		BLADE_DELETE mCustomCamera;
		BLADE_DELETE mSorter;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			RenderOutput::getStepCount() const
	{
		return mStepList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	RenderStep*		RenderOutput::getRenderStep(index_t index) const
	{
		if( index < mStepList.size() )
			return mStepList[index];
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		RenderOutput::getRenderStepByRenderType(index_t start, const TString& type) const
	{
		for(size_t i = start; i < mStepList.size(); ++i)
		{
			if(mStepList[i]->getDesc()->mTypeDesc->mRenderTypeName == type )
				return i;
		}
		return INVALID_INDEX;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t			RenderOutput::addRenderStep(RenderStep* step)
	{
		if( step != NULL )
		{
			mStepList.push_back(step);
			return mStepList.size() - 1;
		}
		else
			return INVALID_INDEX;
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool			RenderOutput::setTarget(IRenderTarget* target)
	{
		mTarget = target;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderOutput::setView(IGraphicsView* view)
	{
		if( this->isUsingFinalView() )
		{
			assert(false);
			return false;
		}
		mView = view;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderOutput::setCustomCamera(ICamera* camera, IGraphicsView* view)
	{
		if( !this->isUsingCustomCamera() || mCustomCamera != NULL)
		{
			assert(false);
			return;
		}
		assert(mView == NULL || mView == view);

		mCustomCamera = camera;

		const CAMERA_DESC& desc = mDesc->mCameraDesc;
		Mask flag = desc.mFlags;

		//init custom data

		//TODO: define camera param block to fill data and set once to avoid too many virtual calls?
		if( flag.checkBits(CDF_CUSTOM_TYPE) )
			mCustomCamera->setProjectionType( desc.mCameraType );

		if( flag.checkBits(CDF_CUSTOM_NEAR) )
			mCustomCamera->setNearClipDistance(desc.mNear);

		if( flag.checkBits(CDF_CUSTOM_FAR) )
			mCustomCamera->setFarClipDistance(desc.mFar);

		if( mDesc->mCameraDesc.mCameraType == PT_PERSPECTIVE )
		{
			if( flag.checkBits(CDF_CUSTOM_FOV) )
				mCustomCamera->setFieldOfView(desc.mFOV);

			if( flag.checkBits(CDF_CUSTOM_ASPECT) )
				mCustomCamera->setAspectRatio(desc.mAspect);
		}
		else
		{
			if( flag.checkBits(CDF_CUSTOM_ORTHO) )
			{
				scalar width = desc.mWidth*(view->getPixelRight()-view->getPixelLeft()) + desc.mWidthPixel;
				scalar height = desc.mHeight*(view->getPixelBottom()-view->getPixelTop()) + desc.mHeightPixel;
				mCustomCamera->setOrthoMode(width,height);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			RenderOutput::setupCustomCamera(ICamera* refernce, IGraphicsView* view)
	{
		if( !this->isUsingCustomCamera() )
		{
			assert(false);
			return false;
		}
		if( refernce == NULL || mCustomCamera == NULL)
		{
			assert(false);
			return false;
		}
		const CAMERA_DESC& desc = mDesc->mCameraDesc;
		Mask flag = desc.mFlags;

		mCustomCamera->setEyePosition( refernce->getEyePosition() );
		mCustomCamera->setOrientation( refernce->getOrientation() );

		if( flag.checkBitsEqual(CDF_CUSTOM_ALL) )
			return true;

		//TODO: define camera param block to fill data and set once to avoid too many virtual calls?
		if( !flag.checkBits(CDF_CUSTOM_TYPE) )
			mCustomCamera->setProjectionType( refernce->getProjectionType() );

		if( !flag.checkBits(CDF_CUSTOM_NEAR) )
			mCustomCamera->setNearClipDistance( refernce->getNearClipDistance() );

		if( !flag.checkBits(CDF_CUSTOM_FAR) )
			mCustomCamera->setFarClipDistance( refernce->getFarClipDistance() );

		if( mDesc->mCameraDesc.mCameraType == PT_PERSPECTIVE )
		{
			if( !flag.checkBits(CDF_CUSTOM_FOV) )
				mCustomCamera->setFieldOfView( refernce->getFieldOfView() );

			if( !flag.checkBits(CDF_CUSTOM_ASPECT) )
				mCustomCamera->setAspectRatio( refernce->getAspectRatio() );
		}
		else
		{
			if( !flag.checkBits(CDF_CUSTOM_ORTHO) )
			{
				assert(mView == NULL || mView == view);
				scalar width = desc.mWidth*(view->getPixelRight()-view->getPixelLeft()) + desc.mWidthPixel;
				scalar height = desc.mHeight*(view->getPixelBottom()-view->getPixelTop()) + desc.mHeightPixel;
				mCustomCamera->setOrthoMode(width,height);
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderOutput::setShaderVarialbeSource(ICamera* camera, IGraphicsView* view)
	{
		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		//set shader variable 's global source
		if (this->isUsingCustomCamera())
		{
			this->setupCustomCamera(camera, view);
			svc.setCamera(this->getCustomCamera());
		}
		else
			svc.setCamera(camera);

		assert(view != NULL);
		assert(mView == NULL || mView == view);
		svc.setView(view);
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderOutput::renderOutput(IRenderDevice* device, ICamera* camera, IGraphicsView* view,  
		const RenderStepInstance* const* instances, size_t instanceCount)
	{
		assert(instanceCount != 0);

		this->setShaderVarialbeSource(camera, view);

		if( mSorter != NULL )
		{
			this->renderMixedSteps(device, camera, view, instances, instanceCount);
			return;
		}

		for(size_t i = 0; i < instanceCount; ++i)
		{
			const RenderStepInstance* stepInstance = instances[i];
			if( !stepInstance->isEnabled() )
				continue;
			RenderStep* step = stepInstance->getRenderStep();

			IRenderQueue* queue = camera->getRenderQueue( step->getType() );
			if(queue == NULL || queue->isEmpty() )
				continue;
			if(mSorter == NULL && step->getSorter() != NULL )
				queue->sort(step->getSorter());

			FILL_MODE mode = view->getFillMode();
			Pass* pass = step->getMaterialPass();
			if( pass != NULL )
			{
				if( !pass->isLoaded() )
					continue;
				
				if( stepInstance->getFillMode() != GFM_DEFAULT || stepInstance->isFixedFillMode())
					mode = stepInstance->getFillMode();

				this->renderStepWithUniquePass(device, mode, queue, pass, step->getDesc()->mTexturing);
			}
			else
				this->renderStepWithDymamicPass(device, mode, queue, step);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	void			RenderOutput::renderStepWithUniquePass(IRenderDevice* device, FILL_MODE mode, IRenderQueue* queue, Pass* pass, bool texturing)
	{
		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		IRenderSchemeManager& rsm = IRenderSchemeManager::getSingleton();

		//BLADE_LW_PROFILING_FUNCTION();
		RenderPropertySet* passProperty = pass->getRenderProperty();
		if( passProperty != NULL )
			rsm.applyRenderProperty(device, *passProperty, mode);

		size_t shaderCount = pass->getShaderOptionCount();
		for(size_t j = 0; j < shaderCount; ++j)
		{
			const HSHADEROPTION& shader = pass->getShaderOption(j);
			const ShaderOption::GroupListLink& groups = shader->getGroupLink();

			if( groups.size() == 0 || queue->countGroups(&groups[0], groups.size()) == 0 )
				continue;
			svc.onShaderSwitch(false);

			rsm.applyGlobalShaderSetting( device, shader );

			for(size_t n = 0; n < groups.size(); ++n)
			{
				index_t groupIndex = groups[n];
				assert(groupIndex < queue->getGroupCount() );

				IRenderGroup* group = queue->getRenderGroup(groupIndex);
				if( group == NULL )
					continue;
				RenderOperation* rops;
				size_t count = group->getROPArray(rops);

				const RenderPropertySet* lastInstanceProperty = NULL;
				for(size_t i = 0; i < count; ++i)
				{
					const RenderOperation& rop = rops[i];
					//assert(rop->material->getActiveShaderGroup() == groupIndex);
					const RenderPropertySet* instanceProperty = rop.material->getRenderProperty();
					if( instanceProperty != NULL || lastInstanceProperty != NULL)
					{
						if(svc.setRenderProperties(passProperty, NULL, instanceProperty) )
							rsm.applyRenderProperty(device, *svc.getRenderProperty(), mode);
						else
							instanceProperty = NULL;
					}
					lastInstanceProperty = instanceProperty;

					svc.setRenderOperation(&rop);
					rsm.applyInstanceShaderSetting(device, shader);

					if (texturing)
					{
						const SamplerState& samplers = pass->getSamplerState();
						rsm.applyTextureSetting(device, shader, rop.material->getTextureStateInstance(), &samplers);
					}
					//draw geometry
					device->renderGeometry( *rop.geometry );
				}
			}//for

		}//for
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderOutput::renderStepWithDymamicPass(IRenderDevice* device, FILL_MODE mode, IRenderQueue* queue, const RenderStep* step)
	{
		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		IRenderSchemeManager& rsm = IRenderSchemeManager::getSingleton();

		size_t groupCount = queue->getGroupCount();
		svc.onShaderSwitch(true);

		const index_t* groupIndices = NULL;
		size_t usedGroupCount = step->getShaderGroups(groupIndices);
		assert(usedGroupCount <= groupCount);
		groupCount = groupIndices != NULL ? usedGroupCount : groupCount;

		for(size_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			IRenderGroup* group = queue->getRenderGroup(groupIndices ? groupIndices[groupIndex] : groupIndex);
			if( group == NULL )
				continue;

			RenderOperation* rops = NULL;
			size_t count  = group->getROPArray(rops);

			for (size_t i = 0; i < count; ++i)
			{
				const RenderOperation& rop = rops[i];

				//use per-renderable pass: renderables are probably using different materials & shaders
				const Pass* pass = rop.material->getDynamicPass();
				const SamplerState& samplers = pass->getSamplerState();

				ShaderOption* shader = rop.material->getDynamicShader();
				if( pass == NULL || shader == NULL || !pass->isLoaded() )
					continue;

				if(svc.setRenderProperties(pass->getRenderProperty(), NULL, rop.material->getRenderProperty()) )
					rsm.applyRenderProperty(device, *svc.getRenderProperty(), mode);

				//@note: set renderable for shader variable before apply render settings
				svc.setRenderOperation(&rop);

				//apply render settings
				rsm.applyRenderSettings(device, shader, rop.material->getTextureStateInstance(), &samplers);

				//draw geometry
				device->renderGeometry( *rop.geometry );
			}

		}//for
	}

	struct MixedRenderOP
	{
		RenderOperation rop;
		const ShaderOption*	shader;
		RenderPropertySet*	passProperties;
		const SamplerState* samplers;
	};
	//////////////////////////////////////////////////////////////////////////
	void			RenderOutput::renderMixedSteps(IRenderDevice* device, ICamera* camera, IGraphicsView* view, 
		const RenderStepInstance* const* instances, size_t instanceCount)
	{
		assert(mSorter != NULL);
		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		IRenderSchemeManager& rsm = IRenderSchemeManager::getSingleton();
		//BLADE_LW_PROFILING_FUNCTION();

		TempVector<MixedRenderOP> mixedOperations;
		mixedOperations.reserve(1024);

		for(size_t instance = 0; instance < instanceCount; ++instance)
		{
			const RenderStepInstance* stepInstance = instances[instance];
			if( !stepInstance->isEnabled() )
				continue;

			RenderStep* step = stepInstance->getRenderStep();
			IRenderQueue* queue = camera->getRenderQueue( step->getType() );
			if(queue == NULL || queue->isEmpty() )
				continue;

			Pass* pass = step->getMaterialPass();
			if( pass != NULL )
			{
				if( !pass->isLoaded() )
					continue;
				//gather renderable from unique passes
				RenderPropertySet* passProperty = pass->getRenderProperty();
				const SamplerState& samplers = pass->getSamplerState();

				size_t shaderCount = pass->getShaderOptionCount();
				for(size_t j = 0; j < shaderCount; ++j)
				{
					const HSHADEROPTION& shader = pass->getShaderOption(j);
					const ShaderOption::GroupListLink& groups = shader->getGroupLink();

					if( groups.size() == 0 || queue->countGroups(&groups[0], groups.size()) == 0 )
						continue;

					for(size_t n = 0; n < groups.size(); ++n)
					{
						index_t groupIndex = groups[n];
						assert(groupIndex < queue->getGroupCount() );

						IRenderGroup* group = queue->getRenderGroup(groupIndex);
						if( group == NULL )
							continue;
						RenderOperation* rops = NULL;
						size_t count = group->getROPArray(rops);

						for (size_t i = 0; i < count; ++i)
						{
							const RenderOperation& rop = rops[i];

							MixedRenderOP op = { rop,  &(*shader), passProperty, &samplers };
							mixedOperations.push_back(op);
						}
					}//for

				}//for
			}//if pass != NULL
			else
			{
				//gather renderables from dynamic pass
				size_t groupCount = queue->getGroupCount();
				const index_t* groupIndices = NULL;
				size_t usedGroupCount = step->getShaderGroups(groupIndices);
				assert(usedGroupCount <= groupCount);
				groupCount = groupIndices != NULL ? usedGroupCount : groupCount;

				for(size_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
				{
					IRenderGroup* group = queue->getRenderGroup(groupIndices ? groupIndices[groupIndex] : groupIndex);

					if( group == NULL )
						continue;

					RenderOperation* rops = NULL;
					size_t count = group->getROPArray(rops);

					for (size_t i = 0; i < count; ++i)
					{
						const RenderOperation& rop = rops[i];

						//use per-renderable pass: renderables are probably using different materials & shaders
						const Pass* dynamicPass = rop.material->getDynamicPass();
						ShaderOption* shader = rop.material->getDynamicShader();
						if(dynamicPass == NULL || shader == NULL || !dynamicPass->isLoaded() )
							continue;

						MixedRenderOP op = { rop,  shader, dynamicPass->getRenderProperty(), &dynamicPass->getSamplerState() };
						mixedOperations.push_back(op);
					}
				}//for
			}//if pass != NULL
		}//for each step

		//sort and draw
		if( mixedOperations.size() == 0 )
			return;
		RenderSortHelper::setSorter(mSorter);
		//note: IRenderable is the first member is MixedRenderOP, so compare function will work, this is kinda dirty but use it for now,
		//as qsort is a C style function, and parameter void* without type
		std::qsort(&mixedOperations[0], mixedOperations.size(), sizeof(MixedRenderOP), &RenderSortHelper::compare);
		RenderSortHelper::clearSorter();

		FILL_MODE mode = view->getFillMode();
		svc.onShaderSwitch(true);
		//BLADE_LW_PROFILING(DRAW);
		for(size_t i = 0; i < mixedOperations.size(); ++i)
		{
			const MixedRenderOP& op = mixedOperations[i];
			svc.setRenderOperation(&op.rop);

			if(svc.setRenderProperties(op.passProperties, NULL, op.rop.material->getRenderProperty()) )
				rsm.applyRenderProperty(device, *svc.getRenderProperty(), mode);

			const SamplerState* samplers = op.samplers;
			rsm.applyRenderSettings(device, op.shader, op.rop.material->getTextureStateInstance(), samplers);
			device->renderGeometry( *op.rop.geometry );
		}
	}


}//namespace Blade
