/********************************************************************
	created:	2014/12/20
	filename: 	AndroidGLESRenderWindow.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include "GLESContextEGL.h"
#include "AndroidGLESRenderWindow.h"
#include <graphics/GLES/GLESTexture.h>

#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <graphics/GLES/GLESGraphicsResourceManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GLESRenderWindow*	GLESRenderWindow::createGLESRenderWindow(IWindow* window, IRenderDevice* device, GLESContext* context)
	{
		return BLADE_NEW AndroidGLESRenderWindow(window, device, context);
	}

	//////////////////////////////////////////////////////////////////////////
	AndroidGLESRenderWindow::AndroidGLESRenderWindow(IWindow* window, IRenderDevice* device, GLESContext* context)
		:GLESRenderWindow(window, device, context)
	{
		assert(mContext != NULL);
		assert(mContext->isInitialized());

		//create back buffer ( render texture )
		GLESContextEGL* eglContext = static_cast<GLESContextEGL*>(mContext);

		EGLint width, height;
		EGLBoolean ret = ::eglQuerySurface(eglContext->getDisplay(), eglContext->getSurface(), EGL_WIDTH, &width);
		assert(ret);
		ret = ::eglQuerySurface(eglContext->getDisplay(), eglContext->getSurface(), EGL_HEIGHT, &height);
		assert(ret);

		PixelFormat format = window->getColorDepth() == 16 ? PF_R5G6B5 : PF_R8G8B8A8;
		format.setLinearColor(false);
		mBackBuffer = IGraphicsResourceManager::getSingleton().createTexture(width, height, format, 1, IGraphicsBuffer::GBUF_RENDERTARGET);
		assert( mBackBuffer != NULL );

		//init back buffer format
		GLESPixelFormatConverter::setBackBufferFormat( GLESPixelFormatConverter(format) );

		//reset texture-parameter
		//GLESTexture* texture = static_cast<GLESTexture*>(mBackBuffer);
		//::glBindTexture(GL_TEXTURE_2D, texture->getTexture());
		//assert( ::glGetError() == GL_NO_ERROR );
		//::glTexParameteri()

		//back buffer attachment: color0
		mFrameBuffer.attachColor(0, mBackBuffer);
	}

	//////////////////////////////////////////////////////////////////////////
	AndroidGLESRenderWindow::~AndroidGLESRenderWindow()
	{
	}

	/************************************************************************/
	/* IRenderTarget interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			AndroidGLESRenderWindow::setDepthBuffer(const HTEXTURE& hDethBuffer)
	{
		bool ret = RenderWindow::setDepthBuffer(hDethBuffer);
		if( ret )
		{
			mFrameBuffer.attachDepth(hDethBuffer);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AndroidGLESRenderWindow::setColorBuffer(index_t index, const HTEXTURE& buffer)
	{
		bool ret = RenderWindow::setColorBuffer(index, buffer);
		if( ret )
		{
			mFrameBuffer.attachColor(index, buffer);
		}
		return ret;
	}

	/************************************************************************/
	/* RenderWindow interface                                                          */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			AndroidGLESRenderWindow::isReady() const
	{
		assert( mContext != NULL);
		if (RenderWindow::isReady() && mContext->isInitialized()
			&& static_cast<GLESContextEGL*>(mContext)->getSurface() != EGL_NO_SURFACE)
		{
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AndroidGLESRenderWindow::swapBuffers()
	{
		assert( mContext != NULL);
		GLESContextEGL* eglContext = static_cast<GLESContextEGL*>(mContext);
		GLuint fbo = mFrameBuffer.activate();
		assert(fbo != 0);
		return eglContext->swapBuffers(fbo, mBackBuffer->getWidth(), mBackBuffer->getHeight());
	}

	//////////////////////////////////////////////////////////////////////////
	bool			AndroidGLESRenderWindow::swapBuffers(IWindow* /*destWindow*/, IRenderView* /*sourceView = NULL*/)
	{
		assert(false);
		return false;
	}

	/************************************************************************/
	/* IWindowEventHandler interface                                                           */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			AndroidGLESRenderWindow::onWindowCreated(IWindow* pwin)
	{
		//this function is not called

		//assert( pwin != NULL && pwin == this->getWindow() );
		//assert( mContext != NULL);
		//GLESContextEGL* eglContext = static_cast<GLESContextEGL*>(mContext);
		//eglContext->createSurface(pwin);
		//eglContext->makeCurrent();
	}

	//////////////////////////////////////////////////////////////////////////
	void			AndroidGLESRenderWindow::onWindowDestroy(IWindow* pwin)
	{
		assert( pwin != NULL && pwin == this->getWindow() );
		assert( mContext != NULL);
		GLESContextEGL* eglContext = static_cast<GLESContextEGL*>(mContext);
		eglContext->clearCurrent();
		eglContext->destroySurface();
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
}//namespace Blade

#endif//BLADE_PLATFORM