/********************************************************************
	created:	2012/08/22
	filename: 	DLMallocPool.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DLMallocPool_h__
#define __Blade_DLMallocPool_h__
#include <interface/IPool.h>
#include <RefCount.h>

//forward declaration
typedef void* mspace;

namespace Blade
{
	class DLMallocPool : public IPool
	{
	public:
		struct SBlockHeader
		{	
			enum EBlockID
			{
				BID_FREE = 0xD100F00D,	//freed block
				//last 4 bits in use
				BID_ALLOC = 0xD10CCED0,	//allocated block

				BID_FORCEWORD = 0xFFFFFFFF,	//size: 32bit
			};
#if BLADE_MEMORY_DEBUG
			SBlockHeader* mNext;
			SBlockHeader* mPrev;
			const char* mFile;
			size_t		mLine;
#endif
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			uint32		mSpaceIndex;
			uint32		mID;
#elif BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_64BIT
			uint64		mSpaceIndex;
			uint32		mGap;
			uint32		mID;	//make it as last. note: using uint64 and removing gap may be not correct due to endian issues.
#else
#error not implemented.
#endif
			inline void* getData() const	{return (char*)this + sizeof(*this);}

#if BLADE_MEMORY_DEBUG
			inline void  linkToHead(SBlockHeader*& head)
			{
				mPrev = NULL;
				mNext = head;
				if( head != NULL )
				{
					assert( head->mPrev == NULL );
					assert(head->mNext != this);
					head->mPrev = this;
				}
				head = this;
			}
			inline void detach(SBlockHeader*& head)
			{
				if( mPrev != NULL )
				{
					assert(head != this);
					assert(mPrev->mNext == this);
					mPrev->mNext = mNext;
				}
				else
				{
					assert(head == this);
					head = mNext;
				}

				if( mNext != NULL )
				{
					assert(mNext->mPrev == this);
					mNext->mPrev = mPrev;
				}
			}
#endif
		};
		static_assert( (sizeof(SBlockHeader)&(BLADE_MEMORY_ALIGNMENT-1)) == 0, "size/alignment error");
	public:

		static const int MAX_CONCURRENT_SPACES = 8;
		static const int DEFAULT_CONCURRENT_SPACES = 2;

		static const int MIN_SPACE_SIZE = 16*1024;			//16M
		static const int DEFAULT_SPACE_SIZE = 64*1024;		//64M

		DLMallocPool();
		~DLMallocPool();

		/************************************************************************/
		/* IPool interface                                                                     */
		/************************************************************************/
		/** @brief kbytes is the allocating IPool size */
		virtual void	initialize(size_t threadCount = DEFAULT_SETTING,size_t unit_size = DEFAULT_SETTING,size_t kbytes = DEFAULT_SETTING);

		/** @brief debug version of allocate */
		virtual void*	allocate(size_t bytes, const char* sourcefile, int lineNO, int type = PAT_DEFAULT);

		/** @brief re-allocate memory */
		virtual void*	reallocate(void* ptr, size_t bytes, const char* sourcefile, int lineNO, int type = PAT_DEFAULT);

		/** @brief fee a block of memory allocated */
		virtual void	deallocate(void* p, int type = PAT_DEFAULT);

		/** @brief free all memory */
		virtual void	release();

		/** @brief profiling */
		virtual bool	getPoolState(SPoolState& outState);
#if BLADE_MEMORY_DEBUG
		/** @brief  */
		void* operator new(size_t bytes, const char*, int, const MEMINFO*)
		{
			return ::malloc(bytes);
		}

		void operator delete(void* ptr, const char*,int, const MEMINFO*)
		{
			return ::free(ptr);
		}
		void operator delete(void* ptr)
		{
			return DLMallocPool::operator delete(ptr,NULL,0,NULL);
		}
#else
		void* operator new(size_t bytes, const MEMINFO*)
		{
			return ::malloc(bytes);
		}

		void operator delete(void* ptr, const MEMINFO*)
		{
			return ::free(ptr);
		}

		void operator delete(void* ptr)
		{
			return DLMallocPool::operator delete(ptr, (const MEMINFO*)NULL);
		}
#endif
		void* operator new(size_t bytes, void* buffer)
		{
			BLADE_UNREFERENCED(bytes);
			return buffer;
		}

		void operator delete(void* ptr, void* buffer)
		{
			BLADE_UNREFERENCED(ptr);
			BLADE_UNREFERENCED(buffer);
		}

	protected:
		RefCount				mAllocCount;
		RefCount				mAllocBytes;
		size_t					mConcurrentSpaceCount;
		Lock					mSpaceLocks[MAX_CONCURRENT_SPACES];
		mspace					mSpaces[MAX_CONCURRENT_SPACES];
#if BLADE_MEMORY_DEBUG
		SBlockHeader*			mUsedBlocks[MAX_CONCURRENT_SPACES];
#endif
	};//class DLMallocPool
	
}//namespace Blade

#endif //  __Blade_DLMallocPool_h__