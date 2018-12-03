/********************************************************************
	created:	2012/08/22
	filename: 	DLMallocPool.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)	//conditional expression is constant
#pragma warning(push)
#pragma warning(disable:4365)	//conversion from int to size_t	(malloc.c.h)
#endif

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	if defined(TEXT)
#		undef TEXT
#	endif
#endif

//NOTE: we're using DLMalloc only, NedMalloc is NOT used.
#define MALLOC_ALIGNMENT BLADE_MEMORY_ALIGNMENT
#define ABORT_ON_ASSERT_FAILURE 0
#define ABORT assert(false)
#define USE_LOCKS 0
#define USE_SPIN_LOCKS 0
#include "nedmalloc/malloc.c.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include "DLMallocPool.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DLMallocPool::DLMallocPool()
		:mAllocCount(0)
		,mAllocBytes(0)
		,mConcurrentSpaceCount(0)
	{
		std::memset(mSpaces, 0, sizeof(mSpaces) );
#if BLADE_MEMORY_DEBUG
		std::memset(mUsedBlocks, 0, sizeof(mUsedBlocks) );
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	DLMallocPool::~DLMallocPool()
	{
		this->DLMallocPool::release();
	}

	/************************************************************************/
	/* IPool interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void	DLMallocPool::initialize(size_t threadCount/* = DEFAULT_SETTING*/,size_t unit_size/* = DEFAULT_SETTING*/,size_t kbytes/* = DEFAULT_SETTING*/)
	{
		BLADE_UNREFERENCED(unit_size);
		if( threadCount == DEFAULT_SETTING )
			threadCount = DEFAULT_CONCURRENT_SPACES;
		else
			threadCount = MAX_CONCURRENT_SPACES > threadCount ? threadCount : MAX_CONCURRENT_SPACES;

		if( kbytes == DEFAULT_SETTING )
			kbytes = DEFAULT_SPACE_SIZE;
		else
			kbytes = kbytes < MIN_SPACE_SIZE ? MIN_SPACE_SIZE : kbytes;

		mConcurrentSpaceCount = threadCount;
		for(size_t i = 0; i < mConcurrentSpaceCount; ++i )
		{
			mSpaces[i] = ::create_mspace(kbytes*1024,0);
			::mspace_mmap_large_chunks(mSpaces[i],0);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void*	DLMallocPool::allocate(size_t bytes, const char* sourcefile, int lineNO, int type/* = PAT_DEFAULT*/)
	{
		BLADE_UNREFERENCED(sourcefile);
		BLADE_UNREFERENCED(lineNO);

		size_t i = 0;
		while( true )
		{
			if( mSpaceLocks[i].tryLock() )
				break;

			if( i == mConcurrentSpaceCount - 1 )
				i = 0;
			else
				++i;
		}

		SBlockHeader* block = (SBlockHeader*)::mspace_malloc(mSpaces[i], bytes+sizeof(SBlockHeader) );
		size_t realSize = ::mspace_usable_size(block) - sizeof(SBlockHeader);
		block->mSpaceIndex = (uint32)i;
		block->mID = (uint32)SBlockHeader::BID_ALLOC | (type&PAT_MASK);
#if BLADE_MEMORY_DEBUG
		block->linkToHead( mUsedBlocks[i] );
		block->mFile = sourcefile;
		block->mLine = (size_t)lineNO;
#endif
		mSpaceLocks[i].unlock();

		mAllocCount.safeIncrement();
		mAllocBytes.safeAdd(realSize);
		void* ptr = block->getData();
		assert( ( (uintptr_t)ptr & (BLADE_MEMORY_ALIGNMENT-1) )  == 0);
		return ptr;
	}

	//////////////////////////////////////////////////////////////////////////
	void*	DLMallocPool::reallocate(void* ptr, size_t bytes, const char* sourcefile, int lineNO, int type/* = PAT_DEFAULT*/)
	{
		if( ptr == NULL )
			return this->allocate(bytes, sourcefile, lineNO, type);

		uint32* data = (uint32*)ptr;
		if( (data[-1]&(~PAT_MASK)) != SBlockHeader::BID_ALLOC )
		{
			assert(false);
			return NULL;
		}
		if( int(data[-1]&PAT_MASK) != (type&PAT_MASK) )
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
		
		SBlockHeader* block = (SBlockHeader*)( (char*)ptr - sizeof(SBlockHeader) );
		size_t index = block->mSpaceIndex;
		assert(index < MAX_CONCURRENT_SPACES );
		mSpaceLocks[index].lock();
		size_t oldBytes = ::mspace_usable_size(block) - sizeof(SBlockHeader);
#if BLADE_MEMORY_DEBUG
		block->detach( mUsedBlocks[index] );
#endif
		SBlockHeader* newblock = (SBlockHeader*)::mspace_realloc(mSpaces[index], block, bytes+sizeof(SBlockHeader));
		size_t realSize = ::mspace_usable_size(block) - sizeof(SBlockHeader);
#if BLADE_MEMORY_DEBUG
		newblock->linkToHead( mUsedBlocks[index] );
		newblock->mFile = sourcefile;
		newblock->mLine = (size_t)lineNO;
#endif
		mSpaceLocks[index].unlock();
		mAllocBytes.safeSubstract(oldBytes);
		mAllocBytes.safeAdd(realSize);
		ptr = newblock->getData();
		assert( ( (uintptr_t)ptr & (BLADE_MEMORY_ALIGNMENT-1) )  == 0);
		return ptr;
	}

	//////////////////////////////////////////////////////////////////////////
	void	DLMallocPool::deallocate(void* p, int type/* = PAT_DEFAULT*/)
	{
		if( p == NULL )
			return;

		uint32* data = (uint32*)p;
		if( (data[-1]&(~PAT_MASK)) != SBlockHeader::BID_ALLOC )
		{
			assert(false);
			return;
		}
		if( int(data[-1]&PAT_MASK) != (type&PAT_MASK) )
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));

		size_t bytes;
		{
			SBlockHeader* block = (SBlockHeader*)( (char*)p - sizeof(SBlockHeader) );
			block->mID = (uint32)SBlockHeader::BID_FREE;
			size_t index = block->mSpaceIndex;
			assert(index < MAX_CONCURRENT_SPACES );
			bytes = ::mspace_usable_size(block) - sizeof(SBlockHeader);
			mSpaceLocks[index].lock();
#if BLADE_MEMORY_DEBUG
			block->detach( mUsedBlocks[index] );
#endif
			::mspace_free( mSpaces[index], block);
			mSpaceLocks[index].unlock();
		}

		mAllocCount.safeDecrement();
		mAllocBytes.safeSubstract(bytes);
	}

	//////////////////////////////////////////////////////////////////////////
	void	DLMallocPool::release()
	{
		mAllocCount.safeSet(0);
		mAllocBytes.safeSet(0);

		for(size_t i = 0; i < mConcurrentSpaceCount; ++i )
		{
			if( mSpaces[i] == NULL )
				continue;

			mSpaceLocks[i].lock();
			//dump leaks
#if BLADE_MEMORY_DEBUG
			if( Memory::isLeakDumpEnabled() )
			{
				SBlockHeader* block = mUsedBlocks[i];
				while( block != NULL )
				{
					const char* file = block->mFile;
					if( file == NULL )
						file = "Unknown";
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
					char buffer[1024];
					sprintf_s(buffer,1024,"%s(%d): %d bytes\n", file, (int)block->mLine, int(::mspace_usable_size(block)-sizeof(SBlockHeader)) );
					::OutputDebugStringA(buffer);
#endif
					block = block->mNext;
				}
			}
#endif
			::destroy_mspace(mSpaces[i]);
			mSpaceLocks[i].unlock();
		}
		std::memset(mSpaces, 0, sizeof(mSpaces) );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	DLMallocPool::getPoolState(SPoolState& outState)
	{
		outState.mAllocatedBytes = mAllocBytes.count();
		outState.mAllocationCount = mAllocCount.count();
		outState.mAllocatedTotal = 0;
		outState.mFreeTotal = 0;
		outState.mFreeBytes = 0;
		for(size_t i = 0; i < mConcurrentSpaceCount; ++i)
		{
			outState.mAllocatedTotal += ::mspace_max_footprint( mSpaces[i] );
			struct mallinfo info = ::mspace_mallinfo(mSpaces[i]);
			outState.mFreeTotal = outState.mFreeBytes = info.fordblks;
		}
		return true;
	}
	
}//namespace Blade