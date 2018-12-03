/********************************************************************
	created:	2015/03/24
	filename: 	GLESVolumeTexture.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESVolumeTexture.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	GLESVolumeTexture::GLESVolumeTexture(GLuint name, IMG_ORIENT layout, size_t width,size_t height, PixelFormat format,
		IGraphicsBuffer::USAGE usage, IGraphicsBuffer::ACCESS GpuAccess, size_t depth, size_t mipmap/* = 1*/)

		:Texture(TT_3DIM, layout, width, height, format, usage, GpuAccess, depth, mipmap)
		,mTextureName(name)
	{
		::glBindTexture(GL_TEXTURE_3D, name);
		assert( ::glGetError() == GL_NO_ERROR );

		::glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, (GLint)mDesc.mMipmapCount-1);
		assert( ::glGetError() == GL_NO_ERROR );

		mGLFormat = GLESPixelFormatConverter(format);
		assert( mipmap > 0 );
		::glTexStorage3D(GL_TEXTURE_3D, (GLsizei)mipmap, mGLFormat.internalFormat, width, height, depth);
		assert( ::glGetError() == GL_NO_ERROR );
	}

	//////////////////////////////////////////////////////////////////////////
	GLESVolumeTexture::~GLESVolumeTexture()
	{
		::glDeleteTextures(1, &mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );
		mTextureName = 0;
	}

	/************************************************************************/
	/* ITexture Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	HTEXTURE		GLESVolumeTexture::cloneTexture(size_t newWidth/* = 0*/,size_t newHeight/* = 0*/,PixelFormat newFormat/* = PF_UNDEFINED*/)
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

		return HTEXTURE(BLADE_NEW GLESVolumeTexture(name, mDesc.mLayout, newWidth, newHeight, newFormat, mDesc.mUsage, mDesc.mGpuAccess, mDesc.mDepth, mipLevels));
	}

	//////////////////////////////////////////////////////////////////////////
	void*			GLESVolumeTexture::lockImpl(const TEXREGION& region, IGraphicsBuffer::LOCKFLAGS lockflags)
	{
		assert(region.mFace == 0);

		if( (mDesc.mUsage.isRenderTarget() || mDesc.mUsage.isDepthStencil()) && !mDesc.mUsage.isDynamic() )
			BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("none-lockable render target or depth stencil.") );

		assert( !mLockedData.isLocked() );
		assert( region.mMipLevel < mDesc.mMipmapCount );

		Box3i box = region.mBox;
		if( &region.mBox == &Box3i::EMPTY || box == Box3i::EMPTY )
			box = Box3i(0, 0, (int)mDesc.mWidth, (int)mDesc.mHeight, 0, mDesc.mDepth);
		size_t mipLevel = region.mMipLevel;
		box = Box3i( box.getLeft() >> mipLevel, box.getTop() >> mipLevel, box.getRight() >> mipLevel, box.getBottom() >> mipLevel, box.getBack(), box.getFront() );

		bool readOnly = ((lockflags&IGraphicsBuffer::GBLF_WRITEONLY) == 0);
		bool ret = mLockedData.lock(box, readOnly, mDesc.mPixelFormat, mDesc.mLayout);
		assert( ret ); BLADE_UNREFERENCED(ret);
		void* bits = mLockedData.getData(&region.mRowPitch, &region.mSlicePitch);

		//read access: download texture data to bits
		if( (lockflags&IGraphicsBuffer::GBLF_READONLY) )
		{
			if( !mDesc.mPixelFormat.isCompressed() )
			{
				::glReadBuffer(GL_COLOR_ATTACHMENT0);
				assert( ::glGetError() == GL_NO_ERROR );
				char* layerBits = (char*)bits;

				for(int i = box.getBack(); i < box.getFront(); ++i )
				{
					//we have unique FBO for reading 
					::glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextureName, mipLevel, i);
					assert( ::glGetError() == GL_NO_ERROR );

					::glReadPixels( (GLint)box.getLeft(), (GLint)box.getTop(), (GLsizei)box.getWidth(), (GLsizei)box.getHeight(), 
						mGLFormat.format, mGLFormat.type, layerBits);
					assert( ::glGetError() == GL_NO_ERROR );

					layerBits += region.mSlicePitch;
				}

				::glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  0, 0, 0);
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
	bool			GLESVolumeTexture::unlockImpl()
	{
		GLint mipLevel = (GLint)this->getLockedMip();
		index_t face = this->getLockedFace();
		assert( face == 0 ); BLADE_UNREFERENCED(face);
		assert( mLockedData.isLocked() );

		//upload image data to texture
		::glBindTexture(GL_TEXTURE_3D, mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );

		if( !mLockedData.isReadOnly() && mLockedData.getDirtyCount() > 0 )
		{
			TempBuffer buffer;
			for(size_t i = 0; i < mLockedData.getDirtyCount(); ++i)
			{
				const Box3i& box = mLockedData.getDirtyRegion(i);
				GLint x = (GLint)box.getLeft();
				GLint y = (GLint)box.getTop();
				GLint z = (GLint)box.getBack();
				GLsizei width = (GLsizei)box.getWidth();
				GLsizei height = (GLsizei)box.getHeight();
				GLsizei depth = (GLsizei)box.getDepth();
				bool ret = mLockedData.getSubData(box, buffer);
				assert(ret); BLADE_UNREFERENCED(ret);

				if( mDesc.mPixelFormat.isCompressed() )
					::glCompressedTexSubImage3D(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, mGLFormat.internalFormat, 
					(GLsizei)buffer.getSize(), buffer.getData());
				else
					::glTexSubImage3D(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, mGLFormat.format, 
					mGLFormat.type, buffer.getData() );

				assert( ::glGetError() == GL_NO_ERROR );
			}
		}

		::glBindTexture(GL_TEXTURE_3D, 0);
		assert( ::glGetError() == GL_NO_ERROR );

		mLockedData.unlock();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESVolumeTexture::updateMipMap(const TEXREGION& region, index_t mipLevel/* = 0*/)
	{
		assert(region.mFace == 0);

		if(this->isLocked() || mDesc.mMipmapCount <= 1)
			return false;

		::glBindTexture(GL_TEXTURE_3D, mTextureName);
		assert( ::glGetError() == GL_NO_ERROR );

		//try hardware mip maps first
		::glGenerateMipmap(GL_TEXTURE_3D);
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
		HIMAGE image = IImageManager::getSingleton().createImage(width, height, IP_TEMPORARY, mDesc.mPixelFormat, mDesc.mLayout, mDesc.mMipmapCount, mDesc.mDepth, 1);

		//fill texture content into image
		{
			::glReadBuffer(GL_COLOR_ATTACHMENT0);
			assert( ::glGetError() == GL_NO_ERROR );
			char* layerBits = (char*)image->getBits();

			for(int i = box.getBack(); i < box.getFront(); ++i )
			{
				//we have unique FBO for reading 
				::glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextureName, mipLevel, i);
				assert( ::glGetError() == GL_NO_ERROR );

				::glReadPixels( (GLint)box.getLeft(), (GLint)box.getTop(), (GLsizei)box.getWidth(), (GLsizei)box.getHeight(), 
					mGLFormat.format, mGLFormat.type, layerBits);
				assert( ::glGetError() == GL_NO_ERROR );

				layerBits += region.mSlicePitch;
			}

			::glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  0, 0, 0);
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
				::glCompressedTexSubImage3D(GL_TEXTURE_3D, (GLint)i, box.getLeft(), box.getRight(), box.getBack(), box.getWidth(), box.getHeight(), box.getDepth(),
				mGLFormat.internalFormat, (GLsizei)IImageManager::getSingleton().calcImageSize(width, height, 1, mDesc.mPixelFormat), mipInfo->data);
			else
				::glTexSubImage3D(GL_TEXTURE_3D, (GLint)i, box.getLeft(), box.getRight(), box.getBack(), box.getWidth(), box.getHeight(), box.getDepth(),
				mGLFormat.format, mGLFormat.type, mipInfo->data);

			assert( ::glGetError() == GL_NO_ERROR );
		}

		::glBindTexture(GL_TEXTURE_3D, 0);
		assert( ::glGetError() == GL_NO_ERROR );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			GLESVolumeTexture::addDirtyRegion(const TEXREGION& region)
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