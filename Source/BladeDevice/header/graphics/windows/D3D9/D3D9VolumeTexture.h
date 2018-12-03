/********************************************************************
	created:	2010/04/15
	filename: 	D3D9VolumeTexture.h
	author:		Crazii
	
	purpose:	3D (Volume) texture implementation
*********************************************************************/
#ifndef __Blade_D3D9VolumeTexture_h__
#define __Blade_D3D9VolumeTexture_h__

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Resource.h"
#include <graphics/Texture.h>
#include "D3D9Util.h"

namespace Blade
{
	class D3D9VolumeTexture : public Texture,public D3D9Resource<IDirect3DVolumeTexture9>,public Allocatable
	{
	public:
		D3D9VolumeTexture(IDirect3DVolumeTexture9* texture,IGraphicsBuffer::ACCESS CpuAccess, IMG_ORIENT layout);
		~D3D9VolumeTexture();

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
		/* D3D9Resource imp                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		rebuildImpl(IDirect3DDevice9* device);

		/** @brief  */
		virtual	void		onSet(IDirect3DVolumeTexture9* source);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		release();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		inline bool			isCacheNeeded() const
		{
			bool ret = mDesc.mUsage.isDynamic() && mDesc.mUsage.isReadable() && !mDesc.mPixelFormat.isCompressed();
			assert( !ret || this->getD3D9Desc().Pool == D3DPOOL_DEFAULT );
			return ret;
		}

	protected:

		HSFOTVOLUME		mMemoryVolume;
	};//class D3D9VolumeTexture
	
}//namespace Blade


#endif //BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS


#endif //__Blade_D3D9VolumeTexture_h__