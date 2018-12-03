/********************************************************************
	created:	2014/12/20
	filename: 	GLESIndexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>

#if BLADE_USE_GLES
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESIndexBuffer.h>

namespace Blade
{
	GLuint GLESIndexBuffer::msLockedPrevious = 0;
	const GLESIndexBuffer* GLESIndexBuffer::msFirstLock = NULL;

	//////////////////////////////////////////////////////////////////////////
	GLESIndexBuffer::GLESIndexBuffer(GLuint name, IIndexBuffer::EIndexType type, size_t count, USAGE usage, const void* data)
		:IndexBuffer(count, usage, type)
		,mBufferName(name)
	{
		GLint old = 0;
		::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &old);
		assert(::glGetError() == GL_NO_ERROR);

		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );
		::glBufferData(GL_ELEMENT_ARRAY_BUFFER, mTotalBytes, data, GLESBufferUsageConverter(usage) );
		assert( ::glGetError() == GL_NO_ERROR );

		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)old);
		assert( ::glGetError() == GL_NO_ERROR );
	}

	//////////////////////////////////////////////////////////////////////////
	GLESIndexBuffer::~GLESIndexBuffer()
	{
		GLint old = 0;
		::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &old);
		assert(::glGetError() == GL_NO_ERROR);

		assert( !mIsLocked );
		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		assert( ::glGetError() == GL_NO_ERROR );

		::glDeleteBuffers(1, &mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );
		mBufferName = 0;

		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)old);
		assert(::glGetError() == GL_NO_ERROR);
	}

	/************************************************************************/
	/* IGraphicsBuffer interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		GLESIndexBuffer::lock(size_t offset,size_t length,LOCKFLAGS lockflags)
	{
		assert( mBufferName != 0 );

		if( mIsLocked )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer already locked.") );

		if( (!mUsage.isReadable() && (lockflags&GBLF_READONLY) != 0)
			|| (!mUsage.isWriteable() && (lockflags&GBLF_WRITEONLY) != 0) )
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid lock operation for buffer access.") );

		if( offset <= mTotalBytes )
		{
			if( length > mTotalBytes - offset )//no overflow check
				length = mTotalBytes - offset;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("lock data of out range.") );

		if (msLockedPrevious == 0)
		{
			assert(msFirstLock == NULL);
			msFirstLock = this;
			::glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, (GLint*)&msLockedPrevious);
			assert(::glGetError() == GL_NO_ERROR);
		}
		else
			assert(msFirstLock != NULL);

		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );

		void* ret = ::glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, offset, length, GLESLockFlagsConverter(lockflags) );
		assert( ::glGetError() == GL_NO_ERROR );

		mIsLocked = true;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GLESIndexBuffer::unlock(void)
	{
		assert( mBufferName != 0 );

		if( !mIsLocked )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer not locked.") );

		::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );

		::glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		assert( ::glGetError() == GL_NO_ERROR );

		assert(msFirstLock != NULL);
		if (msFirstLock == this)
		{
			msFirstLock = NULL;
			::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, msLockedPrevious);
			assert(::glGetError() == GL_NO_ERROR);
			msLockedPrevious = 0;
		}
		mIsLocked = false;
	}

	
}//namespace Blade

#endif//BLADE_USE_GLES