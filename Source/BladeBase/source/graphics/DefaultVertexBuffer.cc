/********************************************************************
	created:	2010/04/14
	filename: 	DefaultVertexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "DefaultVertexBuffer.h"
#include <interface/public/graphics/IGraphicsResourceManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DefaultVertexBuffer::DefaultVertexBuffer(const void* data, size_t vertexSize,size_t vertexCount,USAGE usage, IPool* pool)
		:mIsLocked(false)
		,mUsage(usage)
		,mVertexSize(vertexSize)
		,mVertexCount(vertexCount)
		,mPool(pool)
	{
		mSize = mVertexCount*mVertexSize;
		mBuffer = (char*)BLADE_POOL_ALLOCATE(pool, mSize);
		if(data != NULL)
			std::memcpy(mBuffer, data, mSize);
	}

	//////////////////////////////////////////////////////////////////////////
	DefaultVertexBuffer::~DefaultVertexBuffer()
	{
		BLADE_POOL_DEALLOCATE(mPool, mBuffer);
	}

	/************************************************************************/
	/* IGraphicsBuffer Interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		DefaultVertexBuffer::lock(size_t offset,size_t length,LOCKFLAGS lockflags)
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
	void*		DefaultVertexBuffer::lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(box);
		BLADE_UNREFERENCED(outPitch);
		BLADE_UNREFERENCED(lockflags);
		BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
	}

	//////////////////////////////////////////////////////////////////////////
	void		DefaultVertexBuffer::unlock(void)
	{
		mIsLocked = false;
	}


	//////////////////////////////////////////////////////////////////////////
	IGraphicsBuffer::USAGE		DefaultVertexBuffer::getUsage() const
	{
		return mUsage;
	}


	//////////////////////////////////////////////////////////////////////////
	bool		DefaultVertexBuffer::isLocked() const
	{
		return mIsLocked;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		DefaultVertexBuffer::addDirtyRegion(const Box3i& dirtyBox)
	{
		BLADE_UNREFERENCED(dirtyBox);
		BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
	}

	/************************************************************************/
	/* IVertexBuffer Interface                                                                     */
	/************************************************************************/

	
	//////////////////////////////////////////////////////////////////////////
	size_t		DefaultVertexBuffer::getVertexSize() const
	{
		return mVertexSize;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t		DefaultVertexBuffer::getVertexCount() const
	{
		return mVertexCount;
	}
	
}//namespace Blade