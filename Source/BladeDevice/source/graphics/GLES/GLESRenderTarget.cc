/********************************************************************
	created:	2018/11/17
	filename: 	GLESRenderTarget.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESRenderTarget.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESGraphicsResourceManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GLESRenderTarget::GLESRenderTarget(const TString& name, IRenderDevice* device, size_t viewWidth, size_t viewHeight)
		:RenderTarget(name, device, viewWidth, viewHeight)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	GLESRenderTarget::~GLESRenderTarget()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESRenderTarget::setDepthBuffer(const HTEXTURE& hDethBuffer)
	{
		bool ret = RenderTarget::setDepthBuffer(hDethBuffer);
		if (ret)
		{
			if(hDethBuffer != NULL)
				mFBO.attachDepth(hDethBuffer);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESRenderTarget::setColorBuffer(index_t index, const HTEXTURE& buffer)
	{
		bool ret = RenderTarget::setColorBuffer(index, buffer);
		if (ret)
		{
			if(buffer != NULL)
				mFBO.attachColor(index, buffer);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		GLESRenderTarget::activate()
	{
		return mFBO.activate() != 0;
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES