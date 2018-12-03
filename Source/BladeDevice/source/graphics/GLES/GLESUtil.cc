/********************************************************************
	created:	2014/12/20
	filename: 	GLESUtil.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESUtil.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const GLESVertexAttrib GLESVertexElementConverter::msTable[VET_COUNT] =
	{
		{1,GL_FLOAT,GL_FALSE},	//VET_FLOAT1
		{2,GL_FLOAT,GL_FALSE},	//VET_FLOAT2
		{3,GL_FLOAT,GL_FALSE},	//VET_FLOAT3
		{4,GL_FLOAT,GL_FALSE},	//VET_FLOAT4
		{4,GL_BYTE, GL_FALSE},	//VET_COLOR
		{2,GL_SHORT,GL_FALSE},	//VET_SHORT2
		{4,GL_SHORT,GL_FALSE},	//VET_SHORT4
		{4,GL_UNSIGNED_BYTE, GL_FALSE},	//VET_UBYTE4
		{4,GL_UNSIGNED_BYTE, GL_TRUE},	//VET_UBYTE4N
		{2,GL_SHORT,GL_TRUE},	//VET_SHORT2N
		{4,GL_SHORT,GL_TRUE},	//VET_SHORT4N
		{2,GL_UNSIGNED_SHORT,GL_TRUE},	//VET_USHORT2N
		{4,GL_UNSIGNED_SHORT,GL_TRUE},	//VET_USHORT4N
		{4,GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE}, //VET_UDEC3
		{4,GL_INT_2_10_10_10_REV, GL_TRUE}, //VET_DEC3N
		{2,GL_HALF_FLOAT, GL_FALSE},	//VET_HALF2
		{4,GL_HALF_FLOAT, GL_FALSE},	//VET_HALF4
	};

	//////////////////////////////////////////////////////////////////////////
	GLESPixelFormat GLESPixelFormatConverter::msBackBufferFormat = 
	{
		GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE
	};

	//////////////////////////////////////////////////////////////////////////
	GLESImageCache::GLESImageCache()
		:mLockedBox(Box3i::EMPTY)
	{
		mLocked = false;
		mReadOnly = true;
	}

	//////////////////////////////////////////////////////////////////////////
	GLESImageCache::~GLESImageCache()
	{
		assert( !mLocked );
		mImage.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESImageCache::lock(const Box3i& box, bool readOnlyLock, PixelFormat format, IMG_ORIENT layout)
	{
		if( !mLocked )
		{
			assert(mImage == NULL);
			mLockedBox = box;
			mImage = IImageManager::getSingleton().createImage(box.getWidth(), box.getHeight(), IP_TEMPORARY, format, layout, 1, box.getDepth(), 1);
			mReadOnly = readOnlyLock;
			mLocked = true;
			return true;
		}
		else
		{
			assert( mImage != NULL);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESImageCache::unlock()
	{
		mDirtyRegions.clear();
		if( mLocked )
		{
			assert(mImage != NULL );
			mImage.clear();
			mLocked = false;
			mReadOnly = true;
			return true;
		}
		else
		{
			assert( mImage == NULL);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void*	GLESImageCache::getData(size_t* rowPitch/* = NULL*/, size_t* slicePitch/* = NULL*/) const
	{
		if( mLocked )
		{
			assert(mImage != NULL);
			uint8* data = mImage->getBits();

			if( rowPitch != NULL )
				*rowPitch = mImage->getPitch();
			if( slicePitch != NULL )
				*slicePitch = mImage->getSlicePitch();
			return data;
		}
		else
		{
			assert( mImage == NULL);
			if( rowPitch != NULL )
				*rowPitch = INVALID_INDEX;
			if( slicePitch != NULL )
				*slicePitch = INVALID_INDEX;
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESImageCache::getSubData(const Box3i& box, TempBuffer& buffer) const
	{
		if( mLocked )
		{
			assert( mImage != NULL );
			buffer.clear();
			assert( box.getWidth() <= mImage->getImageWidth() && box.getHeight() <= mImage->getImageHeight() && box.getDepth() <= mImage->getDepth() );
			//early out of simple situation
			if( box.getWidth() == mImage->getImageWidth() && box.getHeight() == mImage->getImageHeight() && box.getDepth() == mImage->getDepth() )
			{
				buffer.reserve( mImage->getBytes() );
				buffer.setSize( buffer.getCapacity() );
				std::memcpy(buffer.getData(), mImage->getBits(), buffer.getSize() );
				return true;
			}

			size_t bytes = IImageManager::getSingleton().calcImageSize(box.getWidth(), box.getHeight(), box.getDepth(), mImage->getFormat() );
			buffer.reserve( bytes );
			buffer.setSize( bytes );

			const char* bits = (const char*)mImage->getBits();
			size_t unitSize = mImage->getFormat().getSizeBytes();
			bits += box.getLeft()*unitSize + box.getHeight()*mImage->getPitch() + box.getBack()*mImage->getSlicePitch();
			char* data = (char*)buffer.getData();

			size_t rowSize = box.getWidth()*unitSize;
			for(int d = box.getBack(); d < box.getFront(); ++d)
			{
				const char* src = bits + d * mImage->getSlicePitch();
				for(size_t i = 0; i < mImage->getHeight(); ++i)	//use image's height: image may be compressed
				{
					std::memcpy(data, src, rowSize);
					data += rowSize;
					src += mImage->getPitch();
				}
			}
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	GLESImageCache::getSize(size_t& width, size_t& height) const
	{
		if( mLocked )
		{
			assert(mImage != NULL);
			width = mImage->getImageWidth();
			height = mImage->getImageHeight();
			return true;
		}
		else
		{
			assert( mImage == NULL);
			width = INVALID_INDEX;
			height = INVALID_INDEX;
			return false;
		}
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES