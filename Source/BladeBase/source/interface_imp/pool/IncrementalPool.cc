/********************************************************************
	created:	2010/01/16
	filename: 	IncrementalPool.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "../../BladeWin32API.h"

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#	define BLADE_ALLOC(size)	VirtualAlloc(NULL,size,MEM_COMMIT|MEM_TOP_DOWN,PAGE_READWRITE)
#	define BLADE_FREE(ptr)		VirtualFree(ptr,0,MEM_RELEASE)

#else

#	define BLADE_ALLOC(size)	malloc(size)
#	define BLADE_FREE(ptr)		free(ptr)

#endif
#include "IncrementalPool.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	IncrementalPool::IncrementalPool()
		:mPageSize(4*1024*1024)
		,mFullPages(NULL)
	{
		std::memset(mConcurrentPage, 0,sizeof(mConcurrentPage));
		std::memset(mCaches, 0,sizeof(mCaches));
		std::memset(&mPoolState,0,sizeof(mPoolState));

		mConcurrentPageCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IncrementalPool::~IncrementalPool()
	{
		this->release();
	}

	//////////////////////////////////////////////////////////////////////////
	//kbytes is the allocating IPool size
	void	IncrementalPool::initialize(size_t threadCount/* = DEFAULT_SETTING*/,size_t unit_size/* = DEFAULT_SETTING*/,size_t kbytes/* = DEFAULT_SETTING*/)
	{
		if( threadCount == DEFAULT_SETTING )
			threadCount = DEFAULT_CONCURRENT_PAGES;
		else
			threadCount = MAX_CONCURRENT_PAGES > threadCount ? threadCount : MAX_CONCURRENT_PAGES;

		mConcurrentPageCount = threadCount;

		BLADE_UNREFERENCED(unit_size);

		//ignore first param
		//if( mMinSize < unit_size )
		//	mMinSize = unit_size;

		kbytes *= 1024;
		if( mPageSize < kbytes )
			mPageSize = kbytes;

		//4K align
		mPageSize = ( (mPageSize + 4*1024 -1)/(4*1024) ) * 4*1024;

		mCurrentPageLock.lock();

		for( size_t i = 0; i < mConcurrentPageCount; ++i)
		{
			mConcurrentPage[i] = this->allocatePage();
			mConcurrentPage[i]->setFlag(IncrementalPage::PF_FREE);
		}
		mCurrentPageLock.unlock();

		mLargestSize = mPageSize/4;
	}

	//////////////////////////////////////////////////////////////////////////
	void*	IncrementalPool::allocate(size_t bytes, const char* sourcefile, int lineNO, int type/* = PAT_DEFAULT*/)
	{
		bytes = (bytes + mMinSize-1)/mMinSize*mMinSize;

		if( bytes > mLargestSize )
		{
			void* ptr = ::BLADE_ALLOC( bytes + BLADE_MEMORY_ALIGNMENT );
			if( ptr == NULL )
				return ptr;

			size_t count = BLADE_MEMORY_ALIGNMENT / sizeof(uint32);
			assert(count >= 2);
			assert(BLADE_MEMORY_ALIGNMENT % sizeof(uint32) == 0);
			uint32* uintPtr = (uint32*)ptr;
			assert(bytes <= std::numeric_limits<uint32>::max());
			uintPtr[count-2] = (uint32)bytes;
			uintPtr[count-1] = (uint32)SIncrementalBlock::BID_EXTERNAL | (type&PAT_MASK);

#if BLADE_MEMORY_DEBUG
			{
				Lock::add(&mPoolState.mAllocatedBytes,bytes);
				Lock::add(&mPoolState.mAllocatedTotal, bytes + BLADE_MEMORY_ALIGNMENT);
				Lock::increment(&mPoolState.mAllocationCount);
			}
#endif
			return &uintPtr[count];
		}
		else
		{
			IncrementalPage* page = NULL;
			size_t i = 0;
			while(page == NULL)
			{
				mCurrentPageLock.lock();
				for(i = 0; i < mConcurrentPageCount;++i)
				{
					if (mConcurrentPage[i]->tryLockPage())
					{
						page = mConcurrentPage[i];
						break;
					}
				}
				mCurrentPageLock.unlock();
			}
			assert( page->getPrevLink() == NULL && page->getNextLink() == NULL && page->getFlag() != IncrementalPage::PF_FULL );

			void* ptr = page->allocate(bytes, type, sourcefile, lineNO);
			assert( ( (uintptr_t)ptr & (BLADE_MEMORY_ALIGNMENT-1) )  == 0);

			//not enough memory in this page
			if( ptr == NULL || page->isFull() )
			{
				IncrementalPage* newPage = this->allocatePage();
				newPage->setFlag(IncrementalPage::PF_FREE);
				newPage->lockPage();

				if( ptr == NULL )
				{
					ptr = newPage->allocate(bytes,type, sourcefile, lineNO);
					if( !page->isFull() && newPage->getLeftBytes() < page->getLeftBytes() )
						std::swap(page,newPage);
					else
					{
						mCurrentPageLock.lock();
						mConcurrentPage[i] = newPage;
						mCurrentPageLock.unlock();
					}
				}
				else
				{
					mCurrentPageLock.lock();
					mConcurrentPage[i] = newPage;
					mCurrentPageLock.unlock();
				}

				mFullPageLock.lock();
				page->setNextLink(mFullPages);
				mFullPages = page;
				page->setFlag(IncrementalPage::PF_FULL);
				mFullPageLock.unlock();

				newPage->unlockPage();
			}
			page->unlockPage();

#if BLADE_MEMORY_DEBUG
			if( ptr != NULL )
			{
				size_t totalBytes = bytes + sizeof(SIncrementalBlock);
				Lock::add(&mPoolState.mAllocatedBytes,bytes);
				Lock::add(&mPoolState.mAllocatedTotal,totalBytes);
				Lock::sub(&mPoolState.mFreeBytes,bytes);
				Lock::sub(&mPoolState.mFreeTotal,totalBytes);
				Lock::increment(&mPoolState.mAllocationCount);
			}
#endif

			return ptr;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void*	IncrementalPool::reallocate(void* ptr, size_t bytes, const char* sourcefile, int lineNO, int type/* = PAT_DEFAULT*/)
	{
		if( ptr == NULL )
			return this->allocate(bytes, sourcefile, lineNO, type);

		uint32* u32Ptr = (uint32*)ptr;
		if( (u32Ptr[-1]&(~PAT_MASK)) == SIncrementalBlock::BID_EXTERNAL )
		{
			if(int(u32Ptr[-1]&PAT_MASK) != (type&PAT_MASK))
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
			size_t size = u32Ptr[-2];
			assert( size <= mPoolState.mAllocatedBytes && mPoolState.mAllocationCount > 0);
#if BLADE_MEMORY_DEBUG
			Lock::sub(&mPoolState.mAllocatedBytes,size);
			Lock::sub(&mPoolState.mAllocatedTotal, size + BLADE_MEMORY_ALIGNMENT );
			Lock::decrement(&mPoolState.mAllocationCount);
#endif

			void* newPtr= this->allocate(bytes, sourcefile, lineNO, type);

			size_t minSize = std::min<size_t>(size, bytes);
			std::memcpy(newPtr, ptr, minSize);
			int off = -int(BLADE_MEMORY_ALIGNMENT / sizeof(uint32));
			::BLADE_FREE( &u32Ptr[off] );
			return newPtr;
		}
		else
		{
			SIncrementalBlock* block = SIncrementalBlock::getBlockfromData(ptr);
			//type verification
			if(int(block->mIdentifier&IPool::PAT_MASK) != (type&IPool::PAT_MASK) )
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));

			IncrementalPage* page = block->getPage();
			BLADE_UNREFERENCED(page);
			assert( page->isValid() );
			size_t size = block->getNumBytes();
			if (bytes <= size)
			{
#if BLADE_MEMORY_DEBUG
				block->checkOverflow();
				block->setAllocatedBytes(bytes);
#endif
				return ptr;
			}

			void* newPtr= this->allocate(bytes, sourcefile, lineNO, type);
			size_t minSize = std::min<size_t>(size, bytes);
			std::memcpy(newPtr, ptr, minSize);
			this->deallocate(ptr, type);
			return newPtr;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//free a block of memory allocated
	void	IncrementalPool::deallocate(void* ptr, int type/* = PAT_DEFAULT*/)
	{
		if( ptr == NULL )
			return;

		uint32* u32Ptr = (uint32*)ptr;
		if( (u32Ptr[-1]&(~PAT_MASK)) == SIncrementalBlock::BID_EXTERNAL )
		{
			if(int(u32Ptr[-1]&PAT_MASK) != (type&PAT_MASK))
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
#if BLADE_MEMORY_DEBUG
			size_t size = u32Ptr[-2];

			assert( size <= mPoolState.mAllocatedBytes && mPoolState.mAllocationCount > 0);
			Lock::sub(&mPoolState.mAllocatedBytes,size);
			Lock::sub(&mPoolState.mAllocatedTotal, size + BLADE_MEMORY_ALIGNMENT );
			Lock::decrement(&mPoolState.mAllocationCount);
#endif
			int off = -int(BLADE_MEMORY_ALIGNMENT / sizeof(uint32));
			::BLADE_FREE( &u32Ptr[off] );
		}
		else
		{
			SIncrementalBlock* block = SIncrementalBlock::getBlockfromData(ptr);
			//type verification
			if(int(block->mIdentifier&IPool::PAT_MASK) != (type&IPool::PAT_MASK) )
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));

			IncrementalPage* page = block->getPage();
			assert( page->isValid() );
			page->lockPage();
			page->deallocate(block);

			{
#if BLADE_MEMORY_DEBUG
				size_t size = block->getAllocatedBytes();
				if (size > mPoolState.mAllocatedBytes || mPoolState.mAllocationCount == 0)
					assert(false);
				size_t totalBytes = size + sizeof(SIncrementalBlock);
				Lock::sub(&mPoolState.mAllocatedBytes,size);
				Lock::sub(&mPoolState.mAllocatedTotal,totalBytes);
				Lock::add(&mPoolState.mFreeBytes,size);
				Lock::add(&mPoolState.mFreeTotal,totalBytes);
				Lock::decrement(&mPoolState.mAllocationCount);
#endif
			}

			if( page->isEmpty() && page->getFlag() == IncrementalPage::PF_FULL )
			{
				mFullPageLock.lock();
				page->detachFromList(mFullPages);
				mFullPageLock.unlock();

				mCacheLock.lock();
				for( int i = 0; i < NUM_CACHED_PAGES; ++i)
				{
					if( mCaches[i] == NULL )
					{
						mCaches[i] = page;
						page->setFlag(IncrementalPage::PF_EMPTY);
						page->unlockPage();
						page = NULL;
						break;
					}
				}
				mCacheLock.unlock();
				if( page != NULL )
				{
					this->freePage(page);
					page = NULL;
				}
			}

			if( page != NULL )
				page->unlockPage();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//free all memory
	void	IncrementalPool::release()
	{
		mFullPageLock.lock();
		while( mFullPages != NULL )
		{
			IncrementalPage* page = mFullPages;
			mFullPages = mFullPages->getNextLink();

			page->dumpLeaks();
			this->freePage(page);
		}
		mFullPages = NULL;
		mFullPageLock.unlock();

		mCurrentPageLock.lock();
		for( size_t i = 0; i < mConcurrentPageCount; ++i)
		{
			if( mConcurrentPage[i] != NULL )
			{
				mConcurrentPage[i]->dumpLeaks();
				this->freePage( mConcurrentPage[i] );
				mConcurrentPage[i] = NULL;
			}
		}
		mCurrentPageLock.unlock();

		mCacheLock.lock();
		for( int i = 0; i < NUM_CACHED_PAGES; ++i)
		{
			if( mCaches[i] != NULL )
			{
				this->freePage(mCaches[i]);
				mCaches[i] = NULL;
			}
		}
		mCacheLock.unlock();

#if BLADE_MEMORY_DEBUG
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		if( mPoolState.mAllocatedBytes > 0 )
		{
			char buffer[1024];
			//note: 'zd/zd' is C++ 11 only
			::sprintf_s(buffer,1024,"Total leak: %u bytes , Total leak count: %u.\n",(unsigned)mPoolState.mAllocatedBytes, (unsigned)mPoolState.mAllocationCount);
			::OutputDebugStringA(buffer);
		}
#endif
#endif

	}

	//////////////////////////////////////////////////////////////////////////
	//allocate one page
	IncrementalPage*	IncrementalPool::allocatePage()
	{
		IncrementalPage* page = NULL;
		bool isOK = false;

		//first try to use the caches
		mCacheLock.lock();
		for( int i =0; i < NUM_CACHED_PAGES; ++i )
		{
			if( mCaches[i] != NULL )
			{
				page = mCaches[i];
				mCaches[i] = NULL;
				break;
			}
		}
		mCacheLock.unlock();
		if( page != NULL )
			return page;

		//second,allocate a empty page slot to use
		{
			//void* buffer = ::malloc(mPageSize);
			void* buffer = BLADE_ALLOC(mPageSize);
			page = new (buffer) IncrementalPage();
			isOK = page->initialize(mPageSize);

#if BLADE_DEBUG
			Lock::add(&mPoolState.mFreeTotal,mPageSize);
			Lock::add(&mPoolState.mFreeBytes,mPageSize);
#endif
		}
		if( !isOK )
		{
			throw std::bad_alloc();
		}
		return page;
	}

	//////////////////////////////////////////////////////////////////////////
	void				IncrementalPool::freePage(IncrementalPage* page)
	{
		assert( page != NULL );
		{
			BLADE_FREE(page);
			assert( mPoolState.mAllocatedTotal+mPoolState.mFreeTotal >= mPageSize );
			Lock::sub(&mPoolState.mFreeTotal,mPageSize);
			Lock::sub(&mPoolState.mFreeBytes,mPageSize);
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	bool	IncrementalPool::getPoolState(SPoolState& outState)
	{
		outState = mPoolState;
		return true;
	}

}	//namespace Blade