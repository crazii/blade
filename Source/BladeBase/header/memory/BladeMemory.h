/********************************************************************
	created:	2010/03/29
	filename: 	BladeMemory.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeMemory_h__
#define __Blade_BladeMemory_h__
#include <BladeBase.h>
#include <interface/IPool.h>
#include <Lock.h>

namespace Blade
{
	///pre-defined global pools
	///if you need custom pool, use PoolFactory to create a pool, and register it with IPoolManager for other to access
	class BLADE_BASE_API Memory : public NonConstructable
	{
	public:
		static const size_t ALIGNMENT = BLADE_MEMORY_ALIGNMENT;

		/**
		* @describe get a pool with specified size
		* @param [in] allocSize size of memory block that the pool could be able to allocate
		* @return request pool interface
		* @remark this function will check the size-map registry,if there's a pool exist, \n
		* it will return it directly, if not exist, it will create a new pool ,and initialize it, \n
		* by calling IPool::initialize(allocSize)
		*/
		static IPool*	requestPoolWithSize(size_t allocSize);


		/**
		* @describe get the global temporary pool
		* @param none
		* @return pool
		*/
		static IPool*	getTemporaryPool();


		/**
		* @describe get the global static pool
		* @param none
		* @return pool
		@note: the static pool is for static persistent memory (singleton, persistent data etc), it actually does not recycle the freed memory
		so be cautious when use it
		*/
		static IPool*	getStaticPool();

		/**
		@describe get the global common pool for all dynamic sized allocation, especially for arrays, vectors, and resources
		@param 
		@return 
		*/
		static IPool*	getResourcePool();

		/**
		@describe enable/disable memory dump. only valid if BLADE_MEMORY_DEBUG is on
		@param
		@return
		*/
		static void		enableLeakDump(bool enable);

		/**
		@describe 
		@param
		@return
		*/
		static bool		isLeakDumpEnabled();

	};//class MemoryUtil

	//add dummy param to avoid conflict with other lib
	typedef struct SMemoryInfo
	{
	}MEMINFO;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BaseAllocatable
	{
	protected:
		inline static void* operator new(size_t size, void* ptr)
		{
			BLADE_UNREFERENCED(size);
			return ptr;
		}
		inline static void* operator new(size_t size, IPool* place)
		{
			assert(place != NULL);
			return allocate(place, size, IPool::PAT_NEW);
		}
		inline static void*	operator new(size_t size, IPool* place, const char* sourcefile, int lineNO)
		{
			assert(place != NULL);
			return allocate(place, size, sourcefile, lineNO, IPool::PAT_NEW);
		}
		inline static void* operator new[](size_t size, IPool* place)
		{
			assert(place != NULL);
			return allocate(place, size, IPool::PAT_NEW_ARRAY);
		}
		inline static void*	operator new[](size_t size, IPool* place, const char* sourcefile, int lineNO)
		{
			assert(place != NULL);
			return allocate(place, size, sourcefile, lineNO, IPool::PAT_NEW_ARRAY);
		}
	public:
		inline static void	operator delete(void* ptr, void* place)
		{
			BLADE_UNREFERENCED(ptr);
			BLADE_UNREFERENCED(place);
		}
		inline static void operator delete(void* ptr, IPool*)
		{
			return operator delete(ptr);
		}
		inline static void		operator delete(void* ptr, IPool* /*place*/, const char* /*sourcefile*/, int /*lineNO*/)
		{
			return operator delete(ptr);
		}
		inline static void operator delete[](void* ptr, IPool* /*place*/)
		{
			return operator delete[](ptr);
		}
		inline static void		operator delete[](void* ptr, IPool* /*place*/, const char* /*sourcefile*/, int /*lineNO*/)
		{
			return operator delete[](ptr);
		}
		inline static void		operator delete(void* ptr)
		{
			return deallocate(ptr, IPool::PAT_NEW);
		}
		inline static void		operator delete[](void* ptr)
		{
			return deallocate(ptr, IPool::PAT_NEW_ARRAY);
		}

		inline static void		operator delete(void* ptr, const MEMINFO*)
		{
			return operator delete(ptr);
		}
		inline static void		operator delete(void* ptr, const char* /*sourcefile*/, int /*lineNO*/, const MEMINFO*)
		{
			return operator delete(ptr);
		}
		inline static void		operator delete[](void* ptr, const MEMINFO*)
		{
			return operator delete[](ptr);
		}
		inline static void		operator delete[](void* ptr, const char* /*sourcefile*/, int /*lineNO*/, const MEMINFO*)
		{
			return operator delete[](ptr);
		}
	protected:
		/** @brief  */
		BLADE_BASE_API static void deallocate(void* ptr, IPool::ALLOC_TYPE type);
		/** @brief  */
		BLADE_BASE_API static void* allocate(IPool* pool, size_t bytes, const char* sourcefile, int lineNO, IPool::ALLOC_TYPE type);
		/** @brief  */
		BLADE_BASE_API static void* allocate(IPool* pool, size_t bytes, IPool::ALLOC_TYPE type);
	};

	/************************************************************************/
	/* fixed size allocation                                                                     */
	/************************************************************************/
	//note: previous version Allocatable<T> won't have same sub objects since its template.
	//be ware that this class is not template and if base/members inherit the same class, its size may be larger than expected
	class Allocatable : public BaseAllocatable
	{
		friend class MemInit;
	public:
		using BaseAllocatable::operator new;
		using BaseAllocatable::operator delete;
		using BaseAllocatable::operator new[];
		using BaseAllocatable::operator delete[];

		///max size for fixed size pooling
		static const size_t MAX_SIZE = 512;
		static const size_t MAX_POOLCOUNT = MAX_SIZE / Memory::ALIGNMENT;

		///common operator new override
		inline static void* operator new(size_t size)
		{
			size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT; //impl detail: extra bytes for pool pointer
			return BaseAllocatable::operator new(size, Allocatable::getPool(size + extra));
		}
		inline static void* operator new(size_t size, const MEMINFO*)
		{
			size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
			return BaseAllocatable::operator new(size, Allocatable::getPool(size + extra));
		}
		///debug version
		inline static void* operator new(size_t size, const char* sourcefile, int lineNO, const MEMINFO*)
		{
			size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
			return BaseAllocatable::operator new(size, Allocatable::getPool(size + extra), sourcefile, lineNO);
		}

		inline static void*	operator new[](size_t size)
		{
			size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
			return BaseAllocatable::operator new[](size, Allocatable::getPool(size + extra));
		}
		inline static void*	operator new[](size_t size, const MEMINFO*)
		{
			size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
			return BaseAllocatable::operator new[](size, Allocatable::getPool(size + extra));
		}
		inline static void*	operator new[](size_t size, const char* sourcefile, int lineNO, const MEMINFO*)
		{
			size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
			return BaseAllocatable::operator new[](size, Allocatable::getPool(size + extra), sourcefile, lineNO);
		}

		/**
		@describe fixed sized pool allocation helper
		@param
		@return
		*/
		template<size_t FIXED_SIZE>
		static inline void*	allocate(size_t bytes, int type = IPool::PAT_DEFAULT)
		{
			IPool* pool = Allocatable::getPool<FIXED_SIZE>();
			assert(pool != NULL);
			return pool->allocate(bytes, type);
		}
		template<size_t FIXED_SIZE>
		static inline void	deallocate(void* ptr, int type = IPool::PAT_DEFAULT)
		{
			IPool* pool = Allocatable::getPool<FIXED_SIZE>();
			assert(pool != NULL);
			return pool->deallocate(ptr, type);
		}
		template<size_t FIXED_SIZE>
		static inline void*	allocate(size_t bytes, const char* source, int line, int type = IPool::PAT_DEFAULT)
		{
			IPool* pool = Allocatable::getPool<FIXED_SIZE>();
			assert(pool != NULL);
			return pool->allocate(bytes, source, line, type);
		}
		template<size_t FIXED_SIZE>
		static inline void	deallocate(void* ptr, const char* source, int line, int type = IPool::PAT_DEFAULT)
		{
			IPool* pool = Allocatable::getPool<FIXED_SIZE>();
			assert(pool != NULL);
			return pool->deallocate(ptr, source, line, type);
		}

		/** @brief  */
		template<size_t FIXED_SIZE>
		static inline IPool*	getPool()
		{
			#if BLADE_COMPILER == BLADE_COMPILER_MSVC
			#pragma warning(push)
			#pragma warning(disable:4127)
			#endif
			if (FIXED_SIZE < MAX_SIZE)
			{
				index_t index = (FIXED_SIZE + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT - 1;
				assert(index < MAX_POOLCOUNT);
				if (msPoolList[index] == NULL)
				{
					IPool*volatile* pPool = &msPoolList[index];
					ScopedLock lock(msLockList[index]);
					if (*pPool == NULL)
						*pPool = Memory::requestPoolWithSize((index + 1)*Memory::ALIGNMENT);
				}
				return msPoolList[index];
			}
			else
				return Memory::getResourcePool();
			#if BLADE_COMPILER == BLADE_COMPILER_MSVC
			#pragma warning(pop)
			#endif
		}

		/** @brief  */
		static inline IPool*	getPool(size_t bytes)
		{
			if (bytes < MAX_SIZE)
			{
				index_t index = (bytes + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT - 1;
				assert(index < MAX_POOLCOUNT);
				IPool*volatile* pPool = &msPoolList[index];
				IPool* result;
				if ((result=*pPool) == NULL)
				{
					ScopedLock lock(msLockList[index]);
					if ((result=*pPool) == NULL)
						result = *pPool = Memory::requestPoolWithSize((index + 1)*Memory::ALIGNMENT);
				}
				return result;
			}
			else
				return Memory::getResourcePool();
		}

	private:
		static BLADE_BASE_API IPool*		msPoolList[MAX_POOLCOUNT];
		static BLADE_BASE_API StaticLock	msLockList[MAX_POOLCOUNT];
	};//class Allocatable

	/************************************************************************/
	/* var sized allocation                                                                     */
	/************************************************************************/
	class ResourceAllocatable : public BaseAllocatable
	{
		friend class MemInit;
	public:
		using BaseAllocatable::operator new;
		using BaseAllocatable::operator delete;
		using BaseAllocatable::operator new[];
		using BaseAllocatable::operator delete[];

		inline static void*	operator new(size_t size)
		{
			return BaseAllocatable::operator new (size, ResourceAllocatable::getPool());
		}
		inline static void*	operator new(size_t size,const MEMINFO*)
		{
			return BaseAllocatable::operator new (size, ResourceAllocatable::getPool());
		}
		inline static void*	operator new(size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return BaseAllocatable::operator new(size, ResourceAllocatable::getPool(), sourcefile, lineNO);
		}
		inline static void*	operator new[](size_t size)
		{
			return BaseAllocatable::operator new[] (size, ResourceAllocatable::getPool());
		}
		inline static void*	operator new[](size_t size,const MEMINFO*)
		{
			return BaseAllocatable::operator new[](size, ResourceAllocatable::getPool());
		}
		inline static void*	operator new[](size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return BaseAllocatable::operator new[](size, ResourceAllocatable::getPool(), sourcefile, lineNO);
		}
		/** @brief make variable local to client & inline boost */
		static inline IPool* getPool()
		{
			return msPool;
		}
	private:
		static BLADE_BASE_API IPool*		msPool;
	};//class ResourceAllocatable

	/************************************************************************/
	/* static allocation                                                                     */
	/************************************************************************/
	class StaticAllocatable : public BaseAllocatable
	{
		friend class MemInit;
	public:
		using BaseAllocatable::operator new;
		using BaseAllocatable::operator delete;
		using BaseAllocatable::operator new[];
		using BaseAllocatable::operator delete[];

		inline static void*	operator new(size_t size)
		{
			return BaseAllocatable::operator new (size, StaticAllocatable::getPool());
		}
		inline static void*	operator new(size_t size, const MEMINFO*)
		{
			return BaseAllocatable::operator new (size, StaticAllocatable::getPool());
		}
		inline static void*	operator new(size_t size, const char* sourcefile, int lineNO, const MEMINFO*)
		{
			return BaseAllocatable::operator new(size, StaticAllocatable::getPool(), sourcefile, lineNO);
		}
		inline static void*	operator new[](size_t size)
		{
			return BaseAllocatable::operator new[](size, StaticAllocatable::getPool());
		}
		inline static void*	operator new[](size_t size, const MEMINFO*)
		{
			return BaseAllocatable::operator new[](size, StaticAllocatable::getPool());
		}
		inline static void*	operator new[](size_t size, const char* sourcefile, int lineNO, const MEMINFO*)
		{
			return BaseAllocatable::operator new[](size, StaticAllocatable::getPool(), sourcefile, lineNO);
		}
		/** @brief make variable local to client & inline boost */
		static inline IPool* getPool()
		{
			return msPool;
		}
	private:
		static BLADE_BASE_API IPool*		msPool;
	};//class StaticAllocatable


	/************************************************************************/
	/* temporary allocation                                                                     */
	/************************************************************************/
	class TempAllocatable : public BaseAllocatable
	{
		friend class MemInit;
	public:
		using BaseAllocatable::operator new;
		using BaseAllocatable::operator delete;
		using BaseAllocatable::operator new[];
		using BaseAllocatable::operator delete[];

		inline static void*	operator new(size_t size)
		{
			return BaseAllocatable::operator new (size, TempAllocatable::getPool());
		}
		inline static void*	operator new(size_t size, const MEMINFO*)
		{
			return BaseAllocatable::operator new (size, TempAllocatable::getPool());
		}
		inline static void*	operator new(size_t size, const char* sourcefile, int lineNO, const MEMINFO*)
		{
			return BaseAllocatable::operator new(size, TempAllocatable::getPool(), sourcefile, lineNO);
		}
		inline static void*	operator new[](size_t size)
		{
			return BaseAllocatable::operator new[](size, TempAllocatable::getPool());
		}
		inline static void*	operator new[](size_t size, const MEMINFO*)
		{
			return BaseAllocatable::operator new[](size, TempAllocatable::getPool());
		}
		inline static void*	operator new[](size_t size, const char* sourcefile, int lineNO, const MEMINFO*)
		{
			return BaseAllocatable::operator new[](size, TempAllocatable::getPool(), sourcefile, lineNO);
		}
		/** @brief make variable local to client & inline boost */
		static inline IPool* getPool()
		{
			return msPool;
		}
	private:
		static BLADE_BASE_API IPool*		msPool;
	};//class TempAllocatable

	///aligned allocation
	//////////////////////////////////////////////////////////////////////////
	static inline void*	AlignMalloc(size_t size,size_t align, IPool* pool, const char* file, int line)
	{
		if( align < sizeof(uintptr_t) )
		{
			assert(false);
			align = sizeof(uintptr_t);
		}

		void* mem_real = pool->allocate(size + 2 * align, file, line, IPool::PAT_DEFAULT);
		if(!mem_real)
			return NULL;

		char* mem_align = (char*)((uintptr_t)(2 * align - (uintptr_t)mem_real % (uintptr_t)align) + (uintptr_t)mem_real);
		*((uintptr_t*)mem_align - 1) = (uintptr_t)mem_real;
		return mem_align;
	}
	//////////////////////////////////////////////////////////////////////////
	static inline void*	AlignMalloc(size_t size,size_t align, IPool* pool)
	{
		if( align < sizeof(uintptr_t) )
		{
			assert(false);
			align = sizeof(uintptr_t);
		}

		void* mem_real = pool->allocate(size + 2 * align, IPool::PAT_DEFAULT);
		if(!mem_real)
			return NULL;

		char* mem_align = (char*)((uintptr_t)(2 * align - (uintptr_t)mem_real % (uintptr_t)align) + (uintptr_t)mem_real);
		*((uintptr_t*)mem_align - 1) = (uintptr_t)mem_real;
		return mem_align;
	}
	//////////////////////////////////////////////////////////////////////////
	static inline void*	AlignRealloc(void* ptr, size_t size,size_t align, IPool* pool, const char* file, int line)
	{
		if( align < sizeof(uintptr_t) )
		{
			assert(false);
			align = sizeof(uintptr_t);
		}

		void* oldptr = ptr == NULL ? ptr : (void*)*((uintptr_t*)ptr - 1); 
		void* mem_real = pool->reallocate(oldptr, size + 2 * align, file, line, IPool::PAT_DEFAULT);
		if(!mem_real)
			return NULL;

		char* mem_align = (char*)((uintptr_t)(2 * align - (uintptr_t)mem_real % (uintptr_t)align) + (uintptr_t)mem_real);
		*((uintptr_t*)mem_align - 1) = (uintptr_t)mem_real;
		return mem_align;
	}
	//////////////////////////////////////////////////////////////////////////
	static inline void*	AlignRealloc(void* ptr, size_t size,size_t align, IPool* pool)
	{
		if( align < sizeof(uintptr_t) )
		{
			assert(false);
			align = sizeof(uintptr_t);
		}

		void* oldptr = ptr == NULL ? ptr : (void*)*((uintptr_t*)ptr - 1); 
		void* mem_real = pool->reallocate(oldptr, size + 2 * align, IPool::PAT_DEFAULT);
		if(!mem_real)
			return NULL;

		char* mem_align = (char*)((uintptr_t)(2 * align - (uintptr_t)mem_real % (uintptr_t)align) + (uintptr_t)mem_real);
		*((uintptr_t*)mem_align - 1) = (uintptr_t)mem_real;
		return mem_align;
	}
	//////////////////////////////////////////////////////////////////////////
	static inline void		AlignFree(void* ptr, IPool* pool)
	{
		if( ptr != NULL )
			pool->deallocate( (void*)*((uintptr_t*)ptr - 1) , IPool::PAT_DEFAULT);
	}

}//namespace Blade


/************************************************************************/
/* global operator new/delete                                                                     */
/************************************************************************/
#if BLADE_MEMORY_DEBUG

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API void*		operator new(size_t size,const char* sourcefile,int lineNO,const Blade::MEMINFO*);

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API	void		operator delete(void* ptr,const char* sourcefile,int lineNO,const Blade::MEMINFO*);

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API void*		operator new[](size_t size,const char* sourcefile,int lineNO,const Blade::MEMINFO*);

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API	void		operator delete[](void* ptr,const char* sourcefile,int lineNO,const Blade::MEMINFO*);

#	define BLADE_NEW new(__FILE__, __LINE__, (const Blade::MEMINFO*)NULL )
#	define BLADE_TRACE_NEW(_file, _line) new(_file, _line, (const Blade::MEMINFO*)NULL )

#else//BLADE_MEMORY_DEBUG

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API void*		operator new(size_t size, const Blade::MEMINFO*);

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API	void		operator delete(void* ptr, const Blade::MEMINFO*);

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API void*		operator new[](size_t size, const Blade::MEMINFO*);

//////////////////////////////////////////////////////////////////////////
BLADE_BASE_API	void		operator delete[](void* ptr, const Blade::MEMINFO*);

#	define BLADE_NEW new ( ((const Blade::MEMINFO*)NULL) )
#	define BLADE_TRACE_NEW(_file, _line) new( (const Blade::MEMINFO*)NULL )

#endif//BLADE_MEMORY_DEBUG

#define BLADE_DELETE delete

#if BLADE_MEMORY_DEBUG
#	define BLADE_POOL_ALIGN_ALLOC(_pool, _bytes, _align) Blade::AlignMalloc(_bytes,_align,_pool,__FILE__,__LINE__)
#	define BLADE_POOL_ALIGN_ALLOC_DEBUG(_pool, _bytes, _align, _file, _line) Blade::AlignMalloc(_bytes,_align,_pool,_file,_line)
#	define BLADE_POOL_ALIGN_REALLOC(_pool, _ptr, _bytes, _align) Blade::AlignRealloc(_ptr, _bytes,_align,_pool,__FILE__,__LINE__)
#	define BLADE_POOL_ALIGN_REALLOC_DEBUG(_pool, _ptr, _bytes, _align, _file, _line) Blade::AlignRealloc(_ptr, _bytes,_align,_pool,_file,_line)
#else
#	define BLADE_POOL_ALIGN_ALLOC(_pool, _bytes, _align) Blade::AlignMalloc(_bytes,_align,_pool)
#	define BLADE_POOL_ALIGN_ALLOC_DEBUG(_pool, _bytes, _align, _file, _line) Blade::AlignMalloc(_bytes,_align,_pool, _file, _line)
#	define BLADE_POOL_ALIGN_REALLOC(_pool, _ptr, _bytes, _align) Blade::AlignRealloc(_ptr, _bytes,_align,_pool)
#	define BLADE_POOL_ALIGN_REALLOC_DEBUG(_pool, _ptr, _bytes, _align, _file, _line) Blade::AlignRealloc(_ptr, _bytes,_align,_pool, _file, _line)
#endif

#define BLADE_POOL_ALIGN_FREE(_pool, _ptr) Blade::AlignFree(_ptr,_pool)


//////////////////////////////////////////////////////////////////////////
#define BLADE_RES_ALLOC(_bytes)					BLADE_POOL_ALLOCATE( Blade::Memory::getResourcePool(), _bytes )
#define BLADE_RES_REALLOC(_ptr, _bytes)			BLADE_POOL_REALLOCATE( Blade::Memory::getResourcePool(), _ptr, _bytes )
#define BLADE_RES_ALLOCT(_type,_count)			(_type*)BLADE_RES_ALLOC( sizeof(_type)*(_count) )
#define BLADE_RES_FREE(_ptr)					BLADE_POOL_DEALLOCATE( Blade::Memory::getResourcePool(), _ptr )
#define BLADE_RES_ALIGN_ALLOC(_bytes, _align)	BLADE_POOL_ALIGN_ALLOC( Blade::Memory::getResourcePool(), _bytes, _align)
#define BLADE_RES_ALIGN_REALLOC(_ptr, _bytes, _align)	BLADE_POOL_ALIGN_REALLOC( Blade::Memory::getResourcePool(), _ptr, _bytes, _align)
#define BLADE_RES_ALIGN_FREE(_ptr)				BLADE_POOL_ALIGN_FREE( Blade::Memory::getResourcePool(), _ptr)


#define BLADE_TMP_ALLOC(_bytes)					BLADE_POOL_ALLOCATE( Blade::Memory::getTemporaryPool(), _bytes)
#define BLADE_TMP_REALLOC(_ptr, _bytes)			BLADE_POOL_REALLOCATE( Blade::Memory::getTemporaryPool(), _ptr, _bytes )
#define BLADE_TMP_ALLOCT(_type,_count)			(_type*)BLADE_TMP_ALLOC( sizeof(_type)*(_count) )
#define BLADE_TMP_FREE(_ptr)					BLADE_POOL_DEALLOCATE( Blade::Memory::getTemporaryPool(),_ptr)
#define BLADE_TMP_ALIGN_ALLOC(_bytes, _align)	BLADE_POOL_ALIGN_ALLOC( Blade::Memory::getTemporaryPool(), _bytes, _align)
#define BLADE_TMP_ALIGN_REALLOC(_ptr, _bytes, _align)	BLADE_POOL_ALIGN_REALLOC( Blade::Memory::getTemporaryPool(), _ptr, _bytes, _align)
#define BLADE_TMP_ALIGN_FREE(_ptr)				BLADE_POOL_ALIGN_FREE( Blade::Memory::getTemporaryPool(), _ptr)

#define BLADE_STATIC_ALLOC(_bytes)				BLADE_POOL_ALLOCATE( Blade::Memory::getStaticPool(), _bytes)
#define BLADE_STATIC_REALLOC(_ptr, _bytes)		BLADE_POOL_REALLOCATE( Blade::Memory::getStaticPool(), _ptr, _bytes )
#define BLADE_STATIC_ALLOCT(_type,_count)		(_type*)BLADE_STATIC_ALLOC( sizeof(_type)*(_count) )
#define BLADE_STATIC_FREE(_ptr)					BLADE_POOL_DEALLOCATE( Blade::Memory::getStaticPool(),_ptr)
#define BLADE_STATIC_ALIGN_ALLOC(_bytes,_align)	BLADE_POOL_ALIGN_ALLOC( Blade::Memory::getStaticPool(), _bytes, _align)
#define BLADE_STATIC_ALIGN_REALLOC(_ptr, _bytes, _align)	BLADE_POOL_ALIGN_REALLOC( Blade::Memory::getStaticPool(), _ptr, _bytes, _align)
#define BLADE_STATIC_ALIGN_FREE(_ptr)			BLADE_POOL_ALIGN_FREE( Blade::Memory::getStaticPool(), _ptr)

#endif //  __Blade_BladeMemory_h__