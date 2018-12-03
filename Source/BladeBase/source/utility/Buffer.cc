/********************************************************************
	created:	2009/04/08
	filename: 	Buffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <StaticHandle.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	Buffer::Buffer( IPool* pool /*=  NULL*/ )
		:mData(&mInternalData)
	{
		mInternalData.mCapacity = 0;
		mInternalData.mSize = 0;
		mInternalData.mData = NULL;
		mInternalData.mPool = pool;
		if( mInternalData.mPool == NULL )
			mInternalData.mPool = Memory::getTemporaryPool();
	}

	//////////////////////////////////////////////////////////////////////////
	Buffer::Buffer(size_t mSize, IPool* pool /*=  NULL*/ )
		:mData(&mInternalData)
	{
		mInternalData.mCapacity = 0;
		mInternalData.mSize = 0;
		mInternalData.mData = NULL;
		mInternalData.mPool = pool;
		if( mInternalData.mPool == NULL )
			mInternalData.mPool = Memory::getTemporaryPool();

		if( !this->reserve(mSize) )
		{
			mInternalData.mCapacity = 0;
			mInternalData.mSize = 0;
			mInternalData.mData = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Buffer::~Buffer()
	{
		this->unshareData();
		this->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Buffer::reserve(size_t buffersize, bool copyOld/* = false*/)
	{
		if( mData->mCapacity == buffersize )
		{
			assert(buffersize == 0 || mData->mData != NULL );
			return true;
		}

		void* old = mData->mData;
		if( buffersize != 0 )
			mData->mData = BLADE_POOL_ALLOCATE(mData->mPool, buffersize);
		else
			mData->mData = NULL;

		if( old != NULL )
		{
			if( copyOld && mData->mData != NULL )
				std::memcpy(mData->mData, old, std::min<size_t>(mData->mCapacity, buffersize) );
			BLADE_POOL_DEALLOCATE(mData->mPool, old);
		}

		mData->mCapacity = buffersize;
		mData->mSize = 0;
		return mData->mData != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Buffer::clear()
	{
		if( mData->mData != NULL)
		{
			BLADE_POOL_DEALLOCATE(mData->mPool, mData->mData);
			mData->mData = NULL;
		}
		mData->mCapacity = 0;
		mData->mSize = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	Buffer::DATA	Buffer::detachData()
	{
		DATA ret = *mData;
		mData->mCapacity = 0;
		mData->mSize = 0;
		mData->mData = NULL;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Buffer::attachData(const DATA& data)
	{
		assert( mInternalData.mPool != NULL );
		if( data.mPool == mInternalData.mPool )
		{
			this->clear();
			mInternalData.mData = data.mData;
			mInternalData.mCapacity = data.mCapacity;
			mInternalData.mSize = data.mSize;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Buffer::setSize(size_t size)
	{
		if( size <= mData->mCapacity )
		{
			mData->mSize = size;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Buffer::freeData(const DATA& data)
	{
		if(data.mPool != NULL && data.mData != NULL )
		{
			BLADE_POOL_DEALLOCATE(data.mPool, data.mData);
			return true;
		}
		else
		{
			//if data shared, detached data is NULL
			//assert(false);
			return false;
		}
	}
	
}//namespace Blade