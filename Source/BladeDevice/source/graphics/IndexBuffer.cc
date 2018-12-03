/********************************************************************
	created:	2010/04/09
	filename: 	IndexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/IndexBuffer.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	IndexBuffer::IndexBuffer()
		:mTotalBytes(0)
		,mIndexCount(0)
		,mLockAt(0)
		,mLockRange(0)
		,mIndexType(IT_16BIT)
		,mUsage(GBU_STATIC)
		,mIsLocked(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IndexBuffer::IndexBuffer(size_t indexCount, USAGE usage, EIndexType idextype)
		:mTotalBytes( indexCount * (idextype == IT_16BIT ? 2 : 4) )
		,mIndexCount(indexCount)
		,mLockAt(0)
		,mLockRange(0)
		,mIndexType(idextype)
		,mUsage(usage)
		,mIsLocked(false)
	{

	}

	/************************************************************************/
	/* IGraphicsBuffer interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		IndexBuffer::lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(box);
		BLADE_UNREFERENCED(outPitch);
		BLADE_UNREFERENCED(lockflags);

		BLADE_EXCEPT(EXC_UNKNOWN,BTString("invalid lock operation.") );
		//return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsBuffer::USAGE		IndexBuffer::getUsage() const
	{
		return mUsage;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		IndexBuffer::isLocked() const
	{
		return mIsLocked;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		IndexBuffer::addDirtyRegion(const Box3i& dirtyBox)
	{
		BLADE_UNREFERENCED(dirtyBox);
		BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
	}

	/************************************************************************/
	/* IIndexBuffer interface                                                                     */
	/************************************************************************/

	
}//namespace Blade