/********************************************************************
	created:	2009/02/08
	filename: 	IPool.h
	author:		Crazii
	
	purpose:	memory IPool interface class
				an pool is abstraction for all memory allocation strategies, 
				by implementation, it may not be a (pool) but a common heap or something else.
*********************************************************************/
#ifndef __Blade_IPool_h__
#define __Blade_IPool_h__
#include <BladeTypes.h>

#if !defined(BLADE_MEMORY_DEBUG)
#	if BLADE_DEBUG
#		define BLADE_MEMORY_DEBUG 1
#	else
#		define BLADE_MEMORY_DEBUG 0
#	endif
#endif

namespace Blade
{

	class IPool
	{
	public:
		struct SPoolState
		{
			size_t	mAllocationCount;
			size_t	mAllocatedBytes;	///requested bytes
			size_t	mAllocatedTotal;	///requested bytes + corresponding overhead(pools themselves used)
			size_t	mFreeBytes;			///available memory for allocation (exclude overhead)left
			size_t	mFreeTotal;			///all left memory including overhead if all left memory allocated
			///mFreeTotal+mAllocatedTotal is the total space of the pool
			///note: mFreeTotal may equal to mFreeBytes if left overhead cannot be simply evaluated
		};

		///built-in allocation type
		typedef enum EPoolAllocationType
		{
			PAT_DEFAULT		= 0x0,
			PAT_NEW			= 0x1,
			PAT_NEW_ARRAY	= 0x2,

			PAT_MASK		= 0xF,
			///user can use 0x3-0xF as custom type to verify the type matching of allocation/deallocation.
		}ALLOC_TYPE;

		///the default setting for different pool implementation maybe different, but this value is recommended
		static const size_t DEFAULT_SETTING = 0;

		virtual	~IPool() {}

		/** @brief kbytes is the allocating IPool size */
		virtual void	initialize(size_t threadCount = DEFAULT_SETTING,size_t unit_size = DEFAULT_SETTING,size_t kbytes = DEFAULT_SETTING) = 0;

		/** @brief debug version of allocate
		type is 4 bit for verify new/delete v.s. new[]/delete[] v.s. other malloc routine, and custom type verification.
		*/
		virtual void*	allocate(size_t bytes, const char* sourcefile, int lineNO, int type = PAT_DEFAULT) = 0;
		inline void*	allocate(size_t bytes, int type)
		{
			return this->allocate(bytes, NULL, 0, type);
		}
		inline void*	allocate(size_t bytes)
		{
			return this->allocate(bytes, NULL, 0);
		}

		/** @brief re-allocate memory */
		virtual void*	reallocate(void* ptr, size_t bytes, const char* sourcefile, int lineNO, int type = PAT_DEFAULT) = 0;
		inline void* reallocate(void* ptr, size_t bytes, int type)
		{
			return this->reallocate(ptr, bytes, NULL, 0, type);
		}

		/** @brief fee a block of memory allocated */
		virtual void	deallocate(void* p, int type = PAT_DEFAULT) = 0;

		inline void	deallocate(void* p, const char*, int, int type = PAT_DEFAULT)
		{
			return this->deallocate(p, type);
		}

		/** @brief free all memory */
		virtual void	release() = 0;

		/** @brief profiling */
		virtual bool	getPoolState(SPoolState& outState) = 0;

	};//class IPool

}//namespace Blade


#if BLADE_MEMORY_DEBUG
#	define BLADE_POOL_ALLOCATE(_pool,_size) _pool->allocate(_size, __FILE__,__LINE__)
#	define BLADE_POOL_REALLOCATE(_pool, _ptr, _size) _pool->reallocate(_ptr, _size, __FILE__,__LINE__)
#	define BLADE_POOL_DEALLOCATE(_pool,_ptr) _pool->deallocate(_ptr, __FILE__,__LINE__)
#	define BLADE_POOL_ALLOCATE_DEBUG(_pool,_ptr,_file,_line) _pool->allocate(_ptr, _file,_line)
#else
#	define BLADE_POOL_ALLOCATE(_pool,_size) _pool->allocate(_size)
#	define BLADE_POOL_REALLOCATE(_pool, _ptr, _size) _pool->reallocate(_ptr, _size)
#	define BLADE_POOL_DEALLOCATE(_pool,_ptr) _pool->deallocate(_ptr)
#	define BLADE_POOL_ALLOCATE_DEBUG(_pool,_ptr,_file,_line) _pool->allocate(_ptr)
#endif

#endif // __Blade_IPool_h__
