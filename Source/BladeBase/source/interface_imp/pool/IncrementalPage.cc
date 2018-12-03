/********************************************************************
	created:	2010/01/16
	filename: 	IncrementalPage.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "../../BladeWin32API.h"
#include "IncrementalPage.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	IncrementalPage::IncrementalPage()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	IncrementalPage::~IncrementalPage()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//check the safe id
	bool	IncrementalPage::isValid() const
	{
		return mIdentifier == PAGE_IDENTIFIER;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IncrementalPage::isEmpty() const
	{
		return mAllocPtr == (char*)this + sizeof(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IncrementalPage::isFull() const
	{
		return mAllocPtr == (char*)this + mSize;
	}

	//////////////////////////////////////////////////////////////////////////
	//init
	bool	IncrementalPage::initialize(size_t pageSizeBytes)
	{
		mIdentifier = (uint32)PAGE_IDENTIFIER;

		mSize = pageSizeBytes;
		mAllocPtr = (char*)this + sizeof(*this);

		mNextPage = NULL;
		mPrevPage = NULL;

		mLastBlock = NULL;

#if BLADE_MEMORY_DEBUG
		mAllocatedBlockList = NULL;
#endif
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//memory allocation
	void*	IncrementalPage::allocate(size_t bytes, int type, const char* sourcefile,int lineNO)
	{
		size_t alloc_size = bytes + sizeof(SIncrementalBlock);
#if BLADE_MEMORY_DEBUG
		//add extra space for safe guard
		
		alloc_size += sizeof(ptrdiff_t);
#endif
		alloc_size = (alloc_size + BLADE_MEMORY_ALIGNMENT - 1) / BLADE_MEMORY_ALIGNMENT*BLADE_MEMORY_ALIGNMENT;

		size_t leftBytes = this->getLeftBytes();
		if( alloc_size > leftBytes)
			return NULL;
		//check the left size is capable of a smallest allocation
		else if(alloc_size + sizeof(SIncrementalBlock) + sizeof(void*) > leftBytes)
			alloc_size = leftBytes;
	
		SIncrementalBlock* block = new(mAllocPtr) SIncrementalBlock(this, 
			uint32( alloc_size - sizeof(SIncrementalBlock) ),
			uint32( SIncrementalBlock::BID_OCCUPIED|(type&IPool::PAT_MASK) ),
			mLastBlock);

		mAllocPtr += alloc_size;
		mLastBlock = block;

#if BLADE_MEMORY_DEBUG
		block->mAllocatedBytes = bytes;	//bytes requested by app
		block->mFileInfo = sourcefile;
		block->mLineNO = lineNO;
		if( mAllocatedBlockList == NULL )
		{
			block->mAllocatedPrev = block->mAllocatedNext = NULL;
			mAllocatedBlockList = block;
		}
		else
		{
			//put it to the head
			block->mAllocatedPrev = NULL;
			block->mAllocatedNext = mAllocatedBlockList;
			mAllocatedBlockList->mAllocatedPrev = block;
			mAllocatedBlockList = block;
		}
		block->setSafeGuard();
#else
		BLADE_UNREFERENCED(sourcefile);
		BLADE_UNREFERENCED(lineNO);
#endif
		return block->getData();
	}

	//////////////////////////////////////////////////////////////////////////
	//memory deallocation
	void	IncrementalPage::deallocate(SIncrementalBlock* block)
	{
#if BLADE_MEMORY_DEBUG
		block->checkOverflow();
#endif
		assert( block->getPage() == this );
		block->setIdentifier( SIncrementalBlock::BID_FREE );

#if BLADE_MEMORY_DEBUG
		if( mAllocatedBlockList == block )
		{
			assert( block->mAllocatedPrev == NULL);
			mAllocatedBlockList = block->mAllocatedNext;
			if( mAllocatedBlockList != NULL )
				mAllocatedBlockList->mAllocatedPrev = NULL;
		}
		else
		{
			assert( block->mAllocatedPrev != NULL );
			block->mAllocatedPrev->mAllocatedNext = block->mAllocatedNext;
			if( block->mAllocatedNext!= NULL )
				block->mAllocatedNext->mAllocatedPrev = block->mAllocatedPrev;
		}
#endif

#ifndef FAST_INCREMENT
		//merge free : prev
		//merge the block into the previous neighbor block
		SIncrementalBlock* prev = block->getPrevBlock();
		if(prev != NULL && prev->isFree() )
		{
			SIncrementalBlock* next = block->getNextBlock();
			if(next != NULL )
				next->setPrevBlock(prev);
			else
			{
				assert(mLastBlock == block);
				mLastBlock = prev;
			}

			uint32 freeBytes = uint32(prev->getNumBytes() + block->getNumBytes() + sizeof(SIncrementalBlock));
			prev->setNumBytes(freeBytes);
			prev->setNextBlock(next);
			block = prev;
		}

		//merge free: next
		//merge the next block into the block
		SIncrementalBlock* next = block->getNextBlock();
		if(next != NULL)
		{
			if(next->isFree() )
			{
				uint32 freeBytes = uint32(block->getNumBytes() + next->getNumBytes() + sizeof(SIncrementalBlock));
				block->setNumBytes(freeBytes);

				SIncrementalBlock* nn = next->getNextBlock();
				if(nn != NULL )
				{
					nn->setPrevBlock( block );
					block->setNextBlock(nn);
				}
				else
				{
					assert(mLastBlock == next);
					mLastBlock = block;
				}
			}
		}
		else
		{
			assert( mLastBlock == block );
		}

		if( mLastBlock == block )
		{
			assert( (char*)block + block->getNumBytes() + sizeof(SIncrementalBlock) == mAllocPtr );
			mAllocPtr -= block->getNumBytes() + sizeof(SIncrementalBlock);
			mLastBlock = block->getPrevBlock();
			if( mLastBlock != NULL )
				mLastBlock->setNextBlock(NULL);
		}
#endif
		//empty?
		if( this->isEmpty() )
		{
#ifndef FAST_INCREMENT
			assert( mLastBlock == NULL );
#endif
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			IncrementalPage::setNextLink(IncrementalPage* next)
	{
		assert( next == NULL || next->getPrevLink() == NULL || next->getPrevLink() == this || next->getPrevLink()->getPrevLink() == this );
		mNextPage = next;
		if( next != NULL && next->getPrevLink() == NULL )
			next->setPrevLink( this );
	}

	//////////////////////////////////////////////////////////////////////////
	void			IncrementalPage::setPrevLink(IncrementalPage* prev)
	{
		if( !( prev == NULL || prev->getNextLink() == NULL || prev->getNextLink() == this || prev->getNextLink()->getNextLink() == this) )
			assert(false);
		//assert( prev == NULL || prev->getNextLink() == NULL || prev->getNextLink() == this );
		mPrevPage = prev;

		if( prev != NULL && prev->getNextLink() == NULL )
			prev->setNextLink( this );
	}

	//////////////////////////////////////////////////////////////////////////
	//remove this from its list
	void		IncrementalPage::detachFromList(IncrementalPage*& head)
	{
		assert( this->mPrevPage != NULL || this->mNextPage != NULL || head == this );
		IncrementalPage* prev = mPrevPage;
		IncrementalPage* next = mNextPage;

		if( next != NULL )
		{
			next->setPrevLink( prev );
			mNextPage = NULL;
		}

		if( prev != NULL )
		{
			prev->setNextLink( next );
			mPrevPage = NULL;
		}
		else
		{
			assert( head == this );
			head = next;
		}
		this->mPrevPage = this->mNextPage = NULL;
	}

	//
	void		IncrementalPage::dumpLeaks()
	{
#if BLADE_MEMORY_DEBUG
		if( Memory::isLeakDumpEnabled() )
		{
			SIncrementalBlock* block = mAllocatedBlockList;
			while( block != NULL )
			{
				const char* file = block->mFileInfo;
				if( file == NULL )
					file = "Unknown";
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
				char buffer[1024];
				sprintf_s(buffer,1024,"%s(%d): %d bytes\n",file,block->mLineNO, (int)block->mBlockBytes);
				::OutputDebugStringA(buffer);
#endif
				block = block->mAllocatedNext;
			}

		}
#endif
	}
 
}	//namespace Blade