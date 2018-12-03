/********************************************************************
	created:	2014/12/23
	filename: 	GLESRenderWindow.h
	author:		Crazii
	
	purpose:	top level wrapper for GLES render window
*********************************************************************/
#ifndef __Blade_GLESRenderWindow_h__
#define __Blade_GLESRenderWindow_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/GLES/GLESFrameBuffer.h>
#include <graphics/RenderWindow.h>

namespace Blade
{
	class GLESRenderWindow : public RenderWindow
	{
	public:
		GLESRenderWindow(IWindow* window, IRenderDevice* device, GLESContext* context)
			:RenderWindow(window, device)
			,mContext(context)
		{

		}
		virtual ~GLESRenderWindow()	{}

		/************************************************************************/
		/* RenderTarget interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			activate()			{ return mFrameBuffer.activate() != 0;}

		/** @brief  */
		inline GLESFrameBuffer&	getFrameBuffer()	{return mFrameBuffer;}

		/** @brief  */
		inline GLESContext*		getContext() const	{return mContext;}


		/** @brief  */
		static GLESRenderWindow*	createGLESRenderWindow(IWindow* window, IRenderDevice* device, GLESContext* context);

	protected:
		GLESContext*			mContext;
		mutable GLESFrameBuffer	mFrameBuffer;
	};//GLESRenderWindow

}//namespace Blade

#endif//BLADE_USE_GLES
#endif//__Blade_GLESRenderWindow_h__