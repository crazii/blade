/********************************************************************
	created:	2015/03/24
	filename: 	GLESCubeTexture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESCubeTexture.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	static GLenum GL_CUBE_FACES[] =
	{
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

	//////////////////////////////////////////////////////////////////////////
	GLESCubeTexture::GLESCubeTexture(GLuint name, IMG_ORIENT layout, size_t width,size_t height, PixelFormat format,
		IGraphicsBuffer::USAGE usage, IGraphicsBuffer::ACCESS GpuAccess, size_t mipmap/* = 1*/)

		:Texture(TT_CUBE, layout, width, height, format, usage, GpuAccess, 1, mipmap)
		,mTextureName(name)
	{
		::glBindTexture(GL_TEXTURE_CUBE_MAP, name);
		assert( ::glGetError() == GL_NO_ERROR );

		::glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, (GLint)mDesc.mMipmapCount-1);
		assert( ::glGetError() == GL_NO_ERROR );

		mGLFormat = GLESPixelFormatConverter(format);
		assert( mipmap > 0 );
		::glTexStorage2D(GL_TEXTURE_CUBE_MAP, (GLsizei)mipmap, mGLFormat.internalFormat, width, height);
		assert( ::glGetError() == GL_NO_ERROR );
	}

	//////////////////////////////////////////////////////////////////////////
	GLESCubeTexture::~GLESCubeTexture()
	{
		::glDeleteTextures(1, &mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );
		mTextureName = 0;
	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		GLESCubeTexture::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
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

		return HTEXTURE(BLADE_NEW GLESCubeTexture(name, mDesc.mLayout, newWidth, newHeight, newFormat, mDesc.mUsage, mDesc.mGpuAccess, mipLevels));;
	}

	//////////////////////////////////////////////////////////////////////////
	void*			GLESCubeTexture::lockImpl(const TEXREGION& region, IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		assert(region.mFace >= 0 && region.mFace < 6 );

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
				::glReadBuffer(GL_COLOR_ATTACHMENT0);
				assert( ::glGetError() == GL_NO_ERROR );

				//we have unique FBO for reading 
				::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_CUBE_FACES[region.mFace], mTextureName, mipLevel);
				assert( ::glGetError() == GL_NO_ERROR );

				::glReadPixels( (GLint)box.getLeft(), (GLint)box.getTop(), (GLsizei)box.getWidth(), (GLsizei)box.getHeight(), 
					mGLFormat.format, mGLFormat.type, bits);
				assert( ::glGetError() == GL_NO_ERROR );

				::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_CUBE_FACES[region.mFace], 0, 0);
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
	bool			GLESCubeTexture::unlockImpl()
	{
		GLint mipLevel = (GLint)this->getLockedMip();
		index_t face = this->getLockedFace();
		assert( face >= 0 && face < 6 );
		assert( mLockedData.isLocked() );

		//upload image data to texture
		::glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureName);
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

				if( mDesc.mPixelFormat.isCompressed() )
					::glCompressedTexSubImage2D(GL_CUBE_FACES[face], mipLevel, x, y, width, height, mGLFormat.internalFormat, 
					(GLsizei)buffer.getSize(), buffer.getData() );
				else
					::glTexSubImage2D(GL_CUBE_FACES[face], mipLevel, x, y, width, height, mGLFormat.format, 
					mGLFormat.type, buffer.getData() );

				assert( ::glGetError() == GL_NO_ERROR );
			}
		}

		::glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		assert( ::glGetError() == GL_NO_ERROR );

		mLockedData.unlock();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESCubeTexture::updateMipMap(const TEXREGION& region, index_t mipLevel/* = 0*/)
	{
		assert(region.mFace == 0);

		if(this->isLocked() || mDesc.mMipmapCount <= 1)
			return false;

		::glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );

		//try hardware mip maps first
		::glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
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
		HIMAGE image = IImageManager::getSingleton().createImage(width, height, IP_TEMPORARY, mDesc.mPixelFormat, mDesc.mLayout, mDesc.mMipmapCount, 1, 6);

		//fill texture content into image
		{
			::glReadBuffer(GL_COLOR_ATTACHMENT0);
			assert( ::glGetError() == GL_NO_ERROR );
			for(size_t f = 0; f < 6; ++f)
			{
				void* bits = image->getMipInfo(mipLevel, f)->data;
				//we have unique FBO for reading 
				::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_CUBE_FACES[f], mTextureName, (GLint)sourceLevel);
				assert( ::glGetError() == GL_NO_ERROR );

				::glReadPixels( (GLint)mipBox.getLeft(), (GLint)mipBox.getTop(), (GLsizei)width, (GLsizei)height, 
					mGLFormat.format, mGLFormat.type, bits);

				assert( ::glGetError() == GL_NO_ERROR );
				::glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  GL_CUBE_FACES[f], 0, 0);
			}
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

			for(size_t f = 0; f < 6; ++f)
			{
				const IMG_MIPINFO* mipInfo = image->getMipInfo(i, f);
				assert( mipInfo != NULL );

				if( mDesc.mPixelFormat.isCompressed() )
					::glCompressedTexSubImage2D(GL_CUBE_FACES[f], (GLint)i, box.getLeft(), box.getRight(), box.getWidth(), box.getHeight(), 
					mGLFormat.internalFormat, (GLsizei)IImageManager::getSingleton().calcImageSize(width, height, 1, mDesc.mPixelFormat), mipInfo->data);
				else
					::glTexSubImage2D(GL_CUBE_FACES[f], (GLint)i, box.getLeft(), box.getRight(), box.getWidth(), box.getHeight(), 
					mGLFormat.format, mGLFormat.type, mipInfo->data);

				assert( ::glGetError() == GL_NO_ERROR );

			}
		}

		::glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		assert( ::glGetError() == GL_NO_ERROR );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESCubeTexture::addDirtyRegion(const TEXREGION& region)
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