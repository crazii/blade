/********************************************************************
	created:	2014/12/20
	filename: 	GLESTexture.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESTexture_h__
#define __Blade_GLESTexture_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include "GLESUtil.h"
#include <graphics/Texture.h>

namespace Blade
{

	class GLESTexture : public Texture, public Allocatable
	{
	public:
		GLESTexture(GLuint name, IMG_ORIENT layout, size_t width,size_t height,PixelFormat format,
			IGraphicsBuffer::USAGE usage,IGraphicsBuffer::ACCESS GpuAccess, size_t mipmap = 1);
		~GLESTexture();

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
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		GLuint			getTexture() const	{return mTextureName;}

	protected:
		GLuint			mTextureName;
		GLESPixelFormat	mGLFormat;
		GLESImageCache	mLockedData;
	};//GLESTexture
	
}//namespace Blade

#endif//BLADE_USE_GLES
#endif // __Blade_GLESTexture_h__