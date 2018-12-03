/********************************************************************
	created:	2010/01/16
	filename: 	FixedSizePage.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "../../BladeWin32API.h"
#include "FixedSizePage.h"

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	FixedSizePage::FixedSizePage()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	FixedSizePage::~FixedSizePage()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//init the page
	bool		FixedSizePage::initialize(size_t fixedBlockSizeBytes)
	{
		mUnitSize = (uint32)fixedBlockSizeBytes;
		mUnitAllocSize = (mUnitSize + BLOCK_SIZE  + BLADE_MEMORY_ALIGNMENT-1)/BLADE_MEMORY_ALIGNMENT*BLADE_MEMORY_ALIGNMENT;
		mIdentifier = PAGE_IDENTIFIER;

		mAllocPtr = (char*)this + sizeof(FixedSizePage);
		mLeftBytes = FixedSizePage::DEF_PAGE_SIZE - sizeof(FixedSizePage);

		mTotalBlocks = (uint32)mLeftBytes/(mUnitAllocSize);
		mCurrentBlocks = (uint32)mTotalBlocks;

		mFreeBlockHead = NULL;
		mFreeBlockTail = NULL;
		mFlag = PF_UNKNOWN;
		//mLargestBlock = NULL;

		mPrev = NULL;
		mNext = NULL;

#if BLADE_MEMORY_DEBUG
		mAllocatedLink = NULL;
#endif
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//allocate a block of memory
	void*		FixedSizePage::allocate(size_t bytes, int type, const char* sourcefile,int lineNO)
	{
		if( !this->hasFreeSpace() )
			return NULL;

		uint32 id = uint32( SFixedSizeBlock::BID_OCCUPIED|(type&IPool::PAT_MASK) );

		SFixedSizeBlock* block;
		if( bytes <= mUnitSize )
		{
			//check the free block
			if( mFreeBlockHead != NULL )
			{
				block = mFreeBlockHead;
				assert( mFreeBlockHead->isFree() );
				assert( mFreeBlockHead->getNumBytes() >= bytes );

				//free block is large and could be split into more
				if (mFreeBlockHead->getNumBytes() + BLOCK_SIZE >= mUnitAllocSize * 2)
				{
					SFixedSizeBlock* splitblock = new( (char*)mFreeBlockHead + mUnitAllocSize) SFixedSizeBlock(this, mFreeBlockHead->getNumBytes() - mUnitAllocSize, (uint32)SFixedSizeBlock::BID_FREE);
					splitblock->setNextFreeBlock( mFreeBlockHead->getNextFreeBlock() );
					assert(splitblock->getNumBytes() >= mUnitSize);

					if( mFreeBlockTail == mFreeBlockHead )
					{
						assert( mFreeBlockHead->getNextFreeBlock() == NULL );
						mFreeBlockTail = splitblock;
					}
					mFreeBlockHead->mBlockBytes = (sizetype)(mUnitAllocSize - BLOCK_SIZE);
					mFreeBlockHead->setIdentifier(id);
					mFreeBlockHead = splitblock;
				}
				else
				{
					//use the free block
					//assert( mFreeBlockHead->getNumBytes() == bytes );
					mFreeBlockHead = mFreeBlockHead->getNextFreeBlock();
					if( mFreeBlockHead == NULL )
					{
						assert(mFreeBlockTail->getNextFreeBlock() == NULL ); 
						mFreeBlockTail = NULL;
					}
					else
						assert( mFreeBlockHead->getNextFreeBlock() != NULL || mFreeBlockTail == mFreeBlockHead );
					block->setIdentifier(id);
				}
			}
			else
			{
				if( !(mLeftBytes >= mUnitAllocSize) )
					assert( false );
				mLeftBytes -= mUnitAllocSize;

				//if( mLeftBytes < mUnitAllocSize )
				//	mLeftBytes = 0;
				block = new(mAllocPtr) SFixedSizeBlock(this, mUnitAllocSize - BLOCK_SIZE, id);
				mAllocPtr += mUnitAllocSize;
			}
			--mCurrentBlocks;
			assert(mCurrentBlocks >= mLeftBytes / mUnitAllocSize);
		}
		else
		{
			size_t multiplier = (bytes + BLOCK_SIZE + mUnitAllocSize - 1)/mUnitAllocSize;
			assert(multiplier >= 2);
			size_t alloc_size = multiplier*(mUnitAllocSize);
			if( mLeftBytes < alloc_size)
			{
				if( bytes + BLOCK_SIZE <= mLeftBytes )
				{
					alloc_size = mLeftBytes;
					//assert(mCurrentBlocks >= multiplier); left bytes may less than a block
					multiplier = mCurrentBlocks;
				}
				else
					return NULL;
			}
			mLeftBytes -= alloc_size;

			if( mLeftBytes < mUnitAllocSize )
			{
				alloc_size += mLeftBytes;
				mLeftBytes = 0;
			}

			block = new(mAllocPtr) SFixedSizeBlock(this, alloc_size-BLOCK_SIZE, id);
			mAllocPtr += alloc_size;
			assert(mCurrentBlocks >= multiplier);
			mCurrentBlocks -= (uint32)multiplier;
			assert(mCurrentBlocks >= mLeftBytes / mUnitAllocSize);
		}

#if BLADE_MEMORY_DEBUG
		block->mFileInfo = sourcefile;
		block->mLineNO = (sizetype)lineNO;

		if( mAllocatedLink == NULL )
		{
			block->mAllocatedPrev = block->mAllocatedNext = NULL;
			mAllocatedLink = block;
		}
		else
		{
			block->mAllocatedPrev = NULL;
			block->mAllocatedNext = mAllocatedLink;
			mAllocatedLink->mAllocatedPrev = block;
			mAllocatedLink = block;
		}
#else
		BLADE_UNREFERENCED(lineNO);
		BLADE_UNREFERENCED(sourcefile);
#endif
#if BLADE_MEMORY_DEBUG
		block->mRequestedBytes = (sizetype)bytes;
		assert(block->mBlockBytes >= block->mRequestedBytes);

		block->setSafeGuard();
#endif
		return block->getData();
	}

	//////////////////////////////////////////////////////////////////////////
	//free a block of memory allocated
	void		FixedSizePage::deallocate(SFixedSizeBlock* block)
	{
		assert( block->getPage() == this && block->isValid() );
#if BLADE_MEMORY_DEBUG
		block->checkOverflow();
		if( mAllocatedLink == block )
		{
			assert( block->mAllocatedPrev == NULL);
			mAllocatedLink = block->mAllocatedNext;
			if( mAllocatedLink != NULL )
				mAllocatedLink->mAllocatedPrev = NULL;
		}
		else
		{
			assert( block->mAllocatedPrev != NULL );
			block->mAllocatedPrev->mAllocatedNext = block->mAllocatedNext;
			if( block->mAllocatedNext!= NULL )
				block->mAllocatedNext->mAllocatedPrev = block->mAllocatedPrev;
		}
#endif
		block->setIdentifier((uint32)SFixedSizeBlock::BID_FREE);

		if( block->getNumBytes() == mUnitAllocSize-BLOCK_SIZE )
		{
			++mCurrentBlocks;
			block->setNextFreeBlock( mFreeBlockHead );
			if( mFreeBlockHead == NULL )
			{
				assert( mFreeBlockTail ==  NULL );
				mFreeBlockTail = block;
			}
			mFreeBlockHead = block;
		}
		else
		{
			//assert( block->getNumBytes()+BLOCK_SIZE >= mUnitAllocSize*2 );
			//link larger block to the tail of the free list
			assert( (block->getNumBytes() + BLOCK_SIZE) >= mUnitAllocSize );
			mCurrentBlocks += uint32(block->getNumBytes() + BLOCK_SIZE)/mUnitAllocSize;
			block->setNextFreeBlock(NULL);

			if( mFreeBlockTail != NULL )
				mFreeBlockTail->setNextFreeBlock( block );
			else
			{
				assert( mFreeBlockHead == NULL );
				mFreeBlockHead = block;
			}
			
			mFreeBlockTail = block;
		}

		if( this->isEmpty() )
		{
			mAllocPtr = (char*)this + sizeof(FixedSizePage);
			mLeftBytes = FixedSizePage::DEF_PAGE_SIZE - sizeof(FixedSizePage);
			mFreeBlockTail = mFreeBlockHead = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			FixedSizePage::setNextLink(FixedSizePage* next)
	{
		//assert( next == NULL || next->getPrevLink() == NULL || next->getPrevLink() == this );
		if( !( next == NULL || next->getPrevLink() == NULL || next->getPrevLink() == this ) )
			assert(false);
		mNext = next;
		if( next != NULL && next->getPrevLink() != this )
			next->setPrevLink( this );
	}

	//////////////////////////////////////////////////////////////////////////
	void			FixedSizePage::setPrevLink(FixedSizePage* prev)
	{
		//assert( prev == NULL /*|| prev->getNextLink() == NULL || prev->getNextLink() == this*/ );
		mPrev = prev;

		if( prev != NULL && prev->getNextLink() != this )
			prev->setNextLink( this );
	}

	//////////////////////////////////////////////////////////////////////////
	//link the page to another page's back
	void		FixedSizePage::linkToBack(FixedSizePage* & link)
	{
		//prev must be empty to link to another
		assert( this->mPrev == NULL && (link == NULL || link->getNextLink() == NULL) );

		if( link != NULL )
			link->setNextLink( this );

		link = this;
	}

	//////////////////////////////////////////////////////////////////////////
	//remove this from its list
	void		FixedSizePage::detachFromList(FixedSizePage*& head)
	{
		assert( this->mPrev != NULL || this->mNext != NULL || head == this );

		FixedSizePage* prev = mPrev;
		FixedSizePage* next = mNext;

		if( next != NULL )
		{
			next->mPrev = prev;
			mNext = NULL;
		}

		if( prev != NULL )
		{
			prev->mNext = next;
			mPrev = NULL;
		}
		else
		{
			assert( head == this );
			head = next;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			FixedSizePage::dumpLeaks()
	{
#if BLADE_MEMORY_DEBUG
		if( Memory::isLeakDumpEnabled() )
		{
			SFixedSizeBlock* block = mAllocatedLink;
			while( block != NULL )
			{
				const char* file = block->mFileInfo;
				if( file == NULL )
					file = "Unknown";
	#if BLADE_IS_WINDOWS_CLASS_SYSTEM
				char buffer[1024];
				sprintf_s(buffer,1024,"%s(%d): %d bytes\n",file,block->mLineNO, (int)block->mRequestedBytes);
				::OutputDebugStringA(buffer);
	#endif
				block = block->mAllocatedNext;
			}
		}
#endif
	}
 
}	//namespace Blade