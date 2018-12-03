/********************************************************************
	created:	2010/04/09
	filename: 	PixelBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/PixelBuffer.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	PixelBuffer::PixelBuffer(ELocation location)
		:mSizeBytes(0)
		,mUsage(GBU_STATIC)
		,mLocation(location)
		,mIsLocked(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	PixelBuffer::PixelBuffer(ELocation location,USAGE usage,PixelFormat format,size_t width,size_t height,size_t depth)
		:mSizeBytes(0)
		,mWidth(width)
		,mHeight(height)
		,mDepth(depth)
		,mFormat(format)		
		,mUsage(usage)
		,mLocation(location)
		,mIsLocked(false)
	{

	}


	/************************************************************************/
	/*IGraphicsBuffer interface                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		PixelBuffer::lock(size_t offset,size_t length,LOCKFLAGS lockflags)
	{
		BLADE_UNREFERENCED(offset);
		BLADE_UNREFERENCED(length);
		BLADE_UNREFERENCED(lockflags);
		BLADE_EXCEPT(EXC_UNKNOWN,BTString("invalid lock operation.") );
		//return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	IGraphicsBuffer::USAGE		PixelBuffer::getUsage() const
	{
		return mUsage;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PixelBuffer::isLocked() const
	{
		return mIsLocked;
	}


	/************************************************************************/
	/* IPixelBuffer interface                                                                    */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	PixelFormat	PixelBuffer::getPixelFormat() const
	{
		return mFormat;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			PixelBuffer::getWidth() const
	{
		return mWidth;
	}


	//////////////////////////////////////////////////////////////////////////
	size_t			PixelBuffer::getHeight() const
	{
		return mHeight;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			PixelBuffer::getDepth() const
	{
		return mDepth;
	}

	//////////////////////////////////////////////////////////////////////////
	IPixelBuffer::ELocation		PixelBuffer::getLocation() const
	{
		return mLocation;
	}
	
}//namespace Blade