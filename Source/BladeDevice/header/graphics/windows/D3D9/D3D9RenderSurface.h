/********************************************************************
	created:	2012/03/25
	filename: 	D3D9RenderSurface.h
	author:		Crazii
	purpose:	fake texture interface for implementation of d3d9 back buffer render target surface (not readable)
*********************************************************************/
#ifndef __Blade_D3D9RenderSurface_h__
#define __Blade_D3D9RenderSurface_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <graphics/windows/D3D9/D3D9Header.h>
#include <graphics/Texture.h>


namespace Blade
{
	class D3D9PixelBuffer;

	class D3D9RenderSurface : public Texture, public Allocatable
	{
	public:
		D3D9RenderSurface(D3D9PixelBuffer* backbufferSurface);
		~D3D9RenderSurface();
		/************************************************************************/
		/* ITexture Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual HTEXTURE		cloneTexture(size_t newWidth = 0,size_t newHeight = 0,PixelFormat newFormat = PF_UNDEFINED);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*			lockImpl(const TEXREGION& region,IGraphicsBuffer::LOCKFLAGS lockflags);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			unlockImpl();

		/*
		@describe
		@param box the region to update. EMPTY means the entire texture, or use the sub rectangle of the texture to generate mipmap.this is used for texture atlas.
		@param mipLevel the target mip level to update, the miplevel 0 must be filled already, and other level will update from it.\n
		set the param to 0 to update all miplevels
		@return
		*/
		virtual bool			updateMipMap(const TEXREGION& region,index_t mipLevel = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addDirtyRegion(const TEXREGION& region);

		/************************************************************************/
		/* Texture overrides                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		inline const HPXBUFFER&		getPixelBuffer(index_t mipMap = 0) const
		{
			if( mipMap == 0)
				return mBackBuffer;
			else
				return HPXBUFFER::EMPTY;
		}
		
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief set by RenderWindow on window size changed, use the window size */
		inline void		setSize(size_t width, size_t height)
		{
			mDesc.mWidth = width;
			mDesc.mHeight = height;
		}

	protected:
		HPXBUFFER	mBackBuffer;
	};
	

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#endif //__Blade_D3D9RenderSurface_h__
