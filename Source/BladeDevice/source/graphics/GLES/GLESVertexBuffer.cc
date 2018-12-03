/********************************************************************
	created:	2014/12/20
	filename: 	GLESVertexBuffer.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <graphics/GLES/GLESHeader.h>
#if BLADE_USE_GLES
#include <graphics/GLES/GLESUtil.h>
#include <graphics/GLES/GLESVertexBuffer.h>

namespace Blade
{
	GLuint GLESVertexBuffer::msLockedPrevious = 0;
	const GLESVertexBuffer* GLESVertexBuffer::msFirstLock = NULL;

	//////////////////////////////////////////////////////////////////////////
	GLESVertexBuffer::GLESVertexBuffer(GLuint name, size_t vertexSize,size_t vertexCount, USAGE usage, const void* data)
		:VertexBuffer(vertexSize, vertexCount, usage)
		,mBufferName(name)
	{
		GLint old = 0;
		::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old);
		assert(::glGetError() == GL_NO_ERROR);

		::glBindBuffer(GL_ARRAY_BUFFER, mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );
		::glBufferData(GL_ARRAY_BUFFER, mSizeBytes, data, GLESBufferUsageConverter(usage) );
		assert( ::glGetError() == GL_NO_ERROR );
		
		::glBindBuffer(GL_ARRAY_BUFFER, (GLuint)old);
		assert( ::glGetError() == GL_NO_ERROR );
	}

	//////////////////////////////////////////////////////////////////////////
	GLESVertexBuffer::~GLESVertexBuffer()
	{
		GLint old = 0;
		::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old);
		assert(::glGetError() == GL_NO_ERROR);

		assert( !mIsLocked );
		::glBindBuffer(GL_ARRAY_BUFFER, 0);
		assert( ::glGetError() == GL_NO_ERROR );

		::glDeleteBuffers(1, &mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );
		mBufferName = 0;

		::glBindBuffer(GL_ARRAY_BUFFER, (GLuint)old);
		assert(::glGetError() == GL_NO_ERROR);
	}

	/************************************************************************/
	/* IGraphicsBuffer interface                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void*		GLESVertexBuffer::lock(size_t offset, size_t length, LOCKFLAGS lockflags)
	{
		assert( mBufferName != 0 );

		if( mIsLocked )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer already locked.") );

		if( (!mUsage.isReadable() && (lockflags&GBLF_READONLY) != 0)
			|| (!mUsage.isWriteable() && (lockflags&GBLF_WRITEONLY) != 0) )
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("invalid lock operation for buffer access.") );

		if( offset <= mSizeBytes )
		{
			if( length > mSizeBytes - offset )//no overflow check
				length = mSizeBytes - offset;
		}
		else
			BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("lock data of out range.") );

		if (msLockedPrevious == 0)
		{
			assert(msFirstLock == NULL);
			msFirstLock = this;
			::glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&msLockedPrevious);
			assert(::glGetError() == GL_NO_ERROR);
		}
		else
			assert(msFirstLock != NULL);

		::glBindBuffer(GL_ARRAY_BUFFER, mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );

		void* ret = ::glMapBufferRange(GL_ARRAY_BUFFER, offset, length, GLESLockFlagsConverter(lockflags) );
		assert( ::glGetError() == GL_NO_ERROR );

		mIsLocked = true;
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GLESVertexBuffer::unlock(void)
	{
		assert( mBufferName != 0 );

		if( !mIsLocked )
			BLADE_EXCEPT(EXC_REDUPLICATE,BTString("buffer not locked.") );

		::glBindBuffer(GL_ARRAY_BUFFER, mBufferName);
		assert( ::glGetError() == GL_NO_ERROR );

		::glUnmapBuffer(GL_ARRAY_BUFFER);
		assert( ::glGetError() == GL_NO_ERROR );

		assert(msFirstLock != NULL);
		if (msFirstLock == this)
		{
			msFirstLock = NULL;
			::glBindBuffer(GL_ARRAY_BUFFER, msLockedPrevious);
			assert(::glGetError() == GL_NO_ERROR);
			msLockedPrevious = 0;
		}
		mIsLocked = false;
	}
	
}//namespace Blade

#endif//BLADE_USE_GLES