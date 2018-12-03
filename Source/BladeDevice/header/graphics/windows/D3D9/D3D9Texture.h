/********************************************************************
	created:	2010/04/15
	filename: 	D3D9Texture.h
	author:		Crazii
	
	purpose:	1D / 2D texture implementation
*********************************************************************/
#ifndef __Blade_D3D9Texture_h__
#define __Blade_D3D9Texture_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <utility/BladeContainer.h>
#include "D3D9Resource.h"
#include <graphics/Texture.h>
#include "D3D9Util.h"

namespace Blade
{
	class D3D9Texture : public Texture, public D3D9Resource<IDirect3DTexture9>,public Allocatable
	{
	public:
		D3D9Texture(IDirect3DTexture9* texture,IGraphicsBuffer::ACCESS CpuAccess, IMG_ORIENT layout);
		~D3D9Texture();

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
		virtual bool			updateMipMap(const TEXREGION& region, index_t mipLevel = 0);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			addDirtyRegion(const TEXREGION& region);


		/************************************************************************/
		/* D3D9Resource imp                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		rebuildImpl(IDirect3DDevice9* device);

		/** @brief  */
		virtual	void		onSet(IDirect3DTexture9* source);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		release();

		/************************************************************************/
		/* Texture overrides                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		const HPXBUFFER&	getPixelBuffer(index_t mipmap = 0) const;

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline bool			isCacheNeeded() const
		{
			return mDesc.mUsage.isDynamic() && mDesc.mUsage.isReadable()/* && !mDesc.mPixelFormat.isCompressed()*/;
		}

	protected:
		typedef Vector<HPXBUFFER>	PixelBufferList;
		mutable PixelBufferList		mPxBufferList;

		//memory data only available on dynamic-readable texture
		HSOFTSURFACE	mMemorySurface;
	};//class D3D9Texture
	
}//namespace Blade


#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#endif //__Blade_D3D9Texture_h__