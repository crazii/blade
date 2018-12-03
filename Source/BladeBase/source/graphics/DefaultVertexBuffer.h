/********************************************************************
	created:	2010/04/14
	filename: 	DefaultVertexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultVertexBuffer_h__
#define __Blade_DefaultVertexBuffer_h__

#include <interface/public/graphics/IVertexBuffer.h>

namespace Blade
{

	class DefaultVertexBuffer : public IVertexBuffer , public Allocatable
	{
	public:
		DefaultVertexBuffer(const void* data, size_t vertexSize,size_t vertexCount,USAGE usage, IPool* pool);
		~DefaultVertexBuffer();

		/************************************************************************/
		/* IGraphicsBuffer Interface                                                                     */
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
		virtual void*		lock(const Box3i& box,size_t& outPitch,LOCKFLAGS lockflags);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void		unlock(void);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual USAGE		getUsage() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		isLocked() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		addDirtyRegion(const Box3i& dirtyBox);

		/************************************************************************/
		/* IVertexBuffer Interface                                                                     */
		/************************************************************************/

		/*
		@describe get size of one single vertex in bytes
		@param 
		@return 
		*/
		virtual size_t	getVertexSize() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t getVertexCount() const;

	protected:
		bool		mIsLocked;
		USAGE		mUsage;
		size_t		mSize;

		size_t		mVertexSize;
		size_t		mVertexCount;
		char*		mBuffer;
		IPool*		mPool;
	};//class DefaultVertexBuffer

	
}//namespace Blade


#endif //__Blade_DefaultVertexBuffer_h__