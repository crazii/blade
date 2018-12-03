/********************************************************************
	created:	2009/02/12
	filename: 	StaticAllocator.h
	author:		Crazii
	
	purpose:	this allocator allocates memory for some object that with one-time consturction,
				without destruction(but until the application terminate)
*********************************************************************/
#ifndef __Blade_StaticAllocator_h__
#define __Blade_StaticAllocator_h__
#include "BladeMemory.h"
#include "AllocatorBase.h"

namespace Blade
{
	template<typename T >
	class StaticAllocator : public AllocatorBase< T >
	{
	public:

		///STL compatible typedefs
		typedef typename  AllocatorBase<T>::value_type value_type;
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;


		///rebind ,for STL compatibility
		template <typename _Other>
		struct rebind
		{
			typedef StaticAllocator<_Other> other;
		};


		///ctors & op=
		StaticAllocator()	{}

		StaticAllocator(const StaticAllocator<T>&){}

		template<typename _Other>
		StaticAllocator(const StaticAllocator<_Other>&){}

		template<typename _Other>
		StaticAllocator<T>& operator=(const StaticAllocator<_Other>&)
		{
			return *this;
		}

		///dector
		~StaticAllocator()
		{

		}

		///op== & op!=
		bool operator==(const StaticAllocator &) const
		{ return true; }
		bool operator!=(const StaticAllocator &) const
		{ return false; }

		///STL methods
		static pointer address(reference r)
		{
			return &r; 
		}

		static const_pointer address(const_reference s)
		{
			return &s;
		}

		static size_type max_size()
		{
			return SIZE_MAX;
		}

		void construct(const pointer ptr, const value_type & t)
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

			void* ptr = Memory::getStaticPool()->allocate( n*sizeof(T) );

			return static_cast<pointer>(ptr);
		}

		static pointer allocate(const size_type n, const void * const)
		{
			return allocate(n);
		}

		static void deallocate(const pointer ptr, const size_type)
		{
			Memory::getStaticPool()->deallocate(ptr);
		}

	};//class StaticAllocator


}//namespace Blade


#endif // __Blade_StaticAllocator_h__