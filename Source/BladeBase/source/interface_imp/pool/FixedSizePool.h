/********************************************************************
	created:	2009/02/13
	filename: 	FixedSizePool.h
	author:		Crazii
	
	purpose:	IPool implementaion: this type of IPool allocate blocks in the same size,
				or blocks with multiple of this size.
*********************************************************************/
#ifndef __Blade_FixedSizePool_h__
#define __Blade_FixedSizePool_h__
#include <memory/BladeMemory.h>
#include <RefCount.h>
#include "FixedSizePage.h"

namespace Blade
{
	class FixedSizePool : public IPool , public StaticAllocatable
	{
	public:

		//the empty page maybe not be freed at once,they'd be cached for next allocation
		static const int NUM_CACHED_PAGES = 0;

		//the number of concurrent pages available also new created pages when initializing
		static const int MAX_CONCURRENT_PAGES = 8;

		//
		static const int DEFAULT_CONCURRENT_PAGES = 2;

		//the page pool 's initial size
		static const int PAGE_POOL_INIT_SIZE = 1024;

		FixedSizePool();
		~FixedSizePool();

		/** @brief kbytes is the allocating Pool size */
		virtual void	initialize(size_t threadCount = DEFAULT_SETTING,size_t unit_size = DEFAULT_SETTING,size_t kbytes = DEFAULT_SETTING);

		/** @brief debug version of allocate */
		virtual void*	allocate(size_t bytes, const char* sourcefile, int lineNO, int type = PAT_DEFAULT);

		/** @brief re-allocate memory */
		virtual void*	reallocate(void* ptr, size_t bytes, const char* sourcefile, int lineNO, int type = PAT_DEFAULT);

		/** @brief free a block of memory allocated */
		virtual void	deallocate(void* p, int type = PAT_DEFAULT);

		/** @brief free all memory */
		virtual void	release();

		/** @brief profiling */
		virtual bool	getPoolState(SPoolState& outState);

#if BLADE_MEMORY_DEBUG
		/** @brief  */
		size_t			getUnitSize() const;

		/** @brief allocation statistics */
		size_t			getMaxAllocatedCount() const;
#endif


	protected:

		/** @brief  */
		FixedSizePage*	allocatePage();

		/** @brief  */
		void			freePage(FixedSizePage* page);

		size_t				mConcurrentPageCount;

		size_t				mUnitSize;
		size_t				mLargestSize;

		FixedSizePage*		mCaches[NUM_CACHED_PAGES+1];
		Lock				mCacheLock;

		FixedSizePage*		mFullPages;
		Lock				mFullPageLock;

		FixedSizePage*		mFreePageHead;
		FixedSizePage*		mFreePageTail;
		size_t				mPageCount;
		Lock				mFreePageLock;

		FixedSizePage*		mConcurrentPage[MAX_CONCURRENT_PAGES];
		Lock				mCurrentPageLock;
		SPoolState			mPoolState;

#if BLADE_MEMORY_DEBUG
		size_t				mMaxAllocationCount;
#endif

	};
}//namespace Blade


#endif // __Blade_FixedSizePool_h__