/********************************************************************
	created:	2012/03/30
	filename: 	D3D9StateBlock.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_D3D9StateBlock_h__
#define __Blade_D3D9StateBlock_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>

#include <math/BladeMath.h>
#include <interface/public/graphics/RenderProperty.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>


namespace Blade
{

	class D3D9StateBlock : public Allocatable
	{
	public:
		inline D3D9StateBlock() :mDevice(NULL),mMaxSamplers(0)
		{
			mDefaultFillMode			= GFM_SOLID;
			mDefaultCullMode			= HCM_CW;
			mDefaultColorWriteMask		= CWC_ALL;
			mDefaultAlphaBlendState = BLADE_NEW AlphaBlendProperty();
			mDefaultDepthState = BLADE_NEW DepthProperty();
			mDefaultStencilState = BLADE_NEW StencilProperty();
			mDefaultScissorState = BLADE_NEW ScissorProperty();
			mDefaultSamplerState = BLADE_NEW SamplerState();

			mDefaultAlphaBlendState->setSourceBlend( BM_ONE );
			mDefaultAlphaBlendState->setDestBlend( BM_ZERO );
			mDefaultAlphaBlendState->setSourceAlphaBlend(BM_ONE);
			mDefaultAlphaBlendState->setDestAlphaBlend(BM_ZERO);
			mDefaultAlphaBlendState->setBlendOperation(BO_ADD);
			mDefaultAlphaBlendState->setAlphaBlendOperation(BO_ADD);
			mDefaultAlphaBlendState->setIndependentBlend(false);

			mDefaultDepthState->setTestEnable(false);
			mDefaultDepthState->setWriteEnable(false);
			mDefaultDepthState->setDepthTestFunc(TF_LESS);
			mDefaultDepthState->setDepthBias(0);
			mDefaultDepthState->setSlopeScaledDepthBias(0);
			mDefaultDepthState->setEneableDepthClip(true);
			mDefaultDepthState->setDepthBiasClamp(0);

			mDefaultStencilState->setEnable(false);
			mDefaultStencilState->setReadMask( (uint8)0xFF);
			mDefaultStencilState->setWriteMask( (uint8)0xFF);
			mDefaultStencilState->setFrontFaceFailOperation(SOP_KEEP);
			mDefaultStencilState->setFrontFacePassOperation(SOP_KEEP);
			mDefaultStencilState->setFrontFaceDepthFailOperation(SOP_KEEP);
			mDefaultStencilState->setFrontFaceTestFunc(TF_ALWAYS);
			mDefaultStencilState->setBackFaceFailOperation(SOP_KEEP);
			mDefaultStencilState->setBackFacePassOperation(SOP_KEEP);
			mDefaultStencilState->setBackFaceDepthFailOperation(SOP_KEEP);
			mDefaultStencilState->setBackFaceTestFunc(TF_ALWAYS);

			mDefaultScissorState->setValidCount(0);

			mGlobalSampler.min = TFM_LINEAR;
			mGlobalSampler.mag = TFM_LINEAR;
			mGlobalSampler.mip = TFM_LINEAR;
			mGlobalSampler.u = TAM_WRAP;
			mGlobalSampler.v = TAM_WRAP;
			mGlobalSampler.w = TAM_WRAP;
			mGlobalSampler.maxAnisotropy = 1;
			
			this->resetState();
		}
		inline ~D3D9StateBlock()
		{
			BLADE_DELETE mDefaultAlphaBlendState;
			BLADE_DELETE mDefaultDepthState;
			BLADE_DELETE mDefaultStencilState;
			BLADE_DELETE mDefaultScissorState;
			BLADE_DELETE mDefaultSamplerState;
		}

		/** @brief  */
		inline void setup(LPDIRECT3DDEVICE9 device,size_t maxSamplerCount)
		{
			mMaxSamplers = maxSamplerCount;

			{
				Sampler sampler;
				for(size_t i = 0; i < maxSamplerCount; ++i)
					mDefaultSamplerState->addSampler(TStringHelper::fromUInt32((uint32)i), sampler);
			}

			mDevice = device;
			this->resetState();
		}

		/** @brief  */
		inline void	resetState()
		{
			mLastFillMode = mDefaultFillMode;
			mLastCullMode = mDefaultCullMode;
			mLastColorWriteMask = mDefaultColorWriteMask;
			mLastAlphaState = NULL;
			mLastAlphaState = NULL;
			mLastDepthState = NULL;
			mLastStencilState = NULL;
			mLastScissorState = NULL;
			mLastSamplerState = NULL;
			if( mDevice )
			{
				mFillMode = mDefaultFillMode;
				mCullMode = mDefaultCullMode;
				mColorWriteMask = mDefaultColorWriteMask;
				mAlphaBlendState = mDefaultAlphaBlendState;
				mDepthState = mDefaultDepthState;
				mStencilState = mDefaultStencilState;
				mScissorState = mDefaultScissorState;

				mDevice->SetRenderState( D3DRS_FILLMODE, (DWORD)mDefaultFillMode );
				mDevice->SetRenderState( D3DRS_CULLMODE, (DWORD)mDefaultCullMode );
				mDevice->SetRenderState( D3DRS_COLORWRITEENABLE, (DWORD)mDefaultColorWriteMask );

				mDevice->SetRenderState(D3DRS_SRCBLEND, (DWORD)mDefaultAlphaBlendState->getSourceBlend() );
				mDevice->SetRenderState(D3DRS_DESTBLEND, (DWORD)mDefaultAlphaBlendState->getDestBlend() );
				mDevice->SetRenderState(D3DRS_SRCBLENDALPHA, (DWORD)mDefaultAlphaBlendState->getSourceAlphaBlend() );
				mDevice->SetRenderState(D3DRS_DESTBLENDALPHA, (DWORD)mDefaultAlphaBlendState->getDestAlphaBlend() );
				mDevice->SetRenderState(D3DRS_BLENDOP, (DWORD)mDefaultAlphaBlendState->getBlendOperation() );
				mDevice->SetRenderState(D3DRS_BLENDOPALPHA, (DWORD)mDefaultAlphaBlendState->getAlphaBlendOperation() );
				mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, (DWORD)(BOOL)mDefaultAlphaBlendState->getIndependentBlend() );

				mDevice->SetRenderState(D3DRS_ZENABLE, (DWORD)(BOOL) (mDefaultDepthState->isTestEnable() || mDefaultDepthState->isWriteEnable()));
				mDevice->SetRenderState( D3DRS_ZFUNC, (DWORD)(mDefaultDepthState->isTestEnable() ? mDefaultDepthState->getDepthTestFunc() : D3DCMP_ALWAYS) );
				mDevice->SetRenderState( D3DRS_ZWRITEENABLE, (DWORD)(BOOL)mDefaultDepthState->isWriteEnable() );
				float bias = (float)mDefaultDepthState->getDepthBias();
				bias /= 1000.0f;
				mDevice->SetRenderState( D3DRS_DEPTHBIAS, reinterpret_cast<DWORD&>(bias) );
				bias = (float)mDefaultDepthState->getSlopeScaledDepthBias();
				mDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, reinterpret_cast<DWORD&>(bias) );
				mDevice->SetRenderState( D3DRS_CLIPPING, (DWORD)(BOOL)mDefaultDepthState->isEnableDepthClip() );

				mDevice->SetRenderState( D3DRS_STENCILENABLE, (DWORD)(BOOL)mDefaultStencilState->isEnable() );
				mDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, (DWORD)TRUE );

				mDevice->SetRenderState( D3DRS_STENCILMASK, (DWORD)mDefaultStencilState->getReadMask() );
				mDevice->SetRenderState( D3DRS_STENCILWRITEMASK, (DWORD)mDefaultStencilState->getWriteMask() );
				mDevice->SetRenderState( D3DRS_STENCILREF, (DWORD)mDefaultStencilState->getRef() );

				mDevice->SetRenderState( D3DRS_STENCILFAIL, (DWORD)mDefaultStencilState->getFrontFaceFailOperation() );
				mDevice->SetRenderState( D3DRS_STENCILPASS, (DWORD)mDefaultStencilState->getFrontFacePassOperation() );
				mDevice->SetRenderState( D3DRS_STENCILZFAIL, (DWORD)mDefaultStencilState->getFrontFaceDepthFailOperation() );
				mDevice->SetRenderState( D3DRS_STENCILFUNC, (DWORD)mDefaultStencilState->getFrontFaceTestFunc() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILFAIL, (DWORD)mDefaultStencilState->getBackFaceFailOperation() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILPASS, (DWORD)mDefaultStencilState->getBackFacePassOperation() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, (DWORD)mDefaultStencilState->getBackFaceDepthFailOperation() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILFUNC, (DWORD)mDefaultStencilState->getBackFaceTestFunc() );

				mDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
				RECT r;
				r.left = r.top = r.right = r.bottom = 0;
				mDevice->SetScissorRect(&r);

				Sampler sampler;
				sampler.setAddressMode( mGlobalSampler.u, mGlobalSampler.v, mGlobalSampler.w );
				sampler.setMaxAnisotropy( (uint8)mGlobalSampler.maxAnisotropy);
				sampler.setMinFilter(mGlobalSampler.min);
				sampler.setMagFilter(mGlobalSampler.mag);
				sampler.setMipFilter(mGlobalSampler.mip);

				for(size_t i = 0; i < mMaxSamplers; ++i )
				{
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSU, mGlobalSampler.u );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSV, mGlobalSampler.v );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSW, mGlobalSampler.w );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_BORDERCOLOR, D3DCOLOR_ARGB(0,0,0,0) );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MAXANISOTROPY, mGlobalSampler.maxAnisotropy );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MAGFILTER , mGlobalSampler.mag );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MINFILTER , mGlobalSampler.min );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MIPFILTER , mGlobalSampler.mip );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_SRGBTEXTURE, TRUE);
					float _bias = 0.0f;
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MIPMAPLODBIAS, reinterpret_cast<DWORD&>(_bias) );

					mDefaultSamplerState->setSampler(i, sampler);
				}
				mSamplerState = mDefaultSamplerState;

				mDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
				mDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
				mDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
				mLastLinearTarget = true;
				std::memset(mLastLinearTexture, 0, sizeof(mLastLinearTexture));
			}
		}

		/** @brief  */
		inline void		setFillMode(FILL_MODE fm)
		{
			if( mFillMode != fm )
			{
				mFillMode = fm;
				mDevice->SetRenderState( D3DRS_FILLMODE, (DWORD)mFillMode );
			}
		}

		/** @brief  */
		inline void		setCullMode(CULL_MODE cm)
		{
			if( mCullMode != cm)
			{
				mCullMode = cm;
				mDevice->SetRenderState( D3DRS_CULLMODE, (DWORD)mCullMode );
			}
		}


		/** @brief  */
		inline void		setColorWriteProperty(const ColorWriteProperty* prop)
		{
			if( prop == NULL )
			{
				if( mLastColorWriteMask != mDefaultColorWriteMask )
				{
					mLastColorWriteMask = mColorWriteMask = mDefaultColorWriteMask;
					mDevice->SetRenderState( D3DRS_COLORWRITEENABLE, (DWORD)mColorWriteMask );
					mDevice->SetRenderState(D3DRS_COLORWRITEENABLE1, (DWORD)mColorWriteMask);
					mDevice->SetRenderState(D3DRS_COLORWRITEENABLE2, (DWORD)mColorWriteMask);
					mDevice->SetRenderState(D3DRS_COLORWRITEENABLE3, (DWORD)mColorWriteMask);
				}
			}
			else if( mColorWriteMask != prop->getWritingColorChannel() )
			{
				mLastColorWriteMask = mColorWriteMask = prop->getWritingColorChannel();
				mDevice->SetRenderState( D3DRS_COLORWRITEENABLE, (DWORD)mColorWriteMask );
				mDevice->SetRenderState(D3DRS_COLORWRITEENABLE1, (DWORD)mColorWriteMask);
				mDevice->SetRenderState(D3DRS_COLORWRITEENABLE2, (DWORD)mColorWriteMask);
				mDevice->SetRenderState(D3DRS_COLORWRITEENABLE3, (DWORD)mColorWriteMask);
			}
		}

		/** @brief  */
		inline void		setAlphaBlendProperty(const AlphaBlendProperty* prop)
		{
			if( mLastAlphaState == prop )
				return;

			if( prop == NULL )
			{
				mDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , FALSE );
			}
			else
			{
				mDevice->SetRenderState( D3DRS_ALPHABLENDENABLE , TRUE );
				this->compareSetAlphaProperty( *prop );
			}
			mLastAlphaState = prop;
		}

		/** @brief  */
		void		setDepthProperty(const DepthProperty* prop)
		{
			if( mLastDepthState == prop )
				return;

			if( prop == NULL )
			{
				if( mLastDepthState != mDefaultDepthState )
				{
					mLastDepthState = mDefaultDepthState;
					this->compareSetDepthProperty(*mDefaultDepthState);
					return;
				}
			}
			else
				this->compareSetDepthProperty(*prop);
			mLastDepthState = prop;
		}

		/** @brief  */
		void		setStencilProperty(const StencilProperty* prop)
		{
			if( mLastStencilState == prop )
				return;
			
			if( prop == NULL )
			{
				if( mLastStencilState != mDefaultStencilState )
				{
					mLastStencilState = mDefaultStencilState;
					this->compareSetStencilProperty(*mDefaultStencilState);
					return;
				}
			}
			else
				this->compareSetStencilProperty(*prop);
			mLastStencilState = prop;
		}

		/** @brief  */
		void		setScissorProperty(const ScissorProperty* prop)
		{
			if( mLastScissorState == prop )
				return;

			if( prop == NULL )
			{
				if( mLastScissorState != mDefaultScissorState )
				{
					mLastScissorState = mDefaultScissorState;
					this->compareSetScissor( *mDefaultScissorState);
					return;
				}
			}
			else
				this->compareSetScissor(*prop);
		}

		/** @brief  */
		void	setGlobalFilter(SAMPLER_OPTION filter)
		{
			mGlobalSampler = filter;
		}
		
		/** @brief  */
		const SAMPLER_OPTION& getGlobalFilter() const
		{
			return mGlobalSampler;
		}

		//////////////////////////////////////////////////////////////////////////
		void		compareSetSampler(const Sampler* sampler, index_t i, const HTEXTURE& texture)
		{
			if( sampler == NULL )
				sampler = &texture->getSampler();

			const Sampler& newsampler = *sampler;
			Sampler& original = *(mSamplerState->getSampler(i));
			if( newsampler.isUsingGlobalAddressMode() )
			{
				if( original.getAdressModeU() != mGlobalSampler.u )
				{
					original.setAdressModeU( mGlobalSampler.u );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSU, mGlobalSampler.u );
				}

				if( original.getAdressModeV() != mGlobalSampler.v )
				{
					original.setAdressModeV( mGlobalSampler.v );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSV, mGlobalSampler.v );
				}

				if( original.getAdressModeW() != mGlobalSampler.w )
				{
					original.setAdressModeW( mGlobalSampler.w );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSW, mGlobalSampler.w );
				}
			}
			else
			{
				if( original.getAdressModeU() != newsampler.getAdressModeU() )
				{
					original.setAdressModeU( newsampler.getAdressModeU() );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSU, newsampler.getAdressModeU() );
				}

				if( original.getAdressModeV() != newsampler.getAdressModeV() )
				{
					original.setAdressModeV( newsampler.getAdressModeV() );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSV, newsampler.getAdressModeV() );
				}

				if( original.getAdressModeW() != newsampler.getAdressModeW() )
				{
					original.setAdressModeW( newsampler.getAdressModeW() );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_ADDRESSW, newsampler.getAdressModeW() );
				}
			}

			//check global settings
			if( newsampler.isUsingGlobalAnisotropy() )
			{
				if( original.getMaxAnisotropy() != mGlobalSampler.maxAnisotropy )
				{
					original.setMaxAnisotropy( (uint8)mGlobalSampler.maxAnisotropy );
					mDevice->SetSamplerState( (DWORD)i,D3DSAMP_MAXANISOTROPY, mGlobalSampler.maxAnisotropy );
				}
			}
			else if( original.getMaxAnisotropy() != newsampler.getMaxAnisotropy() )
			{
				original.setMaxAnisotropy( (uint8)newsampler.getMaxAnisotropy() );
				mDevice->SetSamplerState( (DWORD)i,D3DSAMP_MAXANISOTROPY, newsampler.getMaxAnisotropy() );
			}

			//Global setting
			if( newsampler.isUsingGlobalFilter() )
			{
				if( original.getMagFilter() != mGlobalSampler.mag )
				{
					original.setMagFilter( mGlobalSampler.mag );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MAGFILTER , mGlobalSampler.mag );
				}

				if( original.getMinFilter() != mGlobalSampler.min )
				{
					original.setMinFilter(mGlobalSampler.min);
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MINFILTER , mGlobalSampler.min );
				}

				if( original.getMipFilter() != mGlobalSampler.mip )
				{
					original.setMipFilter( mGlobalSampler.mip );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MIPFILTER , mGlobalSampler.mip );
				}
			}
			else
			{
				if( original.getMagFilter() != newsampler.getMagFilter() )
				{
					original.setMagFilter( newsampler.getMagFilter() );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MAGFILTER , newsampler.getMagFilter() );
				}

				if( original.getMinFilter() != newsampler.getMinFilter() )
				{
					original.setMinFilter( newsampler.getMinFilter() );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MINFILTER , newsampler.getMinFilter() );
				}

				if( original.getMipFilter() != newsampler.getMipFilter() )
				{
					original.setMipFilter( newsampler.getMipFilter() );
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MIPFILTER , newsampler.getMipFilter() );
				}

				if( original.getMipmapLODBias() != newsampler.getMipmapLODBias() )
				{
					original.setMipmapLODBias( newsampler.getMipmapLODBias() );
					float bias = newsampler.getMipmapLODBias();
					mDevice->SetSamplerState( (DWORD)i, D3DSAMP_MIPMAPLODBIAS, reinterpret_cast<DWORD&>(bias) );
				}
			}

			bool linearColor = texture->getPixelFormat().isLinearColor();
			if (linearColor != mLastLinearTexture[i])
			{
				mLastLinearTexture[i] = linearColor;
				//always use linear color space
				//D3DSAMP_SRGBTEXTURE will linearize on sampling. disable it if texture already in linear color space.
				mDevice->SetSamplerState((DWORD)i, D3DSAMP_SRGBTEXTURE, (DWORD)(linearColor ? FALSE : TRUE));
			}
		}

		/** @brief  */
		inline void setRenderTargetState(bool linear)
		{
			if (mLastLinearTarget != linear)
			{
				mLastLinearTarget = linear;
				mDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, DWORD(linear ? FALSE : TRUE));
			}
		}
		
	protected:

		//////////////////////////////////////////////////////////////////////////
		void	compareSetAlphaProperty(const AlphaBlendProperty& prop)
		{
			if( mAlphaBlendState->getSourceBlend() != prop.getSourceBlend() )
			{
				mAlphaBlendState->setSourceBlend( prop.getSourceBlend() );
				mDevice->SetRenderState(D3DRS_SRCBLEND, (DWORD)prop.getSourceBlend() );
			}

			if( mAlphaBlendState->getDestBlend() != prop.getDestBlend() )
			{
				mAlphaBlendState->setDestBlend( prop.getDestBlend() );
				mDevice->SetRenderState(D3DRS_DESTBLEND, (DWORD)prop.getDestBlend() );
			}

			if( mAlphaBlendState->getSourceAlphaBlend() != prop.getSourceAlphaBlend() )
			{
				mAlphaBlendState->setSourceAlphaBlend( prop.getSourceAlphaBlend() );
				mDevice->SetRenderState(D3DRS_DESTBLENDALPHA, (DWORD)prop.getSourceAlphaBlend() );
			}

			if( mAlphaBlendState->getDestAlphaBlend() != prop.getDestAlphaBlend() )
			{
				mAlphaBlendState->setDestAlphaBlend( prop.getDestAlphaBlend() );
				mDevice->SetRenderState(D3DRS_DESTBLENDALPHA, (DWORD)prop.getDestAlphaBlend() );
			}

			if( mAlphaBlendState->getBlendOperation() != prop.getBlendOperation() )
			{
				mAlphaBlendState->setBlendOperation( prop.getBlendOperation() );
				mDevice->SetRenderState(D3DRS_BLENDOP, (DWORD)prop.getBlendOperation() );
			}

			if( mAlphaBlendState->getAlphaBlendOperation() != prop.getAlphaBlendOperation() )
			{
				mAlphaBlendState->setAlphaBlendOperation( prop.getAlphaBlendOperation() );
				mDevice->SetRenderState(D3DRS_BLENDOPALPHA, (DWORD)prop.getAlphaBlendOperation() );
			}

			if( mAlphaBlendState->getIndependentBlend() != prop.getIndependentBlend() )
			{
				mAlphaBlendState->setIndependentBlend( prop.getIndependentBlend() );
				mDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, (DWORD)(BOOL)prop.getIndependentBlend() );
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void	compareSetDepthProperty(const DepthProperty& prop)
		{
			if( mDepthState->isTestEnable() != prop.isTestEnable() )
			{
				mDepthState->setTestEnable( prop.isTestEnable() );
				mDevice->SetRenderState(D3DRS_ZENABLE, (DWORD)(BOOL)(prop.isTestEnable() || prop.isWriteEnable()) );
			}

			if (prop.isTestEnable())
			{
				if (mDepthState->getDepthTestFunc() != prop.getDepthTestFunc())
				{
					mDepthState->setDepthTestFunc(prop.getDepthTestFunc());
					mDevice->SetRenderState(D3DRS_ZFUNC, (DWORD)prop.getDepthTestFunc());
				}
			}
			else if (prop.isWriteEnable())
			{
				mDepthState->setDepthTestFunc(TF_ALWAYS);
				mDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
			}

			if( mDepthState->isWriteEnable() != prop.isWriteEnable() )
			{
				mDepthState->setWriteEnable( prop.isWriteEnable() );
				mDevice->SetRenderState( D3DRS_ZWRITEENABLE, (DWORD)(BOOL)prop.isWriteEnable() );
			}

			if( mDepthState->getDepthBias() != prop.getDepthBias() )
			{
				mDepthState->setDepthBias( prop.getDepthBias() );
				float bias = (float)prop.getDepthBias();
				bias /= 1000.0f;
				mDevice->SetRenderState( D3DRS_DEPTHBIAS, reinterpret_cast<DWORD&>(bias) );
			}

			//compare float points, or set directly ?
			if( !Math::Equal<scalar>(mDepthState->getSlopeScaledDepthBias(),prop.getSlopeScaledDepthBias(), Math::HIGH_EPSILON) )
			{
				mDepthState->setSlopeScaledDepthBias( prop.getSlopeScaledDepthBias() );
				float bias = (float)prop.getSlopeScaledDepthBias();
				mDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, reinterpret_cast<DWORD&>(bias)  );
			}

			//depth clip is part of clipping
			if( mDepthState->isEnableDepthClip() != prop.isEnableDepthClip() )
			{
				mDepthState->setEneableDepthClip( prop.isEnableDepthClip() );
				mDevice->SetRenderState( D3DRS_CLIPPING, (DWORD)(BOOL)prop.isEnableDepthClip() );
			}

			//clamp is not valid for d3d9
			//mDepthState->setDepthBiasClamp(0);
		}

		//////////////////////////////////////////////////////////////////////////
		void		compareSetStencilProperty(const StencilProperty& prop)
		{
			if( mStencilState->isEnable() != prop.isEnable() )
			{
				mStencilState->setEnable( prop.isEnable() );
				mDevice->SetRenderState( D3DRS_STENCILENABLE, (DWORD)(BOOL)prop.isEnable() );
			}
			if( !prop.isEnable() )
				return;

			if( mStencilState->getRef() != prop.getRef() )
			{
				mStencilState->setRef( prop.getRef() );
				mDevice->SetRenderState( D3DRS_STENCILREF, (DWORD)mStencilState->getRef() );
			}

			if( mStencilState->getReadMask() != prop.getReadMask() )
			{
				mStencilState->setReadMask( prop.getReadMask() );
				mDevice->SetRenderState( D3DRS_STENCILMASK, (DWORD)prop.getReadMask() );
			}

			if( mStencilState->getWriteMask() != prop.getWriteMask() )
			{
				mStencilState->setWriteMask( prop.getWriteMask() );
				mDevice->SetRenderState( D3DRS_STENCILWRITEMASK, (DWORD)prop.getWriteMask() );
			}

			if( mStencilState->getFrontFaceFailOperation() != prop.getFrontFaceFailOperation() )
			{
				mStencilState->setFrontFaceFailOperation( prop.getFrontFaceFailOperation() );
				mDevice->SetRenderState( D3DRS_STENCILFAIL, (DWORD)prop.getFrontFaceFailOperation() );
			}

			if( mStencilState->getFrontFacePassOperation() != prop.getFrontFacePassOperation() )
			{
				mStencilState->setFrontFacePassOperation( prop.getFrontFacePassOperation() );
				mDevice->SetRenderState( D3DRS_STENCILPASS, (DWORD)prop.getFrontFacePassOperation() );
			}

			if( mStencilState->getFrontFaceDepthFailOperation() != prop.getFrontFaceDepthFailOperation() )
			{
				mStencilState->setFrontFaceDepthFailOperation( prop.getFrontFaceDepthFailOperation() );
				mDevice->SetRenderState( D3DRS_STENCILZFAIL, (DWORD)prop.getFrontFaceDepthFailOperation() );
			}

			if( mStencilState->getFrontFaceTestFunc() != prop.getFrontFaceTestFunc() )
			{
				mStencilState->setFrontFaceTestFunc( prop.getFrontFaceTestFunc() );
				mDevice->SetRenderState( D3DRS_STENCILFUNC, (DWORD)prop.getFrontFaceTestFunc() );
			}

			//
			if( mStencilState->getBackFaceFailOperation() != prop.getBackFaceFailOperation() )
			{
				mStencilState->setBackFaceFailOperation( prop.getBackFaceFailOperation() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILFAIL, (DWORD)prop.getBackFaceFailOperation() );
			}

			if( mStencilState->getBackFacePassOperation() != prop.getBackFacePassOperation() )
			{
				mStencilState->setBackFacePassOperation( prop.getBackFacePassOperation() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILPASS, (DWORD)prop.getBackFacePassOperation() );
			}

			if( mStencilState->getBackFaceDepthFailOperation() != prop.getBackFaceDepthFailOperation() )
			{
				mStencilState->setBackFaceDepthFailOperation( prop.getBackFaceDepthFailOperation() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, (DWORD)prop.getBackFaceDepthFailOperation() );
			}

			if( mStencilState->getBackFaceTestFunc() != prop.getBackFaceTestFunc() )
			{
				mStencilState->setBackFaceTestFunc( prop.getBackFaceTestFunc() );
				mDevice->SetRenderState( D3DRS_CCW_STENCILFUNC, (DWORD)prop.getBackFaceTestFunc() );
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void		compareSetScissor(const ScissorProperty& prop)
		{
			if( mScissorState->getValidCount() != prop.getValidCount() )
			{
				mScissorState->setValidCount( prop.getValidCount() );
				BOOL bEnable = mScissorState->getValidCount() != 0;
				mDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, (DWORD)bEnable);
				if( !bEnable )
					return;
			}

			if( mScissorState->getValidCount() > 0 )
			{
				//dx9 support only one scissor
				const Box2i* boxes = prop.getScissorBoxes();
				const Box2i* originalBoxes = mScissorState->getScissorBoxes();
				if( originalBoxes[0] != boxes[0] )
				{
					mScissorState->setScissorBox(0, boxes[0] );
					RECT r;
					r.left = boxes->getMinPoint().x;
					r.top = boxes->getMinPoint().y;

					r.right = boxes->getMaxPoint().x;
					r.bottom = boxes->getMaxPoint().y;
					mDevice->SetScissorRect(&r);
				}
			}

		}


		LPDIRECT3DDEVICE9		mDevice;
		size_t					mMaxSamplers;

		//note : color / fog / alpha clip states are managed through shaders

		FILL_MODE				mFillMode;
		CULL_MODE				mCullMode;
		int						mColorWriteMask;
		AlphaBlendProperty*		mAlphaBlendState;
		DepthProperty*			mDepthState;
		StencilProperty*		mStencilState;
		ScissorProperty*		mScissorState;
		SamplerState*		mSamplerState;

		FILL_MODE				mDefaultFillMode;
		CULL_MODE				mDefaultCullMode;
		int						mDefaultColorWriteMask;
		AlphaBlendProperty*		mDefaultAlphaBlendState;
		DepthProperty*			mDefaultDepthState;
		StencilProperty*		mDefaultStencilState;
		ScissorProperty*		mDefaultScissorState;
		SamplerState*			mDefaultSamplerState;

		FILL_MODE				mLastFillMode;
		CULL_MODE				mLastCullMode;
		int						mLastColorWriteMask;
		SAMPLER_OPTION			mGlobalSampler;
		const RenderPropertySet*		mLastPropertySet;
		const AlphaBlendProperty*		mLastAlphaState;
		const DepthProperty*			mLastDepthState;
		const StencilProperty*		mLastStencilState;
		const ScissorProperty*		mLastScissorState;
		const SamplerState*		mLastSamplerState;
		//color space for textures
		bool					mLastLinearTexture[ITexture::MAX_TEXTURE_COUNT];
		//color space for target
		bool					mLastLinearTarget;
	};
	

}//namespace Blade
#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif //__Blade_D3D9StateBlock_h__