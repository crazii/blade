/********************************************************************
	created:	2018/11/17
	filename: 	GLESRenderTarget.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESRenderTarget_h__
#define __Blade_GLESRenderTarget_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/RenderTarget.h>
#include <graphics/GLES/GLESFrameBuffer.h>

namespace Blade
{

	class GLESRenderTarget : public RenderTarget
	{
	public:
		GLESRenderTarget(const TString& name, IRenderDevice* device, size_t viewWidth, size_t viewHeight);
		~GLESRenderTarget();

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setDepthBuffer(const HTEXTURE& hDethBuffer);

		/*
		@describe
		@param
		@return
		*/
		virtual bool			setColorBuffer(index_t index, const HTEXTURE& buffer);

		/**
		@describe used internally
		@param
		@return
		*/
		virtual bool		activate();

	protected:
		GLESFrameBuffer mFBO;
	};

}//namespace Blade
#endif//BLADE_USE_GLES

#endif // __Blade_GLESRenderTarget_h__