/********************************************************************
	created:	2010/04/14
	filename: 	DefaultIndexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_DefaultIndexBuffer_h__
#define __Blade_DefaultIndexBuffer_h__
#include <interface/public/graphics/IIndexBuffer.h>

namespace Blade
{
	class DefaultIndexBuffer : public IIndexBuffer , public Allocatable
	{
	public:
		DefaultIndexBuffer(const void* data, EIndexType type,size_t indexCount,USAGE usage, IPool* pool);
		~DefaultIndexBuffer();

		/************************************************************************/
		/* IGraphicsBuffer interface                                                                     */
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
		/* IIndexBuffer Interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual EIndexType	getIndexType() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual size_t		getIndexCount() const;

		/*
		@describe get size of one single index in bytes
		@param 
		@return 
		*/
		virtual size_t		getIndexSize() const;

		/*
		@describe get the whole buffer size in bytes
		@param 
		@return 
		*/
		virtual size_t		getSizeInBytes() const;

	protected:
		size_t		mSize;
		size_t		mIndexCount;
		void*		mBuffer;
		IPool*		mPool;

		EIndexType	mIndexType;
		USAGE		mUsage;
		bool		mIsLocked;
	};//class DefaultIndexBuffer
	
}//namespace Blade


#endif //__Blade_DefaultIndexBuffer_h__