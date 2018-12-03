/********************************************************************
	created:	2014/12/20
	filename: 	GLESIndexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_GLESIndexBuffer_h__
#define __Blade_GLESIndexBuffer_h__
#include "GLESHeader.h"
#if BLADE_USE_GLES
#include <graphics/IndexBuffer.h>

namespace Blade
{
	class GLESIndexBuffer : public IndexBuffer, public Allocatable
	{
	public:
		GLESIndexBuffer(GLuint name, IIndexBuffer::EIndexType type, size_t count, USAGE usage, const void* data);
		~GLESIndexBuffer();

		/************************************************************************/
		/* IGraphicsBuffer interface                                                                    */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual void*		lock(size_t offset,size_t length,LOCKFLAGS lockflags);

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
			assert(msLockedPrevious == 0 && msFirstLock == NULL);	//avoid any locked buffers while drawing, or mLastIndex of GLESRenderDevice will be messed up
			assert(!mIsLocked);
			assert( mBufferName != 0 );
			::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferName);
			bool ret = (::glGetError() == GL_NO_ERROR);
			assert(ret);
			return ret;
		}
		/** @brief  */
		GLenum		getGLIndexType() const	{return mIndexType == IT_32BIT ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;}
		/** @brief  */
		GLuint		getBufferName() const { return mBufferName; }

	protected:
		GLuint		mBufferName;
		static GLuint		msLockedPrevious;
		static const GLESIndexBuffer* msFirstLock;
	};//GLESIndexBuffer
	
}//namespace Blade


#endif//BLADE_USE_GLES


#endif // __Blade_GLESIndexBuffer_h__