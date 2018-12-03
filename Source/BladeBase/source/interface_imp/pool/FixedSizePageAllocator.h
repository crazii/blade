/********************************************************************
	created:	2012/12/24
	filename: 	FixedSizePageAllocator.h
	author:		Crazii
	purpose:	make large allocation(>4K each time), if page size is small,
				the large allocation(PageBlock) is split into several pages.
*********************************************************************/
#ifndef __Blade_FixedSizePageAllocator_h__
#define __Blade_FixedSizePageAllocator_h__
#include <BladePlatform.h>
#include "../../BladeWin32API.h"

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

#	define BLADE_ALLOC(size)	VirtualAlloc(NULL,size,MEM_COMMIT/*|MEM_TOP_DOWN*/,PAGE_READWRITE)
#	define BLADE_FREE(ptr)		VirtualFree(ptr,0,MEM_RELEASE)

#else

#	define BLADE_ALLOC(size)	malloc(size)
#	define BLADE_FREE(ptr)		free(ptr)

#endif

#include "FixedSizePage.h"
#if ENABLE_FIXEDSIZE_PAGE_ALLOC
#include <set>
#include <Lock.h>
#include <memory/PoolAllocator.h>

namespace Blade
{

	class FixedSizePageAllocator
	{
	public:
		static const uint32 PAGE_BLOCK_SIZE = FixedSizePage::DEF_BLOCK_SIZE;
		static const uint32 BLOCK_PAGE_COUNT = PAGE_BLOCK_SIZE/FixedSizePage::DEF_PAGE_SIZE;

		//////////////////////////////////////////////////////////////////////////
		struct SPageBlock : public PoolSpace::Binding::PooledAllocatable
		{
			FixedSizePage*	mPages[BLOCK_PAGE_COUNT];
			bool			mFlag[BLOCK_PAGE_COUNT];

			/** @brief  */
			SPageBlock()
			{
				for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
				{
					mPages[i] = NULL;
					mFlag[i] = false;
				}
			}

			/** @brief  */
			bool	operator<(const SPageBlock& rhs) const
			{
				return (intptr_t)rhs.mPages[0] > (intptr_t)mPages[0] && (intptr_t)rhs.mPages[BLOCK_PAGE_COUNT-1] > (intptr_t)mPages[BLOCK_PAGE_COUNT-1];
			}

			/** @brief  */
			FixedSizePage*	allocatePage()
			{
				for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
				{
					if( mFlag[i] )
					{
						mFlag[i] = false;
						return mPages[i];
					}
				}
				return NULL;
			}

			/** @brief  */
			bool			freePage(FixedSizePage* page)
			{
				for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
				{
					if( mPages[i] == page)
					{
						mFlag[i] = true;
						return true;
					}
				}
				return false;
			}

			/** @brief  */
			bool			hasFreePage() const
			{
				for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
				{
					if( mFlag[i] )
						return true;
				}
				return false;
			}

			/** @brief  */
			size_t			getFreePageCount() const
			{
				size_t n = 0;
				for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
				{
					if( mFlag[i] )
						++n;
				}
				return n;
			}
		};
	protected:
		/** @brief  */
		SPageBlock*	allocateBlock();

		/** @brief  */
		void		freeBlock(SPageBlock* block);

		struct FnPageBlockLess
		{
			bool operator()(const SPageBlock* lhs, const SPageBlock* rhs) const
			{
				return *lhs < *rhs;
			}
		};

		//sort the block by pointer (lower address at the first, higher at end)
		typedef PoolSpace::Binding::PooledSet<SPageBlock*, FnPageBlockLess>	PageBlockList;

		PageBlockList	mAllocatedBlocks;
		PageBlockList	mFreeBlocks;
		Lock			mLock;
	public:
		//////////////////////////////////////////////////////////////////////////
		FixedSizePageAllocator();

		~FixedSizePageAllocator();

		/** @brief  */
		FixedSizePage*	allocatePage();

		/** @brief  */
		void deallocatePage(FixedSizePage* page);
	};
	
}//namespace Blade


#endif//ENABLE_FIXEDSIZE_PAGE_ALLOC

#endif //  __Blade_FixedSizePageAllocator_h__