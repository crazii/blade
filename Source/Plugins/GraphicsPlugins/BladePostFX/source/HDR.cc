/********************************************************************
	created:	2017/02/04
	filename: 	HDR.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Material.h>
#include <interface/IMaterialManager.h>
#include <interface/public/graphics/IGraphicsView.h>
#include "HDR.h"

namespace Blade
{
	const TString HDR::NAME = BTString("HDR");

	//should match in material definition
	enum HDR_SHADER
	{
		HDRS_DOWNSCALE_L = 0,
		HDRS_DOWNSCALE,
		HDRS_LUMINANCE_ADAPT,
		HDRS_BRIGHT,
		HDRS_BLOOM,
		HDRS_FINAL,
		HDRS_FINAL_NOBLOOM,
	};

	//////////////////////////////////////////////////////////////////////////
	HDR::HDR()
		:ImageEffectBase(NAME, IFXA_HDR2LDR)
		,mMaterial(NULL)
		,mBloomFactor(HDR_DEFAULT_BLOOM_FACTOR)
		,mExposureScale(HDR_DEFAULT_EXPOSURE)
		,mBloomIntensity(HDR_DEFAULT_BLOOM_INTENSITY)
		,mEnableBloom(true)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	HDR::~HDR()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void HDR::initialize()
	{
		if(mEnabled)
			this->onEnable();
	}

	//////////////////////////////////////////////////////////////////////////
	void HDR::shutdown()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void HDR::prepareImpl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool HDR::process(const HTEXTURE& input, const HTEXTURE& output)
	{
		size_t width = input->getWidth()/2;
		size_t height = input->getHeight()/2;

		scalar dim[4];
		size_t pixelDim[4];
		this->getEffectView()->getSizePosition(dim, pixelDim);

		mExposureHandle.setData(&mExposureScale, sizeof(mExposureScale));
		{
#if 0
			TEXCACHE lum[20];
			size_t lumCount = 0;
			for (lumCount = 0; lumCount < countOf(lum); ++lumCount)
			{
				size_t w = width >> lumCount;
				size_t h = height >> lumCount;
				w = w > 2 ? w : 2;
				h = h > 2 ? h : 2;
				lum[lumCount].allocCache(this, PF_R16F, w, h);
				if (w == 2 && h == 2)
				{
					++lumCount;
					break;
				}
			}

			this->blit(input, lum[0], mMaterial, 0, HDRS_DOWNSCALE_L);

			for (size_t i = 0; i < lumCount - 1; ++i)
				this->blit(lum[i], lum[i + 1], mMaterial, 0, HDRS_DOWNSCALE);
#else
			const size_t lumCount = 2;

			TEXCACHE lum[lumCount];
			lum[0].allocCache(this, PF_R16F, width, height);
			lum[1].allocCache(this, PF_R16F, width / 2, height / 2);

			this->blit(input, lum[0], mMaterial, 0, HDRS_DOWNSCALE_L);
			this->blit(lum[0], lum[1], mMaterial, 0, HDRS_DOWNSCALE);

			//note: render sub render texture using view port
			//render texture will record used view to calculate uv space
			//this avoids create full chain of downscaled textures
			for (size_t i = 2; ; i += 2)
			{
				size_t w = std::max<size_t>(width >> i, 2u);
				size_t h = std::max<size_t>(height >> i, 2u);

				this->getEffectView()->setSizePosition(dim[0], dim[1], 0, 0, pixelDim[0], pixelDim[1], w, h);
				this->applyViewChange();
				this->blit(lum[1], lum[0], mMaterial, 0, HDRS_DOWNSCALE);

				if (w == 2 && h == 2)
				{
					std::swap(lum[0], lum[1]);
					break;
				}

				w = std::max<size_t>(w >> 1u, 2u);
				h = std::max<size_t>(h >> 1u, 2u);

				this->getEffectView()->setSizePosition(dim[0], dim[1], 0, 0, pixelDim[0], pixelDim[1], w, h);
				this->applyViewChange();
				this->blit(lum[0], lum[1], mMaterial, 0, HDRS_DOWNSCALE);

				if (w == 2 && h == 2)
					break;
			};
			this->getEffectView()->setSizePosition(dim[0], dim[1], dim[2], dim[3], pixelDim[0], pixelDim[1], pixelDim[2], pixelDim[3]);
			this->applyViewChange();
#endif

			if (this->ensureBuffer(mLastLuminace, PF_R16F, 2, 2))
				this->blit(lum[lumCount - 1], mLastLuminace);

			//adapt luminance
			HTEXTURE lumAdaptSources[] = { mLastLuminace /*last adapted*/, lum[lumCount - 1] /*current*/, };
			this->blit(lumAdaptSources, countOf(lumAdaptSources), &mLuminance, 1, mMaterial, 0, HDRS_LUMINANCE_ADAPT);
		}

		TEXCACHE bloom;
		if (mEnableBloom)
		{
			bloom.allocCache(this, PF_ARGB, width/2, height/2);

			Vector4 bloomFactor = Vector4::UNIT_XYZ * mBloomFactor;
			mBloomFactorHandle.setData(&bloomFactor, sizeof(bloomFactor));

			//bright pass
			HTEXTURE brightSources[] = { input, mLuminance };
			this->blit(brightSources, countOf(brightSources), bloom, 1, mMaterial, 0, HDRS_BRIGHT);

#if 0
			TEXCACHE bloomB(this, PF_ARGB, width, height);
			//bloom pass 0
			Vector2 dir;
			dir = Vector2::UNIT_X;
			mGaussianDirHandle.setData(&dir, sizeof(dir));
			this->blit(bloom, bloomB, mMaterial, 0, HDRS_BLOOM);
			dir = Vector2::UNIT_Y;
			mGaussianDirHandle.setData(&dir, sizeof(dir));
			this->blit(bloomB, bloom, mMaterial, 0, HDRS_BLOOM);

			//bloom pass 1
			TEXCACHE bloom2(this, PF_ARGB, width / 2, height / 2);
			TEXCACHE bloom2B(this, PF_ARGB, width / 2, height / 2);
			dir = Vector2::UNIT_X;
			mGaussianDirHandle.setData(&dir, sizeof(dir));
			this->blit(bloom, bloom2B, mMaterial, 0, HDRS_BLOOM);
			dir = Vector2::UNIT_Y;
			mGaussianDirHandle.setData(&dir, sizeof(dir));
			this->blit(bloom2B, bloom2, mMaterial, 0, HDRS_BLOOM);

			std::swap(bloom, bloom2);

#elif 0
			TEXCACHE bloomB(this, PF_ARGB, width, height);
			for (size_t i = 0; i < mBloomIntensity; ++i)
			{
				this->getEffectView()->setSizePosition(dim[0], dim[1], dim[2] / (1 << i), dim[3] / (1 << i), pixelDim[0], pixelDim[1], pixelDim[2] >> i, pixelDim[3] >> i);
				this->applyViewChange();

				mGaussianDirHandle.setData(&Vector2::UNIT_X, sizeof(Vector2::UNIT_X));
				this->blit(bloom, bloomB, mMaterial, 0, HDRS_BLOOM);
				mGaussianDirHandle.setData(&Vector2::UNIT_Y, sizeof(Vector2::UNIT_Y));
				this->blit(bloomB, bloom, mMaterial, 0, HDRS_BLOOM);
			}
			this->getEffectView()->setSizePosition(dim[0], dim[1], dim[2], dim[3], pixelDim[0], pixelDim[1], pixelDim[2], pixelDim[3]);
			this->applyViewChange();
#else
			this->blur(bloom, mBloomIntensity);
#endif
		}

		//final pass
		HTEXTURE sources[] = {input, mLuminance, bloom};
		this->blit(sources, countOf(sources), &output, 1, mMaterial, 0, mEnableBloom ? HDRS_FINAL : HDRS_FINAL_NOBLOOM);

		std::swap(mLuminance, mLastLuminace);
		return  true;
	}

	//////////////////////////////////////////////////////////////////////////
	void HDR::finalizeImpl()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void HDR::onEnable()
	{
		mMaterial = IMaterialManager::getSingleton().getMaterial(BTString("HDR"));
		mMaterial->loadSync();

		this->ensureBuffer(mLuminance, PF_R16F, 2, 2);

		mMaterial->getActiveTechnique()->getShaderVariableHandle(mBloomFactorHandle, BTString("brightLimiter"), SCT_FLOAT4, 1);
		mMaterial->getActiveTechnique()->getShaderVariableHandle(mExposureHandle, BTString("exposureScale"), SCT_FLOAT1, 1);
	}

	//////////////////////////////////////////////////////////////////////////
	void HDR::onDisable()
	{
		mLuminance.clear();
		mLastLuminace.clear();

		if(mMaterial != NULL)
		{
			mMaterial->unload();
			mMaterial = NULL;
		}
	}

	
}//namespace Blade