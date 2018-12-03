/********************************************************************
	created:	2014/12/20
	filename: 	GLESVertexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESVertexBuffer_h__
#define __Blade_GLESVertexBuffer_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/VertexBuffer.h>

namespace Blade
{
	class GLESVertexBuffer : public VertexBuffer, public Allocatable
	{
	public:
		GLESVertexBuffer(GLuint name, size_t vertexSize,size_t vertexCount, USAGE usage, const void* data);
		~GLESVertexBuffer();

		/************************************************************************/
		/* IGraphicsBuffer interface                                                                    */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*		lock(size_t offset, size_t length, LOCKFLAGS lockflags);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		unlock(void);
		
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		bool		bindBuffer() const
		{
			assert(msLockedPrevious == 0 && msFirstLock == NULL);	//avoid any locked buffers while drawing
			assert(!mIsLocked);
			assert( mBufferName != 0 );
			::glBindBuffer(GL_ARRAY_BUFFER, mBufferName);
			bool ret = (::glGetError() == GL_NO_ERROR);
			assert(ret);
			return ret;
		}

	protected:
		GLuint		mBufferName;
		static GLuint	msLockedPrevious;
		static const GLESVertexBuffer* msFirstLock;
	};
	
}//namespace Blade

#endif//BLADE_USE_GLES
#endif // __Blade_GLESVertexBuffer_h__