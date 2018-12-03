/********************************************************************
	created:	2010/04/14
	filename: 	DefaultIndexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultIndexBuffer.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DefaultIndexBuffer::DefaultIndexBuffer(const void* data, EIndexType type, size_t indexCount, USAGE usage, IPool* pool)
		:mIndexCount(indexCount)
		,mPool(pool)
		,mIndexType(type)
		,mUsage(usage)
		,mIsLocked(false)
	{
		assert(indexCount != 0);
		mSize = indexCount*this->getIndexSize();
		mBuffer = BLADE_POOL_ALLOCATE(pool, mSize);
		if(data != NULL)
			std::memcpy(mBuffer, data, mSize);
	}

	//////////////////////////////////////////////////////////////////////////
	DefaultIndexBuffer::~DefaultIndexBuffer()
	{
		BLADE_POOL_DEALLOCATE(mPool, mBuffer);
	}

	//////////////////////////////////////////////////////////////////////////
	void*		DefaultIndexBuffer::lock(size_t offset,size_t length,LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(lockflags);
		if( !mIsLocked )
		{
			mIsLocked = true;

			if( offset < mSize )
			{
				if( offset + length > mSize )
					length = mSize - offset;
			}
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("lock data of out range.") );

			return &((char*)mBuffer)[offset];
		}
		else
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("already locked,cannot lock again.") );
	}



	//////////////////////////////////////////////////////////////////////////
	void*		DefaultIndexBuffer::lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(box);
		BLADE_UNREFERENCED(outPitch);
		BLADE_UNREFERENCED(lockflags);
		BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
	}


	//////////////////////////////////////////////////////////////////////////
	void		DefaultIndexBuffer::unlock(void)
	{
		mIsLocked = false;
	}


	//////////////////////////////////////////////////////////////////////////
	IGraphicsBuffer::USAGE		DefaultIndexBuffer::getUsage() const
	{
		return mUsage;
	}


	//////////////////////////////////////////////////////////////////////////
	bool		DefaultIndexBuffer::isLocked() const
	{
		return mIsLocked;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		DefaultIndexBuffer::addDirtyRegion(const Box3i& dirtyBox)
	{
		BLADE_UNREFERENCED(dirtyBox);
		BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
	}

	/************************************************************************/
	/* IIndexBuffer Interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	IIndexBuffer::EIndexType	DefaultIndexBuffer::getIndexType() const
	{
		return mIndexType;
	}


	//////////////////////////////////////////////////////////////////////////
	size_t		DefaultIndexBuffer::getIndexCount() const
	{
		return mIndexCount;
	}


	//////////////////////////////////////////////////////////////////////////
	size_t		DefaultIndexBuffer::getIndexSize() const
	{
		return mIndexType == IT_16BIT?2u:4u;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		DefaultIndexBuffer::getSizeInBytes() const
	{
		return mSize;
	}
	
}//namespace Blade