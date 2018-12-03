/********************************************************************
	created:	2010/04/09
	filename: 	VertexBuffer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_VertexBuffer_h__
#define __Blade_VertexBuffer_h__
#include <BladeDevice.h>
#include <interface/public/graphics/IVertexBuffer.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable:4275)
#endif

namespace Blade
{
	class BLADE_DEVICE_API VertexBuffer : public IVertexBuffer
	{
	public:
		VertexBuffer();
		VertexBuffer(size_t vertexSize, size_t vertexCount, USAGE usage);
		virtual ~VertexBuffer()		{}

		/************************************************************************/
		/*IGraphicsBuffer interface                                                                      */
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
		virtual void*		lock(const Box3i& box, size_t& outPitch, LOCKFLAGS lockflags);

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
		/* IVertexBuffer interface                                                                    */
		/************************************************************************/
		/*
		@describe get size of one single vertex in bytes
		@param 
		@return 
		*/
		inline virtual size_t	getVertexSize() const
		{
			return mVertexSize;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		inline virtual size_t getVertexCount() const
		{
			return mVertexCount;
		}

	protected:
		size_t			mSizeBytes;
		size_t			mVertexCount;
		size_t			mVertexSize;
		USAGE			mUsage;
		bool			mIsLocked;
	};//class VertexBuffer


	
}//namespace Blade


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	pragma warning(pop)
#endif


#endif //__Blade_VertexBuffer_h__