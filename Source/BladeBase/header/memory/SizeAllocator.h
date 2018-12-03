/********************************************************************
	created:	2009/02/09
	filename: 	SizeAllocator.h
	author:		Crazii
	
	purpose:	STL compatible allocator with memory IPool feature
*********************************************************************/
#ifndef __Blade_SizeAllocator_h__
#define __Blade_SizeAllocator_h__
#include "BladeMemory.h"
#include "AllocatorBase.h"

namespace Blade
{
	template< typename T >
	class SizeAllocator : public AllocatorBase< T >
	{
	public:
		static const size_t ALLOC_SIZE = sizeof(T);
	public:
		///STL compatible typedefs
		typedef typename AllocatorBase<T>::value_type value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		/** @brief rebind ,for STL compatibility */
		template <typename _Other>
		struct rebind
		{
			typedef SizeAllocator<_Other> other;
		};

		/** @brief  */
		inline SizeAllocator()	{}

		/** @brief  */
		inline SizeAllocator(const SizeAllocator<T>&){}

		/** @brief  */
		template<typename _Other>
		inline SizeAllocator(const SizeAllocator<_Other>&){}

		/** @brief  */
		template<typename _Other>
		inline SizeAllocator<T>& operator=(const SizeAllocator<_Other>&)	{return *this;}

		/** @brief  */
		inline ~SizeAllocator(){}

		/** @brief op== & op!= */
		inline bool operator==(const SizeAllocator &) const	{return true;}
		inline bool operator!=(const SizeAllocator &) const	{return false;}

		///STL methods
		inline pointer address(reference r) const
		{
			return &r;
		}

		/** @brief  */
		inline const_pointer address(const_reference s) const
		{
			return &s;
		}

		/** @brief  */
		static inline size_type max_size()
		{
			return SIZE_MAX;
		}

		/** @brief  */
		static inline void construct(const pointer ptr, const value_type & t)
		{
			new (ptr) T(t);
		}

		/** @brief  */
		static inline void destroy(const pointer ptr)
		{
			ptr->~T();
			BLADE_UNREFERENCED(ptr); // avoid unused variable warning
		}

		/** @brief  */
		static inline pointer allocate(const size_type n)
		{
			return static_cast<pointer>( Allocatable::allocate<sizeof(T)>( n*sizeof(T) ) );
		}

		/** @brief  */
		static inline pointer allocate(const size_type n, const void * const)
		{
			return SizeAllocator::allocate(n);
		}

		/** @brief  */
		static inline void deallocate(const pointer ptr, const size_type)
		{
			Allocatable::deallocate<sizeof(T)>(ptr);
		}
	};//class SizeAllocator


	///allocator for vector. Don't use fixed size since vector size may vary and need re-allocation
	template< typename T >
	class VectorAllocator : public AllocatorBase< T >
	{
	public:
		static const size_t ALLOC_SIZE = sizeof(T);
	public:
		///STL compatible typedefs
		typedef typename AllocatorBase<T>::value_type value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		/** @brief rebind ,for STL compatibility */
		template <typename _Other>
		struct rebind
		{
			typedef VectorAllocator<_Other> other;
		};

		/** @brief  */
		inline VectorAllocator() {}

		/** @brief  */
		inline VectorAllocator(const VectorAllocator<T>&) {}

		/** @brief  */
		template<typename _Other>
		inline VectorAllocator(const VectorAllocator<_Other>&) {}

		/** @brief  */
		template<typename _Other>
		inline VectorAllocator<T>& operator=(const VectorAllocator<_Other>&) { return *this; }

		/** @brief  */
		inline ~VectorAllocator() {}

		/** @brief op== & op!= */
		inline bool operator==(const VectorAllocator &) const { return true; }
		inline bool operator!=(const VectorAllocator &) const { return false; }

		///STL methods
		inline pointer address(reference r) const
		{
			return &r;
		}

		/** @brief  */
		inline const_pointer address(const_reference s) const
		{
			return &s;
		}

		/** @brief  */
		static inline size_type max_size()
		{
			return SIZE_MAX;
		}

		/** @brief  */
		static inline void construct(const pointer ptr, const value_type & t)
		{
			new (ptr) T(t);
		}

		/** @brief  */
		static inline void destroy(const pointer ptr)
		{
			ptr->~T();
			BLADE_UNREFERENCED(ptr); // avoid unused variable warning
		}

		/** @brief  */
		static inline pointer allocate(const size_type n)
		{
			return static_cast<pointer>(Allocatable::operator new(ALLOC_SIZE*n));
			//return static_cast<pointer>(Memory::getResourcePool()->allocate(ALLOC_SIZE*n));
		}

		/** @brief  */
		static inline pointer allocate(const size_type n, const void * const)
		{
			return VectorAllocator::allocate(n);
		}

		/** @brief  */
		static inline void deallocate(const pointer ptr, const size_type)
		{
			//Memory::getResourcePool()->deallocate(ptr);
			return Allocatable::operator delete(ptr);
		}
	};//class VectorAllocator

}//namespace Blade


#endif // __Blade_SizeAllocator_h__