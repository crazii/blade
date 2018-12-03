/********************************************************************
	created:	2010/01/16
	filename: 	IncrementalPool.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IncrementalPool_h__
#define __Blade_IncrementalPool_h__
#include <memory/BladeMemory.h>
#include "IncrementalPage.h"

namespace Blade
{
	class IncrementalPool : public IPool, public ResourceAllocatable
	{
	public:
		//the empty page maybe not be freed at once,they'd be cached for next allocation
		static const int NUM_CACHED_PAGES = 0;

		//the number of concurrent pages available also new created pages when initializing
		static const int MAX_CONCURRENT_PAGES = 8;

		static const int DEFAULT_CONCURRENT_PAGES = 2;


		IncrementalPool();
		~IncrementalPool();

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

	protected:
		static const size_t		mMinSize = sizeof(void*);

		/** @brief allocate one page */
		IncrementalPage*	allocatePage();

		/** @brief free one page, release memory to system */
		void				freePage(IncrementalPage* page);

		size_t				mConcurrentPageCount;
		size_t				mPageSize;
		size_t				mLargestSize;

		IncrementalPage*	mCaches[NUM_CACHED_PAGES+1];
		Lock				mCacheLock;

		IncrementalPage*	mConcurrentPage[MAX_CONCURRENT_PAGES];
		Lock				mCurrentPageLock;

		IncrementalPage*	mFullPages;
		Lock				mFullPageLock;
		SPoolState			mPoolState;
	};//class IncrementalPool
 
}	//namespace Blade
 
#endif //__Blade_IncrementalPool_h__

