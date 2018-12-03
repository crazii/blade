/********************************************************************
	created:	2017/01/27
	filename: 	ImageEffectOutput.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ImageEffectOutput_h__
#define __Blade_ImageEffectOutput_h__
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/IShaderVariableSource.h>
#include "RenderOutput.h"
#include "RenderSchemeDesc.h"
#include "RenderStepInstance.h"
#include <interface/public/IImageEffect.h>
#include "../ShaderVariables/ShaderVariableSource.h"
#include "RenderSchemeConfig.h"

namespace Blade
{
#define BUFFER_CONFLIT_RESOLVE 0
#define CACHE_RELEASE_THRESHOLD 60

	//////////////////////////////////////////////////////////////////////////
	class ImageEffect_Blit : public ImageEffectBase, public Allocatable
	{
	public:
		static const TString BLIT_NAME;
	public:
		ImageEffect_Blit() :ImageEffectBase(BLIT_NAME) {}
		~ImageEffect_Blit() {}

		/** @brief  */
		virtual void initialize() {}

		/** @brief  */
		virtual void shutdown() {}

		/** @brief  */
		virtual void prepareImpl() {}

		/** @brief  */
		virtual bool process(const HTEXTURE& input, const HTEXTURE& output)
		{
			this->blit(input, output);
			return true;
		}

		/** @brief  */
		virtual void finalizeImpl() {}

	};//class ImageEffect_Blit

	class ImageEffectOutput;

	//shared data across image effects within the same scheme
	class ImageEffectHelper : public Allocatable
	{
	public:
		typedef TempTStringMap<int> BufferRefCount;
#if BUFFER_CONFLIT_RESOLVE
		typedef struct SStorage
		{
			const HTEXTURE*	first;
			const HTEXTURE*	second;
			HTEXTURE	storage;
		}STORAGE;
		typedef TStringMap<STORAGE>		BufferStorage;
#endif
		typedef TStringMap<HTEXTURE>	BufferMap;
	public:
		ImageEffectHelper(const TString* profile);

		/** @brief  */
		inline const HTEXTURE& getInput() const
		{
			assert(mInputIndex < 2);
			return mImageFXChain[mInputIndex];
		}

		/** @brief  */
		inline const HTEXTURE& getOutput() const
		{
			size_t outputIndex = (mInputIndex + 1) & 0x01;
			return mImageFXChain[outputIndex];
		}

		/** @brief  */
		inline void advance(bool changed, bool nextIFX)
		{
			if (changed)
			{
				mResultChanged = true;
				size_t outputIndex = (mInputIndex + 1) & 0x01;
				mInputIndex = outputIndex;
			}

			if (!nextIFX)
			{
				//if next render output is not an IFX, then its output should always be mFinal (mImageFXChain[0])
				//and any successive IFX should use mImageFXChain[0] as input
				//so make mInputIndex to 0
				//TODO: this can be statically decided on initialization, and extra blit operation can be replaced with linking non-IFX output to next IFX input.
				if(mResultChanged && mInputIndex != 0)
				{
					assert(mImageFXChain[1] != NULL);
					//mImageFXChain[0] is used by successive (non IFX) output, swap won't work for that
					//std::swap(mImageFXChain[1], mImageFXChain[0]);
					this->blit(mImageFXChain[1], mImageFXChain[0]);
					mInputIndex = 0;
				}
				mResultChanged = false;
			}
		}

		/** @brief  */
		inline void beginHDR2LDR()
		{
			assert(mHasHDR && mFinalLDR != NULL);
			size_t outputIndex = (mInputIndex + 1) & 0x01;
			if(outputIndex != 0)	//final LDR must be the first one
			{
				assert(outputIndex == 1);
				std::swap(mImageFXChain[outputIndex], mImageFXChain[mInputIndex]);
				mInputIndex = outputIndex;
				outputIndex = 0;
			}
			mImageFXChain[outputIndex] = mFinalLDR;
		}

		/** @brief  */
		inline void endHDR2LDR()
		{
			assert(!mLDRIFX || mChainBufferLDR != NULL);
			assert(mLDRIFX || mChainBufferLDR == NULL);
			mImageFXChain[1] = mChainBufferLDR;
		}

		/** @brief  */
		inline HTEXTURE	allocCache(PixelFormat format, size_t width, size_t height, size_t mip)
		{
			for(size_t i = 0; i < mCache.size(); ++i)
			{
				if(mCache[i].mUsed > 0)
					continue;
				HTEXTURE& buffer = mCache[i].mRT;
				if(buffer->getPixelFormat() == format && buffer->getWidth() == width && buffer->getHeight() == height && buffer->getMipmapCount() == mip)
				{
					++mCache[i].mUsed;
					mCache[i].mUnusedFrames = 0;
					return buffer;
				}
			}
			HTEXTURE ret;
			this->ensureBuffer(ret, format, width, height, mip);

			BufferCache c = { ret, 0, 1 };
			mCache.push_back( c );
			return ret;
		}

		/** @brief  */
		inline void		releaseCache(HTEXTURE& texture)
		{
			if(texture == NULL)
				return;

			HTEXTURE* found = NULL;
			for(size_t i = 0; i < mCache.size(); ++i)
			{
				if(mCache[i].mRT == texture)
				{
					found = &mCache[i].mRT;
					assert(mCache[i].mUsed > 0);
					--mCache[i].mUsed;
					break;
				}
			}
			assert(found != NULL);
			texture.clear();
		}

		inline void updateCache()
		{
			for (size_t i = 0; i < mCache.size(); ++i)
			{
				if (mCache[i].mUsed <= 0)
				{
					assert(mCache[i].mUsed == 0);
					if (++mCache[i].mUnusedFrames >= CACHE_RELEASE_THRESHOLD)
					{
						mCache.erase(mCache.begin() + (indexdiff_t)i);
						--i;
					}
				}
				else
					assert(mCache[i].mUsed == 1);
			}
		}

		/** @brief  */
		inline IRenderDevice*	getDevice() const
		{
			return mDevice;
		}

		/** @brief  */
		inline IRenderTarget*	getTarget() const
		{
			return mImageTarget;
		}

		/** @brief  */
		inline const TString&	getProfile() const
		{
			return *mProfile;
		}

		/** @brief blit image effect result to dest */
		inline void blit(const HTEXTURE& srouce, const HTEXTURE& dest, ICamera* cam = NULL, IGraphicsView* view = NULL);

		/** @brief  */
		inline size_t getEffectOutputs(ImageEffectOutput*const*& list) const
		{
			size_t count = mList.size();
			list = (count == 0) ? NULL : (&mList[0]);
			return count;
		}

		/** @brief  */
		inline void	setEnable(bool enabled) { mHasImageFX = enabled; }

		inline const Matrix44& getReprojectionMatrix() const { return mReProjectionMatrix; }

		/** @brief  */
		Material*	getBlitMaterial() const { return mBlitMaterial; }

		/** @brief  */
		Material*	getBlurMaterial() const { return mBlurMaterial; }

		/** @brief  */
		inline void setBlurDirectionX() { mBlurDirHandle.setData(&Vector2::UNIT_X, sizeof(Vector2::UNIT_X)); }
		inline void setBlurDirectionY() { mBlurDirHandle.setData(&Vector2::UNIT_Y, sizeof(Vector2::UNIT_Y)); }
		inline void setBilateralBlurSharpness(float sharpness) { mBlurSharpnessHandle.setData(&sharpness, sizeof(sharpness)); }

		/** @brief  */
		bool	ensureBuffer(HTEXTURE& buffer, PixelFormat format, size_t width = -1, size_t height = -1, size_t mip = 1);

		/** @brief  */
		void	updateBufferInfo(BufferRefCount& outCounts);

		/** @brief  */
		void	createFinalBuffer(const IRenderTarget* finalTarget, HTEXTURE& output, HTEXTURE& outputLDR);

#if BUFFER_CONFLIT_RESOLVE
		/** @brief  */
		void	preapreStorage(const BufferMap& buffers);

		/** @brief  */
		void	setupInputOutput(ImageEffectOutput& ifxout, TempVector<HTEXTURE>& input, TempVector<HTEXTURE>& output, const HTEXTURE*& depth);
#endif

		/** @brief  */
		void	preRender(IRenderTarget* finalTarget, ICamera* mainCam, IGraphicsView* mainView);

		/** @brief  */
		void	postRender(const HTEXTURE& outputTarget);

		/** @brief  */
		void	initialize(IRenderDevice* device, RenderOutput** outputs, size_t outputCount);

		/** @brief  */
		void	shutdown();

	protected:
		void	calculateReprojectionMatrix(ICamera* cam);

		IRenderTarget*	mImageTarget;
		IRenderDevice*	mDevice;
		ICamera*		mMainCamera;
		IGraphicsView*	mMainView;
		const TString*	mProfile;
		Material*		mBlitMaterial;
		Material*		mBlurMaterial;
		ShaderVariableMap::Handles mBlurDirHandle;
		ShaderVariableMap::Handles mBlurSharpnessHandle;
		index_t			mInputIndex;
		
		HTEXTURE		mImageFXChain[2];
		HTEXTURE		mChainBuffer;
		HTEXTURE		mChainBufferLDR;
		//used by render scheme 
		HTEXTURE		mFinal;
		HTEXTURE		mFinalLDR;
		Matrix44		mPrevView;
		Matrix44		mPrevProjection;
		Matrix44		mReProjectionMatrix;

		typedef Vector<ImageEffectOutput*> ImageEffectList;

		struct BufferCache
		{
			HTEXTURE	mRT;
			uint16		mUnusedFrames;
			int16		mUsed;
		};
		typedef Vector< BufferCache > BufferCacheList;

		ImageEffectList	mList;
		BufferCacheList	mCache;

#if BUFFER_CONFLIT_RESOLVE
		BufferStorage	mStorage;
#endif
		bool			mHasImageFX;
		bool			mHasHDR;
		bool			mLDRIFX;	//whether has LDR image effects enabled
		bool			mResultChanged;
		bool			mInitReprojection;
	};//class ImageEffectHelper

	//////////////////////////////////////////////////////////////////////////
	class ImageEffectOutput : public RenderOutput
	{
	public:
		inline ImageEffectOutput(const OUTPUT_DESC* info)
			:RenderOutput(info)
		{
			mEffect = BLADE_FACTORY_CREATE(IImageEffect, mDesc->mImageEffect);
			mEffect->mOutput = this;

			const HCONFIG& cfg = RenderSchemeConfig::getSingleton().getIFXConfig(mDesc->mImageEffect);
			if (cfg != NULL)
				cfg->synchronize(static_cast<ImageEffectBase*>(mEffect));

			mHelper = NULL;
			mOverriddenCount = 0;
			mNextEffect = false;
			mLocal = false;
		}

		inline ImageEffectOutput(IImageEffect* effect, const OUTPUT_DESC* info)
			:RenderOutput(info)
			,mEffect(effect)
		{
			mEffect->mOutput = this;
			mHelper = NULL;
			mOverriddenCount = 0;
			mNextEffect = false;
			mLocal = true;
		}

		inline ~ImageEffectOutput()
		{
			mEffect->shutdown();
			if (!mLocal)
				BLADE_DELETE mEffect;
		}

		/** @brief  */
		inline void initialize(ImageEffectHelper* helper)
		{
			mHelper = helper;
			mEffect->initialize();

#if BUFFER_CONFLIT_RESOLVE
			const IMAGEFX_DESC& desc = this->getDesc();
			for (size_t j = 0; j < desc.mInputDescList.size(); ++j)
			{
				const BUFFER_DESC* inputDesc = desc.mInputDescList[j];
				if (desc.mDepthTarget == inputDesc)
					mConflicts.push_back(inputDesc->mName);
				else for (size_t k = 0; k < desc.mTargetDescList.size(); ++k)
				{
					if (desc.mTargetDescList[k] == inputDesc)
						mConflicts.push_back(inputDesc->mName);
				}
			}
#endif
		}

		/** @brief  */
		inline void preRender(ICamera* mainCam, IGraphicsView* mainView)
		{
			mOverriddenCount = 0;
			mEffect->prepare(mHelper->getDevice(), mHelper->getTarget(), mainCam, mainView);
		}

		/** @brief  */
		virtual void renderOutput(IRenderDevice* device, ICamera* camera, IGraphicsView* view, const RenderStepInstance* const* instances, size_t instanceCount)
		{
			this->doImageEffect(camera, view);
			if(instanceCount > 0)
				RenderOutput::renderOutput(device, camera, view, instances, instanceCount);
		}

		/** @brief  */
		inline void	doImageEffect(ICamera* camera, IGraphicsView* view)
		{
			IRenderTarget* target = this->getTarget();
			RenderOutput::setShaderVarialbeSource(camera, view);
			ShaderVariableSource::getSingleton().setImageEffectOutput(this, true);
			if (this->isUsingCustomCamera())
				camera = this->getCustomCamera();

			bool hdr2ldr = mEffect->getAttributes()&IImageEffect::IFXA_HDR2LDR;
			if(hdr2ldr)
				mHelper->beginHDR2LDR();

#if BUFFER_CONFLIT_RESOLVE
			TempVector<HTEXTURE> outputs;
			TempVector<HTEXTURE> inputs;
			const HTEXTURE* depth = &target->getDepthBuffer();
			mHelper->setupInputOutput(*this, inputs, outputs, depth);

			bool outputChanged = mEffect->process_full(mHelper->getInput(), mHelper->getOutput(), target->getDepthBuffer(),
				camera, view,
				inputs.size() > 0 ? &inputs[0] : NULL, inputs.size(), outputs.size() > 0 ? &outputs[0] : NULL, outputs.size());
#else
			bool outputChanged = mEffect->process_full(mHelper->getInput(), mHelper->getOutput(), target->getDepthBuffer(),
				camera, view,
				&mInputs[0], mInputCount, &target->getColorBuffer(0), target->getColorBufferCount());
#endif
			if(hdr2ldr)
				mHelper->endHDR2LDR();

			mHelper->advance(outputChanged, mNextEffect);

			mEffect->finalize();
		}

		/** @brief  */
		inline void	postRender()
		{
			this->restoreInput();
			mOverriddenCount = 0;
		}

		/** @brief  */
		inline bool	ensureBuffer(HTEXTURE& buffer, PixelFormat format, size_t width = -1, size_t height = -1, size_t mip = 1) const
		{
			return mHelper->ensureBuffer(buffer, format, width, height, mip);
		}

		/** @brief  */
		inline const TString& getProfile() const
		{
			return mHelper->getProfile();
		}

		/** @brief  */
		inline IImageEffect*	getEffect() const
		{
			return mEffect;
		}

		/** @brief  */
		inline const HTEXTURE&	getIFXInput() const
		{
			return mHelper->getInput();
		}

		/** @brief  */
		inline void			setNextEffect(bool next)
		{
			mNextEffect = next;
		}

#if BUFFER_CONFLIT_RESOLVE
		/** @brief  */
		inline const TStringList&	getBufferConflicts() const
		{
			return mConflicts;
		}
#endif

		/** @brief  */
		inline HTEXTURE	allocCache(PixelFormat format, size_t width, size_t height, size_t mip)
		{
			return mHelper->allocCache(format, width, height, mip);
		}

		/** @brief  */
		inline void		releaseCache(HTEXTURE& texture)
		{
			return mHelper->releaseCache(texture);
		}

		/** @brief  */
		inline const Matrix44&	getReprojectionMatrix() const { return mHelper->getReprojectionMatrix(); }

		/** @brief  */
		Material*	getBlitMaterial() const { return mHelper->getBlitMaterial(); }

		/** @brief  */
		Material*	getBlurMaterial() const { return mHelper->getBlurMaterial(); }

		/** @brief  */
		inline void setBlurDirectionX() { mHelper->setBlurDirectionX(); }
		inline void setBlurDirectionY() { mHelper->setBlurDirectionY(); }
		inline void setBilateralBlurSharpness(float sharpness) { mHelper->setBilateralBlurSharpness(sharpness); }

		/** @brief  */
		void	overrideInput(const HTEXTURE* inputs, size_t inputCount);

		/** @brief  */
		void	restoreInput();

	protected:
		IImageEffect*		mEffect;
		ImageEffectHelper*	mHelper;
#if BUFFER_CONFLIT_RESOLVE
		TStringList			mConflicts;
#endif
		InputBufferList		mBackup;
		size_t				mOverriddenCount;
		bool				mNextEffect;
		bool				mLocal;
	};//class ImageEffectOutput


	//////////////////////////////////////////////////////////////////////////
	inline void ImageEffectHelper::blit(const HTEXTURE& source, const HTEXTURE& dest, ICamera* cam/* = NULL*/, IGraphicsView* view/* = NULL*/)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640)//construction of local static object is not thread-safe
#endif

		if (cam == NULL)
			cam = mMainCamera;
		if (view == NULL)
			view = mMainView;

		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		svc.setCamera(cam);
		svc.setView(view);

		static ImageEffect_Blit blit;
		static IMAGEFX_DESC desc;
		static ImageEffectOutput output(&blit, &desc);
		output.initialize(this);
		blit.prepare(mDevice, mImageTarget, cam, view);
		blit.process_full(source, dest, HTEXTURE::EMPTY, cam, view, NULL, 0, NULL, 0);

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif
	}

}//namespace Blade


#endif // __Blade_ImageEffectOutput_h__