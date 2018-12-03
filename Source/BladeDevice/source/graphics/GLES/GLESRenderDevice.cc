/********************************************************************
	created:	2014/01/14
	filename: 	GLESRenderDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESRenderDevice.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESRenderWindow.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/public/graphics/IRenderView.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <interface/public/graphics/IVertexSource.h>
#include <graphics/GLES/GLESGraphicsResourceManager.h>
#include <graphics/GLES/GLESShader.h>
#include <graphics/GLES/GLESVertexDeclaration.h>
#include <graphics/GLES/GLESIndexBuffer.h>
#include <graphics/GLES/GLESVertexBuffer.h>
#include <graphics/GLES/GLESTexture.h>
#include <graphics/GLES/GLESVolumeTexture.h>
#include <graphics/GLES/GLESCubeTexture.h>
#include <graphics/GLES/GLESRenderTarget.h>
#include "GLESStateBlock.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GLESRenderDevice::GLESRenderDevice()
		:mContext(NULL)
		,mStateBlock(NULL)
	{
		RENDERDEV_CAPS& caps = mCaps;
		caps.mMaxMRT = 1;
		caps.mMaxStreams = 4;
		caps.mMaxTextures = 4;

		mGlobalFilter.maxAnisotropy = 1;
		mGlobalFilter.u = TAM_CLAMP;
		mGlobalFilter.v = TAM_CLAMP;
		mGlobalFilter.w = TAM_CLAMP;
		mGlobalFilter.min = TFM_POINT;
		mGlobalFilter.mip = TFM_POINT;
		mGlobalFilter.mag = TFM_LINEAR;

		mStateBlock = BLADE_NEW GLESStateBlock();

		mLastTarget = NULL;
		mLastIndex = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	GLESRenderDevice::~GLESRenderDevice()
	{

	}

	/************************************************************************/
	/* common IDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	GLESRenderDevice::open()
	{
		IGraphicsResourceManager::interchange(GLESGraphicsResourceManager::TYPE);
		mContext = GLESContext::createContext();
		GLESGraphicsResourceManager::setContex(mContext);
		return mContext != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESRenderDevice::isOpen() const
	{
		return mContext != NULL && mContext->isInitialized();
	}

	//////////////////////////////////////////////////////////////////////////
	void	GLESRenderDevice::close()
	{
		mStateBlock->clearup();

		if( mContext != NULL )
		{
			mContext->deinitialize();
			BLADE_DELETE mContext;
			mContext = NULL;
		}
		mLastProgram = HSHADER::EMPTY;
		mLastTarget = NULL;
		mLastIndex = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESRenderDevice::reset()
	{
		mLastProgram = HSHADER::EMPTY;
		mLastTarget = NULL;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESRenderDevice::update()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	GLESRenderDevice::getDeviceOption(ParamList& defVal, Options& outOption)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESRenderDevice::applyDeviceOption(const ParamList& result)
	{
		return true;
	}

	/************************************************************************/
	/* IRenderDevice specs                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const RENDERDEV_CAPS&	GLESRenderDevice::getDeviceCaps() const
	{
		return mCaps;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*	GLESRenderDevice::createRenderWindow(IWindow* pwin, bool depthStencil/* = false*/, PixelFormat depthFormat/* = PF_DEPTH24S8*/)
	{
		ScopedLock lock(mThreadLock);
		const TString& name = pwin->getUniqueName();
		HRENDERWINDOW& hWin = mRenderWindowList[name];
		HRENDERTARGET& hTarget = mRenderTargetList[name];
		if( hWin != NULL || hTarget != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE, BTString("unable to create renderwindow -\"")+name+BTString("\", a instance with the same name already exist."));

		bool ret = mContext->isInitialized();
		if( !ret )
			ret = mContext->initialize(pwin);
		assert(ret);
		assert( !mContext->checkCurrent() );
		ret = mContext->makeCurrent(true);
		assert(ret);

		GLESRenderWindow* window = GLESRenderWindow::createGLESRenderWindow(pwin, this, mContext);

		if( ret )
		{
			//init device caps
			GLint maxGLTexture = 0;
			::glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxGLTexture);
			assert( ::glGetError() == GL_NO_ERROR );

			GLint maxVertexAttrib = 0;
			::glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttrib);
			assert( ::glGetError() == GL_NO_ERROR );

			GLint maxFBOAttachment = 0;
			::glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxFBOAttachment);
			assert( ::glGetError() == GL_NO_ERROR );

			mCaps.mMaxMRT = maxFBOAttachment;
			mCaps.mMaxTextures = maxGLTexture;
			//note: GL doesn't limit STREAMS (bound VBO count), but vertex attrib is limited
			//1 VBO can bound to multiple attrib, so 1 stream(VBO) may still exceed the attrib limit
			mCaps.mMaxStreams = maxVertexAttrib;
			GLESVertexDeclaration::setAttribLimit(maxVertexAttrib);

			mStateBlock->setup(mCaps.mMaxTextures);
		}

		hWin.bind(window);
		hTarget = hWin;
		return window;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderWindow*	GLESRenderDevice::getRenderWindow(const TString& name) const
	{
		ScopedLock lock(mThreadLock);
		RenderWindowMap::const_iterator i = mRenderWindowList.find(name);
		if( i == mRenderWindowList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			GLESRenderDevice::getNumRenderWindows() const
	{
		ScopedLock lock(mThreadLock);
		return mRenderWindowList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::notifyRenderWindowChange(IRenderWindow* /*rw*/)
	{
		//do nothing.
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	GLESRenderDevice::createRenderTarget(const TString& name, size_t width, size_t height,
		bool creatColorBuffer/* = false*/,bool createDepthBuffer/* = false*/,
		PixelFormat format/* = PF_UNDEFINED*/,PixelFormat depthFormat/* = PF_UNDEFINED*/)
	{
		ScopedLock lock(mThreadLock);

		HRENDERTARGET& hTarget = mRenderTargetList[name];
		if( hTarget != NULL )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("unable to create rendertarget -\"")+name+BTString("\", a instance with the same name already exist.")  );

		hTarget.bind( BLADE_NEW GLESRenderTarget(name, this, width, height) );

		if( creatColorBuffer )
		{
			HTEXTURE hColorBuffer = IGraphicsResourceManager::getSingleton().createTexture(width, height, depthFormat, 1, IGraphicsBuffer::GBUF_RENDERTARGET,
				TT_2DIM, IMGO_DEFAULT, 1);
			hTarget->setColorBuffer(0, hColorBuffer);
		}

		if( createDepthBuffer )
		{
			HTEXTURE hDepthBuffer = IGraphicsResourceManager::getSingleton().createDepthStencilBuffer(width,height,depthFormat);
			hTarget->setDepthBuffer( hDepthBuffer );
		}
		return hTarget;
	}

	//////////////////////////////////////////////////////////////////////////
	const SAMPLER_OPTION&	GLESRenderDevice::getGlobalSamplerOption() const
	{
		return mGlobalFilter;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setGlobalSamplerOption(const SAMPLER_OPTION& filter)
	{
		mGlobalFilter = filter;
	}

	//////////////////////////////////////////////////////////////////////////
	IRenderTarget*	GLESRenderDevice::getRenderTarget(const TString& name) const
	{
		ScopedLock lock(mThreadLock);
		RenderTargetMap::const_iterator i = mRenderTargetList.find(name);
		if( i == mRenderTargetList.end() )
			return NULL;
		else
			return i->second;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			GLESRenderDevice::getNumRenderTargets() const
	{
		ScopedLock lock(mThreadLock);
		return mRenderTargetList.size();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESRenderDevice::destroyRenderTarget(const TString& name)
	{
		ScopedLock lock(mThreadLock);
		RenderWindowMap::iterator i = mRenderWindowList.find(name);
		if( i != mRenderWindowList.end() )
		{
			mRenderWindowList.erase(i);
			mRenderTargetList.erase(name);

			if( mRenderWindowList.size() == 0 )
			{
				//no render targets if no window at all
				assert(mRenderTargetList.size() == 0);
				//re-init the device (reopen) ?
				//this->close();
				//this->open();
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setRenderTarget(IRenderTarget* target)
	{
		if( target == NULL )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid render target."));

		if( mContext == NULL || !mContext->isInitialized() )
			BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("context not ready."));

		if (mLastTarget != target && target->activate())
		{
			target->setViewRect(0, 0, (int32)target->getViewWidth(), (int32)target->getViewHeight());
			mLastTarget = target;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESRenderDevice::beginScene()
	{
		//for(index_t i = 0; i < mCaps.mMaxTextures; ++i)
		//{
		//	::glActiveTexture(GL_TEXTURE0 + i);
		//	assert(::glGetError() == GL_NO_ERROR);
		//	::glBindTexture(GL_TEXTURE_2D, 0);
		//	assert(::glGetError() == GL_NO_ERROR);
		//	::glBindTexture(GL_TEXTURE_3D, 0);
		//	assert(::glGetError() == GL_NO_ERROR);
		//	::glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		//	assert(::glGetError() == GL_NO_ERROR);
		//}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESRenderDevice::endScene()
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESRenderDevice::clear(uint32 frameBufferType, const Color& color, scalar depth/* = 1.0*/, uint16 stencil/* = 0*/)
	{
		GLbitfield mask = 0;
		bool clearColor = (frameBufferType&FBT_COLOR);
		if(clearColor)
		{
			mask |= GL_COLOR_BUFFER_BIT;
			::glClearColor(color.r, color.g, color.b, color.a);
			assert(::glGetError() == GL_NO_ERROR);
		}

		bool clearDepth = frameBufferType & FBT_DEPTH;
		if(clearDepth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
			::glClearDepthf(depth);
			assert(::glGetError() == GL_NO_ERROR);
		}

		bool clearStencil = frameBufferType & FBT_STENCIL;
		if(clearStencil)
		{
			mask |= GL_STENCIL_BUFFER_BIT;
			::glClearStencil( (GLint)stencil );	//FIXME: stencil can be 32bit
			assert(::glGetError() == GL_NO_ERROR);
		}

		if (mask != 0)
		{
			//color mask affects glClear by GL spec, disable it
			int colorMask = mStateBlock->mColorWriteMask;
			bool changeColorMask = clearColor && colorMask != CWC_ALL;
			if(changeColorMask)
				::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			//depth mask affects glClear
			bool changeDepthMask = clearDepth && !mStateBlock->mDepthState->isWriteEnable();
			if (changeDepthMask)
			{
				::glDepthMask(GL_TRUE);
				assert(::glGetError() == GL_NO_ERROR);
			}

			uint8 stencilMask = mStateBlock->mStencilState->getWriteMask();
			bool changeStencilMask = clearStencil && stencilMask != 0xFF;
			if (changeStencilMask)
				::glStencilMask(0xFF);

			::glClear(mask);
			assert(::glGetError() == GL_NO_ERROR);

			//restore states
			if (changeColorMask)
				mStateBlock->mColorWriteMask = CWC_ALL;
			if (changeDepthMask)
				mStateBlock->mDepthState->setWriteEnable(true);
			if (changeStencilMask)
				mStateBlock->mStencilState->setWriteMask(0xFF);
		}
		assert(::glGetError() == GL_NO_ERROR);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setRenderView(IRenderView* view)
	{
		if( view != NULL )
		{
			int32 left = view->getPixelLeft();
			int32 top = view->getPixelTop();
			int32 width = view->getPixelWidth();
			int32 height = view->getPixelHeight();
			::glViewport(left, top, width, height);
			::glScissor(left, top, width, height);	//note: glClear ignores viewport, only scissor works
			assert(::glGetError() == GL_NO_ERROR);

			if (mLastTarget != NULL)
				mLastTarget->setViewRect(left, top, width, height);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setRenderProperty(const RenderPropertySet& propSet)
	{
		mStateBlock->setFillMode( propSet.getFillMode() );
		mStateBlock->setCullMode( propSet.getCullMode() );

		mStateBlock->setColorWriteProperty( static_cast<ColorWriteProperty*>(propSet.getProperty(RPT_COLORWIRTE)) );
		mStateBlock->setAlphaBlendProperty( static_cast<AlphaBlendProperty*>(propSet.getProperty(RPT_ALPHABLEND)) );
		mStateBlock->setDepthProperty( static_cast<DepthProperty*>(propSet.getProperty(RPT_DEPTH)) );
		mStateBlock->setStencilProperty( static_cast<StencilProperty*>(propSet.getProperty(RPT_STENCIL)) );
		mStateBlock->setScissorProperty( static_cast<ScissorProperty*>(propSet.getProperty(RPT_SCISSOR)) );
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setTexture(EShaderType type, index_t samplerIndex, const HTEXTURE& texture, const Sampler* sampler/* = NULL*/)
	{
		if( texture == NULL )
		{
			::glActiveTexture(GL_TEXTURE0 + samplerIndex);
			assert(::glGetError() == GL_NO_ERROR);
			::glBindTexture(GL_TEXTURE_2D, 0);
			assert(::glGetError() == GL_NO_ERROR);
			::glBindTexture(GL_TEXTURE_3D, 0);
			assert(::glGetError() == GL_NO_ERROR);
			::glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			assert(::glGetError() == GL_NO_ERROR);
			return;
		}

		//no read access, probably a surface(back buffer render target,see @D3D9RenderSurface ), not a texture
		if( (texture->getGpuAccess() & IGraphicsBuffer::GBAF_READ) == 0 )
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("unable to set a none-read texture.") );

		mStateBlock->compareSetSampler(sampler, samplerIndex);

		bool result = false;
		if( type == SHADER_ANY || type == SHADER_VERTEX || type == SHADER_FRAGMENT )
		{
			ETextureType texType = texture->getType();
			assert(texType >= TT_1DIM && texType <= TT_CUBE);

			static const GLenum GL_TEXTURE_TARGETS[] =
			{
				GL_TEXTURE_2D,
				GL_TEXTURE_2D,
				GL_TEXTURE_3D,
				GL_TEXTURE_CUBE_MAP,
			};
			GLenum glTarget = GL_TEXTURE_TARGETS[ texType ];
			GLuint glTex = 0;

			if( texType == TT_2DIM || texType == TT_1DIM )
			{
				GLESTexture* pTexture = static_cast<GLESTexture*>(texture);
				glTex = pTexture->getTexture();
			}
			else if( texType == TT_3DIM )
			{
				GLESVolumeTexture* pTexture = static_cast<GLESVolumeTexture*>(texture);
				glTex = pTexture->getTexture();
			}
			else if( texType == TT_CUBE )
			{
				GLESCubeTexture* pTexture = static_cast<GLESCubeTexture*>(texture);
				glTex = pTexture->getTexture();
			}

			::glActiveTexture(GL_TEXTURE0 + samplerIndex);
			GLenum err = ::glGetError();
			assert(err == GL_NO_ERROR);
			result = (err == GL_NO_ERROR);
			::glBindTexture(glTarget, glTex);
			err = ::glGetError();
			assert(err == GL_NO_ERROR);
			result = result && (err == GL_NO_ERROR);
		}
		if( !result )
			BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to bind texture"));
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setShader(const HSHADER& program)
	{
		if (mLastProgram != program)
		{
			if (program != NULL && program->isReady())
			{
				GLESShader* glesProgram = static_cast<GLESShader*>(program);
				::glUseProgram(glesProgram->getProgram());
				assert(::glGetError() == GL_NO_ERROR);
				glesProgram->bindDecl(NULL);	//decl may changed if program changes!
				mLastProgram = program;
			}
			else
			{
				::glUseProgram(0);
				mLastProgram = HSHADER::EMPTY;
				assert(::glGetError() == GL_NO_ERROR);
			}
		}
		else
		{
#if BLADE_DEBUG
			GLuint glprogram = 0;
			::glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&glprogram);
			assert(glprogram == static_cast<GLESShader*>(program)->getProgram());
#endif
		}
	}

#define SHADER_GET_COUNT(_bytes, _elemSize) (_bytes/_elemSize)
	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const float* pData)
	{
		assert(key.mShaderType == SHADER_ANY);	//GLSL has one program with no shader
		/*
		https://www.khronos.org/opengles/sdk/docs/man3/html/glUniform.xhtml
		If transpose is GL_FALSE, each matrix is assumed to be supplied in column major order.
		If transpose is GL_TRUE, each matrix is assumed to be supplied in row major order.
		*/
		//Blade's matrix is ROW MAJOR
		GLboolean transpose = GL_TRUE;
		size_t validBytes = std::min<size_t>(bytes, key.mMaxBytes);	//GLES allows count/bytes less that declared size (partially update)

		switch( key.mType )
		{
		case SCT_FLOAT4:
			::glUniform4fv(key.mIndex, SHADER_GET_COUNT(validBytes, 16), pData);
			break;
		case SCT_MATRIX44:
			::glUniformMatrix4fv(key.mIndex, SHADER_GET_COUNT(validBytes, 64), transpose,  pData);
			break;
		case SCT_FLOAT3:
			::glUniform3fv(key.mIndex, SHADER_GET_COUNT(validBytes, 12), pData);
			break;
		case SCT_MATRIX33:
			::glUniformMatrix3fv(key.mIndex, SHADER_GET_COUNT(validBytes, 36), transpose, pData);
			break;
		case SCT_FLOAT2:
			::glUniform2fv(key.mIndex, SHADER_GET_COUNT(validBytes, 8), pData);
			break;
		case SCT_FLOAT1:
			::glUniform1fv(key.mIndex, SHADER_GET_COUNT(validBytes, 4), pData);
			break;

		case SCT_MATRIX22:
			::glUniformMatrix2fv(key.mIndex, SHADER_GET_COUNT(validBytes, 16), transpose, pData);
			break;
		case SCT_MATRIX23:
			::glUniformMatrix2x3fv(key.mIndex, SHADER_GET_COUNT(validBytes, 24), transpose, pData);
			break;
		case SCT_MATRIX24:
			::glUniformMatrix2x4fv(key.mIndex, SHADER_GET_COUNT(validBytes, 32), transpose, pData);
			break;
		case SCT_MATRIX32:
			::glUniformMatrix3x2fv(key.mIndex, SHADER_GET_COUNT(validBytes, 24), transpose, pData);
			break;
		case SCT_MATRIX34:
			::glUniformMatrix3x4fv(key.mIndex, SHADER_GET_COUNT(validBytes, 48), transpose, pData);
			break;
		case SCT_MATRIX42:
			::glUniformMatrix4x2fv(key.mIndex, SHADER_GET_COUNT(validBytes, 32), transpose, pData);
			break;
		case SCT_MATRIX43:
			::glUniformMatrix4x3fv(key.mIndex, SHADER_GET_COUNT(validBytes, 48), transpose, pData);
			break;
		default:
			assert(false);
			break;
		}
		GLenum err = ::glGetError();
		assert(err == GL_NO_ERROR); BLADE_UNREFERENCED(err);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const int* pData)
	{
		assert(key.mShaderType == SHADER_ANY);	//GLSL has one program with no shader
		size_t validBytes = std::min<size_t>(bytes, key.mMaxBytes);
		switch( key.mType )
		{
		case SCT_INT4:
			::glUniform4iv(key.mIndex, SHADER_GET_COUNT(validBytes, 16), pData);
			break;
		case SCT_INT3:
			::glUniform3iv(key.mIndex, SHADER_GET_COUNT(validBytes, 12), pData);
			break;
		case SCT_INT2:
			::glUniform2iv(key.mIndex, SHADER_GET_COUNT(validBytes, 8), pData);
			break;
		case SCT_INT1:
			::glUniform1iv(key.mIndex, SHADER_GET_COUNT(validBytes, 4), pData);
			break;
		case SCT_UINT4:
			::glUniform4uiv(key.mIndex, SHADER_GET_COUNT(validBytes, 16), (const GLuint*)pData);
			break;
		case SCT_UINT3:
			::glUniform3uiv(key.mIndex, SHADER_GET_COUNT(validBytes, 12), (const GLuint*)pData);
			break;
		case SCT_UINT2:
			::glUniform2uiv(key.mIndex, SHADER_GET_COUNT(validBytes, 8), (const GLuint*)pData);
			break;
		case SCT_UINT1:
			::glUniform1uiv(key.mIndex, SHADER_GET_COUNT(validBytes, 4), (const GLuint*)pData);
			break;
		default:
			assert(false);
			break;
		}
		assert(::glGetError() == GL_NO_ERROR);
	}

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::setShaderConstant(const IShader::CONSTANT_KEY& key, size_t bytes, const SC_BOOL* pData)
	{
		assert(key.mShaderType == SHADER_ANY);	//GLSL has one program with no shader
		size_t validBytes = std::min<size_t>(bytes, key.mMaxBytes);
		switch( key.mType )
		{
		case SCT_BOOL4:
			::glUniform4iv(key.mIndex, SHADER_GET_COUNT(validBytes, 16), (const GLint*)pData);
			break;
		case SCT_BOOL3:
			::glUniform3iv(key.mIndex, SHADER_GET_COUNT(validBytes, 12), (const GLint*)pData);
			break;
		case SCT_BOOL2:
			::glUniform2iv(key.mIndex, SHADER_GET_COUNT(validBytes, 8), (const GLint*)pData);
			break;
		case SCT_BOOL1:
			::glUniform1iv(key.mIndex, SHADER_GET_COUNT(validBytes, 4), (const GLint*)pData);
			break;
		default:
			assert(false);
			break;
		}
		assert(::glGetError() == GL_NO_ERROR);
	}
#undef SHADER_GET_COUNT

	//////////////////////////////////////////////////////////////////////////
	void			GLESRenderDevice::renderGeometry(const GraphicsGeometry& geom)
	{
		if( geom.mVertexCount == 0 || mLastProgram == NULL)
			return;

		//set indexes & draw primitives
		GLsizei pointCount = GLsizei( geom.hasIndexBuffer() ? geom.mIndexCount : geom.mVertexCount);
		GLenum primitiveType;
		GLsizei primitiveCount;
		switch( geom.mPrimitiveType )
		{
		case GraphicsGeometry::GPT_POINT_LIST:
			primitiveType = GL_POINTS;
			primitiveCount = pointCount;
			break;
		case GraphicsGeometry::GPT_LINE_LIST:
			primitiveType = GL_LINES;
			primitiveCount = pointCount/2;
			break;
		case GraphicsGeometry::GPT_LINE_STRIP:
			primitiveType = GL_LINE_STRIP;
			primitiveCount = pointCount - 1;
			break;
		case GraphicsGeometry::GPT_TRIANGLE_LIST:
			primitiveType = GL_TRIANGLES;
			assert( pointCount%3 == 0 );
			primitiveCount = pointCount/3;
			break;
		case GraphicsGeometry::GPT_TRIANGLE_STRIP:
			primitiveType = GL_TRIANGLE_STRIP;
			primitiveCount = pointCount - 2;
			break;
		case GraphicsGeometry::GPT_TRIANGLE_FAN:
			primitiveType = GL_TRIANGLE_FAN;
			primitiveCount = pointCount - 2;
			break;
		default:
			primitiveType = GL_POINTS;
			primitiveCount = 0;
			assert(false);
		}//switch

		if( primitiveCount == 0 )
			return;

		//set vertex declaration
		const GLESVertexDeclaration* decl = static_cast<const GLESVertexDeclaration*>(geom.mVertexDecl);	
		GLESShader* program = static_cast<GLESShader*>(mLastProgram);

		//apply vertex declaration & set stream source
		decl->apply(program, geom.mVertexSource, geom.mVertexStart);

		//draw
		if(geom.hasIndexBuffer())
		{
			const GLESIndexBuffer* ib = static_cast<const GLESIndexBuffer*>(geom.mIndexBuffer);
			size_t offset = geom.mIndexStart*ib->GLESIndexBuffer::getIndexSize();	//byte offset
			//bind IBO
			if (mLastIndex != ib)
			{
				ib->bindBuffer();
				mLastIndex = ib;
			}
			else
			{
#if BLADE_DEBUG
				GLint ibn = 0;
				::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ibn);
				assert(ibn == ib->getBufferName());
#endif
			}
			if( geom.isInstancing() )
				::glDrawElementsInstanced(primitiveType, geom.mIndexCount, ib->getGLIndexType(), (const void*)offset, geom.mInstanceCount);
			else
				::glDrawElements(primitiveType, geom.mIndexCount, ib->getGLIndexType(), (const void*)offset);
		}
		else
			::glDrawArrays(primitiveType, (GLint)0, (GLsizei )geom.mVertexCount);

		GLenum err = ::glGetError();
		if( err != GL_NO_ERROR )
			BLADE_EXCEPT(EXC_API_ERROR, BTString("failed to draw primitives.") );
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES