/********************************************************************
	created:	2009/02/28
	filename: 	Handle.h
	author:		Crazii
	
	purpose:	this is a smart-pointer template
*********************************************************************/
#ifndef __Blade_Handle_h__
#define __Blade_Handle_h__
#include <Lock.h>
#include <RefCount.h>
#include <memory/BladeMemory.h>

namespace Blade
{
	template < typename T >
	class Handle
	{
	public:
		typedef T value_type;
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;

		///notice:
		///for more safely use,copy function and construction from a naked pointer is not supported.
		///there is a "bind" method alternatively,only use it if necessary
		explicit Handle(pointer ptr)
		{
			if( ptr != NULL )
			{
				ScopedLock _lock(mLock);

				this->mPtr = ptr;
				this->mRefcount = BLADE_NEW RefCount(1);
			}
		}

		///ctor & dector
		Handle() throw() :mPtr(NULL),mRefcount(NULL) {}

		Handle(const Handle& rhs) throw()
		{
			ScopedLock rhsLock(rhs.mLock);

			this->mPtr = rhs.mPtr;
			this->mRefcount = rhs.mRefcount;
			this->incRef();
		}

		~Handle() throw()
		{
			this->clear();
		}

		///operators
		Handle& operator = (const Handle& rhs) throw()
		{
			if( this == &rhs )
				return *this;

			ScopedLock _lock(mLock);
			ScopedLock rhsLock(rhs.mLock);
			if( this->mPtr != rhs.mPtr )
			{
				this->decRef();
				this->mPtr = rhs.mPtr;
				this->mRefcount = rhs.mRefcount;
				this->incRef();
			}

			return *this;
		}

		///notice: bind a pointer to the handle
		///one same pointer can only be binded to a handle once
		void bindSafe(pointer naked_pointer) throw()
		{
			ScopedLock _lock(mLock);

			this->decRef();
			this->mPtr = naked_pointer;
			this->mRefcount = BLADE_NEW RefCount(1);
		}

		/** @brief  */
		void bind(pointer naked_pointer) throw()
		{
			this->decRef();
			this->mPtr = naked_pointer;
			this->mRefcount = BLADE_NEW RefCount(1);
		}

		/** @brief  */
		///could unbind only if ref counting is 1
		pointer unbindSafe() throw()
		{
			ScopedLock _lock(mLock);

			assert( (this->mPtr == NULL && this->mRefcount == NULL) || (this->mPtr != NULL && this->mRefcount != NULL));

			if( this->mRefcount == NULL && this->mPtr == NULL )
				return NULL;	

			pointer ptr = this->mPtr;
			this->mPtr = NULL;

			if( mRefcount != NULL )
			{
				assert( *mRefcount == 1 );
				BLADE_DELETE mRefcount;
				this->mRefcount = NULL;
			}
			return ptr;
		}

		/** @brief  */
		pointer unbind() throw()
		{
			assert( (this->mPtr == NULL && this->mRefcount == NULL) || (this->mPtr != NULL && this->mRefcount != NULL));

			if( this->mRefcount == NULL && this->mPtr == NULL )
				return NULL;

			pointer ptr = this->mPtr;
			this->mPtr = NULL;

			if( mRefcount != NULL )
			{
				assert( *mRefcount == 1 );
				BLADE_DELETE mRefcount;
				this->mRefcount = NULL;
			}
			return ptr;
		}

		/** @brief  */
		void clear() throw()
		{
			ScopedLock _lock(mLock);

			if( mPtr == NULL )
				return;		

			this->decRef();

			mRefcount = NULL;
			mPtr = NULL;
		}

		/** @brief  */
		bool operator == ( const Handle& rhs) const throw()
		{
			return this->mPtr == rhs.mPtr;
		}

		/** @brief  */
		bool operator == ( const pointer rhs ) const throw()
		{
			return this->mPtr == rhs;
		}

		/** @brief  */
		bool operator != ( const pointer rhs ) const throw()
		{
			return this->mPtr != rhs;
		}

		///compare helper, this helps some STL containers sort by the address
		bool operator < ( const Handle& rhs) const throw()
		{
			return this->mPtr < rhs.mPtr;
		}

		/** @brief  */
		bool operator > ( const Handle& rhs) const throw()
		{
			return this->mPtr > rhs.mPtr;
		}

		/** @brief  */
		operator pointer(void) throw()
		{
			return this->mPtr;
		}

		/** @brief  */
		operator pointer(void) const throw()
		{
			return this->mPtr;
		}

		/** @brief  */
		template<typename Other>
		operator Other*() throw()
		{
			return static_cast<Other*>(mPtr);
		}

		/** @brief  */
		template<typename Other>
		operator Other*() const throw()
		{
			return static_cast<Other*>(mPtr);
		}

		/** @brief  */
		template<typename Other>
		operator Handle<Other>() const throw()
		{
			return this->staticCast<Other>();
		}

		pointer	ptr() const throw()
		{
			return this->mPtr;
		}

		pointer ptr() throw()
		{
			return this->mPtr;
		}

		/** @brief  */
		pointer operator-> (void) throw()
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		reference operator * (void) throw()
		{
			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		pointer operator -> (void) const throw()
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		reference operator * (void) const  throw()
		{
			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		///Handle's pointer initializer
		template< typename _Cty >
		const Handle& constructInstance(void) throw()
		{
			assert( this->mPtr == NULL && this->mRefcount == NULL );
			ScopedLock _lock(mLock);

			if( mPtr == NULL && mRefcount == NULL )
			{
				this->mPtr = static_cast<T*>(BLADE_NEW _Cty);
				this->mRefcount = BLADE_NEW RefCount(1);
			}
			return *this;
		}

		///Handle's pointer initializer
		template< typename _Cty >
		const Handle& constructInstance(const _Cty& src) throw()
		{
			assert( this->mPtr == NULL && this->mRefcount == NULL );
			ScopedLock _lock(mLock);

			if( mPtr == NULL && mRefcount == NULL )
			{
				this->mPtr = static_cast<T*>(BLADE_NEW _Cty(src) );
				this->mRefcount = BLADE_NEW RefCount(1);
			}
			return *this;
		}

		///type casting
		template< typename _Other >
		Handle<_Other> staticCast(void) const throw()
		{	//static cast
			Handle<_Other> other_handle;
			ScopedLock _lock(mLock);

			other_handle.mPtr = static_cast<_Other*>(this->mPtr);
			other_handle.mRefcount = this->mRefcount;

			//this is NULL?
			if( other_handle.mPtr != NULL )
				other_handle.incRef();

			return other_handle;
		}

		/** @brief dynamic casting to other handle type */
		template< typename _Other >
		Handle<_Other> dynamicCast(void) throw()
		{	//dynamic cast
			Handle<_Other> other_handle;

			ScopedLock _lock(mLock);

			other_handle.mPtr = dynamic_cast<_Other*>(this->mPtr);
			//check if the dynamic cast is ok.
			if( other_handle.mPtr != NULL )
			{
				other_handle.mRefcount = this->mRefcount;
				other_handle.incRef();
			}

			return other_handle;
		}

		/** @brief  */
		///for thread-safe handle initialization
		void	lock() throw()
		{
			mLock.lock();
		}

		/** @brief  */
		void	unlock() throw()
		{
			mLock.unlock();
		}

		/** @brief  */
		size_t	refcount() const  throw()
		{
			return mRefcount == NULL ? 0 : mRefcount->count();
		}

		/** @brief  */
		RefCount*	getRefCount() const  throw()
		{
			return mRefcount;
		}

		/** @brief  */
		Lock&		getLock() const throw()
		{
			return mLock;
		}

	protected:

		///friend declaration needed for static_cast and dynamic_cast
		template< typename _Aty >
		friend class Handle;

		void incRef(void) throw()
		{
			assert( (this->mPtr == NULL && this->mRefcount == NULL) || (this->mPtr != NULL && this->mRefcount != NULL));
			if( this->mRefcount != NULL )
				this->mRefcount->increment();
		}

		void decRef(void) throw()
		{
			assert( (this->mPtr == NULL && this->mRefcount == NULL) || (this->mPtr != NULL && this->mRefcount != NULL));
			if( this->mRefcount != NULL )
			{
				if ( this->mRefcount->decrement() == 0 )
					this->destroy();
			}
		}

		void destroy(void) throw()
		{
			RefCount* pRefcount = this->mRefcount;
			pointer ptr = mPtr;

			//to avoid unexpected dead recursion, first clear the data
			this->mRefcount = NULL;
			this->mPtr = NULL;

			BLADE_DELETE pRefcount;
			BLADE_DELETE ptr;
		}

		pointer		mPtr;
		RefCount*	mRefcount;
	private:
		mutable Lock		mLock;
	public:
		static const Handle	EMPTY;
	};

	template <typename T>
	const Handle<T> Handle<T>::EMPTY;

	class BLADE_BASE_API ALock : public Lock, public Allocatable {};
	typedef Handle<ALock> SHARED_LOCK;

}//namespace Blade

#endif // __Blade_Handle_h__