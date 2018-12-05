/********************************************************************
	created:	2017/02/02
	filename: 	ImageEffectOutput.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ImageEffectOutput.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include "../Material/MaterialManager.h"

namespace Blade
{
	const TString ImageEffect_Blit::BLIT_NAME = BTString("blit");

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	ImageEffectHelper::ImageEffectHelper(const TString* profile)
		:mProfile(profile)
	{
		mImageTarget = NULL;
		mDevice = NULL;
		mMainCamera = NULL;
		mMainView = NULL;
		mBlitMaterial = NULL;
		mBlurMaterial = NULL;
		mInputIndex = 0;

		mPrevView = Matrix44::IDENTITY;
		mPrevProjection = Matrix44::IDENTITY;
		mReProjectionMatrix = Matrix44::IDENTITY;

		mHasImageFX = false;
		mHasHDR = false;
		mLDRIFX = false;
		mResultChanged = false;
		mInitReprojection = true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool ImageEffectHelper::ensureBuffer(HTEXTURE& buffer, PixelFormat format, size_t width/* = -1*/, size_t height/* = -1*/, size_t mip/* = 1*/)
	{
		IRenderTarget* target = mImageTarget;
		if (width == size_t(-1))
			width = target->getViewWidth();
		if (height == size_t(-1))
			height = target->getViewHeight();

		if (buffer == NULL || buffer->getWidth() != width || buffer->getHeight() != height || buffer->getPixelFormat() != format)
		{
			if(format.isDepth())
				buffer = IGraphicsResourceManager::getSingleton().createDepthStencilBuffer(width, height, format, IGraphicsBuffer::GBAF_READ_WRITE);
			else
				buffer = IGraphicsResourceManager::getSingleton().createTexture(width, height, format, (int)mip, IGraphicsBuffer::GBUF_RENDERTARGET);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::updateBufferInfo(BufferRefCount& outCounts)
	{
		mHasImageFX = false;
		mHasHDR = false;
		for (size_t i = 0; i < mList.size(); ++i)
		{
			ImageEffectOutput* output = mList[i];
			IImageEffect* ifx = output->getEffect();
			if (!ifx->isEnabled() || !output->isEnabled())
				continue;
			mHasImageFX = true;

			if (ifx->getAttributes()&IImageEffect::IFXA_HDR2LDR)
				mHasHDR = true;
			else if(mHasHDR)
				mLDRIFX = true;

			const IMAGEFX_DESC& desc = output->getDesc();
			for (size_t j = 0; j < desc.mTargetDescList.size(); ++j)
			{
				if (desc.mTargetDescList[j]->mBufferUsage == BU_IMAGEFX)	//image effect only buffer
					++outCounts[desc.mTargetDescList[j]->mName];
			}

			for (size_t j = 0; j < desc.mInputDescList.size(); ++j)
			{
				if (desc.mInputDescList[j]->mBufferUsage == BU_IMAGEFX)	//image effect only buffer
					++outCounts[desc.mInputDescList[j]->mName];
			}

			if (desc.mDepthTarget != NULL && desc.mDepthTarget->mBufferUsage == BU_IMAGEFX)
				++outCounts[desc.mDepthTarget->mName];
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::createFinalBuffer(const IRenderTarget* finalTarget, HTEXTURE& output, HTEXTURE& outputLDR)
	{
		if (mHasImageFX)
		{
			PixelFormat finalFmt = finalTarget->getPixelFormat(0);

			if (mHasHDR)
			{
				this->ensureBuffer(mFinal, PF_R16G16B16A16F, finalTarget->getViewWidth(), finalTarget->getViewHeight());
				this->ensureBuffer(mFinalLDR, finalFmt, finalTarget->getViewWidth(), finalTarget->getViewHeight());
			}
			else
			{
				this->ensureBuffer(mFinal, finalFmt, finalTarget->getViewWidth(), finalTarget->getViewHeight());
				outputLDR = HTEXTURE::EMPTY;
			}
		}
		else
		{
			mFinal = mFinalLDR = finalTarget->getColorBuffer(0);	//this keeps a reference of back buffer. TODO?
			mChainBuffer.clear();
			mChainBufferLDR.clear();
		}

		output = mFinal;
		outputLDR = mFinalLDR;
		mCache.clear();
	}

#if BUFFER_CONFLIT_RESOLVE
	//////////////////////////////////////////////////////////////////////////
	struct FnConflictFinder
	{
		const TString* target;
		FnConflictFinder(const TString* str) :target(str) {}
		bool operator()(TString const*const elem) const { return *target == *elem; }
	};
	void	ImageEffectHelper::preapreStorage(const BufferMap& buffers)
	{
		typedef TempVector<const TString*> ConflictList;
		ConflictList conflicts;
		conflicts.reserve(16);

		for (size_t i = 0; i < mList.size(); ++i)
		{
			ImageEffectOutput* output = mList[i];
			IImageEffect* ifx = output->getEffect();
			if (output->isEnabled() && ifx->isEnabled())
			{
				const TStringList& _conflicts = output->getBufferConflicts();
				for (size_t j = 0; j < _conflicts.size(); ++j)
					conflicts.push_back(&_conflicts[j]);
			}
		}

		for (BufferMap::const_iterator i = buffers.begin(); i != buffers.end(); ++i)
		{
			const TString& name = i->first;
			const HTEXTURE& buffer = i->second;
			STORAGE& store = mStorage[name];
			store.first = &buffer;

			ConflictList::iterator iter = std::find_if(conflicts.begin(), conflicts.end(), FnConflictFinder(&name));
			if (iter != conflicts.end())
			{
				this->ensureBuffer(store.storage, buffer->getPixelFormat(), buffer->getWidth(), buffer->getHeight(), buffer->getMipmapCount());
				store.second = &store.storage;
			}
			else
			{
				store.second = NULL;
				store.storage.clear();	//clear storage if image fx disabled.
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::setupInputOutput(ImageEffectOutput& ifxout, TempVector<HTEXTURE>& input, TempVector<HTEXTURE>& output, const HTEXTURE*& depth)
	{
		const IMAGEFX_DESC& desc = ifxout.getDesc();

		//set up input
		for (size_t i = 0; i < desc.mInputDescList.size(); ++i)
		{
			const BUFFER_DESC* buffer = desc.mInputDescList[i];
			assert(buffer->mBufferRef != BR_EMPTY);	//empty input?
			BufferStorage::const_iterator iter = mStorage.find(buffer->mName);
			if (iter == mStorage.end() || iter->second.first == NULL)
			{
				assert(false);
				continue;
			}
			ifxout.setInputBuffer(i, *iter->second.first);
			input.push_back(*iter->second.first);
		}

		//swap buffer for input/output conflicts
		for (size_t i = 0; i < ifxout.getBufferConflicts().size(); ++i)
		{
			const TString& conflit = ifxout.getBufferConflicts()[i];
			BufferStorage::iterator iter = mStorage.find(conflit);
			assert(iter != mStorage.end() && iter->second.second != NULL);
			std::swap(iter->second.first, iter->second.second);
		}

		//gather output
		for (size_t i = 0; i < desc.mTargetDescList.size(); ++i)
		{
			const BUFFER_DESC* buffer = desc.mTargetDescList[i];
			if (buffer->mBufferRef == BR_EMPTY)
				output.push_back(HTEXTURE::EMPTY);
			{
				BufferStorage::const_iterator iter = mStorage.find(buffer->mName);
				if (iter == mStorage.end() || iter->second.first == NULL)
				{
					assert(false);
					continue;
				}
				output.push_back(*iter->second.first);
			}
		}

		if (desc.mDepthTarget != NULL)
		{
			BufferStorage::const_iterator iter = mStorage.find(desc.mDepthTarget->mName);
			if (iter == mStorage.end())
				assert(false);
			else
				depth = iter->second.first;
		}
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::preRender(IRenderTarget* finalTarget, ICamera* mainCam, IGraphicsView* mainView)
	{
		if (!mHasImageFX)
			return;

		this->calculateReprojectionMatrix(mainCam);

		this->ensureBuffer(mChainBuffer, mFinal->getPixelFormat(), mFinal->getWidth(), mFinal->getHeight());
		if(mLDRIFX)
			this->ensureBuffer(mChainBufferLDR, finalTarget->getPixelFormat(), mFinal->getWidth(), mFinal->getHeight());
		else
			mChainBufferLDR.clear();

		mImageFXChain[0] = mFinal;
		mImageFXChain[1] = mChainBuffer;
		mResultChanged = false;

#if BUFFER_CONFLIT_RESOLVE
		this->preapreStorage()
#endif

		mainView->calculateSizeInPixel(finalTarget);
		scalar width = scalar(mainView->getPixelRight() - mainView->getPixelLeft());
		scalar height = scalar(mainView->getPixelBottom() - mainView->getPixelTop());
		mainCam->setAspectRatio(width, height);

		mMainCamera = mainCam;
		mMainView = mainView;
		mImageTarget->setViewWidth(finalTarget->getViewWidth());
		mImageTarget->setViewHeight(finalTarget->getViewHeight());

		for (size_t i = 0; i < mList.size(); ++i)
		{
			if(mList[i]->isEnabled())
				mList[i]->preRender(mainCam, mainView);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::postRender(const HTEXTURE& outputTarget)
	{
		//copy result to final buffer
		if (mHasImageFX)
		{
			assert(mImageFXChain[0] != outputTarget);
			this->blit(mImageFXChain[0], outputTarget);
			for (size_t i = 0; i < mList.size(); ++i)
			{
				if (mList[i]->isEnabled())
					mList[i]->postRender();
			}

			this->updateCache();
		}

		assert(mResultChanged == false);
		mImageFXChain[0] = HTEXTURE::EMPTY;
		mInputIndex = 0;

		mMainCamera = NULL;
		mMainView = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::initialize(IRenderDevice* device, RenderOutput** outputs, size_t outputCount)
	{
		TempVector<ImageEffectOutput*> list;
		bool hasHDR = false;	//whether has HDR IFX, even if it is not enabled

		for (size_t i = 0; i < outputCount; ++i)
		{
			RenderOutput& output = *outputs[i];
			if(hasHDR)
				output.setLDR();

			if(!output.hasImageEffect())
				continue;

			ImageEffectOutput* ifxOut = static_cast<ImageEffectOutput*>(&output);
			list.push_back(ifxOut);
			if (i + 1 < outputCount)
				ifxOut->setNextEffect(outputs[i + 1]->hasImageEffect());

			if(ifxOut->getEffect()->getAttributes()&IImageEffect::IFXA_HDR2LDR )
				hasHDR = true;
		}

		mList.resize(list.size());
		for (size_t i = 0; i < list.size(); ++i)
		{
			mList[i] = list[i];
			mList[i]->initialize(this);
		}

		assert(mImageTarget == NULL);
		mImageTarget = device->createRenderTarget(BTString("ImageFX Target") + TStringHelper::fromPointer(this), 0, 0);
		mDevice = device;

		mBlitMaterial = MaterialManager::getSingleton().getMaterial(BTString("blit_copy"));
		mBlitMaterial->loadSync();

		mBlurMaterial = MaterialManager::getSingleton().getMaterial(BTString("gaussian_blur"));
		mBlurMaterial->loadSync();

		//setup Gaussian offsets & weights
		Vector4 bloomWeights[15];
		Vector2	bloomOffsets[14];
		float deviation = 3.0f;

		// central sample, no offset
		bloomWeights[14] = Vector4::UNIT_ALL * Math::GaussianDistribution(0, 0, deviation);
		bloomWeights[14][3] = 1.0f;

		for (int i = 0; i < 7; ++i)
		{
			bloomWeights[i] = Vector4::UNIT_ALL * (1.25f * Math::GaussianDistribution((scalar)(i + 1), 0, deviation));
			bloomWeights[i][3] = 1.0f;
			bloomOffsets[i] = Vector2::UNIT_ALL * (i + 1.0f);

			bloomWeights[i + 7] = bloomWeights[i];
			bloomOffsets[i + 7] = -bloomOffsets[i];
		}

		mBlurMaterial->getActiveTechnique()->setShaderVariable(BTString("sampleWeights"), SCT_FLOAT4, countOf(bloomWeights), bloomWeights);
		mBlurMaterial->getActiveTechnique()->setShaderVariable(BTString("sampleOffsets"), SCT_FLOAT2, countOf(bloomOffsets), bloomOffsets);
		mBlurMaterial->getActiveTechnique()->getShaderVariableHandle(mBlurDirHandle, BTString("sampleDir"), SCT_FLOAT2, 1);
		mBlurMaterial->getActiveTechnique()->getShaderVariableHandle(mBlurSharpnessHandle, BTString("sharpness"), SCT_FLOAT1, 1);
		assert(mBlurDirHandle.isValid());

	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::shutdown()
	{
		if (mImageTarget != NULL)
		{
			bool ret = mDevice->destroyRenderTarget(mImageTarget->getName());
			assert(ret); BLADE_UNREFERENCED(ret);
			mImageTarget = NULL;
		}
		mDevice = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectHelper::calculateReprojectionMatrix(ICamera* cam)
	{
		const Matrix44& view = cam->getViewMatrix();
		Matrix44 projection = cam->getProjectionMatrix();
		IGraphicsResourceManager::getSingleton().convertProjectionMatrix(projection);

		if (mInitReprojection)
		{
			mInitReprojection = false;
			mReProjectionMatrix = Matrix44::IDENTITY;
		}
		else
		{
			//separate translation to avoid large world precision error
			//reprojection	= (v*p)-1 * (preV * preP)
			//				= p-1 * v-1 * preV * prevP
			//				= p-1 * (v-1 * preV) * prevP
			Matrix44 invVPrevV;
			Matrix44 orientView = static_cast<const Matrix33&>(view);	//drop translation
			Matrix44 prevOrientView = static_cast<const Matrix33&>(mPrevView);
			if (orientView == prevOrientView)
				invVPrevV = Matrix44::generateTranslation(mPrevView.getTranslation() - view.getTranslation());
			else
			{
				const POINT3& prevEye = mPrevView.getInverse().getTranslation();
				Matrix44 translation = Matrix44::generateTranslation(cam->getEyePosition() - prevEye);
				invVPrevV = orientView.getTranspose() * translation * prevOrientView;
			}

			mReProjectionMatrix = projection.getInverse() * invVPrevV * mPrevProjection;
		}
		mPrevView = view;
		mPrevProjection = projection;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectOutput::overrideInput(const HTEXTURE* inputs, size_t inputCount)
	{
		if (mOverriddenCount < inputCount)
		{
			size_t i;
			for (i = 0; i < mOverriddenCount; ++i)
				this->setInputBuffer(i, inputs[i]);

			for (; i < inputCount; ++i)
			{
				mBackup[i] = this->getInputBuffer(i);
				this->setInputBuffer(i, inputs[i]);
			}
			mOverriddenCount = inputCount;
		}
		else
		{
			for (size_t i = 0; i < inputCount; ++i)
				this->setInputBuffer(i, inputs[i]);
		}
		ShaderVariableSource::getSingleton().setRenderOutput(this, true);
	}

	//////////////////////////////////////////////////////////////////////////
	void	ImageEffectOutput::restoreInput()
	{
		if (mOverriddenCount > 0)
		{
			for (size_t i = 0; i < mOverriddenCount; ++i)
			{
				this->setInputBuffer(i, mBackup[i]);
				mBackup[i] = HTEXTURE::EMPTY;
			}
			mOverriddenCount = 0;
			ShaderVariableSource::getSingleton().setRenderOutput(this, true);
		}
	}
	
}//namespace Blade