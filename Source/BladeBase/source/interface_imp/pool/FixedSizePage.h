/********************************************************************
	created:	2010/01/14
	filename: 	FixedSizePage.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_FixedSizePage_h__
#define __Blade_FixedSizePage_h__
#include <Lock.h>
#include <interface/IPool.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4324)
#pragma warning(disable:4355)
#endif

#if BLADE_IS_WINDOWS_CLASS_SYSTEM && BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
#	define ENABLE_FIXEDSIZE_PAGE_ALLOC 1
#else
#	define ENABLE_FIXEDSIZE_PAGE_ALLOC 0
#endif

namespace Blade
{
	class FixedSizePage;

	struct SFixedSizeBlock
	{
	public:
		enum EBlockID
		{
			BID_FREE		= 0xDEADBABE,
			//last 4 bits in use
			BID_OCCUPIED	= 0xBEEFFED0,
			BID_EXTERNAL	= 0xBADF00D0,
		};

		enum EBlockGuard
		{
			BG_HEAD = 0xBED0FACE,
			BG_TAIL = 0xFACE0BED,
		};

#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_64BIT
		typedef uint32 sizetype;
#else
		typedef uint16 sizetype;
#endif


		static SFixedSizeBlock*	getBlockfromData(void* data)
		{
			uint32* ptr = (uint32*)data;
			uint32 id = ptr[-1];
			if( (id&(~IPool::PAT_MASK))  !=  SFixedSizeBlock::BID_OCCUPIED )
			{
				assert("the block is already freed" && false);
			}
			return reinterpret_cast<SFixedSizeBlock*>( (char*)data - sizeof(SFixedSizeBlock) );
		}

		SFixedSizeBlock(FixedSizePage* page, size_t freeBytes, uint32 id)
			:mPageOffset( (sizetype)( (char*)this - (char*)page) )
			,mBlockBytes((sizetype)freeBytes)
			,mIdentifier( id)
		{
			assert(freeBytes <= (size_t)std::numeric_limits<sizetype>::max());
			assert( (char*)this - (char*)page > 0 );
			assert(sizetype((char*)this - (char*)page) < std::numeric_limits<sizetype>::max() );
		}

		/** @brief  */
		inline	bool	isValid()
		{
			return (mIdentifier&(~IPool::PAT_MASK)) == BID_OCCUPIED;
		}

		/** @brief  */
		inline bool		isFree()
		{
			return mIdentifier == BID_FREE;
		}

		/** @brief block content bytes(without header), maybe larger than size requested by app */
		inline size_t			getNumBytes()
		{
			return mBlockBytes;
		}

		/** @brief  */
		char*			getData()
		{
			return (char*)this + sizeof(*this);
		}

#if BLADE_MEMORY_DEBUG
		/** @brief  */
		inline size_t			getAllocatedBytes()
		{
			return mRequestedBytes;
		}

		/** @brief  */
		inline void			setSafeGuard()
		{
			ptrdiff_t& guard = *(ptrdiff_t*)(this->getData() + mRequestedBytes);
			guard = BG_TAIL;
		}

		/** @brief  */
		inline void			checkOverflow()
		{
			ptrdiff_t guard = *(ptrdiff_t*)(this->getData() + mRequestedBytes);
			assert( guard == (ptrdiff_t)BG_TAIL );
		}
#endif

		/** @brief  */
		inline SFixedSizeBlock*	getNextFreeBlock()
		{
			assert( this->isFree() );
			return *reinterpret_cast<SFixedSizeBlock**>(getData());
		}

		/** @brief  */
		inline void				setNextFreeBlock(SFixedSizeBlock* block)
		{
			assert( this->isFree() );
			*reinterpret_cast<SFixedSizeBlock**>(getData()) = block;
		}

		/** @brief  */
		inline void				setIdentifier(uint32 id)
		{
			mIdentifier = id;
		}

		/** @brief  */
		inline size_t				getIdentifier() const
		{
			return mIdentifier;
		}

		/** @brief  */
		inline FixedSizePage*		getPage() const
		{
			return reinterpret_cast<FixedSizePage*>((char*)this - mPageOffset);
		}

	public:
#if BLADE_MEMORY_DEBUG
		SFixedSizeBlock*	mAllocatedPrev;
		SFixedSizeBlock*	mAllocatedNext;

		const char*			mFileInfo;
		sizetype			mLineNO;
		sizetype			mRequestedBytes;
#endif

#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_64BIT
		sizetype		padding;
#endif
		sizetype		mPageOffset;
		sizetype		mBlockBytes;
		//this must be the last member
		uint32			mIdentifier;
	};//struct SFixedSizeBlock
	static_assert( (sizeof(SFixedSizeBlock)&(BLADE_MEMORY_ALIGNMENT-1)) == 0, "size/aligment error");

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class BLADE_ALIGNED(BLADE_MEMORY_ALIGNMENT) FixedSizePage
	{
	public:
		typedef SFixedSizeBlock::sizetype sizetype;

		static const uint32 PAGE_IDENTIFIER = 0xFADE0ACE;
#if ENABLE_FIXEDSIZE_PAGE_ALLOC
		static const uint32 DEF_PAGE_SIZE = 4*1024;
		static const uint32 DEF_BLOCK_SIZE = 64*1024;
#else
		static const uint32 DEF_PAGE_SIZE = 64*1024;	//16*1024
#endif
		
		enum
		{
#if BLADE_MEMORY_DEBUG
			//add extra space for safe guard
			BLOCK_SIZE = sizeof(SFixedSizeBlock) + sizeof(ptrdiff_t),
#else
			BLOCK_SIZE = sizeof(SFixedSizeBlock),
#endif
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

			PF_CURRENT,
		};

		FixedSizePage();
		~FixedSizePage();

		/** @brief init the page */
		bool		initialize(size_t FixedBlockSizeBytes);

		/** @brief allocate a block of memory */
		void*		allocate(size_t bytes, int type, const char* sourcefile,int lineNO);

		/** @brief free a block of memory allocated */
		void		deallocate(SFixedSizeBlock* block);

		/** @brief whether could be use */
		inline bool		hasFreeSpace() const { return mCurrentBlocks != 0; }

		/** @brief free space that are not convert into free blocks or used blocks */
		inline size_t	getFreeBytes() const 
		{
			return mLeftBytes > BLOCK_SIZE ? mLeftBytes - BLOCK_SIZE : 0;
		}

		/** @brief get the allocatable bytes left */
		inline size_t	getMaxAllocationBytes() const { return mCurrentBlocks*mUnitSize; }

		/** @brief whether the page has no blocks allocated */
		inline bool		isEmpty() const { return mCurrentBlocks == mTotalBlocks; }

		/** @brief  lock the page ,mostly for deallocation */
		inline void		lockPage() { mPageLock.lock(); }

		/** @brief try lock the page for allocation,if locked successfully,return true */
		inline bool		tryLockPage() { return mPageLock.tryLock(); }

		/** @brief unlock the page when allocation is done. */
		inline void		unlockPage() { mPageLock.unlock(); }

		/** @brief  */
		inline bool		isLocked() const { return mPageLock.isLocked(); }

		/** @brief check the safe id */
		inline bool		isValid() { return mIdentifier == PAGE_IDENTIFIER; }

		/** @brief set the page flag */
		inline void		setFlag(EPageFlag pf) { mFlag = pf; }

		/** @brief get the page flag */
		inline EPageFlag	getFlag() const { return mFlag; }

		//link list operation

		/** @brief  */
		inline FixedSizePage*	getNextLink() const { return mNext; }
		/** @brief  */
		inline FixedSizePage*	getPrevLink() const { return mPrev; }
		/** @brief  */
		void			setNextLink(FixedSizePage* next);
		/** @brief  */
		void			setPrevLink(FixedSizePage* prev);
		/** @brief link the page to another page's back */
		void			linkToBack(FixedSizePage* & link);
		/** @brief remove this from its list */
		void			detachFromList(FixedSizePage* & head);
		/** @brief  */
		void			dumpLeaks();

	protected:
		Lock				mPageLock;
		uint32				mIdentifier;
		EPageFlag			mFlag;

		char*				mAllocPtr;
		size_t				mLeftBytes;

		uint32				mTotalBlocks;
		uint32				mCurrentBlocks;

		SFixedSizeBlock*	mFreeBlockHead;
		SFixedSizeBlock*	mFreeBlockTail;

		uint32				mUnitSize;
		uint32				mUnitAllocSize;

		//link list
		FixedSizePage*		mPrev;
		FixedSizePage*		mNext;

#if BLADE_MEMORY_DEBUG
		SFixedSizeBlock*	mAllocatedLink;
#endif
	};

	static_assert((1uLL<<(sizeof(SFixedSizeBlock::sizetype)*8uLL)) >= FixedSizePage::DEF_PAGE_SIZE, "size exceeded");
 
}	//namespace Blade
 
#endif //__Blade_FixedSizePage_h__