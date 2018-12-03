/********************************************************************
	created:	2015/01/05
	filename: 	GLESStateBlock.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESStateBlock_h__
#define __Blade_GLESStateBlock_h__
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES

namespace Blade
{
	class GLESStateBlock
	{
	public:
		inline GLESStateBlock() :mMaxSamplers(0),mGLSamplers(NULL)
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
		}
		inline ~GLESStateBlock()
		{
			BLADE_DELETE mDefaultAlphaBlendState;
			BLADE_DELETE mDefaultDepthState;
			BLADE_DELETE mDefaultStencilState;
			BLADE_DELETE mDefaultScissorState;
			BLADE_DELETE mDefaultSamplerState;
		}

		/** @brief  */
		inline void setup(size_t maxSamplerCount)
		{
			mMaxSamplers = maxSamplerCount;
			mGLSamplers = BLADE_RES_ALLOCT(GLuint, mMaxSamplers);
			::glGenSamplers((GLsizei)mMaxSamplers, mGLSamplers);
			assert( ::glGetError() == GL_NO_ERROR );

			for(size_t i = 0; i < mMaxSamplers; ++i )
			{
				::glBindSampler( (GLuint)i, mGLSamplers[i]);
				assert( ::glGetError() == GL_NO_ERROR );
			}

			{
				Sampler sampler;
				for(size_t i = 0; i < maxSamplerCount; ++i)
					mDefaultSamplerState->addSampler(TStringHelper::fromUInt32((uint32)i), sampler);
			}
			this->resetState();

			::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			assert( ::glGetError() == GL_NO_ERROR );
		}

		/** @brief  */
		inline void clearup()
		{
			if( mGLSamplers != NULL )
			{
				::glDeleteSamplers(mMaxSamplers, mGLSamplers);
				assert( ::glGetError() == GL_NO_ERROR );
				BLADE_RES_FREE(mGLSamplers);
				mGLSamplers = NULL;
			}

			for(size_t i = 0; i < mMaxSamplers; ++i)
			{
				::glActiveTexture(GL_TEXTURE0 + i);
				::glBindTexture(GL_TEXTURE_2D, 0);
				::glBindTexture(GL_TEXTURE_3D, 0);
				::glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
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
			mLastTextureState = NULL;
			mFillMode = mDefaultFillMode;
			mCullMode = mDefaultCullMode;
			mColorWriteMask = mDefaultColorWriteMask;
			mAlphaBlendState = mDefaultAlphaBlendState;
			mDepthState = mDefaultDepthState;
			mStencilState = mDefaultStencilState;
			mScissorState = mDefaultScissorState;

			//No fill mode for GLES

			GLenum front = mDefaultCullMode == HCM_CCW ? GL_CW : GL_CCW;
			::glFrontFace(front);
			::glCullFace(GL_BACK);
			::glEnable(GL_CULL_FACE);
			
			::glColorMask( mDefaultColorWriteMask&CWC_RED, mDefaultColorWriteMask&CWC_GREEN, mDefaultColorWriteMask&CWC_BLUE, mDefaultColorWriteMask&CWC_ALPHA);

			if( mDefaultAlphaBlendState->getIndependentBlend() )
			{
				GLenum srcAlpha = GLESBlendModeConverter( mDefaultAlphaBlendState->getSourceAlphaBlend() );
				GLenum destAlpha = GLESBlendModeConverter( mDefaultAlphaBlendState->getDestAlphaBlend() );
				GLenum srcColor = GLESBlendModeConverter(mDefaultAlphaBlendState->getSourceBlend());
				GLenum destColor = GLESBlendModeConverter(mDefaultAlphaBlendState->getDestBlend());
				::glBlendFuncSeparate(srcColor, destColor, srcAlpha, destAlpha);
				GLenum colorOp = GLESBlendOpConverter(mDefaultAlphaBlendState->getBlendOperation());
				GLenum alphaOp = GLESBlendOpConverter(mDefaultAlphaBlendState->getAlphaBlendOperation());
				::glBlendEquationSeparate(colorOp, alphaOp);
			}
			else
			{
				::glBlendFunc(GLESBlendModeConverter(mDefaultAlphaBlendState->getSourceBlend()), GLESBlendModeConverter(mDefaultAlphaBlendState->getDestBlend()));
				::glBlendEquation( GLESBlendOpConverter(mDefaultAlphaBlendState->getBlendOperation()) );
			}
			::glDisable(GL_BLEND);

			::glDepthFunc( GLESDepthSencilFuncConverter( mDefaultDepthState->getDepthTestFunc() ) );
			if( mDefaultDepthState->isTestEnable() )
				::glEnable(GL_DEPTH_TEST);
			else
				::glDisable(GL_DEPTH_TEST);

			::glDepthMask( (GLboolean)mDefaultDepthState->isWriteEnable() );
			float bias = (float)mDefaultDepthState->getDepthBias();
			bias /= 1000.0f;
			if( bias > 0 )
			{
				::glPolygonOffset(mDefaultDepthState->getSlopeScaledDepthBias(), bias);
				::glEnable(GL_POLYGON_OFFSET_FILL);
			}
			else
				::glDisable(GL_POLYGON_OFFSET_FILL);
			//Depth Clip

			::glStencilMask( mDefaultStencilState->getWriteMask() );
			::glStencilFuncSeparate( GL_FRONT, 
				GLESDepthSencilFuncConverter(mDefaultStencilState->getFrontFaceTestFunc()), mDefaultStencilState->getRef(), mDefaultStencilState->getReadMask() );

			::glStencilFuncSeparate( GL_BACK, 
				GLESDepthSencilFuncConverter(mDefaultStencilState->getBackFaceTestFunc()), mDefaultStencilState->getRef(), mDefaultStencilState->getReadMask()  );

			::glStencilOpSeparate( GL_FRONT, 
				GLESStencilOpConverter(mDefaultStencilState->getFrontFaceFailOperation()), 
				GLESStencilOpConverter(mDefaultStencilState->getFrontFaceDepthFailOperation()),
				GLESStencilOpConverter(mDefaultStencilState->getFrontFacePassOperation()) );

			::glStencilOpSeparate( GL_BACK, 
				GLESStencilOpConverter(mDefaultStencilState->getBackFaceFailOperation()), 
				GLESStencilOpConverter(mDefaultStencilState->getBackFaceDepthFailOperation()),
				GLESStencilOpConverter(mDefaultStencilState->getBackFacePassOperation()) );

			//if( mDefaultStencilState->isEnable() )
			//	::glEnable(GL_STENCIL_TEST);
			//else
			//	::glDisable(GL_STENCIL_TEST);

			{
				GLint x,y,w,h;
				x = y = w = h = 0;
				::glScissor(x, y, w, h);
			}
			//::glDisable(GL_SCISSOR_TEST);
			::glEnable(GL_SCISSOR_TEST);

			Sampler sampler;
			sampler.setAddressMode( mGlobalSampler.u, mGlobalSampler.v, mGlobalSampler.w );
			sampler.setMaxAnisotropy( (uint8)mGlobalSampler.maxAnisotropy);
			sampler.setMinFilter(mGlobalSampler.min);
			sampler.setMagFilter(mGlobalSampler.mag);
			sampler.setMipFilter(mGlobalSampler.mip);

			for(size_t i = 0; i < mMaxSamplers; ++i )
			{
				//NOTE: glTexParameteri DOESN'T take effect to active texture unit, but to the texture bound to active texture unit
				//use samplers (glBindSampler) equivalent to D3D

				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_COMPARE_MODE, sampler.isShadowSampler() ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);

				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_COMPARE_FUNC, GLESDepthSencilFuncConverter(sampler.getTestFunc()));
				if (sampler.isShadowSampler())
				{
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				else
				{
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GLESTexAddressModeConverter(mGlobalSampler.u));
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GLESTexAddressModeConverter(mGlobalSampler.v));
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GLESTexAddressModeConverter(mGlobalSampler.w));

					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GLESTexFilterConverter(mGlobalSampler.mag));
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GLESTexFilterConverter(mGlobalSampler.min, mGlobalSampler.mip));
				}
				//Max Anisotropy not supported by GLES

				mDefaultSamplerState->setSampler(i, sampler);
			}
			mTextureState = mDefaultSamplerState;

		}

		/** @brief  */
		inline void		setFillMode(FILL_MODE fm)
		{
			if( mFillMode != fm )
			{
				mFillMode = fm;
			}
		}

		/** @brief  */
		inline void		setCullMode(CULL_MODE cm)
		{
			if( mCullMode != cm)
			{
				mCullMode = cm;
				if( mCullMode != HCM_NONE )
				{
					GLenum front = mDefaultCullMode == HCM_CCW ? GL_CW : GL_CCW;
					::glFrontFace(front);
					::glCullFace(GL_BACK);
					::glEnable(GL_CULL_FACE);
				}
				else
					::glDisable(GL_CULL_FACE);
			}
		}


		/** @brief  */
		inline void		setColorWriteProperty(const ColorWriteProperty* prop)
		{
			if( prop == NULL )
			{
				if( mLastColorWriteMask != mDefaultColorWriteMask )
				{
					mLastColorWriteMask = mDefaultColorWriteMask;
					mColorWriteMask = mDefaultColorWriteMask;
					::glColorMask( mColorWriteMask&CWC_RED, mColorWriteMask&CWC_GREEN, mColorWriteMask&CWC_BLUE, mColorWriteMask&CWC_ALPHA);
				}
			}
			else if( mColorWriteMask != prop->getWritingColorChannel() )
			{
				mColorWriteMask = prop->getWritingColorChannel();
				mLastColorWriteMask = prop->getWritingColorChannel();
				::glColorMask( mColorWriteMask&CWC_RED, mColorWriteMask&CWC_GREEN, mColorWriteMask&CWC_BLUE, mColorWriteMask&CWC_ALPHA);
			}
		}

		/** @brief  */
		inline void		setAlphaBlendProperty(const AlphaBlendProperty* prop)
		{
			if( mLastAlphaState == prop )
				return;

			if( prop == NULL )
				::glDisable(GL_BLEND);
			else
			{
				::glEnable(GL_BLEND);
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
		void		setSamplerState(const SamplerState* prop)
		{
			if( mLastTextureState == prop )
				return;

			if( prop == NULL )
			{
				if( mLastTextureState != mDefaultSamplerState )
				{
					mLastTextureState = mDefaultSamplerState;
					this->compareSetSampler(*mDefaultSamplerState);
					return;
				}
			}
			else
				this->compareSetSampler(*prop);
			mLastTextureState = prop;
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
		void		compareSetSampler(const Sampler* sampler, index_t i)
		{
			if(sampler == NULL)
				sampler = &Sampler::DEFAULT;

			const Sampler& newsampler = *sampler;
			Sampler& original = *(mTextureState->getSampler(i));

			//anisotropy settings
			if (newsampler.isUsingGlobalAnisotropy())
			{
				if (original.getMaxAnisotropy() != mGlobalSampler.maxAnisotropy)
				{
					original.setMaxAnisotropy((uint8)mGlobalSampler.maxAnisotropy);
				}
			}
			else if (original.getMaxAnisotropy() != newsampler.getMaxAnisotropy())
			{
				original.setMaxAnisotropy((uint8)newsampler.getMaxAnisotropy());
			}

			//LOD (mip maps)
			if (original.getMaxLOD() != newsampler.getMaxLOD())
			{
				original.setMaxLOD(newsampler.getMaxLOD());
				::glSamplerParameterf(mGLSamplers[i], GL_TEXTURE_MAX_LOD, original.getMaxLOD());
			}
			if (original.getMinLOD() != newsampler.getMinLOD())
			{
				original.setMinLOD(newsampler.getMinLOD());
				::glSamplerParameterf(mGLSamplers[i], GL_TEXTURE_MIN_LOD, original.getMinLOD());
			}

			//compare function
			if (original.getTestFunc() != newsampler.getTestFunc())
			{
				original.setTestFunc(newsampler.getTestFunc());
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_COMPARE_FUNC, GLESDepthSencilFuncConverter(newsampler.getTestFunc()));
			}

			//shadow maps settings
			if (original.isShadowSampler() != newsampler.isShadowSampler())
			{
				original.setShadowSampler(newsampler.isShadowSampler());
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_COMPARE_MODE, newsampler.isShadowSampler() ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
			}
			if (newsampler.isShadowSampler())
			{
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
			else
			{
				//not shadow maps use global setting
				if (newsampler.isUsingGlobalAddressMode())
				{
					if (original.getAdressModeU() != mGlobalSampler.u)
					{
						original.setAdressModeU(mGlobalSampler.u);
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GLESTexAddressModeConverter(original.getAdressModeU()));
					}

					if (original.getAdressModeV() != mGlobalSampler.v)
					{
						original.setAdressModeV(mGlobalSampler.v);
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GLESTexAddressModeConverter(original.getAdressModeV()));
					}

					if (original.getAdressModeW() != mGlobalSampler.w)
					{
						original.setAdressModeW(mGlobalSampler.w);
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GLESTexAddressModeConverter(original.getAdressModeW()));
					}
				}
				else
				{
					if (original.getAdressModeU() != newsampler.getAdressModeU())
					{
						original.setAdressModeU(newsampler.getAdressModeU());
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GLESTexAddressModeConverter(original.getAdressModeU()));
					}

					if (original.getAdressModeV() != newsampler.getAdressModeV())
					{
						original.setAdressModeV(newsampler.getAdressModeV());
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GLESTexAddressModeConverter(original.getAdressModeV()));
					}

					if (original.getAdressModeW() != newsampler.getAdressModeW())
					{
						original.setAdressModeW(newsampler.getAdressModeW());
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GLESTexAddressModeConverter(original.getAdressModeW()));

					}
				}

				if (newsampler.isUsingGlobalFilter())
				{
					if (original.getMagFilter() != mGlobalSampler.mag)
					{
						original.setMagFilter(mGlobalSampler.mag);
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GLESTexFilterConverter(original.getMagFilter()));
					}

					if (original.getMinFilter() != mGlobalSampler.min || original.getMipFilter() != mGlobalSampler.mip)
					{
						original.setMinFilter(mGlobalSampler.min);
						original.setMipFilter(mGlobalSampler.mip);
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GLESTexFilterConverter(original.getMinFilter(), original.getMipFilter()));
					}
				}
				else
				{
					if (original.getMagFilter() != newsampler.getMagFilter())
					{
						original.setMagFilter(newsampler.getMagFilter());
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GLESTexFilterConverter(original.getMagFilter()));
					}

					if (original.getMinFilter() != newsampler.getMinFilter() || original.getMipFilter() != newsampler.getMipFilter())
					{
						original.setMinFilter(newsampler.getMinFilter());
						original.setMipFilter(newsampler.getMipFilter());
						::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GLESTexFilterConverter(original.getMinFilter(), original.getMipFilter()));
					}

					if (original.getMipmapLODBias() != newsampler.getMipmapLODBias())
					{
						original.setMipmapLODBias(newsampler.getMipmapLODBias());
					}
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void	compareSetAlphaProperty(const AlphaBlendProperty& prop)
		{
			if( mAlphaBlendState->getIndependentBlend() == prop.getIndependentBlend() )
			{
				if( prop.getIndependentBlend() )
				{
					if( mAlphaBlendState->getSourceBlend() != prop.getSourceBlend() 
						|| mAlphaBlendState->getDestBlend() != prop.getDestBlend() 
						|| mAlphaBlendState->getSourceAlphaBlend() != prop.getSourceAlphaBlend()
						|| mAlphaBlendState->getDestAlphaBlend() != prop.getDestAlphaBlend() )
					{
						GLenum srcAlpha = GLESBlendModeConverter( prop.getSourceAlphaBlend() );
						GLenum destAlpha = GLESBlendModeConverter( prop.getDestAlphaBlend() );
						GLenum srcColor = GLESBlendModeConverter(prop.getSourceBlend());
						GLenum destColor = GLESBlendModeConverter(prop.getDestBlend());

						mAlphaBlendState->setDestBlend( prop.getDestBlend() );
						mAlphaBlendState->setSourceBlend( prop.getSourceBlend() );
						mAlphaBlendState->setSourceAlphaBlend( prop.getSourceAlphaBlend() );
						mAlphaBlendState->setDestAlphaBlend( prop.getDestAlphaBlend() );
						::glBlendFuncSeparate(srcColor, destColor, srcAlpha, destAlpha);
					}

					if( mAlphaBlendState->getBlendOperation() != prop.getBlendOperation()
						|| mAlphaBlendState->getAlphaBlendOperation() != prop.getAlphaBlendOperation() )
					{
						GLenum colorOp = GLESBlendOpConverter(prop.getBlendOperation());
						GLenum alphaOp = GLESBlendOpConverter(prop.getAlphaBlendOperation());
						mAlphaBlendState->setBlendOperation( prop.getBlendOperation() );
						mAlphaBlendState->setAlphaBlendOperation( prop.getAlphaBlendOperation() );
						::glBlendEquationSeparate(colorOp, alphaOp);
					}
				}
				else
				{
					if( mAlphaBlendState->getSourceBlend() != prop.getSourceBlend() 
						|| mAlphaBlendState->getDestBlend() != prop.getDestBlend() )
					{
						mAlphaBlendState->setDestBlend( prop.getDestBlend() );
						mAlphaBlendState->setSourceBlend( prop.getSourceBlend() );
						::glBlendFunc(GLESBlendModeConverter(prop.getSourceBlend()), GLESBlendModeConverter(prop.getDestBlend()));
					}

					if( mAlphaBlendState->getBlendOperation() != prop.getBlendOperation() )
					{
						mAlphaBlendState->setBlendOperation( prop.getBlendOperation() );
						::glBlendEquation( GLESBlendOpConverter(prop.getBlendOperation()) );
					}
				}
			}
			else
			{
				if( prop.getIndependentBlend() )
				{
					GLenum srcAlpha = GLESBlendModeConverter( prop.getSourceAlphaBlend() );
					GLenum destAlpha = GLESBlendModeConverter( prop.getDestAlphaBlend() );
					GLenum srcColor = GLESBlendModeConverter(prop.getSourceBlend());
					GLenum destColor = GLESBlendModeConverter(prop.getDestBlend());

					if( mAlphaBlendState->getSourceBlend() != prop.getSourceBlend() 
						|| mAlphaBlendState->getDestBlend() != prop.getDestBlend() 
						|| mAlphaBlendState->getSourceAlphaBlend() != prop.getSourceAlphaBlend()
						|| mAlphaBlendState->getDestAlphaBlend() != prop.getDestAlphaBlend() )
					{
						mAlphaBlendState->setDestBlend( prop.getDestBlend() );
						mAlphaBlendState->setSourceBlend( prop.getSourceBlend() );
						mAlphaBlendState->setSourceAlphaBlend( prop.getSourceAlphaBlend() );
						mAlphaBlendState->setDestAlphaBlend( prop.getDestAlphaBlend() );
					}
					::glBlendFuncSeparate(srcColor, destColor, srcAlpha, destAlpha);


					GLenum colorOp = GLESBlendOpConverter(prop.getBlendOperation());
					GLenum alphaOp = GLESBlendOpConverter(prop.getAlphaBlendOperation());
					if( mAlphaBlendState->getBlendOperation() != prop.getBlendOperation()
						|| mAlphaBlendState->getAlphaBlendOperation() != prop.getAlphaBlendOperation() )
					{
						mAlphaBlendState->setBlendOperation( prop.getBlendOperation() );
						mAlphaBlendState->setAlphaBlendOperation( prop.getAlphaBlendOperation() );
					}
					::glBlendEquationSeparate(colorOp, alphaOp);
				}
				else
				{
					if( mAlphaBlendState->getSourceBlend() != prop.getSourceBlend() 
						|| mAlphaBlendState->getDestBlend() != prop.getDestBlend() )
					{
						mAlphaBlendState->setDestBlend( prop.getDestBlend() );
						mAlphaBlendState->setSourceBlend( prop.getSourceBlend() );
					}

					if( mAlphaBlendState->getBlendOperation() != prop.getBlendOperation() )
						mAlphaBlendState->setBlendOperation( prop.getBlendOperation() );

					::glBlendFunc(GLESBlendModeConverter(prop.getSourceBlend()), GLESBlendModeConverter(prop.getDestBlend()));
					::glBlendEquation( GLESBlendOpConverter(prop.getBlendOperation()) );
				}
			}

		}

		//////////////////////////////////////////////////////////////////////////
		void	compareSetDepthProperty(const DepthProperty& prop)
		{
			if( mDepthState->isTestEnable() != prop.isTestEnable() )
			{
				mDepthState->setTestEnable( prop.isTestEnable() );
				if( mDepthState->isTestEnable() )
					::glEnable(GL_DEPTH_TEST);
				else
					::glDisable(GL_DEPTH_TEST);
			}

			if( prop.isTestEnable() )
			{
				if( mDepthState->getDepthTestFunc() != prop.getDepthTestFunc() )
				{
					mDepthState->setDepthTestFunc( prop.getDepthTestFunc() );
					::glDepthFunc( GLESDepthSencilFuncConverter( mDepthState->getDepthTestFunc() ) );
				}
			}

			if( mDepthState->isWriteEnable() != prop.isWriteEnable() )
			{
				mDepthState->setWriteEnable( prop.isWriteEnable() );
				::glDepthMask( (GLboolean)mDepthState->isWriteEnable() );
			}

			if( mDepthState->getDepthBias() != prop.getDepthBias() || !Math::Equal<scalar>(mDepthState->getSlopeScaledDepthBias(),prop.getSlopeScaledDepthBias(),1e-6f) )
			{
				mDepthState->setDepthBias( prop.getDepthBias() );
				mDepthState->setSlopeScaledDepthBias( prop.getSlopeScaledDepthBias() );

				float bias = (float)mDepthState->getDepthBias();
				bias /= 1000.0f;
				if( bias > 0 )
				{
					::glPolygonOffset(mDepthState->getSlopeScaledDepthBias(), bias);
					::glEnable(GL_POLYGON_OFFSET_FILL);
				}
				else
					::glDisable(GL_POLYGON_OFFSET_FILL);
			}

			//depth clip is part of clipping
			if( mDepthState->isEnableDepthClip() != prop.isEnableDepthClip() )
			{
				mDepthState->setEneableDepthClip( prop.isEnableDepthClip() );
			}

			//setDepthBiasClamp is not valid for GLES
		}

		//////////////////////////////////////////////////////////////////////////
		void		compareSetStencilProperty(const StencilProperty& prop)
		{
			if( mStencilState->isEnable() != prop.isEnable() )
			{
				mStencilState->setEnable( prop.isEnable() );
				if( mStencilState->isEnable() )
					::glEnable(GL_STENCIL_TEST);
				else
					::glDisable(GL_STENCIL_TEST);
			}
			if( !prop.isEnable() )
				return;

			if( mStencilState->getWriteMask() != prop.getWriteMask() )
			{
				mStencilState->setWriteMask( prop.getWriteMask() );
				::glStencilMask( mStencilState->getWriteMask() );
			}

			if( mStencilState->getReadMask() != prop.getReadMask()
				|| mStencilState->getRef() != prop.getRef() )
			{
				mStencilState->setReadMask( prop.getReadMask() );
				mStencilState->setRef( prop.getRef() );

				if( mStencilState->getFrontFaceTestFunc() != prop.getFrontFaceTestFunc() )
					mStencilState->setFrontFaceTestFunc( prop.getFrontFaceTestFunc() );

				if( mStencilState->getBackFaceTestFunc() != prop.getBackFaceTestFunc() )
					mStencilState->setBackFaceTestFunc( prop.getBackFaceTestFunc() );

				::glStencilFuncSeparate( GL_FRONT, 
					GLESDepthSencilFuncConverter(mStencilState->getFrontFaceTestFunc()), mStencilState->getRef(), mStencilState->getReadMask() );

				::glStencilFuncSeparate( GL_BACK, 
					GLESDepthSencilFuncConverter(mStencilState->getFrontFaceTestFunc()), mStencilState->getRef(), mStencilState->getReadMask() );
			}

			if( mStencilState->getFrontFaceFailOperation() != prop.getFrontFaceFailOperation()
				|| mStencilState->getFrontFaceDepthFailOperation() != prop.getFrontFaceDepthFailOperation()
				|| mStencilState->getFrontFacePassOperation() != prop.getFrontFacePassOperation() )
			{
				mStencilState->setFrontFaceFailOperation( prop.getFrontFaceFailOperation() );
				mStencilState->setFrontFacePassOperation( prop.getFrontFacePassOperation() );
				mStencilState->setFrontFaceDepthFailOperation( prop.getFrontFaceDepthFailOperation() );

				::glStencilOpSeparate( GL_FRONT, 
					GLESStencilOpConverter(mStencilState->getFrontFaceFailOperation()), 
					GLESStencilOpConverter(mStencilState->getFrontFaceDepthFailOperation()),
					GLESStencilOpConverter(mStencilState->getFrontFacePassOperation()) );
			}

			if( mStencilState->getBackFaceFailOperation() != prop.getBackFaceFailOperation()
				|| mStencilState->getBackFaceDepthFailOperation() != prop.getBackFaceDepthFailOperation()
				|| mStencilState->getBackFacePassOperation() != prop.getBackFacePassOperation() )
			{
				mStencilState->setBackFaceFailOperation( prop.getBackFaceFailOperation() );
				mStencilState->setBackFacePassOperation( prop.getBackFacePassOperation() );
				mStencilState->setBackFacePassOperation( prop.getBackFacePassOperation() );

				::glStencilOpSeparate( GL_BACK, 
					GLESStencilOpConverter(mStencilState->getBackFaceFailOperation()), 
					GLESStencilOpConverter(mStencilState->getBackFaceDepthFailOperation()),
					GLESStencilOpConverter(mStencilState->getBackFacePassOperation()) );
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void		compareSetScissor(const ScissorProperty& prop)
		{
			if( mScissorState->getValidCount() != prop.getValidCount() )
			{
				mScissorState->setValidCount( prop.getValidCount() );
				GLboolean bEnable = mScissorState->getValidCount() != 0;
				if( bEnable )
					::glEnable(GL_SCISSOR_TEST);
				else
					::glDisable(GL_SCISSOR_TEST);
				if( !bEnable )
					return;
			}

			if( mScissorState->getValidCount() > 0 )
			{
				//GLES support only one scissor
				const Box2i* boxes = prop.getScissorBoxes();
				const Box2i* originalBoxes = mScissorState->getScissorBoxes();
				if( originalBoxes[0] != boxes[0] )
				{
					mScissorState->setScissorBox(0, boxes[0] );
					GLint x = boxes->getMinPoint().x;
					GLint y = boxes->getMinPoint().y;

					GLsizei w = boxes->getWidth();
					GLsizei h = boxes->getHeight();
					::glScissor(x, y, w, h);
				}
			}

		}

		//////////////////////////////////////////////////////////////////////////
		void		compareSetSampler(const SamplerState& prop)
		{
			size_t texCount = std::min<size_t>( mTextureState->getSamplerCount(), prop.getSamplerCount() );

			for(size_t i = 0; i < texCount; ++i)
			{
				const Sampler& newsampler = *prop.getSampler(i);
				Sampler& original = *(mTextureState->getSampler(i));

				//anisotropy settings
				if (newsampler.isUsingGlobalAnisotropy())
				{
					if (original.getMaxAnisotropy() != mGlobalSampler.maxAnisotropy)
					{
						original.setMaxAnisotropy((uint8)mGlobalSampler.maxAnisotropy);
					}
				}
				else if (original.getMaxAnisotropy() != newsampler.getMaxAnisotropy())
				{
					original.setMaxAnisotropy((uint8)newsampler.getMaxAnisotropy());
				}

				//LOD (mip maps)
				if (original.getMaxLOD() != newsampler.getMaxLOD())
				{
					original.setMaxLOD(newsampler.getMaxLOD());
					::glSamplerParameterf(mGLSamplers[i], GL_TEXTURE_MAX_LOD, original.getMaxLOD());
				}
				if (original.getMinLOD() != newsampler.getMinLOD())
				{
					original.setMinLOD(newsampler.getMinLOD());
					::glSamplerParameterf(mGLSamplers[i], GL_TEXTURE_MAX_LOD, original.getMinLOD());
				}

				//compare function
				if (original.getTestFunc() != newsampler.getTestFunc())
				{
					original.setTestFunc(newsampler.getTestFunc());
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_COMPARE_FUNC, GLESDepthSencilFuncConverter(newsampler.getTestFunc()));
				}

				//not shadow maps use global setting
				if (original.isShadowSampler() != newsampler.isShadowSampler())
				{
					original.setShadowSampler(newsampler.isShadowSampler());
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_COMPARE_MODE, newsampler.isShadowSampler() ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
				}
				if (newsampler.isShadowSampler())
				{
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				else
				{
					//not shadow maps use global setting
					if (newsampler.isUsingGlobalAddressMode())
					{
						if (original.getAdressModeU() != mGlobalSampler.u)
						{
							original.setAdressModeU(mGlobalSampler.u);
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GLESTexAddressModeConverter(original.getAdressModeU()));
						}

						if (original.getAdressModeV() != mGlobalSampler.v)
						{
							original.setAdressModeV(mGlobalSampler.v);
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GLESTexAddressModeConverter(original.getAdressModeV()));
						}

						if (original.getAdressModeW() != mGlobalSampler.w)
						{
							original.setAdressModeW(mGlobalSampler.w);
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GLESTexAddressModeConverter(original.getAdressModeW()));
						}
					}
					else
					{
						if (original.getAdressModeU() != newsampler.getAdressModeU())
						{
							original.setAdressModeU(newsampler.getAdressModeU());
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_S, GLESTexAddressModeConverter(original.getAdressModeU()));
						}

						if (original.getAdressModeV() != newsampler.getAdressModeV())
						{
							original.setAdressModeV(newsampler.getAdressModeV());
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_T, GLESTexAddressModeConverter(original.getAdressModeV()));
						}

						if (original.getAdressModeW() != newsampler.getAdressModeW())
						{
							original.setAdressModeW(newsampler.getAdressModeW());
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_WRAP_R, GLESTexAddressModeConverter(original.getAdressModeW()));

						}
					}

					if (newsampler.isUsingGlobalFilter())
					{
						if (original.getMagFilter() != mGlobalSampler.mag)
						{
							original.setMagFilter(mGlobalSampler.mag);
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GLESTexFilterConverter(original.getMagFilter()));
						}

						if (original.getMinFilter() != mGlobalSampler.min || original.getMipFilter() != mGlobalSampler.mip)
						{
							original.setMinFilter(mGlobalSampler.min);
							original.setMipFilter(mGlobalSampler.mip);
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GLESTexFilterConverter(original.getMinFilter(), original.getMipFilter()));
						}
					}
					else
					{
						if (original.getMagFilter() != newsampler.getMagFilter())
						{
							original.setMagFilter(newsampler.getMagFilter());
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MAG_FILTER, GLESTexFilterConverter(original.getMagFilter()));
						}

						if (original.getMinFilter() != newsampler.getMinFilter() || original.getMipFilter() != newsampler.getMipFilter())
						{
							original.setMinFilter(newsampler.getMinFilter());
							original.setMipFilter(newsampler.getMipFilter());
							::glSamplerParameteri(mGLSamplers[i], GL_TEXTURE_MIN_FILTER, GLESTexFilterConverter(original.getMinFilter(), original.getMipFilter()));
						}

						if (original.getMipmapLODBias() != newsampler.getMipmapLODBias())
						{
							original.setMipmapLODBias(newsampler.getMipmapLODBias());
						}
					}
				}
			}
		}

		size_t					mMaxSamplers;
		GLuint*					mGLSamplers;
		//note : color / fog / alpha clip states are managed through shaders

		AlphaBlendProperty*		mAlphaBlendState;
		DepthProperty*			mDepthState;
		StencilProperty*		mStencilState;
		ScissorProperty*		mScissorState;
		SamplerState*			mTextureState;

		AlphaBlendProperty*		mDefaultAlphaBlendState;
		DepthProperty*			mDefaultDepthState;
		StencilProperty*		mDefaultStencilState;
		ScissorProperty*		mDefaultScissorState;
		SamplerState*		mDefaultSamplerState;

		const RenderPropertySet*		mLastPropertySet;
		const AlphaBlendProperty*		mLastAlphaState;
		const DepthProperty*			mLastDepthState;
		const StencilProperty*		mLastStencilState;
		const ScissorProperty*		mLastScissorState;
		const SamplerState*		mLastTextureState;

		SAMPLER_OPTION			mGlobalSampler;
		int						mLastColorWriteMask;
		int						mColorWriteMask;
		int						mDefaultColorWriteMask;
		FILL_MODE				mFillMode;
		CULL_MODE				mCullMode;
		FILL_MODE				mDefaultFillMode;
		CULL_MODE				mDefaultCullMode;
		FILL_MODE				mLastFillMode;
		CULL_MODE				mLastCullMode;
	};

}//namespace Blade

#endif//BLADE_USE_GLES
#endif//__Blade_GLESStateBlock_h__