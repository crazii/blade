/********************************************************************
	created:	2017/01/28
	filename: 	ImageffectBase.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IGraphicsView.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IShaderVariableSource.h>
#include <interface/public/IImageEffect.h>
#include <RenderUtility.h>
#include <Material.h>
#include "../Material/MaterialManager.h"
#include "RenderSchemeManager.h"
#include "ImageEffectOutput.h"

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::setOutputBuffer(const HTEXTURE& color, const HTEXTURE& depth)
	{
		assert(color != NULL || depth != NULL);

		IRenderTarget* target = this->getTarget();
		IRenderDevice* device = this->getDevice();

		if (color != NULL)
		{
			target->setColorBuffer(0, color);
			target->setColorBufferCount(1);
		}
		else
			target->setColorBufferCount(0);
		target->setDepthBuffer(depth);

		mOutput->setTarget(target);
		this->checkView(color, depth);
		device->setRenderTarget(target);
		device->setRenderView(mEffectView);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::setOutputBuffers(const HTEXTURE* colors, size_t colorCount, const HTEXTURE& depth)
	{
		IRenderTarget* target = this->getTarget();
		IRenderDevice* device = this->getDevice();

		assert(colorCount >= 1);
		target->setColorBufferCount(colorCount);

		for(size_t i = 0; i < colorCount; ++i)
			target->setColorBuffer(i, colors[i]);

		target->setDepthBuffer(depth);

		device->setRenderTarget(target);
		this->checkView(colors[0], depth);
		device->setRenderView(mEffectView);
	}

	//////////////////////////////////////////////////////////////////////////
	bool ImageEffectBase::ensureBuffer(HTEXTURE& buffer, PixelFormat format, size_t width/* = -1*/, size_t height/* = -1*/, size_t mip/* = 1*/)
	{
		return mOutput->ensureBuffer(buffer, format, width, height, mip);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::drawQuad(Material* material, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		this->drawQuadImpl(true, material, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::drawQuad(MaterialInstance* material, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		this->drawQuadImpl(true, material, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::doProcess(const HTEXTURE& dest, Material* mat/* = NULL*/, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mat == NULL)
		{
			mat = mOutput->getBlitMaterial();
			pass = shader = 0;
		}
		if (!mat->isLoaded())
		{
			assert(false);
			return;
		}

		mOutput->restoreInput();
		this->setOutputBuffer(dest, HTEXTURE::EMPTY);
		this->drawQuadImpl(false, mat, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::blit(const HTEXTURE& source, const HTEXTURE& dest,  Material* mat/* = NULL*/, size_t pass/* = BP_COLOR*/, size_t shader/* = 0*/)
	{
		if (mat == NULL)
		{
			mat = mOutput->getBlitMaterial();
			shader = 0;
			if (pass >= (size_t)BP_COUNT)
			{
				assert(false);
				pass = BP_COLOR;
			}
		}
		if (!mat->isLoaded())
		{
			assert(false);
			return;
		}

		//override input
		mOutput->overrideInput(&source, 1);

		if (dest->getPixelFormat().isDepth())
		{
			assert(mat != NULL || pass == BP_DEPTH || pass == BP_LINEARIZE_DEPTH);
			this->setOutputBuffer(HTEXTURE::EMPTY, dest);
		}
		else
			this->setOutputBuffer(dest, HTEXTURE::EMPTY);
		this->drawQuadImpl(false, mat, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::blit(const HTEXTURE* sources, size_t sourceCount, const HTEXTURE* dests, size_t destCount, Material* mat, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mat == NULL || !mat->isLoaded())
		{
			assert(false);
			return;
		}

		//override input
		mOutput->overrideInput(sources, sourceCount);

		this->setOutputBuffers(dests, destCount,  HTEXTURE::EMPTY);
		this->drawQuadImpl(false, mat, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::blur(const HTEXTURE& texture, size_t iterations /* = 1*/, size_t tech/* = size_t(BT_GAUSSIAN)*/,
		const HTEXTURE& depth/* = HTEXTURE::EMPTY*/, float bilateralSharpness/* = 0.5f*/)
	{
		TEXCACHE cache(this, texture);

		HTEXTURE inputs[] = { texture, depth };

		mOutput->setBilateralBlurSharpness(bilateralSharpness);

		for (size_t i = 0; i < iterations; ++i)
		{
			mOutput->overrideInput(inputs, 1);
			this->setOutputBuffers(cache, 1, HTEXTURE::EMPTY);

			mOutput->setBlurDirectionX();
			this->drawQuadImpl(false, mOutput->getBlurMaterial(), 0, tech);

			mOutput->overrideInput(cache, 1);
			this->setOutputBuffers(inputs, 1, HTEXTURE::EMPTY);
			mOutput->setBlurDirectionY();
			this->drawQuadImpl(false, mOutput->getBlurMaterial(), 0, tech);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::doProcess(const HTEXTURE& dest, MaterialInstance* mat, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mat == NULL || !mat->isLoaded())
		{
			assert(false);
			return;
		}

		mOutput->restoreInput();
		this->setOutputBuffer(dest, HTEXTURE::EMPTY);
		this->drawQuadImpl(false, mat, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::blit(const HTEXTURE& source, const HTEXTURE& dest, MaterialInstance* mat, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mat == NULL || !mat->isLoaded())
		{
			assert(false);
			return;
		}

		//override input
		mOutput->overrideInput(&source, 1);
		this->setOutputBuffer(dest, HTEXTURE::EMPTY);
		this->drawQuadImpl(false, mat, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::blit(const HTEXTURE* sources, size_t sourceCount, const HTEXTURE* dests, size_t destCount, MaterialInstance* mat, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mat == NULL || !mat->isLoaded())
		{
			assert(false);
			return;
		}

		//override input
		mOutput->overrideInput(sources, sourceCount);

		this->setOutputBuffers(dests, destCount, HTEXTURE::EMPTY);
		this->drawQuadImpl(false, mat, pass, shader);
	}

	//////////////////////////////////////////////////////////////////////////
	void		ImageEffectBase::applyCameraChange()
	{
		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		svc.setCamera(NULL);
		svc.setCamera(mEffectCamera);
	}

	//////////////////////////////////////////////////////////////////////////
	void		ImageEffectBase::applyViewChange()
	{
		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		svc.setView(NULL);
		svc.setView(mEffectView);
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	ImageEffectBase::getSchemeProfile() const
	{
		return mOutput->getProfile();
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::drawQuadImpl(bool restoreInputs, Material* material, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mDevice == NULL)
		{
			assert(false);
			return;
		}

		Pass* matPass = material->getActiveTechnique()->getPass(pass);
		const HSHADEROPTION& shaderOpt = matPass->getShaderOption(shader);

		if (restoreInputs)
			mOutput->restoreInput();

		IShaderVariableSource::getSingleton().onShaderSwitch(true);

		RenderSchemeManager& rsm = RenderSchemeManager::getSingleton();

		RenderPropertySet* propSet = matPass->getRenderProperty();
		if(propSet != NULL)
			rsm.applyRenderProperty(mDevice, *propSet, GFM_DEFAULT);
		rsm.applyGlobalShaderSetting(mDevice, shaderOpt);

		const GraphicsGeometry& geom = RenderUtility::getUnitQuad();
		mDevice->renderGeometry(geom);

		//successive draw calls with the same shader usually have different input texture and custom shader uniforms
		//invalidate shader, to force update
		rsm.applyRenderSettings(mDevice, NULL, TextureStateInstance(), NULL);
		this->restoreView();
	}

	//////////////////////////////////////////////////////////////////////////
	void ImageEffectBase::drawQuadImpl(bool restoreInputs, MaterialInstance* material, size_t pass/* = 0*/, size_t shader/* = 0*/)
	{
		if (mDevice == NULL)
		{
			assert(false);
			return;
		}

		Pass* matPass = material->getActiveTechnique()->getPass(pass);
		const HSHADEROPTION& shaderOpt = matPass->getShaderOption(shader);
		mDevice->setShader(shaderOpt->getProgram());

		if (restoreInputs)
			mOutput->restoreInput();

		IShaderVariableSource& svc = IShaderVariableSource::getSingleton();
		RenderSchemeManager& rsm = RenderSchemeManager::getSingleton();
		svc.onShaderSwitch(true);

		const RenderPropertySet* propSet = NULL;
		if (svc.setRenderProperties(matPass->getRenderProperty(), NULL, material->getRenderProperty()))
			propSet = svc.getRenderProperty();
		if (propSet != NULL)
			RenderSchemeManager::getSingleton().applyRenderProperty(mDevice, *propSet, GFM_DEFAULT);
		const SamplerState& samplers = matPass->getSamplerState();
		rsm.applyRenderSettings(mDevice, shaderOpt, material->getTextureStateInstance(), &samplers);

		const GraphicsGeometry& geom = RenderUtility::getUnitQuad();
		mDevice->renderGeometry(geom);

		//successive draw calls with the same shader usually have different input texture and custom shader uniforms
		//invalidate shader, to force update
		rsm.applyRenderSettings(mDevice, NULL, TextureStateInstance(), NULL);
		this->restoreView();
	}

	//////////////////////////////////////////////////////////////////////////
	const Matrix44& ImageEffectBase::getReprojectionMatrix() const
	{
		return mOutput->getReprojectionMatrix();
	}

	//////////////////////////////////////////////////////////////////////////
	HTEXTURE	ImageEffectBase::allocCache(PixelFormat fmt, size_t width, size_t height, size_t mip/* = 1*/)
	{
		return mOutput->allocCache(fmt, width, height, mip);
	}

	//////////////////////////////////////////////////////////////////////////
	void		ImageEffectBase::releaseCache(HTEXTURE& buffer)
	{
		return mOutput->releaseCache(buffer);
	}

	//////////////////////////////////////////////////////////////////////////
	void		ImageEffectBase::checkView(const HTEXTURE& color, const HTEXTURE& depth)
	{
		assert(color != NULL || depth != NULL);

		IRenderTarget* target = this->getTarget();
		scalar sizes[4];
		size_t offsets[4];
		mEffectView->getSizePosition(sizes, offsets);
		bool hasOffset = offsets[0] != 0 || offsets[1] != 0 || offsets[2] != 0 || offsets[3] != 0;
		bool hasScale = sizes[0] != 0 || sizes[1] != 0 || sizes[2] != 1 || sizes[3] != 1;

		HTEXTURE buffer = (color != NULL) ? color : depth;

		if(	buffer->getWidth() < target->getViewWidth() || buffer->getHeight() < target->getViewHeight()
			|| hasOffset || hasScale)
		{
			mEffectView->calculateSizeInPixel((ITexture*)buffer);
			scalar width = scalar(mEffectView->getPixelRight()-mEffectView->getPixelLeft());
			scalar height = scalar(mEffectView->getPixelBottom()-mEffectView->getPixelTop());
			mEffectCamera->setAspectRatio(width, height);
			this->applyCameraChange();
			this->applyViewChange();
			mViewChanged = true;
		}
		else
			mEffectView->calculateSizeInPixel(target);
	}

	//////////////////////////////////////////////////////////////////////////
	void		ImageEffectBase::restoreView()
	{
		if(mViewChanged)
		{
			IRenderTarget* target = this->getTarget();
			mEffectView->calculateSizeInPixel(target);
			scalar width = scalar(mEffectView->getPixelRight()-mEffectView->getPixelLeft());
			scalar height = scalar(mEffectView->getPixelBottom()-mEffectView->getPixelTop());
			mEffectCamera->setAspectRatio(width, height);
			this->applyCameraChange();
			this->applyViewChange();
			mViewChanged = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			ImageEffectBase::backupRenderTarget()
	{
		mOriginalTarget = mOutput->getTarget();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ImageEffectBase::restoreRenderTarget()
	{
		mOutput->setTarget(mOriginalTarget);
	}

}//namespace Blade