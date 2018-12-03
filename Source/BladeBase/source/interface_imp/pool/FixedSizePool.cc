/********************************************************************
	created:	2009/02/13
	filename: 	FixedSizePool.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "FixedSizePageAllocator.h"	//this header must be the first one when compiled in Windows
#include "FixedSizePool.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)		//while(true)
#endif

namespace Blade
{
	namespace Internal
	{
#if ENABLE_FIXEDSIZE_PAGE_ALLOC
		extern FixedSizePageAllocator lFixedSizePageAlloc;
#endif
	}
	using namespace Internal;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	FixedSizePool::FixedSizePool()
		:mUnitSize(4)
		,mFullPages(NULL)
		,mPageCount(0)
#if BLADE_MEMORY_DEBUG
		,mMaxAllocationCount(0)
#endif
	{
		std::memset( mConcurrentPage, 0, sizeof(mConcurrentPage) );
		std::memset( mCaches, 0,sizeof(mCaches) );
		std::memset(&mPoolState,0,sizeof(mPoolState));
		mConcurrentPageCount = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	FixedSizePool::~FixedSizePool()
	{
		this->release();
	}

	//////////////////////////////////////////////////////////////////////////
	void FixedSizePool::initialize(size_t threadCount/* = DEFAULT_SETTING*/,size_t unit_size/* = DEFAULT_SETTING*/, size_t kbytes/* = DEFAULT_SETTING*/)
	{
		if( threadCount == DEFAULT_SETTING )
			threadCount = DEFAULT_CONCURRENT_PAGES;
		else
			threadCount = MAX_CONCURRENT_PAGES > threadCount ? threadCount : MAX_CONCURRENT_PAGES;

		mConcurrentPageCount = threadCount;

		if( unit_size != DEFAULT_SETTING )
			mUnitSize = unit_size;

		if( mUnitSize < sizeof(void*) )
			mUnitSize = sizeof(void*);

		mUnitSize = ((mUnitSize + sizeof(void*) -1)/sizeof(void*)) * sizeof(void*);

		if( kbytes != DEFAULT_SETTING )
			kbytes *= 1024;

		mCurrentPageLock.lock();
		mFreePageLock.lock();

		for( size_t i = 0; i < threadCount; ++i)
		{
			mConcurrentPage[i] = this->allocatePage();
			mConcurrentPage[i]->setFlag(FixedSizePage::PF_CURRENT);
			if( i == 0)
				mFreePageHead = mFreePageTail = mConcurrentPage[i];
			else
				mConcurrentPage[i]->linkToBack(mFreePageTail);
		}
		mFreePageLock.unlock();
		mCurrentPageLock.unlock();

		mLargestSize = mConcurrentPage[0]->getMaxAllocationBytes();
	}



	//////////////////////////////////////////////////////////////////////////
	void*	FixedSizePool::allocate(size_t bytes, const char* sourcefile, int lineNO, int type/* = PAT_DEFAULT*/)
	{
		//allocation is too large,throw it to the OS/CRT
		if( bytes > mLargestSize)
		{
			uint32* ptr = (uint32*)::malloc(bytes + BLADE_MEMORY_ALIGNMENT);
			size_t count = BLADE_MEMORY_ALIGNMENT / sizeof(uint32);
			assert(count >= 2);
			assert(bytes <= std::numeric_limits<uint32>::max());
			assert(BLADE_MEMORY_ALIGNMENT % sizeof(uint32) == 0);

			ptr[count - 2] = (uint32)bytes;
			ptr[count - 1] = (uint32)SFixedSizeBlock::BID_EXTERNAL|(type&PAT_MASK);

#if BLADE_MEMORY_DEBUG
			size_t totalbytes = bytes+ BLADE_MEMORY_ALIGNMENT;
			Lock::add(&mPoolState.mAllocatedBytes,bytes);
			Lock::add(&mPoolState.mAllocatedTotal, totalbytes);
			Lock::increment(&mPoolState.mAllocationCount);
			if( mMaxAllocationCount < mPoolState.mAllocationCount )
				mMaxAllocationCount = mPoolState.mAllocationCount;
#endif
			return &ptr[count];
		}

		FixedSizePage* page = NULL;
		index_t concurrentIndex = INVALID_INDEX;

		//allocate the right unit size , just find a non-full page.
		if( bytes <= mUnitSize )
		{
			bytes = mUnitSize;
			mFreePageLock.lock();
			//allocate from all free pages
			page = mFreePageHead;
			while( true )
			{
				if( page->tryLockPage() )
					break;

				if( page == mFreePageTail )
					page = mFreePageHead;
				else
					page = page->getNextLink();
			}
			mFreePageLock.unlock();

			if( page->getFlag() == FixedSizePage::PF_CURRENT )
			{
#if BLADE_MEMORY_DEBUG
				bool found = false;
#endif
				ScopedLock lock(mCurrentPageLock);
				for( size_t i = 0; i < mConcurrentPageCount; ++i)
				{
					if( page == mConcurrentPage[i] )
					{
						concurrentIndex = i;
#if BLADE_MEMORY_DEBUG
						found = true;
#endif
						break;
					}
				}
#if BLADE_MEMORY_DEBUG
				assert(found);
#endif
			}
		}
		else
			//allocate size larger than unit size, possibly an array
		{
			size_t i = 0;
			while(page == NULL)
			{
				//release the lock on each full pass to avoid dead lock
				mCurrentPageLock.lock();
				for(i = 0; i < mConcurrentPageCount; ++i)
				{
					//fetch a page
					page = mConcurrentPage[i];

					//lock the page for allocation
					if( page->tryLockPage() )
						break;
					else
						page = NULL;
				}//for

				mCurrentPageLock.unlock();
			}		
			concurrentIndex = i;

			assert( page->getFlag() == FixedSizePage::PF_CURRENT );
		}

		assert( page->getFlag() == FixedSizePage::PF_FREE || page->getFlag() == FixedSizePage::PF_CURRENT );
		void* ptr = page->allocate(bytes, type, sourcefile, lineNO);
		assert( ( (uintptr_t)ptr & (BLADE_MEMORY_ALIGNMENT-1) )  == 0);

		if(ptr == NULL )
		{
			//unit size allocation never fail
			assert( bytes > mUnitSize && page->getFlag() == FixedSizePage::PF_CURRENT && concurrentIndex != INVALID_INDEX);
			//find the page suitable for this size
			FixedSizePage* rightPage = NULL;
			{
				ScopedLock lock(mFreePageLock);
				FixedSizePage* freePage = mFreePageHead;
				while(freePage != NULL )
				{
					if( freePage != page && freePage->tryLockPage() )
					{
						assert( freePage->hasFreeSpace() );

						if( freePage->getFreeBytes() >= bytes )					
							break;

						freePage->unlockPage();
					}
					freePage = freePage->getNextLink();
				}
				if( freePage != NULL )
					rightPage = freePage;
			}
			//not found, add new page
			if( rightPage == NULL)
			{
				rightPage = this->allocatePage();
				assert( !rightPage->isLocked() );
				rightPage->lockPage();
				rightPage->setFlag(FixedSizePage::PF_CURRENT);
				{
					ScopedLock lock(mFreePageLock);
					rightPage->linkToBack(mFreePageTail);
				}

				ScopedLock lock(mCurrentPageLock);
				assert(mConcurrentPage[concurrentIndex] == page);
				page->setFlag(FixedSizePage::PF_FREE);
				mConcurrentPage[concurrentIndex] = rightPage;
			}
			page->unlockPage();
			ptr = rightPage->allocate(bytes, type, sourcefile, lineNO);
			assert( ( (uintptr_t)ptr & (BLADE_MEMORY_ALIGNMENT-1) )  == 0);
			assert(ptr != NULL);
			page = rightPage;

			if( rightPage->getFlag() == FixedSizePage::PF_CURRENT && !rightPage->hasFreeSpace() )
			{
#if BLADE_MEMORY_DEBUG
				bool found = false;
#endif
				ScopedLock lock(mCurrentPageLock);
				for( size_t i = 0; i < mConcurrentPageCount; ++i)
				{
					if( rightPage == mConcurrentPage[i] )
					{
						concurrentIndex = i;
#if BLADE_MEMORY_DEBUG
						found = true;
#endif
					}
				}
#if BLADE_MEMORY_DEBUG
				assert(found);
#endif
			}
		}

		//page is full,move it to the full page list
		if( !page->hasFreeSpace() )
		{
			//remove from free list
			mFreePageLock.lock();
			if( page->getNextLink() == NULL )
			{
				assert( page == mFreePageTail );
				mFreePageTail = page->getPrevLink();
			}
			page->detachFromList(mFreePageHead);
			if( mFreePageHead == NULL )
			{
				assert(mFreePageTail == NULL);
				mFreePageHead = mFreePageTail = this->allocatePage();
				mFreePageHead->setFlag(FixedSizePage::PF_FREE);
			}
			//check whether it is in the current concurrent pages
			if( page->getFlag() == FixedSizePage::PF_CURRENT )
			{
				assert(concurrentIndex != INVALID_INDEX);
				//find the largest page and replace it with concurrent page
				FixedSizePage* largestPage = mFreePageHead;
				{
					size_t ConcurrentCount = 0;
					size_t spin = 0;
					const size_t SPIN_COUNT = 0x200;
					while( true )
					{
						if( largestPage->tryLockPage() )
						{
							if( largestPage->getFlag() == FixedSizePage::PF_CURRENT )//this flag should be checked when page is locked
							{
								++ConcurrentCount;
								largestPage->unlockPage();
							}
							else
								break;
						}

						if( largestPage == mFreePageTail )
						{
							//all pages in free list are concurrent, we need a new page
							if( ConcurrentCount == mConcurrentPageCount-1 )
							{
								largestPage = NULL;
								break;
							}
							else
							{
								ConcurrentCount = 0;
								largestPage = mFreePageHead;

								//sometimes there are more task/threads running than mConcurrentPageCount and 
								//all concurrent pages may be locked waiting free page lock
								//so we may not get a free page in the concurrent list
								//and we need spin a while to test this situation
								if( ++spin > SPIN_COUNT )
								{
									largestPage = NULL;
									break;
								}
							}
						}
						else
							largestPage = largestPage->getNextLink();
					}
					if( largestPage == NULL )
					{
						largestPage = this->allocatePage();
						largestPage->lockPage();
						largestPage->linkToBack(mFreePageTail);
						largestPage->setFlag(FixedSizePage::PF_FREE);
					}
					else
					{
						FixedSizePage* freePage = mFreePageHead;
						while(freePage != NULL )
						{
							assert(freePage != page);
							if(freePage != largestPage && freePage->tryLockPage() )
							{
								assert( freePage->hasFreeSpace() );
								if( freePage->getFlag() != FixedSizePage::PF_CURRENT && freePage->getFreeBytes() > largestPage->getFreeBytes() )
								{
									largestPage->unlockPage();
									largestPage = freePage;
								}
								else
									freePage->unlockPage();
							}
							freePage = freePage->getNextLink();
						}
					}
				}
				mFreePageLock.unlock();
				assert(largestPage->getFlag() != FixedSizePage::PF_CURRENT );

				mCurrentPageLock.lock();
				assert(mConcurrentPage[concurrentIndex] == page);
				largestPage->setFlag(FixedSizePage::PF_CURRENT);
				mConcurrentPage[concurrentIndex] = largestPage;
				largestPage->unlockPage();
				mCurrentPageLock.unlock();
			}
			else
				mFreePageLock.unlock();

			//add to full page list
			mFullPageLock.lock();
			page->setNextLink( mFullPages );
			mFullPages = page;
			page->setFlag( FixedSizePage::PF_FULL);
			mFullPageLock.unlock();
		}//!hasFreeSpace
		page->unlockPage();

		if( ptr != NULL )
		{
#if BLADE_MEMORY_DEBUG
			size_t totalBytes = SFixedSizeBlock::getBlockfromData(ptr)->getNumBytes()+FixedSizePage::BLOCK_SIZE;
			Lock::add(&mPoolState.mAllocatedBytes,bytes);
			Lock::add(&mPoolState.mAllocatedTotal, totalBytes);
			Lock::sub(&mPoolState.mFreeBytes, bytes);
			Lock::sub(&mPoolState.mFreeTotal, totalBytes);
			Lock::increment(&mPoolState.mAllocationCount);

			if( mMaxAllocationCount < mPoolState.mAllocationCount )
				mMaxAllocationCount = mPoolState.mAllocationCount;
#endif
		}
		return ptr;
	}

	//////////////////////////////////////////////////////////////////////////
	void*	FixedSizePool::reallocate(void* ptr, size_t bytes, const char* sourcefile, int lineNO, int type/* = PAT_DEFAULT*/)
	{
		if( ptr == NULL )
			return this->allocate(bytes, sourcefile, lineNO, type);

		uint32* u32ptr = (uint32*)ptr;
		if( (u32ptr[-1]&(~PAT_MASK)) == SFixedSizeBlock::BID_EXTERNAL )
		{
			if( int(u32ptr[-1]&PAT_MASK) != (type&PAT_MASK) )
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
			uint32 size = u32ptr[-2];
			assert(mPoolState.mAllocatedBytes >= size);
#if BLADE_MEMORY_DEBUG
			Lock::sub(&mPoolState.mAllocatedBytes,size);
			Lock::sub(&mPoolState.mAllocatedTotal, size+sizeof(uint32)*2 );
			Lock::decrement(&mPoolState.mAllocationCount);
#endif
			void* newPtr = this->allocate(bytes, sourcefile, lineNO, type);
			size_t minSize = std::min<size_t>(size, bytes);
			::memcpy(newPtr, ptr, minSize);
			int off = -int(BLADE_MEMORY_ALIGNMENT / sizeof(uint32));
			::free(&u32ptr[off]);
			return newPtr;
		}
		else
		{
			SFixedSizeBlock* block = SFixedSizeBlock::getBlockfromData(ptr);
			if((type&IPool::PAT_MASK) != int(block->mIdentifier&IPool::PAT_MASK))
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
			FixedSizePage* page = block->getPage();
			BLADE_UNREFERENCED(page);
			assert( page->isValid() );
			assert( page->getFlag() != FixedSizePage::PF_UNKNOWN );
			size_t size = block->getNumBytes();
			assert(size >= mUnitSize);
			if( bytes <= size )
			{
#if BLADE_MEMORY_DEBUG
				block->mFileInfo = sourcefile;
				block->mLineNO = (FixedSizePage::sizetype)lineNO;
				block->mRequestedBytes = (FixedSizePage::sizetype)bytes;
#endif
				return ptr;
			}

			void* newPtr = this->allocate(bytes, sourcefile, lineNO, type);
			size_t minSize = std::min<size_t>(size, bytes);
			::memcpy(newPtr, ptr, minSize);
			this->deallocate(ptr, type);
			return newPtr;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	void FixedSizePool::deallocate(void* ptr, int type/* = PAT_DEFAULT*/)
	{
		if( ptr == NULL )
			return;

		uint32* u32ptr = (uint32*)ptr;
		if( (u32ptr[-1]&(~PAT_MASK)) == SFixedSizeBlock::BID_EXTERNAL )
		{
			if( int(u32ptr[-1]&PAT_MASK) != (type&PAT_MASK) )
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
#if BLADE_MEMORY_DEBUG
			uint32 size = u32ptr[-2];
			assert( mPoolState.mAllocatedBytes >= size );
			Lock::sub(&mPoolState.mAllocatedBytes,size);
			Lock::sub(&mPoolState.mAllocatedTotal, size+sizeof(uint32)*2 );
			Lock::decrement(&mPoolState.mAllocationCount);
#endif
			int off = -int(BLADE_MEMORY_ALIGNMENT / sizeof(uint32));
			return ::free(&u32ptr[off]);
		}
		else
		{
			SFixedSizeBlock* block = SFixedSizeBlock::getBlockfromData(ptr);
			if((type&IPool::PAT_MASK) != int(block->mIdentifier&IPool::PAT_MASK))
				BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("new delete/new[] delete[] /default allocation mismatch."));
			FixedSizePage* page = block->getPage();

			assert( page->isValid() );
			assert( page->getFlag() != FixedSizePage::PF_UNKNOWN );

			page->lockPage();
			page->deallocate(block);

#if BLADE_MEMORY_DEBUG
			{
				size_t size = block->getAllocatedBytes();
				size_t totalbytes = block->getNumBytes()+FixedSizePage::BLOCK_SIZE;
				Lock::sub(&mPoolState.mAllocatedBytes,size);
				Lock::sub(&mPoolState.mAllocatedTotal, totalbytes );
				Lock::add(&mPoolState.mFreeBytes, size);
				Lock::add(&mPoolState.mFreeTotal, totalbytes);
				Lock::decrement(&mPoolState.mAllocationCount);
			}
#endif

			// a full page just freed a block, link it to the head of free list
			if( page->getFlag() == FixedSizePage::PF_FULL && page->hasFreeSpace() )
			{
				mFullPageLock.lock();
				page->detachFromList(mFullPages);
				mFullPageLock.unlock();

				page->setFlag(FixedSizePage::PF_FREE );

				mFreePageLock.lock();
				page->setNextLink( mFreePageHead );
				mFreePageHead = page;
				mFreePageLock.unlock();
			}

			//the page is totally empty
			if( page->getFlag() == FixedSizePage::PF_FREE && page->isEmpty() )
			{
				//the page is already free type ,not current type
				//so there must be more pages than concurrent pages
				assert( mPageCount > mConcurrentPageCount );

				mFreePageLock.lock();
				if( page->getNextLink() == NULL )
				{
					assert( page == mFreePageTail );
					mFreePageTail = page->getPrevLink();
				}
				page->detachFromList( mFreePageHead );
				mFreePageLock.unlock();

				//put it into caches list,if caches are not full.
				mCacheLock.lock();
				for( int i = 0; i < NUM_CACHED_PAGES; ++i)
				{
					if( mCaches[i] == NULL )
					{
						mCaches[i] = page;
						page->setFlag( FixedSizePage::PF_EMPTY );
						page->unlockPage();
						page = NULL;
						break;
					}
				}
				mCacheLock.unlock();

				//caches full,free the page
				if( page != NULL )
				{
					this->freePage(page);
					page = NULL;
				}
			}

			//make the concurrent pages always the largest
			if( page != NULL && page->getFlag() == FixedSizePage::PF_FREE )
			{
				mCurrentPageLock.lock();

				//PF_FREE is not in mConcurrentPage,just add debug check
#if BLADE_MEMORY_DEBUG
				if( mConcurrentPageCount > 1)
				{
					bool already_concurrent = false;
					for( size_t i = 0; i < mConcurrentPageCount ; ++i )
					{
						if( page == mConcurrentPage[i] )
						{
							already_concurrent = true;
							break;
						}
					}

					assert( !already_concurrent );
				}
#endif

				for( size_t i = 0; i < mConcurrentPageCount ; ++i )
				{
					//note: no busy waiting for this page,
					//because this page may be locked by allocation routine, and of more importance,
					//the allocation routine which locked this page may be waiting for the mCurrentPageLock
					//so there may be a dead lock if we wait here
					if( mConcurrentPage[i]->tryLockPage() )
					{
						if( mConcurrentPage[i]->getFreeBytes() < page->getFreeBytes() )
						{
							page->setFlag(FixedSizePage::PF_CURRENT);
							mConcurrentPage[i]->setFlag(FixedSizePage::PF_FREE);
							std::swap( mConcurrentPage[i],page);
						}
						mConcurrentPage[i]->unlockPage();
					}
				}

				mCurrentPageLock.unlock();
			}
			
			if( page != NULL )
				page->unlockPage();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void FixedSizePool::release()
	{
		//mPoolLock.lock();
		//mEmptyPageLock.lock();

		mFreePageLock.lock();
		mCurrentPageLock.lock();
		while( mFreePageHead != NULL )
		{
			FixedSizePage* page = mFreePageHead;
			mFreePageHead = mFreePageHead->getNextLink();
			page->lockPage();
			page->dumpLeaks();
			this->freePage(page);
		}
		mFreePageTail = NULL;

		for( size_t i = 0; i < mConcurrentPageCount; ++i)
		{
			mConcurrentPage[i] = NULL;
		}
		mCurrentPageLock.unlock();
		mFreePageLock.unlock();

		mFullPageLock.lock();
		while( mFullPages != NULL )
		{
			FixedSizePage* page = mFullPages;
			mFullPages = mFullPages->getNextLink();
			page->lockPage();
			page->dumpLeaks();
			this->freePage(page);
		}
		mFullPageLock.unlock();

		mCacheLock.lock();
		for( int i = 0; i < NUM_CACHED_PAGES; ++i)
		{
			if( mCaches[i] != NULL )
			{
				this->freePage( mCaches[i] );
				mCaches[i] = NULL;
			}
		}
		mCacheLock.unlock();


#if BLADE_MEMORY_DEBUG
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		if( mPoolState.mAllocatedBytes > 0 )
		{
			char buffer[1024];
			//VC'pritnf doesn't have 'zd' for size_t, but Id
			sprintf_s(buffer,1024,"Total leak: %u bytes , Total leak count: %u.\n",(unsigned)mPoolState.mAllocatedBytes, (unsigned)mPoolState.mAllocationCount);
			::OutputDebugStringA(buffer);
		}
#endif
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FixedSizePool::getPoolState(SPoolState& outState)
	{
		outState = mPoolState;
		return true;
	}

#if BLADE_MEMORY_DEBUG
	//////////////////////////////////////////////////////////////////////////
	size_t			FixedSizePool::getUnitSize() const
	{
		return mUnitSize;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			FixedSizePool::getMaxAllocatedCount() const
	{
		//return mPoolState.mAllocationCount;
		return mMaxAllocationCount;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	FixedSizePage*	FixedSizePool::allocatePage()
	{
		FixedSizePage* newPage = NULL;
		bool isOK = false;

		//find a page from cache
		mCacheLock.lock();
		for( int i = 0; i < NUM_CACHED_PAGES; ++i)
		{
			if( mCaches[i] != NULL )
			{
				newPage = mCaches[i];
				mCaches[i] = NULL;
				break;
			}
		}
		mCacheLock.unlock();
		if( newPage != NULL )
			return newPage;
		else
		{
#if ENABLE_FIXEDSIZE_PAGE_ALLOC
			newPage = lFixedSizePageAlloc.allocatePage();
#else
			void* buffer = ::BLADE_ALLOC(FixedSizePage::DEF_PAGE_SIZE);
			newPage = new(buffer) FixedSizePage();
#endif
			isOK= newPage->initialize(mUnitSize);
			newPage->unlockPage();

			Lock::add(&mPoolState.mFreeTotal, FixedSizePage::DEF_PAGE_SIZE);
			Lock::add(&mPoolState.mFreeBytes, newPage->getMaxAllocationBytes() );
			Lock::increment(&mPageCount);

		}
		if( !isOK)
		{
			Lock::decrement(&mPageCount);
			throw std::bad_alloc();
		}
		return newPage;
	}

	//////////////////////////////////////////////////////////////////////////
	void			FixedSizePool::freePage(FixedSizePage* page)
	{
		Lock::decrement(&mPageCount);
		Lock::sub(&mPoolState.mFreeTotal, FixedSizePage::DEF_PAGE_SIZE );
		Lock::sub(&mPoolState.mFreeBytes, page->getMaxAllocationBytes() );

		assert( page->isLocked() );
		page->setFlag(FixedSizePage::PF_EMPTY);

#if ENABLE_FIXEDSIZE_PAGE_ALLOC
		lFixedSizePageAlloc.deallocatePage(page);
#else
		::BLADE_FREE(page);
#endif	
	}

}//namespace Blade