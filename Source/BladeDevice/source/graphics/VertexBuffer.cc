/********************************************************************
	created:	2010/04/09
	filename: 	VertexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/VertexBuffer.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	VertexBuffer::VertexBuffer()
		:mSizeBytes(0)
		,mVertexCount(0)
		,mVertexSize(0)
		,mUsage(GBU_STATIC)
		,mIsLocked(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	VertexBuffer::VertexBuffer(size_t vertexSize, size_t vertexCount, USAGE usage)
		:mSizeBytes(vertexSize*vertexCount)
		,mVertexCount(vertexCount)
		,mVertexSize(vertexSize)
		,mUsage(usage)
		,mIsLocked(false)
	{
	}


	/************************************************************************/
	/*IGraphicsBuffer interface                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void*		VertexBuffer::lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(box);
		BLADE_UNREFERENCED(outPitch);
		BLADE_UNREFERENCED(lockflags);

		BLADE_EXCEPT(EXC_UNKNOWN,BTString("invalid lock operation.") );
		//return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsBuffer::USAGE		VertexBuffer::getUsage() const
	{
		return mUsage;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		VertexBuffer::isLocked() const
	{
		return mIsLocked;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		VertexBuffer::addDirtyRegion(const Box3i& dirtyBox)
	{
		BLADE_UNREFERENCED(dirtyBox);
		BLADE_EXCEPT(EXC_INVALID_OPERATION,BTString("method not supported.") );
	}

	/************************************************************************/
	/* IVertexBuffer interface                                                                    */
	/************************************************************************/

}//namespace Blade