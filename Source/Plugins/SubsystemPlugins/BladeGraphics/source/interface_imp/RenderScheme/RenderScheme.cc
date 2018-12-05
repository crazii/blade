/********************************************************************
	created:	2010/05/06
	filename: 	RenderScheme.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderScheme.h"
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>

#include <interface/ISpaceCoordinator.h>
#include <interface/IGraphicsSystem.h>
#include <interface/ICamera.h>
#include <interface/ILightManager.h>
#include <interface/ICameraManager.h>
#include <interface/IShaderVariableSource.h>
#include <interface/IMaterialManager.h>
#include <interface/IRenderTypeManager.h>
#include <interface/public/IImageEffect.h>
#include <utility/Profiling.h>

#include <Material.h>
#include <Technique.h>
#include <Pass.h>
#include "RenderPipeline.h"
#include "../View.h"
#include "ImageEffectOutput.h"
#include "RenderSchemeConfig.h"

namespace Blade
{
	RenderScheme::SharedBufferMap	RenderScheme::msSharedBuffers;
	size_t							RenderScheme::msViewID = 0;
	bool							RenderScheme::msUseShareBuffer = false;

	//////////////////////////////////////////////////////////////////////////
	RenderScheme::RenderScheme(const SCHEME_DESC* desc)
		:mPrevListener(NULL)
		,mDesc(desc)
		,mLightManager(NULL)
		,mCameraManager(NULL)
		,mFinalOutput(NULL)
		,mFinalView(NULL)
		,mMainCamera(NULL)
		,mIFXHelper(NULL)
		,mMaxMRT(1)
		,mProfileIndex(0)
	{

		for (OutputDescList::const_iterator i = desc->mOutputList.begin(); i != desc->mOutputList.end(); ++i)
		{
			const OUTPUT_DESC& outputDesc = *(*i);
			RenderOutput* output;

			if (outputDesc.mHasImageEfect)
				output = BLADE_NEW ImageEffectOutput(&outputDesc);
			else
				output = BLADE_NEW RenderOutput(&outputDesc);

			IGraphicsView* view;
			if (outputDesc.mViewDesc.mViewRef == VR_FINAL)
			{
				view = NULL;
			}
			else if (outputDesc.mViewDesc.mViewRef == VR_MANUAL)
			{
				const TString DEFAULT = BTString("[renderscheme]default_view");

				const TString viewName = DEFAULT + TStringHelper::fromUInt(msViewID);
				Lock::increment(&msViewID);

				view = IGraphicsSystem::getSingleton().createView(viewName);
				const VIEW_DESC& viewDesc = outputDesc.mViewDesc;
				view->setBackgroundColor(viewDesc.mClearColor);
				view->setClearMode(viewDesc.mClearMode, viewDesc.mClearDepth, viewDesc.mClearStencil);
				view->setSizePosition(viewDesc.mLeft, viewDesc.mTop, viewDesc.mRight, viewDesc.mBottom,
					viewDesc.mLeftPixel, viewDesc.mTopPixel, viewDesc.mRightPixel, viewDesc.mBottomPixel);
				output->setView(view);
			}
			else
			{
				assert(false);
				view = NULL;
			}

			for (size_t j = 0; j < outputDesc.mStepDescList.size(); ++j)
			{
				const STEP_DESC& stepDesc = *(outputDesc.mStepDescList[j]);
				RenderStep* step = BLADE_NEW RenderStep(&stepDesc);
				output->addRenderStep(step);
			}
			this->addRenderOutput(output);
		}

		for (size_t i = 0; i < desc->mOperatorList.size(); ++i)
		{
			const OPERATOR_DESC& opDesc = desc->mOperatorList[i];
			IRenderSchemeOperation* op = BLADE_FACTORY_CREATE(IRenderSchemeOperation, opDesc.mType);
			op->setTargetPrefix(opDesc.mPrefix);

			const HCONFIG& cfg = RenderSchemeConfig::getSingleton().getOperationConfig(opDesc.mType);
			if (cfg != NULL)
				cfg->synchronize(op);
			mOperators[opDesc.mType] = op;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	RenderScheme::~RenderScheme()
	{
		if( mFinalOutput != NULL && mFinalOutput->getListener() == this )
			mFinalOutput->setListener(mPrevListener);

		for(size_t i = 0; i < mOutputList.size(); ++i)
			BLADE_DELETE mOutputList[i];

		if (!msUseShareBuffer)
			this->getBuffers().clear();

		for (OperatorList::const_iterator i = mOperators.begin(); i != mOperators.end(); ++i)
		{
			i->second->shutdown(NULL, NULL);
			BLADE_DELETE i->second;
		}

		for (RenderTargetMap::iterator i = mRenderTargets.begin(); i != mRenderTargets.end(); ++i)
			mIFXHelper->getDevice()->destroyRenderTarget(i->second->getName());

		mIFXHelper->shutdown();
		BLADE_DELETE mIFXHelper;
	}

	/************************************************************************/
	/* IRenderScheme interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&RenderScheme::getBuffer(index_t index) const
	{
		const BufferMap& map = this->getBuffers();
		if( index >= map.size() )
			return HTEXTURE::EMPTY;

		BufferMap::const_iterator i = map.begin();
		std::advance(i, index);
		return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&RenderScheme::getBuffer(const TString& name) const
	{
		const BufferMap& map = this->getBuffers();
		BufferMap::const_iterator i = map.find( name );
		if( i != map.end() )
			return i->second;
		else
			return HTEXTURE::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderScheme::updateCulling(ICamera* mainCamera/* = NULL*/) const
	{
		if( mainCamera == NULL )
			mainCamera = mMainCamera;

		mainCamera->update();

		for (OperatorList::const_iterator i = mOperators.begin(); i != mOperators.end(); ++i)
			i->second->preCull(mainCamera, mCameraManager, mLightManager);

		size_t outputCount = mOutputList.size();

		for(size_t i = 0; i < outputCount; ++i)
		{
			RenderOutput* output = mOutputList[i];
			if( !output->isEnabled() )
				continue;

			ICamera* camera;

			if( output->getDesc().mCameraDesc.mCameraRef == CR_MAIN )
				camera = mainCamera;
			else
			{
				//get camera from camera manager
				camera = mCameraManager->getCamera( output->getDesc().mCameraDesc.mCameraName );
				assert(camera);
			}
			camera->update();	//culling
		}

		//update shadow view projection
		for (OperatorList::const_iterator i = mOperators.begin(); i != mOperators.end(); ++i)
			i->second->postCull(mainCamera, mCameraManager, mLightManager);

		//update profile 
		//note: if multiple runtime scheme is supported (different view has different scheme)
		//we need setActiveTechnique for each Scheme's render
		//since camera render buffer need material's current active technique before render,
		//we setActiveTechnique for each scheme's update culling.
		IRenderTypeManager& rtm = IRenderTypeManager::getSingleton();
		for(TypeDescList::const_iterator iter = mDesc->mTypeList.begin(); iter != mDesc->mTypeList.end(); ++iter)
		{
			RenderType* type = rtm.getRenderType( iter->mRenderTypeName );
			if( type == NULL )
				continue;

			bool profileUpdated = false;
			Material* material = type->updateProfile(this->getProfile(), profileUpdated);
			if( material == NULL )
				continue;

			for(size_t i = 0; i < outputCount; ++i )
			{
				RenderOutput* output = mOutputList[i];
				if( !output->isEnabled() )
					continue;
				for(size_t j = output->getRenderStepByRenderType(0, iter->mRenderTypeName); j != INVALID_INDEX; j = output->getRenderStepByRenderType(j+1, iter->mRenderTypeName))
				{
					RenderStep* step = output->getRenderStep(j);
					if(step == NULL)
						continue;

					const index_t* groupIndices = NULL;
					if(profileUpdated || step->getShaderGroups(groupIndices) == 0)
						step->setShaderGroups( material->getActiveTechnique() );

					const STEP_DESC* stepDesc = step->getDesc();
					//note: empty pass indicates objects will be drawn in dynamic pass/shaders, i.e. effects
					//@RenderOutput::renderOutput
					if( stepDesc->mPassName == TString::EMPTY )
						continue;

					if( (!profileUpdated && step->getMaterialPass() != NULL) )
						continue;

					Technique* tech = material->getActiveTechnique();
					if(tech != NULL )
						step->setMaterialPass( tech->getPass(stepDesc->mPassName) );
					else
						BLADE_LOG(Warning, BTString("not suitable profile for render type:") + type->getName());
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderScheme::render(IRenderDevice* device, bool swapBuffer,
		IRenderTarget* target/* = NULL*/, IGraphicsView* view/* = NULL*/, ICamera* camera/* = NULL*/)
	{
		//setup target
		IRenderTarget* finalOutput;
		if( target != NULL )
			finalOutput = target;
		else
			finalOutput = mFinalOutput;

		assert( static_cast<View*>(mFinalView)->getAttachedRenderScheme() == this );

		//setup view
		View* finalView;
		IRenderScheme* lastScheme = NULL;
		if( view != NULL && mFinalView != view)
		{
			View* newView = static_cast<View*>(view);
			finalView = newView;
			lastScheme = newView->attachRenderScheme(this);
		}
		else
			 finalView = static_cast<View*>(mFinalView);
		//setup camera
		ICamera* mainCamera;
		if( camera != NULL && camera != mMainCamera )
		{
			camera->update();
			mainCamera = camera;
		}
		else
			mainCamera = mMainCamera;

		RenderPipeline* pipeline = static_cast<RenderPipeline*>( finalView->getRenderPipeline() );
		const RenderPipeline::PhaseList& phaseList = pipeline->getPhaseList();
		if( phaseList.size() == 0 )
			return;

		ShaderVariableSource& svc = ShaderVariableSource::getSingleton();

		svc.beginFrame();
		//setup per scene light-manager
		svc.setLightManager(mLightManager);
		//prepare operators
		for (OperatorList::const_iterator i = mOperators.begin(); i != mOperators.end(); ++i)
			i->second->preRender(mainCamera);
		//prepare post effects
		mIFXHelper->preRender(finalOutput, mainCamera, finalView);

		//render outputs
		for(size_t i = 0; i < phaseList.size(); ++i)
		{
			RenderPhase* phase = phaseList[i];

			if( !phase->isEnabled() )
				continue;
			RenderOutput* output = phase->getRenderOutput();
			const RenderPhase::StepInstanceList& list = phase->getStepInstances();
			if( list.empty() && !output->hasImageEffect() )
				continue;

			//phase target
			IRenderTarget* phaseTarget = output->getTarget();
			assert(phaseTarget != NULL);

			//phase view
			IGraphicsView* phaseView = output->getView();
			if( phaseView == NULL )
				phaseView = finalView;
			phaseView->calculateSizeInPixel(phaseTarget);

			//phase camera
			ICamera* phaseCamera;
			if( output->getDesc().mCameraDesc.mCameraRef == CR_MAIN )
				phaseCamera = mainCamera;
			else
				//get camera from camera manager
				phaseCamera = mCameraManager->getCamera( output->getDesc().mCameraDesc.mCameraName );
			assert(phaseCamera != NULL);
			scalar width = scalar(phaseView->getPixelRight()-phaseView->getPixelLeft());
			scalar height = scalar(phaseView->getPixelBottom()-phaseView->getPixelTop());
			phaseCamera->setAspectRatio(width, height);

			if (!output->hasImageEffect()) //image effect may have target switches and they set on their own
			{
				device->setRenderTarget(phaseTarget);
				device->setRenderView(phaseView);
				const VIEW_DESC& desc = output->getDesc().mViewDesc;
				if (desc.mClearMode != 0)
					device->clear(desc.mClearMode, desc.mClearColor, desc.mClearDepth, desc.mClearStencil);
			}
			//BLADE_LW_PROFILING_TAG(OUTPUT, output->getName());
			svc.setRenderOutput(output);
			//if list is empty, it should be an image effect
			output->renderOutput(device, phaseCamera, phaseView, list.size() > 0 ? &list[0] : NULL, list.size());
		}

		//clear shader variable bindings
		svc.setRenderProperties(NULL, NULL, NULL);
		svc.setRenderOperation(NULL);
		svc.setView(NULL);
		svc.setCamera(NULL);
		svc.setRenderOutput(NULL);
		svc.setLightManager(NULL);

		//finish operators
		for (OperatorList::const_iterator i = mOperators.begin(); i != mOperators.end(); ++i)
			i->second->postRender(mainCamera);
		//finish post effects
		mIFXHelper->postRender(finalOutput->getColorBuffer(0));

		if( finalView != mFinalView )
			finalView->attachRenderScheme(lastScheme);

		if( swapBuffer )
			finalOutput->swapBuffers();
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderScheme::setupRender(IRenderDevice* device, IRenderTarget* target, IGraphicsView* view, ICamera* camera,
		ILightManager* lightManager, ICameraManager* cameraManager)
	{
		if (mFinalOutput == target && mFinalView == view && mMainCamera == camera && mLightManager == lightManager && mCameraManager == cameraManager)
		{
			return;
		}

		if( mFinalOutput != NULL )
			mFinalOutput->setListener(mPrevListener);

		mPrevListener = NULL;
		mFinalOutput = target;
		mFinalView = view;
		static_cast<View*>(mFinalView)->attachRenderScheme( this );
		mMainCamera = camera;
		mLightManager = lightManager;
		mCameraManager = cameraManager;
		mMaxMRT = (uint16)device->getDeviceCaps().mMaxMRT;

		if( target == NULL && mDesc->mUseFinalTarget )
			BLADE_EXCEPT(EXC_NEXIST, BTString("render target needed"));
		else if(target != NULL )
			mPrevListener = target->setListener(this);

		//render target's HDR/LDR will be updated here, and HDR/LDR info will be used as a key to share RTs
		if (mIFXHelper == NULL)
		{
			mIFXHelper = BLADE_NEW ImageEffectHelper(&this->getProfile());
			mIFXHelper->initialize(device, mOutputList.size() > 0 ? &mOutputList[0] : NULL, mOutputList.size());
			mIFXHelper->setEnable(mFinalView->isImageEffectEnabled());
		}

		//create views & targets
		size_t outputCount = mOutputList.size();
		for(size_t i = 0; i < outputCount; ++i)
		{
			RenderOutput* output = mOutputList[i];
			if( !output->isUsingFinalView() )
				assert( output->getView() != NULL );

			if( output->isUsingCustomCamera() && output->getCustomCamera() == NULL)
			{
				ICamera* cam = IGraphicsSystem::getSingleton().createCamera();
				IGraphicsView* v = output->isUsingFinalView() ? mFinalView : output->getView();
				output->setCustomCamera(cam, v);
			}

			if (output->getTarget() == NULL)
			{
				IRenderTarget* output_target = this->getRenderTarget(device, output, target->getViewWidth(), target->getViewHeight());
				output->setTarget(output_target);
			}
			else
			{
				output->getTarget()->setViewWidth(target->getViewWidth());
				output->getTarget()->setViewHeight(target->getViewHeight());
			}
		}

		for (OperatorList::const_iterator i = mOperators.begin(); i != mOperators.end(); ++i)
		{
			IRenderSchemeOperation* op = i->second;
			op->shutdown(mCameraManager, mLightManager);

			const TString& prefix = op->getTargetPrefix();
			TempVector<IRenderPhase*> phases;
			for (size_t j = 0; j < view->getPhaseCount(); ++j)
			{
				IRenderPhase* phase = static_cast<IRenderPhase*>(view->getPhase(j));
				if (TStringHelper::isStartWith(phase->getName(), prefix))
					phases.push_back(phase);
			}
			op->initialize(mCameraManager, mLightManager, phases.size() > 0 ? &phases[0] : NULL, phases.size());
		}

		if(target->getViewWidth() != 0 && target->getViewHeight() != 0)
			this->rebindBuffers();
		else
		{
			//probably on render window closing, and restoring render window to default
		}
	}

	/************************************************************************/
	/* render/output control                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IRenderPipeline*	RenderScheme::createPipeline()
	{
		RenderPipeline* pipeline = BLADE_NEW RenderPipeline(this);
		if( mOutputList.size() > 0 )
			pipeline->initialize( &mOutputList[0], mOutputList.size() );
		return pipeline;
	}

	/************************************************************************/
	/* IRenderTarget::IListener interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			RenderScheme::onTargetSizeChange(IRenderTarget* target)
	{
		//buffers may have changed, release buffer reference 
		size_t outputCount = mOutputList.size();
		for(size_t i = 0; i < outputCount; ++i )
		{
			RenderOutput* output = mOutputList[i];
			IRenderTarget* t = output->getTarget();

			if( t != NULL && t != target)
			{
				t->setViewWidth(target->getViewWidth());
				t->setViewHeight(target->getViewHeight());
			}
			if( output->isUsingEmptyTarget() )
				continue;

			t->setDepthBuffer(HTEXTURE::EMPTY);
			size_t MRT_Count = t->getColorBufferCount();
			for(size_t j = 0; j < MRT_Count; ++j)
				t->setColorBuffer(j, HTEXTURE::EMPTY);
		}

		this->rebindBuffers();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			RenderScheme::addRenderOutput(RenderOutput* output)
	{
		if( output == NULL )
			return false;
		else
		{
			mOutputList.push_back(output);
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	RenderScheme::getRenderTarget(IRenderDevice* device, const RenderOutput* output, size_t width, size_t height)
	{
		//note: render targets are shared if they have same color & depth output. on some platform it has optimizations, i.e. on GL it may eliminate succeeding FBO switches for CSM
		const TargetDescList& descList = output->getDesc().mTargetDescList;
		assert(mInputBufferID.size() < 64 );	//reserve 1 bit for HDR/LDR: same output before/after HDR is different: they have different texture attachments

		Mask64 outputMask;
		if (output->isLDR())
			outputMask.raiseBitAtIndex(63);

		for (size_t i = 0; i < descList.size(); ++i)
		{
			const BUFFER_DESC* desc = descList[i];
			const TString& name = desc->mName;
			index_t id = mInputBufferID.find(name);
			if (id == INVALID_INDEX)
			{
				id = mInputBufferID.size();
				mInputBufferID.push_back(name);
			}
			assert(id < 63);
			outputMask.raiseBitAtIndex(id);
		}

		const BUFFER_DESC* desc = output->getDesc().mDepthTarget;
		if (desc != NULL)
		{
			const TString& name = desc->mName;
			index_t id = mInputBufferID.find(name);
			if (id == INVALID_INDEX)
			{
				id = mInputBufferID.size();
				mInputBufferID.push_back(name);
			}
			assert(id < 63);
			outputMask.raiseBitAtIndex(id);
		}

		IRenderTarget*& target = mRenderTargets[outputMask];
		if(target == NULL)
			target = device->createRenderTarget(output->getName() + BTString("_of_") + TStringHelper::fromHex(uintptr_t(this)), width, height);
		return target;
	}

	////////////////////////////////////////////////////////////////////////////
	void			RenderScheme::rebindBuffers(bool checkIFXBufferOnly/* = false*/)
	{
		TempTStringMap<HTEXTURE> oldMap;
		BufferMap& usedBuffers = this->getBuffers();
		oldMap.insert(usedBuffers.begin(), usedBuffers.end());
		if(!checkIFXBufferOnly)
			usedBuffers.clear();

		//count image effect buffers
		ImageEffectHelper::BufferRefCount IFXBuffers;
		if(mFinalView->isImageEffectEnabled())
			mIFXHelper->updateBufferInfo(IFXBuffers);

		HTEXTURE finalBuffer, finalLDR;
		mIFXHelper->createFinalBuffer(mFinalOutput, finalBuffer, finalLDR);

		//create render buffer
		{
			IRenderTarget* target = mFinalOutput;

			const BufferDescList& bufferDescList = mDesc->mBufferList;
			for (BufferDescList::const_iterator i = bufferDescList.begin(); i != bufferDescList.end(); ++i)
			{
				const BUFFER_DESC& desc = *i;

				if (desc.mBufferUsage == BU_IMAGEFX) //image fx only buffers
				{
					if (IFXBuffers[desc.mName] == 0)
					{
						if (checkIFXBufferOnly)		//remove buffers if image fx disabled
							usedBuffers.erase(desc.mName);
						continue;
					}
				}
				if(checkIFXBufferOnly && usedBuffers[desc.mName] != NULL)	//skip existing buffer
					continue;

				size_t scaledWidth = 0;
				size_t scaledHeight = 0;
				if (desc.mWidthScale != 0 || desc.mHeightScale != 0)
				{
					assert(target != NULL);
					scaledWidth = (size_t)(target->getViewWidth()*desc.mWidthScale);
					scaledHeight = (size_t)(target->getViewHeight()*desc.mHeightScale);
				}
			
				if (desc.mType == RBT_COLOR)
				{
					size_t width = scaledWidth + desc.mWidth;
					size_t height = scaledHeight + desc.mHeight;
					PixelFormat format = desc.mFormat;
					Sampler sampler = Sampler::DEFAULT_RTT;

					for (OperatorList::const_iterator iter = mOperators.begin(); iter != mOperators.end(); ++iter)
					{
						if (TStringHelper::isStartWith(desc.mName, iter->second->getTargetPrefix()))
							iter->second->adjustBuffer(desc.mName, width, height, format, sampler);
					}

					HTEXTURE& oldTex = oldMap[desc.mName];
					if (oldTex == NULL || oldTex->getWidth() || oldTex->getWidth() != width || oldTex->getHeight() != height || oldTex->getPixelFormat() != format
						|| !oldTex->getTextureUsage().isRenderTarget())
					{
						HTEXTURE tex = IGraphicsResourceManager::getSingleton().createTexture(width, height, format, 1,
							IGraphicsBuffer::GBUF_RENDERTARGET, TT_2DIM, IMGO_DEFAULT, 1);
						if (sampler != Sampler::DEFAULT_RTT)
							tex->setSampler(sampler);
						usedBuffers[desc.mName] = tex;
					}
					else
						usedBuffers[desc.mName] = oldTex;
				}
				else if (desc.mType == RBT_DEPTH)
				{
					size_t width = scaledWidth + desc.mWidth;
					size_t height = scaledHeight + desc.mHeight;
					PixelFormat format = desc.mFormat;
					//depth surface must >= color surface's size,
					//if depth buffers not paired by final target, then they are guaranteed by user to make the size match.
					if ( (desc.mBufferUsage&BU_FINAL) )
					{
						assert(target != NULL);
						width = std::max(width, target->getWidth());
						height = std::max(height, target->getHeight());
					}

					Sampler sampler = Sampler::DEFAULT_RTT_DEPTH;

					for (OperatorList::const_iterator iter = mOperators.begin(); iter != mOperators.end(); ++iter)
					{
						if (TStringHelper::isStartWith(desc.mName, iter->second->getTargetPrefix()))
							iter->second->adjustBuffer(desc.mName, width, height, format, sampler);
					}

					HTEXTURE& oldTex = oldMap[desc.mName];
					if (oldTex == NULL || oldTex->getWidth() < width || oldTex->getHeight() < height ||
						oldTex->getPixelFormat() != format || oldTex->getGpuAccess() != desc.mGpuAccess)
					{
						HTEXTURE tex = IGraphicsResourceManager::getSingleton().createDepthStencilBuffer(width, height, format, desc.mGpuAccess);
						if(sampler != Sampler::DEFAULT_RTT_DEPTH)
							tex->setSampler(sampler);
						usedBuffers[desc.mName] = tex;
					}
					else
						usedBuffers[desc.mName] = oldTex;
				}
			}
		}

		size_t outputCount = mOutputList.size();
		for(size_t i = 0; i < outputCount; ++i )
		{
			RenderOutput* output = mOutputList[i];
			IRenderTarget* target = output->getTarget();
			assert(target != NULL);
			if( output->isUsingFinalTarget() )
			{
				target->setColorBuffer(0, output->isLDR() ? finalLDR : finalBuffer);
			}
			else if( output->isUsingEmptyTarget() )
			{
				//nothing to do
			}
			else
			{
				const TargetDescList& descList = output->getDesc().mTargetDescList;
				if( descList.size() > mMaxMRT )
					BLADE_EXCEPT(EXC_API_ERROR, BTString("render output has rendertargets more than device can handle."));
				target->setColorBufferCount(descList.size());

				for(size_t j = 0; j < descList.size(); ++j)
				{
					const BUFFER_DESC& desc = *descList[j];
					const HTEXTURE& buffer = usedBuffers[desc.mName];
					assert(buffer != NULL);
					if(!output->hasImageEffect())
						assert(buffer->getWidth() >= target->getViewWidth() && buffer->getHeight() >= target->getViewHeight());
					target->setColorBuffer(j, buffer);
				}
			}

			const BUFFER_DESC* depthDesc = output->getDesc().mDepthTarget;
			if( depthDesc != NULL )
			{
				const HTEXTURE& depthBuffer = usedBuffers[depthDesc->mName];
				assert(depthBuffer != NULL);
				target->setDepthBuffer(depthBuffer);

				//special handling of render target size. TODO: better ways
				if (output->isUsingEmptyTarget())
				{
					assert(output->getDesc().mTargetDescList.size() == 1);
					const TString& name = output->getDesc().mTargetDescList[0]->mName;
					assert(name == BTString("NONE") || name == BTString("EMPTY"));
					if (name == BTString("EMPTY"))
					{
						target->setViewWidth(depthBuffer->getWidth());
						target->setViewHeight(depthBuffer->getHeight());
					}
				}
				assert(depthBuffer->getWidth() >= target->getViewWidth() && depthBuffer->getHeight() >= target->getViewHeight());
			}

			const InputDescList& inputList = output->getDesc().mInputDescList;
			size_t j;
			for(j = 0; j < inputList.size(); ++j)
			{
				const BUFFER_DESC* desc = inputList[j];
				assert(desc->mType != RBT_NONE);
				output->setInputBuffer(j, usedBuffers[desc->mName] );
			}
			for(size_t k = j; k < MAX_INPUT_COUNT; ++k)
				output->setInputBuffer(k, HTEXTURE::EMPTY);
		}//for
	}

	//////////////////////////////////////////////////////////////////////////
	void			RenderScheme::onImageEffectEnable(void* /*data*/)
	{
		this->rebindBuffers(true);
	}

	//////////////////////////////////////////////////////////////////////////
	RenderScheme::BufferMap&		RenderScheme::getBuffers()
	{
		if (!msUseShareBuffer)
			return mBufferMap;
		return msSharedBuffers[mDesc];
	}

}//namespace Blade