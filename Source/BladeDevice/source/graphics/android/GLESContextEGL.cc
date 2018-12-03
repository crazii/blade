/********************************************************************
	created:	2014/12/19
	filename: 	GLESContextEGL.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include "GLESContextEGL.h"
#include "AndroidGLESRenderWindow.h"
#include <interface/public/window/IWindow.h>
#include <utility/Profiling.h>
//max version: GLES 3.0
#define BLADE_GLES_MAX_VERSION 3

#define EGL_OPENGL_ES3_BIT_KHR 0x00000040

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GLESContext*	GLESContext::createContext()
	{
		return BLADE_NEW GLESContextEGL();
	}

	//////////////////////////////////////////////////////////////////////////
	GLESContextEGL::GLESContextEGL()
		:mDisplay(EGL_NO_DISPLAY)
		,mContext(EGL_NO_CONTEXT)
		,mConfig(NULL)
		,mSurface(EGL_NO_SURFACE)
		,mSharedContext(EGL_NO_CONTEXT)
		,mFBORead(0)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GLESContextEGL::~GLESContextEGL()
	{

	}

	/************************************************************************/
	/* GLESContext interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::initializeImpl(IWindow* window, uint version)
	{
		int RBits, GBits, BBits, ABits, ZBits;

		//note: Blade's render device don't create depth buffer by default
		ZBits = 0;

		int depth = window->getColorDepth();
		if( depth == 16 )
		{
			RBits = 5;
			GBits = 6;
			BBits = 5;
			ABits = 0;
		}
		else if( depth == 32 )
			RBits = GBits = BBits = ABits = 8;
		else
		{
			assert(false);
			return false;
		}
		
		mDisplay = ::eglGetDisplay(EGL_DEFAULT_DISPLAY);
		::eglInitialize(mDisplay, NULL, NULL);
		EGLint count = 0;
		GLint renderType = EGL_OPENGL_ES2_BIT;

		const char *extensions = eglQueryString(mDisplay, EGL_EXTENSIONS);
		// check whether EGL_KHR_create_context is in the extension string
		if (extensions != NULL && ::strstr(extensions, "EGL_KHR_create_context"))
		{
			// extension is supported
			renderType = EGL_OPENGL_ES3_BIT_KHR;
			BLADE_LOG(Information, BTString("GLES 3.0"));
		}


		{
			EGLint attributes[] = 
			{
				EGL_RENDERABLE_TYPE, renderType,
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				//EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
				EGL_RED_SIZE, RBits,
				EGL_GREEN_SIZE, GBits,
				EGL_BLUE_SIZE, BBits,
				EGL_ALPHA_SIZE, ABits,
				EGL_DEPTH_SIZE, ZBits,
				EGL_STENCIL_SIZE, 0,
				EGL_NONE
			};	
			::eglChooseConfig(mDisplay, attributes, &mConfig, 1, &count);
		}

		if( count == 1 )
		{
			bool ret = false;
			{
				assert(mSurface == EGL_NO_SURFACE);
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
				 /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
				* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
				* As soon as we picked a EGLConfig, we can safely reconfigure the
				* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
				EGLint format;
				::eglGetConfigAttrib(mDisplay, mConfig, EGL_NATIVE_VISUAL_ID, &format);
				EGLNativeWindowType handle = (EGLNativeWindowType)(void*)window->getNativeHandle();
				::ANativeWindow_setBuffersGeometry(handle, 0, 0, format);
#else
#error check if miss something.
#endif
				

				this->createSurface( window );
				ret = mSurface != EGL_NO_SURFACE;
				assert(ret);
			}

			if (ret)
			{
				assert(version >= 2 && version <= BLADE_GLES_MAX_VERSION);
				EGLint attributes[] =
				{
					EGL_CONTEXT_CLIENT_VERSION, (EGLint)version,
					EGL_NONE
				};

				mContext = ::eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT, attributes);
				ret = mContext != EGL_NO_CONTEXT;
			}
			return ret;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::initializeImpl(GLESContext* sharedContext, uint version)
	{
		GLESContextEGL* sharedEGLContext = static_cast<GLESContextEGL*>(sharedContext);
		assert(sharedContext != NULL && sharedContext->isInitialized());
		EGLContext context = sharedEGLContext->mContext;

		assert(version >= 2 && version <= BLADE_GLES_MAX_VERSION);
		EGLint attributes[] =
		{
			EGL_CONTEXT_CLIENT_VERSION, (EGLint)version,
			EGL_NONE,
		};

		mContext = ::eglCreateContext(sharedEGLContext->mDisplay, sharedEGLContext->mConfig, context, attributes);
		
		bool ret = mContext != EGL_NO_CONTEXT;
		assert(ret);
		if (ret)
		{
			mSharedContext = context;
			mDisplay = sharedEGLContext->mDisplay;
			mConfig = sharedEGLContext->mConfig;
			mSurface = sharedEGLContext->mSurface;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::deinitializeImpl()
	{
		assert( mDisplay != EGL_NO_DISPLAY );
		assert( mContext != EGL_NO_CONTEXT );
		bool ret = ::eglDestroyContext(mDisplay, mContext) == EGL_TRUE;
		if( ret )
		{
			if(!this->isSharedContext())
				::eglTerminate(mDisplay);

			mDisplay = EGL_NO_DISPLAY;
			mContext = EGL_NO_CONTEXT;
			mSharedContext = NULL;
			mConfig = 0;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::makeCurrentImpl(bool rendering)
	{
		assert( mDisplay != EGL_NO_DISPLAY );
		assert( mContext != EGL_NO_CONTEXT );

		if (rendering && this->checkCurrentImpl())
			this->clearCurrentImpl();

		EGLSurface surface = rendering ? mSurface : EGL_NO_SURFACE;
		
		//note: from EGL spec:
		// one window can have only one surface
		// surface cannot be shared among multiple "current" context: if one context is current and bound to a surface, other context must set surface to EGL_NO_SURFACE
		// contexts made current with EGL_NO_SURFACE don't have default frame buffer (FBO 0), they can perform off-screen rendering or resource operation.
		// frequent context switch ("make current" for one thread) : performance is bad
		// frame buffer objects are not shared among shared contexts.
		//
		//conclusion:
		// for multi threading, there should be only one rendering thread with context that bound to a surface,
		// other threads should make current context with empty surface, to perform resource operation (or off-screen rendering) only.
		// if multiple threads are rendering, whether they have different window(surface) bound, or they need unbound previous surface - this causes frequent context switch.
		// for Blade's task model: rendering tasks are by default executed on random threads, so it's hard to avoid frequent context switch
		// 1, set rendering task's affinity to TA_MAIN, then other operation is needed, shared context is not needed.
		// 2, set rendering task's affinity to TA_ANY, then: use shared context if no-surface context created in main sync mode (no context switch), or create unique context directly in rendering thread.
		// 3, rendering task render to an off-screen FBO then perform swap-buffer in main thread (sync state).
		// the performance of the aboves are basically equal, so the most simplified ITask::TA_MAIN is used. (this is the most "by chance" situation when TBB is used).
		EGLBoolean ret = ::eglMakeCurrent(mDisplay, surface, surface, mContext);
		if (!ret)
		{
			EGLint err = ::eglGetError();
			BLADE_LOG(Error, BTString("eglMakeCurrent error: ") << err);
			BLADE_UNREFERENCED(err);
		}
		else
		{
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			this->clearFBO();
			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));	//fails if current context has no surface

			if (mFBORead == 0)
			{
				::glGenFramebuffers(1, &mFBORead);
				assert(::glGetError() == GL_NO_ERROR);
				::glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBORead);
				assert(::glGetError() == GL_NO_ERROR);
			}
		}
		assert(ret);
		return ret == EGL_TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::checkCurrentImpl() const
	{
		return ::eglGetCurrentContext() == mContext;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::clearCurrentImpl()
	{
		assert(this->checkCurrentImpl());
		if (mFBORead != 0)
		{
			::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			assert(::glGetError() == GL_NO_ERROR);
			::glDeleteFramebuffers(1, &mFBORead);
			assert(::glGetError() == GL_NO_ERROR);
			mFBORead = 0;
		}

		assert(mContext != EGL_NO_CONTEXT);
		assert(mDisplay != EGL_NO_DISPLAY);
		EGLBoolean ret = ::eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		return ret == EGL_TRUE;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::destroySurface()
	{
		if( mSurface != EGL_NO_SURFACE )
		{
			if(!this->isSharedContext())
				::eglDestroySurface(mDisplay, mSurface);
			mSurface = EGL_NO_SURFACE;
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::createSurface(IWindow* window)
	{
		if( mSurface == EGL_NO_SURFACE )
		{
			EGLint attributes[] = 
			{
				//note: we use frame buffer object(RTT) and copy the render texture to default FBO(FBO 0)
				//it acts like a back buffer and we don't need extra double buffering
				//this may save a bit of memory
				EGL_RENDER_BUFFER, EGL_BACK_BUFFER,	//note: using back buffer fixes Adreno: render object's outline white glitches
				EGL_NONE,
			};
			EGLNativeWindowType handle = (EGLNativeWindowType)(void*)window->getNativeHandle();
			mSurface = ::eglCreateWindowSurface(mDisplay, mConfig, handle, attributes);
			assert( mSurface != EGL_NO_SURFACE );
			return mSurface != EGL_NO_SURFACE;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESContextEGL::swapBuffers(GLuint drawFBO, size_t width, size_t height)
	{
		if( mSurface == EGL_NO_SURFACE )
			return false;

#if 0
#else
		{
			//BLADE_LW_PROFILING(BIND_READ);
			::glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFBO);
			assert( ::glGetError() == GL_NO_ERROR );
			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));
		}

		{
			//BLADE_LW_PROFILING(BIND_DRAW);
			::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			GLenum err = ::glGetError();
			assert(err == GL_NO_ERROR); BLADE_UNREFERENCED(err);
			assert(GL_FRAMEBUFFER_COMPLETE == ::glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));	//fails if current context has no surface
		}

		GLint w = (GLint)width;
		GLint h = (GLint)height;
		{
			//BLADE_LW_PROFILING(BIND_BLIT);
			::glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST );
			GLenum err = ::glGetError();
			assert( err == GL_NO_ERROR ); BLADE_UNREFERENCED(err);
		}
#endif

		EGLBoolean ret;
		{
			//BLADE_LW_PROFILING(SWAP);
			ret = ::eglSwapBuffers(mDisplay, mSurface);
			assert( ::glGetError() == GL_NO_ERROR );
		}

		//this->clearFBO();

		//bind read fbo (empty) (image will be attached on texture read back, @see GLESTexture::lockImpl)
		::glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBORead);
		assert(::glGetError() == GL_NO_ERROR);

		return (ret == EGL_TRUE);
	}

	//////////////////////////////////////////////////////////////////////////
	void	GLESContextEGL::clearFBO()
	{
		GLboolean colormask[4];
		::glGetBooleanv(GL_COLOR_WRITEMASK, colormask);
		assert(::glGetError() == GL_NO_ERROR);
		bool colorMasked = (!colormask[0] || !colormask[1] || !colormask[2] || !colormask[3]);

		//note: the code is used to clear FBO 0, and Blade doesn't create depth for it.

		GLboolean depthMask;
		GLuint stencilMask;
		::glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
		assert(::glGetError() == GL_NO_ERROR);
		::glGetIntegerv(GL_STENCIL_WRITEMASK, (GLint*)&stencilMask);
		assert(::glGetError() == GL_NO_ERROR);

		if(colorMasked)
		{
			::glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			assert(::glGetError() == GL_NO_ERROR);
		}

		if (!depthMask)
		{
			::glDepthMask(GL_TRUE);
			assert(::glGetError() == GL_NO_ERROR);
		}
		if (stencilMask != 0xFF)
		{
			::glStencilMask(0xFF);
			assert(::glGetError() == GL_NO_ERROR);
		}

		::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		assert(::glGetError() == GL_NO_ERROR);

		if (colorMasked)
		{
			::glColorMask(colormask[0], colormask[1], colormask[2], colormask[3]);
			assert(::glGetError() == GL_NO_ERROR);
		}
		if (!depthMask)
		{
			::glDepthMask(GL_FALSE);
			assert(::glGetError() == GL_NO_ERROR);
		}

		if (stencilMask != 0xFF)
		{
			::glStencilMask(stencilMask);
			assert(::glGetError() == GL_NO_ERROR);
		}
	}
	
}//namespace Blade


#endif//BLADE_PLATFORM