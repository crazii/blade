/********************************************************************
	created:	2010/03/29
	filename: 	StaticHandle.h
	author:		Crazii
	
	purpose:	static handle,withour reference couting
*********************************************************************/
#ifndef __Blade_StaticHandle_h__
#define __Blade_StaticHandle_h__
#include <Lock.h>
#include <memory/BladeMemory.h>

namespace Blade
{

	///static Handle could be use at start time,without construction.
	///because we could not be sure how the compiler set the static variables' initialization-order,
	///if we use set a static Handle value before the Handle's construction,
	///then later the Handle's constructor will erase the value already been set!

	///!Be careful when use StaticHandle as Vector's element!
	///because vector may perform a re-build array action,
	///which will destroy the none-RefCounted StaticHandle,thus the object will be destroyed
	///use it only on none reallocation vectors (resize once for all)
	template < typename T>
	class StaticHandle
	{
	public:
		typedef T*	pointer;
		struct Static
		{};

		StaticHandle(Static)
			:mLock(Lock::STATIC)
		{
		}

		StaticHandle()
			:mPtr(NULL)
		{

		}

		StaticHandle(pointer ptr)
			:mPtr(ptr)
		{
		}

		~StaticHandle()
		{
			if( mPtr != NULL )
			{
				BLADE_DELETE mPtr;
				mPtr = NULL;
			}
		}

		bool	operator ==(const pointer rhs) const
		{
			return mPtr == rhs;
		}

		operator pointer()
		{
			return mPtr;
		}

		operator pointer() const
		{
			return mPtr;
		}

		pointer operator->()
		{
			return mPtr;
		}

		pointer operator->()const
		{
			return mPtr;
		}

		StaticHandle&	operator=(pointer rhs )
		{
			if( mPtr != NULL )
				BLADE_DELETE mPtr;

			mPtr = rhs;
			return *this;
		}

		void			bind(pointer rhs)
		{
			this->lock();
			if( mPtr != NULL )
				BLADE_DELETE mPtr;

			mPtr = rhs;
			this->unlock();
		}

		pointer			unbind()
		{
			ScopedLock lock(mLock);
			pointer ret = mPtr;
			mPtr = NULL;
			return ret;
		}

		template<typename OT>
		const StaticHandle& constructInstance()
		{
			if( mPtr == NULL )
			{
				ScopedLock lock(mLock);
				mPtr = BLADE_NEW OT();
			}
			return *this;
		}

		void	lock()
		{
			mLock.lock();
		}

		void	unlock()
		{
			mLock.unlock();
		}

		void	clear()
		{
			if( mPtr == NULL )
				return;
			lock();
			if( mPtr != NULL )
				BLADE_DELETE mPtr;
			mPtr = NULL;
			unlock();
		}
	protected:
		Lock	mLock;
		pointer	mPtr;
	};//class StaticHandle
	
}//namespace Blade


#endif //__Blade_StaticHandle_h__