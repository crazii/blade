/********************************************************************
	created:	2010/01/16
	filename: 	IncrementalPage.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IncrementalPage_h__
#define __Blade_IncrementalPage_h__
#include <Lock.h>
#include <interface/IPool.h>

//switch for fast alloc (speedup mode with less runtime check, but lower re-usage of memory)
//this may cause memory exhaustion if keep allocating a page of blocks & free part of them
//#define FAST_INCREMENT

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4324)
#endif

namespace Blade
{
	class IncrementalPage;

	struct SIncrementalBlock
	{
	public:
		typedef enum EBlockID
		{
			BID_FREE		= 0xDEADBEEF,
			//last 4 bits in use
			BID_OCCUPIED	= 0xBADBABE0,
			BID_EXTERNAL	= 0xBADCAFE0,
		}BLOCK_ID;

		enum EBlockGuard
		{
			BG_HEAD = 0xBEE00BED,
			BG_TAIL = 0xB1EED0AD,
		};

		static SIncrementalBlock*	getBlockfromData(void* data)
		{
			uint32* ptr = (uint32*)data;
			uint32 id = ptr[-1];
			if( (id&(~IPool::PAT_MASK)) != SIncrementalBlock::BID_OCCUPIED)
				assert( false && "internal error: block id error." );
			return reinterpret_cast<SIncrementalBlock*>( (char*)data - sizeof(SIncrementalBlock) );
		}

		SIncrementalBlock(IncrementalPage* page, uint32 freeBytes, uint32 id, SIncrementalBlock* prev)
			:mBlockBytes(freeBytes)
			,mIdentifier(id)
		{
			assert((char*)this - (char*)page > 0 && uintptr_t((char*)this - (char*)page) <= std::numeric_limits<uint32>::max());
			mPageOffset = uint32((char*)this - (char*)page);
			mHasNext = 0;
#ifndef FAST_INCREMENT
			this->setPrevBlock(prev);
			if( prev != NULL )
				prev->setNextBlock(this);
#else
			BLADE_UNREFERENCED(prev);
#endif
		}

		/** @brief  */
		inline	bool	isValid() const
		{
			return (mIdentifier&(~IPool::PAT_MASK)) == BID_OCCUPIED;
		}

		/** @brief  */
		inline bool		isFree() const
		{
			return mIdentifier == BID_FREE;
		}

		/** @brief  */
		char*			getData() const
		{
			return (char*)this + sizeof(SIncrementalBlock);
		}

		/** @brief  */
		size_t			getNumBytes() const
		{
			return mBlockBytes;
		}

		/** @brief  */
		void			setNumBytes(uint32 bytes)
		{
			mBlockBytes = bytes;
		}

#if BLADE_MEMORY_DEBUG
		/** @brief  */
		size_t			getAllocatedBytes() const
		{
			return mAllocatedBytes;
		}

		/** @brief  */
		void			setAllocatedBytes(size_t bytes)
		{
			assert(bytes <= mBlockBytes - sizeof(ptrdiff_t));	//extra guard
			mAllocatedBytes = bytes;
			ptrdiff_t& guard = *(ptrdiff_t*)(this->getData() + mAllocatedBytes);
			guard = BG_TAIL;
		}

		/** @brief  */
		void			setSafeGuard()
		{
			ptrdiff_t& guard = *(ptrdiff_t*)(this->getData() + mAllocatedBytes);
			guard = BG_TAIL;
			mHeadGuard = BG_HEAD;
		}

		/** @brief  */
		void			checkOverflow() const
		{
			ptrdiff_t guard = *(ptrdiff_t*)(this->getData() + mAllocatedBytes);
			assert( guard == (ptrdiff_t)BG_TAIL );
			assert( mHeadGuard == (ptrdiff_t)BG_HEAD);
		}
#endif

#ifndef FAST_INCREMENT
		/** @brief  */
		SIncrementalBlock*	getNextBlock() const
		{
			assert( this->isFree() );
			return mHasNext == 0 ? NULL : reinterpret_cast<SIncrementalBlock*>( (char*)this + sizeof(SIncrementalBlock) + mBlockBytes);
		}

		/** @brief  */
		SIncrementalBlock*	getPrevBlock() const
		{
			assert( this->isFree() );
			return mAddrPrev == 0 ? NULL : reinterpret_cast<SIncrementalBlock*>((char*)this - mAddrPrev);
		}

		/** @brief  */
		void				setNextBlock(SIncrementalBlock* next)
		{
			if (next == NULL)
				mHasNext = 0;
			else
			{
				mHasNext = 1;
				assert(next > this);
				assert((char*)this + sizeof(SIncrementalBlock) + mBlockBytes == (char*)next);
				assert((char*)this - (char*)next >= std::numeric_limits<int32>::min() && (char*)this - (char*)next <= std::numeric_limits<int32>::max());
			}
		}

		/** @brief  */
		void				setPrevBlock(SIncrementalBlock* prev)
		{
			if (prev == NULL)
				mAddrPrev = 0;
			else
			{
				assert(prev < this);
				assert((char*)this - (char*)prev >= std::numeric_limits<int32>::min() && (char*)this - (char*)prev <= std::numeric_limits<int32>::max());
				mAddrPrev = uint32((char*)this - (char*)prev);
			}
		}
#endif

		/** @brief  */
		IncrementalPage*		getPage() const
		{
			return reinterpret_cast<IncrementalPage*>( (char*)this - mPageOffset);
		}

		/** @brief  */
		void				setIdentifier(EBlockID id)
		{
			mIdentifier = id;
		}

		/** @brief  */
		size_t				getIdentifier() const
		{
			return mIdentifier;
		}

		//relative to page
		uint32				mPageOffset;
		//real allocated bytes without header, maybe larger that app requested
		uint32				mBlockBytes : 31;
		uint32				mHasNext	: 1;

#if BLADE_MEMORY_DEBUG
		//block content bytes without header (size that app requested)
		size_t				mAllocatedBytes;
		SIncrementalBlock*	mAllocatedPrev;
		SIncrementalBlock*	mAllocatedNext;

		const char*			mFileInfo;
		int					mLineNO;
		//this should the last second member 
		ptrdiff_t			mHeadGuard;
#endif

#ifndef FAST_INCREMENT
		uint32				mAddrPrev;
#else
		int32				padding[1];
#endif

		//@note!
		//this must be the last element
		uint32				mIdentifier;
	};//struct SIncrementalBlock
	static_assert( (sizeof(SIncrementalBlock)&(BLADE_MEMORY_ALIGNMENT-1)) == 0, "size/aligment error");

	class BLADE_ALIGNED(BLADE_MEMORY_ALIGNMENT) IncrementalPage
	{
	public:
		enum EPageID
		{
			PAGE_IDENTIFIER = 0xADD0FACE,
		};
		enum EPageFlag
		{
			PF_UNKNOWN,
			//partially free
			PF_FREE,
			//full page
			PF_FULL,
			//empty page
			PF_EMPTY,
		};

		IncrementalPage();
		~IncrementalPage();

		/** @brief check the safe id */
		bool			isValid() const;

		/** @brief whether the page has no blocks allocated */
		bool			isEmpty() const;

		/** @brief */
		bool			isFull() const;

		/** @brief init */
		bool			initialize(size_t pageSizeBytes);

		/** @brief memory allocation */
		void*			allocate(size_t bytes, int type, const char* sourcefile,int lineNO);

		/** @brief memory deallocation */
		void			deallocate(SIncrementalBlock* block);

		/** @brief lock the page for access */
		void			lockPage() { mPageLock.lock(); }

		/** @brief try lock the page for allocation */
		bool			tryLockPage() { return mPageLock.tryLock(); }

		/** @brief unlock the page after alloc/de-alloc */
		void			unlockPage() { mPageLock.unlock(); }

		/** @brief set the page flag */
		void			setFlag(EPageFlag pf) { mFlag = pf; }

		/** @brief get the page flag */
		inline EPageFlag		getFlag() const {return mFlag;}

		/** @brief  */
		inline size_t			getLeftBytes() const {return size_t((char*)this + mSize - mAllocPtr);}

		//link list operation

		/** @brief */
		inline IncrementalPage*	getNextLink() const {return mNextPage;}

		/** @brief */
		inline IncrementalPage*	getPrevLink() const {return mPrevPage;}

		/** @brief */
		void				setNextLink(IncrementalPage* next);

		/** @brief */
		void				setPrevLink(IncrementalPage* prev);

		/** @brief remove this from its list */
		void				detachFromList(IncrementalPage* & head);

		/** @brief */
		void				dumpLeaks();

	protected:
		Lock				mPageLock;

		uint32				mIdentifier;
		EPageFlag			mFlag;
		char*				mAllocPtr;
		size_t				mSize;
		SIncrementalBlock*	mLastBlock;

		//link list data
		IncrementalPage*	mNextPage;
		IncrementalPage*	mPrevPage;
#if BLADE_MEMORY_DEBUG
		SIncrementalBlock*	mAllocatedBlockList;
#else
		void*				padding;
#endif
	};//class IncrementalPage
 
}	//namespace Blade
 
#endif //__Blade_IncrementalPage_h__