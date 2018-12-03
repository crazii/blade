/********************************************************************
	created:	2012/03/25
	filename: 	D3D9RenderSurface.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows//D3D9/D3D9RenderSurface.h>
#include <graphics/windows/D3D9/D3D9PixelBuffer.h>


namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	D3D9RenderSurface::D3D9RenderSurface(D3D9PixelBuffer* backbufferSurface)
		:Texture(TT_2DIM,IMGO_TOP_DOWN,
		backbufferSurface->getWidth(),backbufferSurface->getHeight(),backbufferSurface->getPixelFormat(),
		backbufferSurface->getUsage(),IGraphicsBuffer::GBAF_WRITE)
	{
		mBackBuffer.bind( backbufferSurface );
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9RenderSurface::~D3D9RenderSurface()
	{

	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		D3D9RenderSurface::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
	{
		BLADE_UNREFERENCED(newWidth);
		BLADE_UNREFERENCED(newHeight);
		BLADE_UNREFERENCED(newFormat);
		BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to clone from d3d9 render target surface") );
	}

	//////////////////////////////////////////////////////////////////////////
	void*			D3D9RenderSurface::lockImpl(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(region);
		BLADE_UNREFERENCED(lockflags);
		BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to lock a d3d9 render target surface") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderSurface::unlockImpl()
	{
		BLADE_EXCEPT(EXC_API_ERROR,BTString("unable to lock a d3d9 render target surface") );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderSurface::updateMipMap(const TEXREGION& region,index_t mipLevel/* = 0*/)
	{
		BLADE_UNREFERENCED(region);
		BLADE_UNREFERENCED(mipLevel);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			D3D9RenderSurface::addDirtyRegion(const TEXREGION& region)
	{
		BLADE_UNREFERENCED(region);
		return false;
	}

}//namespace Blade

#endif