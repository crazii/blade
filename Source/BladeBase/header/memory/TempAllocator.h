/********************************************************************
	created:	2009/02/13
	filename: 	TempAllocator.h
	author:		Crazii

	for containers whose elements frequently change
	purpose:	this allocator allocates memory for temporary objects
*********************************************************************/
#ifndef __Blade_TempAllocator_h__
#define __Blade_TempAllocator_h__
#include "BladeMemory.h"
#include "AllocatorBase.h"

namespace Blade
{
	template<typename T >
	class TempAllocator : public AllocatorBase< T >
	{
	public:

		///STL compatible typedefs
		typedef typename AllocatorBase<T>::value_type value_type;
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
			typedef TempAllocator<_Other> other;
		};


		///ctors & op=
		TempAllocator()	{}

		TempAllocator(const TempAllocator<T>&){}

		template<typename _Other>
		TempAllocator(const TempAllocator<_Other>&){}

		template<typename _Other>
		TempAllocator<T>& operator=(const TempAllocator<_Other>&)
		{
			return *this;
		}

		///dector
		~TempAllocator()
		{

		}

		///op== & op!=
		bool operator==(const TempAllocator &) const
		{ return true; }
		bool operator!=(const TempAllocator &) const
		{ return false; }

		///STL methods
		static pointer address(reference r)
		{
			return &r; 
		}

		static inline const_pointer address(const_reference s)
		{
			return &s;
		}

		static inline size_type max_size()
		{
			return SIZE_MAX;
		}

		static inline void construct(const pointer ptr, const value_type & t)
		{
			new (ptr) T(t);
		}

		static inline void destroy(const pointer ptr)
		{
			ptr->~T();
			BLADE_UNREFERENCED(ptr); // avoid unused variable warning
		}

		static inline pointer allocate(const size_type n)
		{
			void* ptr = Memory::getTemporaryPool()->allocate( n*sizeof(T) );
			return static_cast<pointer>(ptr);
		}

		static inline pointer allocate(const size_type n, const void * const)
		{
			return TempAllocator::allocate(n);
		}

		static inline void deallocate(const pointer ptr, const size_type)
		{
			Memory::getTemporaryPool()->deallocate(ptr);
		}

	};//class TempAllocator

}//namespace Blade


#endif // __Blade_TempAllocator_h__