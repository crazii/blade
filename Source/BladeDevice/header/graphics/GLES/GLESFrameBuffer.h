/********************************************************************
	created:	2014/12/21
	filename: 	GLESFrameBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESFrameBuffer_h__
#define __Blade_GLESFrameBuffer_h__
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/RenderTarget.h>//MAX_MRT_COUNT
#include "GLESTexture.h"

#if BLADE_GLES_SHARED_CONTEXT
#include <threading/Thread.h>
#endif

namespace Blade
{
	//note: by GL spec, whether FBO is shared across shared contexts is implementation behavior,
	//so we better keep a copy of FBO objects per context.

	class GLESFrameBuffer
	{
	protected:
#if BLADE_GLES_SHARED_CONTEXT
		struct ContextFBO
		{
			GLuint		mFBO;
			GLuint		mColorAttachment[RenderTarget::MAX_MRT_COUNT];
			GLuint		mDepthAttachment;
			GLuint		mStencilAttachment;
			ContextFBO()
			{
				mFBO = 0;
				mStencilAttachment = mDepthAttachment = 0;
				for (size_t i = 0; i < RenderTarget::MAX_MRT_COUNT; ++i)
					mColorAttachment[i] = 0;
			}
		};

		//note: frame buffers are not guaranteed to be shared across shared contexts
		typedef Map<Thread::ID, ContextFBO> ContextFBOMap;
		mutable ContextFBOMap	mFBOMap;

		GLuint		mColorAttachment[RenderTarget::MAX_MRT_COUNT];
		GLuint		mDepthAttachment;
		GLuint		mStencilAttachment;
#else
		GLuint		mFrameBuffer;

		GLuint		mColorAttachment[RenderTarget::MAX_MRT_COUNT];
		GLuint		mDepthAttachment;
		GLuint		mStencilAttachment;
#endif
	public:
		GLESFrameBuffer()
		{
#if BLADE_GLES_SHARED_CONTEXT
			mStencilAttachment = mDepthAttachment = 0;
			for (size_t i = 0; i < RenderTarget::MAX_MRT_COUNT; ++i)
				mColorAttachment[i] = 0;
#else
			mFrameBuffer = 0;
			::glGenFramebuffers(1, &mFrameBuffer);
			assert(mFrameBuffer != 0 && ::glGetError() == GL_NO_ERROR );
#endif
		}

		~GLESFrameBuffer()
		{
#if BLADE_GLES_SHARED_CONTEXT
			for (ContextFBOMap::iterator i = mFBOMap.begin(); i != mFBOMap.end(); ++i)
			{
				GLuint& fbo = i->second.mFBO;
				::glDeleteFramebuffers(1, &fbo);
				assert(::glGetError() == GL_NO_ERROR);
			}
			mFBOMap.clear();
#else

			this->bind();

			::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
			assert(::glGetError() == GL_NO_ERROR);
			::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
			assert(::glGetError() == GL_NO_ERROR);

			for (size_t i = 0; i < RenderTarget::MAX_MRT_COUNT; ++i)
			{
				::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
				assert(::glGetError() == GL_NO_ERROR);
			}

			this->unbind();

			::glDeleteFramebuffers(1, &mFrameBuffer);
			assert( ::glGetError() == GL_NO_ERROR );
			mFrameBuffer = 0;
#endif
		}

		/** @brief  */
		GLuint		getFBO()
		{
#if BLADE_GLES_SHARED_CONTEXT
			GLuint& fbo = mFBOMap[Thread::getCurrentID()].mFBO;
			return fbo;
#else
			assert(mFrameBuffer != 0);
			return mFrameBuffer;
#endif
		}

		/** @brief  */
		void		bind()
		{
#if BLADE_GLES_SHARED_CONTEXT
			ContextFBO& ctxFbo = mFBOMap[Thread::getCurrentID()];

			if (ctxFbo.mFBO == 0)
			{
				::glGenFramebuffers(1, &ctxFbo.mFBO);
				assert(::glGetError() == GL_NO_ERROR);
			}
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ctxFbo.mFBO);
			assert(::glGetError() == GL_NO_ERROR);
#else
			GLuint fbo = this->getFBO();
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			assert(::glGetError() == GL_NO_ERROR);
#endif
		}

		/** @brief  */
		GLuint		activate()
		{
#if BLADE_GLES_SHARED_CONTEXT
			ContextFBO& ctxFbo = mFBOMap[Thread::getCurrentID()];

			if (ctxFbo.mFBO == 0)
			{
				::glGenFramebuffers(1, &ctxFbo.mFBO);
				assert(::glGetError() == GL_NO_ERROR);
			}
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ctxFbo.mFBO);
			assert(::glGetError() == GL_NO_ERROR);

			for (size_t i = 0; i < RenderTarget::MAX_MRT_COUNT; ++i)
			{
				if (ctxFbo.mColorAttachment[i] != mColorAttachment[i])
				{
					::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mColorAttachment[i], 0);
					assert(::glGetError() == GL_NO_ERROR);
					ctxFbo.mColorAttachment[i] = mColorAttachment[i];
				}
			}

			if (ctxFbo.mDepthAttachment != mDepthAttachment)
			{
				::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthAttachment, 0);
				assert(::glGetError() == GL_NO_ERROR);
				ctxFbo.mDepthAttachment = mDepthAttachment;
			}
			if (ctxFbo.mStencilAttachment != mStencilAttachment)
			{
				::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mStencilAttachment, 0);
				assert(::glGetError() == GL_NO_ERROR);
				ctxFbo.mStencilAttachment = mStencilAttachment;
			}

			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

			return ctxFbo.mFBO;
#else
			this->bind();

			//MRT configuration
			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
			GLenum	drawBuffers[RenderTarget::MAX_MRT_COUNT];
			std::memset(drawBuffers, GL_NONE, sizeof(drawBuffers));

			int colorCount = 1;
			for (int i = 0; i < (int)RenderTarget::MAX_MRT_COUNT; ++i)
			{
				if (mColorAttachment[i] != 0)
				{
					colorCount = i+1;
					drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
				}
			}
			::glDrawBuffers(colorCount, drawBuffers);

			return mFrameBuffer;
#endif
		}

		/** @brief  */
		void		unbind()
		{
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			assert( ::glGetError() == GL_NO_ERROR );
		}

		/** @brief  */
		bool		attachColor(index_t index, const HTEXTURE& color)
		{
			GLuint texture = color != NULL ? static_cast<GLESTexture*>(color)->getTexture() : 0;

			assert(index < RenderTarget::MAX_MRT_COUNT);
			mColorAttachment[index] = texture;

#if !BLADE_GLES_SHARED_CONTEXT
			this->bind();
			::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture, 0);
			assert(::glGetError() == GL_NO_ERROR);
			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
#endif
			return true;
		}

		/** @brief  */
		bool		attachDepth(const HTEXTURE& depth)
		{
			GLuint texture = depth != NULL ? static_cast<GLESTexture*>(depth)->getTexture() : 0;
			mDepthAttachment = texture;

#if !BLADE_GLES_SHARED_CONTEXT
			this->bind();

			if(depth->getPixelFormat().isDepthStencil() )
				::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
			else
				::glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

			assert(::glGetError() == GL_NO_ERROR);
			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
#endif
			return true;
		}

	};
	
}//namespace Blade

#endif//BLADE_USE_GLES
#endif // __Blade_GLESFrameBuffer_h__