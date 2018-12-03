/********************************************************************
	created:	2017/1/25
	filename: 	IImageEffect.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_IImageEffect_h__
#define __Blade_IImageEffect_h__
#include <BladeGraphics.h>
#include <utility/Bindable.h>
#include <interface/public/graphics/ITexture.h>

namespace Blade
{
	class ICamera;
	class IGraphicsView;
	class IRenderTarget;
	class IRenderDevice;
	class Material;
	class MaterialInstance;
	class ImageEffectOutput;

	class BLADE_GRAPHICS_API IImageEffect
	{
	public:
		enum EAttributes
		{
			IFXA_NONE = 0,
			IFXA_HDR2LDR = 0x01,
			IFXA_LDR_INPUT = 0x02,
		};
	public:
		virtual ~IImageEffect() {}

		/** @brief  */
		virtual const TString& getName() const = 0;

		/** @brief  */
		virtual void	setEnable(bool enable) = 0;

		/** @brief  */
		virtual bool	isEnabled() const = 0;

		/** @brief  */
		virtual EAttributes	getAttributes() const = 0;

		/** @brief  */
		virtual void initialize() = 0;

		/** @brief  */
		virtual void shutdown() = 0;

		/** @brief per frame preparation before scene rendering */
		virtual void prepare(IRenderDevice* device, IRenderTarget* target, ICamera* mainSceneCam, IGraphicsView* mainSceneView) = 0;

		/**
		@describe image process
		@param chainIn chained input buffer managed by image effect framework. already linked to IMAGEFX_INPUT in shader
		@param chainOut chained output buffer  managed by image effect framework.
		@param depth reflect to input depth defined in render scheme, as 'target_depth'
		@param inputs reflect to input defined in render scheme, as 'input[0]'...'inputN'. already linked to RENDERBUFFER_INPUT0..RENDERBUFFER_INPUTN
		@param outputs reflect to target defined int render scheme, as 'target[0]'...targetN.
		@return chainOut's content is changed or not
		*/
		virtual bool process_full(const HTEXTURE& chainIn, const HTEXTURE& chainOut, const HTEXTURE& depth,
			ICamera* effectCam, IGraphicsView* effectView,
			const HTEXTURE* extraInputs, size_t inputCount, const HTEXTURE* extraOuputs, size_t outputCount) = 0;

		/** @brief per frame ending after image effect rendering */
		virtual void finalize() = 0;

	protected:
		friend class ImageEffectOutput;
		ImageEffectOutput* mOutput;
	};//class IImageEffect

	extern template class BLADE_GRAPHICS_API Factory<IImageEffect>;

	class BLADE_GRAPHICS_API ImageEffectBase : public IImageEffect, public Bindable
	{
	public:
		inline ImageEffectBase(const TString& name, EAttributes attr = IFXA_NONE)
			:mName(name)
			,mAttributes(attr)
		{
			mDevice = NULL;
			mTarget = NULL;
			mOriginalTarget = NULL;
			mSceneCamera = NULL;
			mSceneView = NULL;
			mEnabled = true;
			mViewChanged = false;
		}

		virtual const TString& getName() const { return mName; }

		/** @brief  */
		virtual void	setEnable(bool enable)
		{
			if (mEnabled != enable)
			{
				mEnabled = enable;
				if (mEnabled)
					this->onEnable();
				else
					this->onDisable();
			}
		}

		/** @brief  */
		virtual bool	isEnabled() const { return mEnabled; }

		/** @brief  */
		virtual EAttributes	getAttributes() const { return mAttributes; }

		/** @brief  */
		virtual void prepare(IRenderDevice* device, IRenderTarget* target, ICamera* mainSceneCam, IGraphicsView* mainSceneView)
		{
			mDevice = device;
			mTarget = target;
			mSceneCamera = mainSceneCam;
			mSceneView = mainSceneView;
			mEffectCamera = NULL;
			mEffectView = NULL;
			this->backupRenderTarget();
			this->prepareImpl();
		}

		/** @brief  */
		virtual void finalize()
		{
			this->finalizeImpl();
			this->restoreRenderTarget();
			mDevice = NULL;
			mTarget = NULL;
			mOriginalTarget = NULL;
			mSceneCamera = NULL;
			mSceneView = NULL;
			mEffectCamera = NULL;
			mEffectView = NULL;
		}

		/** @brief  */
		virtual bool process_full(const HTEXTURE& chainIn, const HTEXTURE& chainOut, const HTEXTURE& depth,
			ICamera* effectCam, IGraphicsView* effectView,
			const HTEXTURE* extraInputs, size_t inputCount, const HTEXTURE* extraOuputs, size_t outputCount)
		{
			mEffectCamera = effectCam;
			mEffectView = effectView;
			return this->process_inputoutput(chainIn, chainOut, depth, extraInputs, inputCount, extraOuputs, outputCount);
		}

		/** @brief  */
		virtual bool process_inputoutput(const HTEXTURE& chainIn, const HTEXTURE& chainOut, const HTEXTURE& depth,
			const HTEXTURE* extraInputs, size_t inputCount, const HTEXTURE* extraOuputs, size_t outputCount)
		{
			BLADE_UNREFERENCED(depth);
			BLADE_UNREFERENCED(extraInputs); BLADE_UNREFERENCED(inputCount);
			BLADE_UNREFERENCED(extraOuputs); BLADE_UNREFERENCED(outputCount);
			return this->process(chainIn, chainOut);
		}

		/** @brief  */
		virtual bool process(const HTEXTURE& input, const HTEXTURE& output) { BLADE_UNREFERENCED(input); BLADE_UNREFERENCED(output); return false; }

		/** @brief  */
		virtual void prepareImpl() = 0;

		/** @brief  */
		virtual void finalizeImpl() = 0;

		/** @brief  */
		virtual void onEnable() {}

		/** @brief  */
		virtual void onDisable() {}

	protected:
		/** @brief bind output buffer to mTarget cache */
		void setOutputBuffer(const HTEXTURE& color, const HTEXTURE& depth);
		/** @brief bind output buffers to mTarget cache */
		void setOutputBuffers(const HTEXTURE* colors, size_t colorCount, const HTEXTURE& depth);
		/** @brief make sure buffer are created and in the right dimensions/format, otherwise recreate it */
		/** return true if buffer recreated */
		bool ensureBuffer(HTEXTURE& buffer, PixelFormat format, size_t width = -1, size_t height = -1, size_t mip = 1);
		/** @brief draw full screen quad using standard buffer input */
		void drawQuad(Material* material, size_t pass = 0, size_t shader = 0);
		/** @brief  */
		void drawQuad(MaterialInstance* material, size_t pass = 0, size_t shader = 0);
		/** @brief process using standard buffer input IMAGEFX_INPUT, RENDERBUFFER_INPUT0 ... RENDERBUFFER_INPUT[N] */
		void doProcess(const HTEXTURE& dest, Material* mat = NULL, size_t pass = 0, size_t shader = 0);
		/** @brief blit overriding standard input RENDERBUFFER_INPUT0 with source */
		enum EBlitPass	//blit shader, only valid when mat is NULL
		{
			BP_COLOR,					//blit to color
			BP_LINEARIZE_DEPTH2COLOR,	//blit to color, perform depth linearize
			BP_DEPTH,					//blit to depth stencil
			BP_LINEARIZE_DEPTH,			//blit to depth stencil, perform depth linearize

			BP_COUNT,
			BP_BEGIN = BP_COLOR,
		};
		void blit(const HTEXTURE& source, const HTEXTURE& dest, Material* mat = NULL, size_t pass = BP_COLOR, size_t shader = 0);
		/** @brief  */
		void blit(const HTEXTURE* sources, size_t sourceCount, const HTEXTURE* dests, size_t destCount, Material* mat, size_t pass = 0, size_t shader = 0);
		enum EBlurTech
		{
			BT_GAUSSIAN,
			BT_BILATERAL,
		};
		/** @brief  */
		void blur(const HTEXTURE& texture, size_t iterations = 1, size_t tech = size_t(BT_GAUSSIAN), const HTEXTURE& depth = HTEXTURE::EMPTY, float bilateralSharpness = 0.5f);

		/** @brief process using standard buffer input IMAGEFX_INPUT, RENDERBUFFER_INPUT0 ... RENDERBUFFER_INPUT[N] */
		void doProcess(const HTEXTURE& dest, MaterialInstance* mat, size_t pass = 0, size_t shader = 0);
		/** @brief blit overriding standard input RENDERBUFFER_INPUT0 with source */
		void blit(const HTEXTURE& source, const HTEXTURE& dest, MaterialInstance* mat, size_t pass = 0, size_t shader = 0);
		/** @brief  */
		void blit(const HTEXTURE* sources, size_t sourceCount, const HTEXTURE* dests, size_t destCount, MaterialInstance* mat, size_t pass = 0, size_t shader = 0);

		/** @brief if camera's view/projection changed manually, call this to reflect camera change to shader semantics */
		void		applyCameraChange();
		/** @brief if view's dimension changed manually, call this to reflect view change to shader semantics */
		void		applyViewChange();

		/** @brief  */
		const TString&	getSchemeProfile() const;
		/** @brief  */
		ICamera* getSceneCamera() const { return mSceneCamera; }
		/** @brief  */
		IGraphicsView* getSceneView() const { return mSceneView; }
		/** @brief  */
		ICamera* getEffectCamera() const { return mEffectCamera; }
		/** @brief  */
		IGraphicsView* getEffectView() const { return mEffectView; }
		/** @brief  */
		IRenderTarget*	getTarget() const { return mTarget; }
		/** @brief  */
		IRenderDevice*	getDevice() const { return mDevice; }
	protected:
		/** @brief  */
		void drawQuadImpl(bool restoreInputs, Material* material, size_t pass = 0, size_t shader = 0);
		/** @brief  */
		void drawQuadImpl(bool restoreInputs, MaterialInstance* material, size_t pass = 0, size_t shader = 0);
		/** @brief  */
		const Matrix44& getReprojectionMatrix() const;

		IRenderDevice*	mDevice;
		IRenderTarget*	mTarget;
		IRenderTarget*	mOriginalTarget;
		ICamera*		mSceneCamera;
		IGraphicsView*	mSceneView;
		ICamera*		mEffectCamera;
		IGraphicsView*	mEffectView;
		TString			mName;
		EAttributes		mAttributes;
		bool			mEnabled;
	private:
		/** @brief  */
		/** @brief  */
		HTEXTURE	allocCache(PixelFormat fmt, size_t width, size_t height, size_t mip = 1);
		/** @brief  */
		void		releaseCache(HTEXTURE& buffer);
		void		checkView(const HTEXTURE& color, const HTEXTURE& depth);
		void		restoreView();
		void		backupRenderTarget();
		void		restoreRenderTarget();
		bool			mViewChanged;
		using IImageEffect::mOutput;

	public:
			struct TEXCACHE
			{
			private:
				TEXCACHE(const TEXCACHE& /*src*/)
				{

				}
			public:
				HTEXTURE mCache;
				ImageEffectBase* mOwner;

				inline TEXCACHE() :mOwner(NULL) { }

				inline TEXCACHE(ImageEffectBase* owner, PixelFormat pf, size_t width, size_t height, size_t mip = 1) : mOwner(owner)
				{
					mCache = mOwner->allocCache(pf, width, height, mip);
				}

				inline TEXCACHE(ImageEffectBase* owner, const HTEXTURE& texRef) : mOwner(owner)
				{
					mCache = mOwner->allocCache(texRef->getPixelFormat(), texRef->getWidth(), texRef->getHeight(), texRef->getMipmapCount());
				}

				inline ~TEXCACHE()
				{
					assert(mCache == NULL || mOwner != NULL);
					assert(mOwner == NULL || mCache != NULL);
					if(mCache != NULL)
						mOwner->releaseCache(mCache);
				}

				inline void allocCache(ImageEffectBase* owner, PixelFormat pf, size_t width, size_t height, size_t mip = 1)
				{
					assert(mOwner == NULL && mCache == NULL);
					if (mOwner != NULL && mCache != NULL)
						mOwner->releaseCache(mCache);

					mOwner = owner;
					mCache = mOwner->allocCache(pf, width, height, mip);
				}

				inline void allocCache(ImageEffectBase* owner, const HTEXTURE& texRef)
				{
					this->allocCache(owner, texRef->getPixelFormat(), texRef->getWidth(), texRef->getHeight(), texRef->getMipmapCount());
				}

				inline operator const HTEXTURE&() const { return mCache; }
				inline operator const HTEXTURE*() const { return &mCache; }

				inline void swap(TEXCACHE& rhs)
				{
					assert(mOwner == rhs.mOwner || mOwner == NULL || rhs.mOwner == NULL);
					assert(mOwner == NULL || mCache != NULL);
					assert(rhs.mOwner == NULL || rhs.mCache != NULL);
					HTEXTURE th = mCache;
					mCache = rhs.mCache;
					rhs.mCache = th;
				}
			};
	};//class ImageEffectBase
	
}//namespace Blade

namespace std
{

	inline void swap(Blade::ImageEffectBase::TEXCACHE& lhs, Blade::ImageEffectBase::TEXCACHE& rhs)
	{
		lhs.swap(rhs);
	}

}

#endif//__Blade_IImageEffect_h__