/********************************************************************
	created:	2014/12/19
	filename: 	GLESContextEGL.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESContextEGL_h__
#define __Blade_GLESContextEGL_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <graphics/GLES/GLESContext.h>

namespace Blade
{
	class GLESContextEGL : public GLESContext, public Allocatable
	{
	public:
		GLESContextEGL();
		~GLESContextEGL();

		/************************************************************************/
		/* GLESContext interface                                                                     */
		/************************************************************************/

		/** @brief  */
		virtual bool	initializeImpl(IWindow* window, uint version);

		/** @brief  */
		virtual bool	initializeImpl(GLESContext* sharedContext, uint version);

		/** @brief  */
		virtual bool	deinitializeImpl();

		/** @brief  */
		virtual bool	makeCurrentImpl(bool rendering);

		/** @brief  */
		virtual bool	checkCurrentImpl() const;

		/** @brief  */
		virtual bool	clearCurrentImpl();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline EGLDisplay	getDisplay() const	{return mDisplay;}
		/** @brief  */
		inline EGLContext	getContext() const	{return mContext;}
		/** @brief  */
		inline EGLConfig	getConfig() const	{return mConfig;}
		/** @brief  */
		inline EGLSurface	getSurface() const	{return mSurface;}
		/** @brief  */
		inline bool			isSharedContext() const { return mSharedContext != EGL_NO_CONTEXT; }

		/** @brief  */
		bool	destroySurface();

		/** @brief  */
		bool	createSurface(IWindow* window);

		/** @brief  */
		bool	swapBuffers(GLuint drawFBO, size_t width, size_t height);

	protected:
		void	clearFBO();

		EGLDisplay	mDisplay;
		EGLContext	mContext;
		EGLConfig	mConfig;
		EGLSurface	mSurface;
		EGLContext	mSharedContext;	//shared context holding shared resources
		GLuint		mFBORead;
	};
	
}//namespace Blade

#endif//BLADE_PLATFORM


#endif // __Blade_GLESContextEGL_h__