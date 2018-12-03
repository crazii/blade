/********************************************************************
	created:	2014/12/20
	filename: 	GLESTexture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESTexture.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	GLESTexture::GLESTexture(GLuint name, IMG_ORIENT layout, size_t width,size_t height, PixelFormat format,
		IGraphicsBuffer::USAGE usage, IGraphicsBuffer::ACCESS GpuAccess, size_t mipmap/* = 1*/)
		
		:Texture(TT_2DIM, layout, width, height, format, usage, GpuAccess, 1, mipmap)
		,mTextureName(name)
	{
		mGLFormat = GLESPixelFormatConverter(format);

		assert(mipmap > 0);
		//sRGB check
		assert(format.isLinearColor() || mGLFormat.internalFormat == GL_COMPRESSED_SRGB8_ETC2 || mGLFormat.internalFormat == GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
			|| mGLFormat.internalFormat == GL_COMPRESSED_RG11_EAC || mGLFormat.internalFormat == GL_COMPRESSED_R11_EAC
			|| mGLFormat.internalFormat == GL_SRGB || mGLFormat.internalFormat == GL_SRGB8_ALPHA8);

		::glBindTexture(GL_TEXTURE_2D, name);
		assert( ::glGetError() == GL_NO_ERROR );

		::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)mDesc.mMipmapCount - 1);
		assert(::glGetError() == GL_NO_ERROR);

		::glTexStorage2D(GL_TEXTURE_2D, (GLsizei)mipmap, mGLFormat.internalFormat, width, height);
		assert(::glGetError() == GL_NO_ERROR);

		//note: shadow map compare function is set by shadowmap manager @see ShadowManager::adjustBuffer
		//if (format.isDepth())
		//{
		//	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		//	assert(::glGetError() == GL_NO_ERROR);
		//}

		/*
		glTexStorage2D create a immutable-format texture, which is equivalent to:
		glTexStorage2D don't allow further format change and can be updated only using ::glCompressedSubTexImage2D

		for(size_t i = 0; i < mipmap; i++)
		{
			if( mDesc.mPixelFormat.isCompressed() )
				::glCompressedTexImage2D(GL_TEXTURE_2D, (GLint)i, mGLFormat.internalFormat, width, height, 0, 
				(GLsizei)IImageManager::getSingleton().calcImageSize(width, height, 1, format), NULL);
			else
				::glTexImage2D(GL_TEXTURE_2D, (GLint)i, mGLFormat.internalFormat, width, height, 0, 
				mGLFormat.format, mGLFormat.type, NULL);
			assert( ::glGetError() == GL_NO_ERROR );
			width = std::max(1u, (width/2));
			height = std::max(1u, (height/2));
		}

		::glBindTexture(GL_TEXTURE_2D, 0);
		assert( ::glGetError() == GL_NO_ERROR );
		*/
	}

	//////////////////////////////////////////////////////////////////////////
	GLESTexture::~GLESTexture()
	{
		::glDeleteTextures(1, &mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );
		mTextureName = 0;
	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		GLESTexture::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
	{
		assert( mTextureName != 0 );

		if( newWidth == 0 )
			newWidth = mDesc.mWidth;
		if( newHeight == 0 )
			newHeight = mDesc.mHeight;
		if( newFormat == PF_UNDEFINED )
			newFormat = mDesc.mPixelFormat;

		size_t mipLevels;
		if( mDesc.mMipmapCount == Math::CalcMaxMipCount(mDesc.mWidth, mDesc.mHeight) )
			mipLevels = Math::CalcMaxMipCount(newWidth, newHeight);
		else
			mipLevels = mDesc.mMipmapCount;

		GLuint name;
		::glGenTextures(1, &name);
		assert( ::glGetError() == GL_NO_ERROR );

		return HTEXTURE(BLADE_NEW GLESTexture(name, mDesc.mLayout, newWidth, newHeight, newFormat, mDesc.mUsage, mDesc.mGpuAccess, mipLevels));
	}

	//////////////////////////////////////////////////////////////////////////
	void*			GLESTexture::lockImpl(const TEXREGION& region, IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		assert(region.mFace == 0);

		if( (mDesc.mUsage.isRenderTarget() || mDesc.mUsage.isDepthStencil()) && !mDesc.mUsage.isDynamic() )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("none-lockable render target or depth stencil.") );

		assert( !mLockedData.isLocked() );
		assert( region.mMipLevel < mDesc.mMipmapCount );

		Box3i box = region.mBox;
		if( &region.mBox == &Box3i::EMPTY || box == Box3i::EMPTY )
			box = Box3i(0, 0, (int)mDesc.mWidth, (int)mDesc.mHeight, 0, 1);
		size_t mipLevel = region.mMipLevel;
		box = Box3i( box.getLeft() >> mipLevel, box.getTop() >> mipLevel, box.getRight() >> mipLevel, box.getBottom() >> mipLevel, box.getBack(), box.getFront() );

		bool readOnly = ((lockflags&IGraphicsBuffer::GBLF_WRITEONLY) == 0);
		bool ret = mLockedData.lock(box, readOnly, mDesc.mPixelFormat, mDesc.mLayout);
		assert(ret); BLADE_UNREFERENCED(ret);
		void* bits = mLockedData.getData(&region.mRowPitch, &region.mSlicePitch);

		//read access: download texture data to bits
		if( (lockflags&IGraphicsBuffer::GBLF_READONLY) )
		{
			if( !mDesc.mPixelFormat.isCompressed() )
			{
				//we have unique FBO for reading 
				::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_TEXTURE_2D, mTextureName, mipLevel);
				assert( ::glGetError() == GL_NO_ERROR );

				::glReadBuffer(GL_COLOR_ATTACHMENT0);
				assert( ::glGetError() == GL_NO_ERROR );
				::glReadPixels( (GLint)box.getLeft(), (GLint)box.getTop(), (GLsizei)box.getWidth(), (GLsizei)box.getHeight(), 
					mGLFormat.format, mGLFormat.type, bits);
				assert( ::glGetError() == GL_NO_ERROR );

				::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_TEXTURE_2D, 0, 0);
				assert( ::glGetError() == GL_NO_ERROR );
			}
			else
				BLADE_LOG(Error, BTString("reading a compressed texture content is not supported."));
		}

		if( !(lockflags&IGraphicsBuffer::GBLF_NO_DIRTY_UPDATE) )
			mLockedData.addDirtyRegion(box);
		return bits;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESTexture::unlockImpl()
	{
		GLint mipLevel = (GLint)this->getLockedMip();
		index_t face = this->getLockedFace();
		assert(face == 0); BLADE_UNREFERENCED(face);
		assert( mLockedData.isLocked() );

		//upload image data to texture
		::glBindTexture(GL_TEXTURE_2D, mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );

		if( !mLockedData.isReadOnly() && mLockedData.getDirtyCount() > 0 )
		{
			TempBuffer buffer;
			for(size_t i = 0; i < mLockedData.getDirtyCount(); ++i)
			{
				const Box3i& box = mLockedData.getDirtyRegion(i);
				GLint x = (GLint)box.getLeft();
				GLint y = (GLint)box.getTop();
				GLsizei width = (GLsizei)box.getWidth();
				GLsizei height = (GLsizei)box.getHeight();
				bool ret = mLockedData.getSubData(box, buffer);
				assert(ret); BLADE_UNREFERENCED(ret);

				//Note: update "sub" image of compressed texture is NOT supported by GLES
				//only if the sub image matches the whole texture, it will succeed
				if( mDesc.mPixelFormat.isCompressed() )
					::glCompressedTexSubImage2D(GL_TEXTURE_2D, mipLevel, x, y, width, height, mGLFormat.internalFormat, 
					(GLsizei)buffer.getSize(), buffer.getData() );
				else
					::glTexSubImage2D(GL_TEXTURE_2D, mipLevel, x, y, width, height, mGLFormat.format, 
					mGLFormat.type, buffer.getData() );

				assert( ::glGetError() == GL_NO_ERROR );
			}
		}

		::glBindTexture(GL_TEXTURE_2D, 0);
		assert( ::glGetError() == GL_NO_ERROR );

		mLockedData.unlock();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESTexture::updateMipMap(const TEXREGION& region, index_t mipLevel/* = 0*/)
	{
		assert(region.mFace == 0);

		if(this->isLocked() || mDesc.mMipmapCount <= 1)
			return false;

		::glBindTexture(GL_TEXTURE_2D, mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );

		//try hardware mip maps first
		::glGenerateMipmap(GL_TEXTURE_2D);
		bool ret = ::glGetError() == GL_NO_ERROR;
		if( ret )
			return ret;
		if( mDesc.mPixelFormat.isCompressed() )
			return false;

		//generate mip maps manually
		Box3i box = region.mBox;
		if( &region.mBox == &Box3i::EMPTY || box == Box3i::EMPTY )
			box = Box3i(0, 0, (int)mDesc.mWidth, (int)mDesc.mHeight, 0, 1);

		index_t sourceLevel = mipLevel == 0 ? 0 : mipLevel - 1;
		Box3i mipBox = Box3i( box.getLeft() >> sourceLevel, box.getTop() >> sourceLevel, box.getRight() >> sourceLevel, box.getBottom() >> sourceLevel, box.getBack(), box.getFront() );

		size_t width = mipBox.getWidth();
		size_t height = mipBox.getHeight();
		HIMAGE image = IImageManager::getSingleton().createImage(width, height, IP_TEMPORARY, mDesc.mPixelFormat, mDesc.mLayout, mDesc.mMipmapCount, 1, 1);

		//fill texture content into image
		{
			::glReadBuffer(GL_COLOR_ATTACHMENT0);
			assert( ::glGetError() == GL_NO_ERROR );
			//we have unique FBO for reading 
			::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_TEXTURE_2D, mTextureName, (GLint)sourceLevel);
			assert( ::glGetError() == GL_NO_ERROR );
			::glReadPixels( (GLint)mipBox.getLeft(), (GLint)mipBox.getTop(), (GLsizei)width, (GLsizei)height, 
				mGLFormat.format, mGLFormat.type, image->getBits());
			assert( ::glGetError() == GL_NO_ERROR );
			::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_TEXTURE_2D, 0, 0);
			assert( ::glGetError() == GL_NO_ERROR );
		}

		//generate mipmap for image
		IImageManager::getSingleton().generateMipmaps(image, mipLevel == 0 ? -1 : 1);

		//upload data to texture
		for(size_t i = 1; i < mDesc.mMipmapCount; ++i)
		{
			box = Box3i( box.getLeft()/2, box.getTop()/2, box.getRight()/2, box.getBottom()/2, box.getBack(), box.getFront() );
			if( box.getWidth() == 0 || box.getHeight() == 0 )
				break;

			if( mipLevel != i && mipLevel != 0 )
				continue;

			const IMG_MIPINFO* mipInfo = image->getMipInfo(i);
			assert( mipInfo != NULL );

			if( mDesc.mPixelFormat.isCompressed() )
				::glCompressedTexSubImage2D(GL_TEXTURE_2D, (GLint)i, box.getLeft(), box.getRight(), box.getWidth(), box.getHeight(), 
				mGLFormat.internalFormat, (GLsizei)IImageManager::getSingleton().calcImageSize(width, height, 1, mDesc.mPixelFormat), mipInfo->data);
			else
				::glTexSubImage2D(GL_TEXTURE_2D, (GLint)i, box.getLeft(), box.getRight(), box.getWidth(), box.getHeight(), 
				mGLFormat.format, mGLFormat.type, mipInfo->data);

			assert( ::glGetError() == GL_NO_ERROR );
		}

		::glBindTexture(GL_TEXTURE_2D, 0);
		assert( ::glGetError() == GL_NO_ERROR );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESTexture::addDirtyRegion(const TEXREGION& region)
	{
		if( this->isLocked() )
		{
			assert( mLockedData.isLocked() );
			mLockedData.addDirtyRegion(region.mBox);
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES