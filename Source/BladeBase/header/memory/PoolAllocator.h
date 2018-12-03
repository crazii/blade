/********************************************************************
	created:	2011/06/06
	filename: 	PoolAllocator.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_PoolAllocator_h__
#define __Blade_PoolAllocator_h__
#include "BladeMemory.h"
#include <RefCount.h>
#include <StaticHandle.h>
#include <interface/IPoolManager.h>
#include <utility/BladeContainer.h>

namespace Blade
{
	typedef StaticHandle<IPool> SPOOL;
	
	template< typename T ,SPOOL& thePool >
	class PoolAllocator : public AllocatorBase< T >
	{
	public:
		//STL compatible typedefs
		typedef typename AllocatorBase<T>::value_type value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;


		//rebind ,for STL compatibility
		template <typename _Other>
		struct rebind
		{
			typedef PoolAllocator<_Other,thePool> other;
		};


		//ctors & op=
		PoolAllocator()	{}

		PoolAllocator(const PoolAllocator<T,thePool>& ){}

		template<typename _Other>
		PoolAllocator(const PoolAllocator<_Other,thePool>&){}

		template<typename _Other>
		PoolAllocator<T,thePool>& operator=(const PoolAllocator<_Other,thePool>&)
		{
			return *this;
		}


		//dector
		~PoolAllocator()
		{

		}

		//op== & op!=
		bool operator==(const PoolAllocator &) const
		{ return true; }
		bool operator!=(const PoolAllocator &) const
		{ return false; }

		//STL methods
		pointer address(reference r) const
		{
			return &r; 
		}

		const_pointer address(const_reference s) const
		{
			return &s;
		}

		static size_type max_size()
		{
			return SIZE_MAX;
		}

		static void construct(const pointer ptr, const value_type & t)
		{
			new (ptr) T(t);
		}

		static void destroy(const pointer ptr)
		{
			ptr->~T();
			BLADE_UNREFERENCED(ptr); // avoid unused variable warning
		}

		static pointer allocate(const size_type n)
		{
			return static_cast<pointer>( (thePool)->allocate( sizeof(T)*n ) );
		}

		static pointer allocate(const size_type n, const void * const)
		{
			return allocate(n);
		}

		static void deallocate(const pointer ptr, const size_type)
		{
			(thePool)->deallocate(ptr);
		}
	};//class PoolAllocator




	template <SPOOL& thePool>
	class PoolAllocatable
	{
		///this template have inheritable operator new/delete
	public:

		///common operator new override
		inline static void* operator new(size_t size,const MEMINFO*)
		{
			return (thePool)->allocate(size, IPool::PAT_NEW);
		}

		///common operator delete override
		inline static void operator delete(void *ptr,const MEMINFO*)
		{
			(thePool)->deallocate(ptr, IPool::PAT_NEW);
		}

		///common operator delete override
		inline static void operator delete(void *ptr)
		{
			(thePool)->deallocate(ptr, IPool::PAT_NEW);
		}

		///debug version
		inline static void* operator new(size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return (thePool)->allocate(size, sourcefile, lineNO, IPool::PAT_NEW);
		}

		///debug version
		inline void operator delete(void* ptr,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			(thePool)->deallocate(ptr, sourcefile, lineNO, IPool::PAT_NEW);
		}

		///operator new with IPool interface
		inline static void* operator new(size_t size,IPool* place)
		{
			assert( place != NULL );
			return place->allocate(size, IPool::PAT_NEW);
		}

		///operator delete with IPool
		inline static void operator delete(void* ptr,IPool* place)
		{
			assert( place != NULL );
			place->deallocate(ptr, IPool::PAT_NEW);
		}


		//below:
		///operator new[] and delete[]
		inline static void* operator new[](size_t size,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			return (thePool)->allocate(size, sourcefile, lineNO, IPool::PAT_NEW_ARRAY);
		}

		///common operator delete override
		inline static void operator delete[](void* ptr,const char* sourcefile,int lineNO,const MEMINFO*)
		{
			(thePool)->deallocate(ptr, sourcefile, lineNO, IPool::PAT_NEW_ARRAY);
		}

		inline static void* operator new[](size_t size,const MEMINFO*)
		{
			return (thePool)->allocate(size, IPool::PAT_NEW_ARRAY);
		}

		///common operator delete override
		inline static void operator delete[](void *ptr,const MEMINFO*)
		{
			(thePool)->deallocate(ptr, IPool::PAT_NEW_ARRAY);
		}

		///common operator delete override
		inline static void operator delete[](void *ptr)
		{
			(thePool)->deallocate(ptr, IPool::PAT_NEW_ARRAY);
		}

		///operator new with IPool
		inline static void* operator new[](size_t size,IPool* place)
		{
			assert( place != NULL );
			return place->allocate(size, IPool::PAT_NEW_ARRAY);
		}

		///operator delete with IPool
		inline static void operator delete[](void* ptr,IPool* place)
		{
			assert( place != NULL );
			place->deallocate(ptr, IPool::PAT_NEW_ARRAY);
		}
	};//template class PoolAllocatable


	template< SPOOL& thePool>
	class PoolBinding
	{
	public:

		template<typename T>
		class Allocator : public PoolAllocator<T,thePool>
		{
		public:
			Allocator()	{}
			Allocator(PoolAllocator<T,thePool>& src) :PoolAllocator<T,thePool>(src)	{}
		};

		typedef PoolAllocatable<thePool> PooledAllocatable;

		template<typename T>
		class PooledVector : public TempVector<T,PoolAllocator<T,thePool>,PooledAllocatable>{};

		template<typename T>
		class PooledQueue : public TempQueue<T,PoolAllocator<T,thePool>,PooledAllocatable>	{};

		template<typename T>
		class PooledList : public TempList<T,PoolAllocator<T,thePool>,PooledAllocatable>	{};

		template<typename T,typename C = std::less<T> >
		class PooledSet : public TempSet<T,C,PoolAllocator<T,thePool>,PooledAllocatable>	{};

		template<typename K,typename V,typename C = std::less<K> >
		class PooledMap : public TempMap<K,V,C,PoolAllocator<K,thePool>,PooledAllocatable>	{};
	};

	///note this class is not exported, so each of the client libs will has the class

	namespace
	{
		class PoolSpace
		{
		protected:
			static	SPOOL		msPool;
			static	RefCount	msPoolRef;

		public:
			/** @brief  */
			inline static void createPool()
			{
				if( msPool == NULL )
				{
					//TODO: double check lock
					msPool.bind( BLADE_FACTORY_CREATE(IPool,BTString("Incremental") ) );
					msPool->initialize();
				}
				msPoolRef.safeIncrement();
			}

			/** @brief  */
			inline static void releasePool()
			{
				if( msPoolRef.safeDecrement() == 0 )
					msPool.clear();
			}

			/** @brief  */
			static IPool*	getPool()
			{
				return msPool;
			}

			/** @brief  */
			static void		setPool(IPool* pool)
			{
				msPool.unbind();
				msPool.bind( pool );
			}

			typedef PoolBinding<msPool>	Binding;
		};//class PoolSpace

		SPOOL PoolSpace::msPool = SPOOL( SPOOL::Static() );

		RefCount PoolSpace::msPoolRef;
	}//anonymous name space

}//namespace Blade



#endif // __Blade_PoolAllocator_h__