/********************************************************************
	created:	2014/02/18
	filename: 	FixedSizePageAllocator.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "FixedSizePageAllocator.h"

#if ENABLE_FIXEDSIZE_PAGE_ALLOC

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	FixedSizePageAllocator::FixedSizePageAllocator()
	{
		PoolSpace::setPool( Memory::getResourcePool() );
	}

	//////////////////////////////////////////////////////////////////////////
	FixedSizePageAllocator::~FixedSizePageAllocator()
	{
		mFreeBlocks.clear();
		mAllocatedBlocks.clear();
		assert(mAllocatedBlocks.size() == 0);
		PoolSpace::setPool(NULL);
	}

	//////////////////////////////////////////////////////////////////////////
	FixedSizePageAllocator::SPageBlock*	FixedSizePageAllocator::allocateBlock()
	{
		char* buffer = (char*)::BLADE_ALLOC(PAGE_BLOCK_SIZE);
		SPageBlock* block = BLADE_NEW SPageBlock();
		for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
		{
			block->mFlag[i] = true;
			block->mPages[i] = new (buffer) FixedSizePage();
			buffer += FixedSizePage::DEF_PAGE_SIZE;
		}

		std::pair<PageBlockList::iterator,bool> ret = mAllocatedBlocks.insert(block);
		assert(ret.second);
		ret = mFreeBlocks.insert(block);
		assert(ret.second);
		return *(ret.first);
	}

	//////////////////////////////////////////////////////////////////////////
	void		FixedSizePageAllocator::freeBlock(SPageBlock* block)
	{
		for(uint32 i = 0; i < BLOCK_PAGE_COUNT; ++i)
		{
			assert( block->mFlag[i] == true);
			if( i > 0)
			{
				assert( (const char*)block->mPages[i-1] + FixedSizePage::DEF_PAGE_SIZE == (char*)block->mPages[i] );
			}
		}
		void* buffer = block->mPages[0];
		size_t count = mFreeBlocks.erase(block);
		assert(count == 1);
		BLADE_UNREFERENCED(count);
		::BLADE_FREE(buffer);
		BLADE_DELETE block;
	}

	//////////////////////////////////////////////////////////////////////////
	FixedSizePage*	FixedSizePageAllocator::allocatePage()
	{
		ScopedLock _lock(mLock);

		if( mFreeBlocks.size() == 0 )
		{
			SPageBlock* block = this->allocateBlock();
			assert(block != NULL);
			assert(mFreeBlocks.size() != 0);
			BLADE_UNREFERENCED(block);
		}

		PageBlockList::iterator i = mFreeBlocks.begin();
		SPageBlock* pageBlock = *i;
		if( !pageBlock->hasFreePage() )
		{
			assert(false);
			throw std::runtime_error("internal error.");
		}

		FixedSizePage* page = pageBlock->allocatePage();
		if( !pageBlock->hasFreePage() )
			mFreeBlocks.erase(i);

		return page;
	}

	//////////////////////////////////////////////////////////////////////////
	void FixedSizePageAllocator::deallocatePage(FixedSizePage* page)
	{
		if( page == NULL || page->getFlag() != FixedSizePage::PF_EMPTY )
		{
			assert(false);
			return;
		}

		ScopedLock _lock(mLock);

		SPageBlock finder;
		finder.mPages[0] = finder.mPages[BLOCK_PAGE_COUNT-1] = page;
		PageBlockList::iterator i = mAllocatedBlocks.find( &finder );
		if( i == mAllocatedBlocks.end() || !(*i)->freePage(page) )
		{
			assert(false);
			throw std::runtime_error("internal error.");
		}

		SPageBlock* pageBlock = *i;
		if( pageBlock->getFreePageCount() == 1 )
		{
			bool success = mFreeBlocks.insert( pageBlock ).second;
			assert(success);
			BLADE_UNREFERENCED(success);
		}
		else
		{
			assert( mFreeBlocks.find(pageBlock) != mFreeBlocks.end() );
			if( pageBlock->getFreePageCount() == BLOCK_PAGE_COUNT )
			{
				mAllocatedBlocks.erase(i);
				this->freeBlock(pageBlock);
			}
		}
	}
	
}//namespace Blade

#endif