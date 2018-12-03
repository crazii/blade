/********************************************************************
	created:	2010/04/09
	filename: 	IndexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IndexBuffer_h__
#define __Blade_IndexBuffer_h__
#include <BladeDevice.h>
#include <interface/public/graphics/IIndexBuffer.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4275)
#endif

namespace Blade
{
	class BLADE_DEVICE_API IndexBuffer : public IIndexBuffer
	{
	public:
		IndexBuffer();
		IndexBuffer(size_t indexCount, USAGE usage, EIndexType idextype);
		virtual ~IndexBuffer()	{}


		/************************************************************************/
		/* IGraphicsBuffer interface                                                                     */
		/************************************************************************/
		/**
		@describe
		@param
		@return
		*/
		virtual void*		lock(size_t offset, size_t length, LOCKFLAGS lockflags) = 0;	//redeclaration: makes compiler happy

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
		/* IIndexBuffer interface                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		inline virtual EIndexType	getIndexType() const
		{
			return mIndexType;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		inline virtual size_t		getIndexCount() const
		{
			return mIndexCount;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		inline virtual size_t		getIndexSize() const
		{
			return mIndexType == IT_16BIT ? 2u : 4u;
		}

		/*
		@describe get the whole buffer size in bytes
		@param 
		@return 
		*/
		inline virtual size_t		getSizeInBytes() const
		{
			return mTotalBytes;
		}


	protected:
		size_t		mTotalBytes;;
		size_t		mIndexCount;
		size_t		mLockAt;
		size_t		mLockRange;
		EIndexType	mIndexType;
		USAGE		mUsage;
		bool		mIsLocked;
	};//class IndexBuffer
	
}//namespace Blade

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif

#endif //__Blade_IndexBuffer_h__