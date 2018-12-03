/********************************************************************
	created:	2009/02/10
	filename: 	Lock.h
	author:		Crazii
	
	purpose:	multi-thread simple lock
		note:	this type of lock is spin lock.
				it is light weight without blocking current thread,
				if you have simple synchronization needs, use it.
				it doesn't support recursion
*********************************************************************/
#ifndef __Blade_Lock_h__
#define __Blade_Lock_h__
#include <BladeBase.h>

namespace Blade
{

	class Lock
	{
	public:
		///static initializer helper
		struct Static
		{
			Static() {}
		};
		BLADE_BASE_API static const Static STATIC;
	public:
		enum
		{
			LV_LOCKED		= 0xFFFFFFFF,
			LV_NOT_LOCKED	= 0
		};
	public:

		Lock()	:mLocked(LV_NOT_LOCKED)	{}
		///static locks maybe accessed/initialized by other pre-main routine(i.e. constructor of other static variables) before its own constructor
		///static constructor:DO NOT initialize members, to avoid reset data. this makes the static Lock objects' definition order not critical
		///otherwise your probably need define the lock object before any other static objects that uses it.
		Lock(const Static){}
		~Lock()	{}

		/** @brief  */
		inline bool	isLocked() const
		{
			return mLocked != LV_NOT_LOCKED;
		}

		/** @brief  */
		inline bool	tryLock()
		{
			size_t val = Lock::compareAndSwap(&mLocked, (size_t)LV_LOCKED, (size_t)LV_NOT_LOCKED );
			bool ret = val == LV_NOT_LOCKED;
			//guarantee all memory accessing after lock is NOT re-ordered.
			//on x86, memory barrier is not needed, but maybe needed on other platform.
			//just make it safe for all architectures
			if( ret )
				Lock::memoryBarrier();
			return ret;
		}

		/** @brief busy spin lock without block/sleep */
		void	lock()
		{
			bool locked;
			while ( ( locked = tryLock() ) == false );
		}

		/** @brief  */
		inline void	unlock()
		{
			Lock::compareAndSwap(&mLocked, (size_t)LV_NOT_LOCKED, (size_t)LV_LOCKED);
			Lock::memoryBarrier();
		}

		/** @brief memory barrier utility */
		BLADE_BASE_API static void		memoryBarrier();

		/** @brief return the incremented value */
		BLADE_BASE_API static int32	increment32(volatile int32* data);

		/** @brief return the decremented value */
		BLADE_BASE_API static int32	decrement32(volatile int32* data);

		/** @brief return the original value */
		/** @note: for 32bit systems this is usually not needed, unless
		your data is not aligned */
		BLADE_BASE_API static int32	set32(volatile int32* data, int32 val);

		/** @brief return the original value */
		BLADE_BASE_API static int32	add32(volatile int32* data, int32 val);

		/** @brief return the original value */
		BLADE_BASE_API static int32	sub32(volatile int32* data, int32 val);


		/** @brief return the incremented value */
		BLADE_BASE_API static int64	increment64(volatile int64* data);

		/** @brief return the decremented value */
		BLADE_BASE_API static int64	decrement64(volatile int64* data);

		/** @brief return the original value */
		/** @note: for 64bit systems this is usually not needed, unless
		your data is not aligned */
		BLADE_BASE_API static int64	set64(volatile int64* data, int64 val);

#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
		/** @brief  */
		BLADE_BASE_API static int64	get64(const volatile int64* data);
#else
		///for 64bit architecture, if the data is aligned(and it always should be), we can fetch the data directly
		/** @brief  */
		BLADE_BASE_API static inline int64	get64(const volatile int64* data)	{return *data;}
#endif

		/** @brief return the original value */
		BLADE_BASE_API static int64	add64(volatile int64* data, int64 val);

		/** @brief return the original value */
		BLADE_BASE_API static int64	sub64(volatile int64* data, int64 val);

		/** @brief return the incremented value */
		inline static size_t	increment(volatile size_t* data)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)increment32( (volatile int32*)data );
#else
			return (size_t)increment64( (volatile int64*)data );
#endif
		}

		/** @brief return the decremented value */
		inline static size_t	decrement(volatile size_t* data)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)decrement32( (volatile int32*)data );
#else
			return (size_t)decrement64( (volatile int64*)data );
#endif
		}

		/** @brief return the original value */
		inline static size_t	set(volatile size_t* data, size_t val)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)set32( (volatile int32*)data, (int32)val );
#else
			return (size_t)set64( (volatile int64*)data, (int64)val );
#endif
		}

		/** @brief set pointer */
		inline static size_t	set(const void** data, const void* val)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)set32( (volatile int32*)data, (int32)(size_t)(uintptr_t)val );
#else
			return (size_t)set64( (volatile int64*)data, (int64)(size_t)(uintptr_t)val );
#endif
		}

		/** @brief  */
		inline static size_t	get(const volatile size_t* data)	{return *data;}

		/** @brief return the original value */
		inline static size_t	add(volatile size_t* data, size_t val)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)add32( (volatile int32*)data, (int32)val );
#else
			return (size_t)add64( (volatile int64*)data, (int64)val );
#endif
		}

		/** @brief return the original value */
		inline static size_t	sub(volatile size_t* data, size_t val)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)sub32( (volatile int32*)data, (int32)val );
#else
			return (size_t)sub64( (volatile int64*)data, (int64)val );
#endif
		}

		/** @brief CAS 32*/
		BLADE_BASE_API static int32	compareAndSwap32(volatile int32* data, int32 val, int32 comparand);

		/** @brief CAS 64*/
		BLADE_BASE_API static int64	compareAndSwap64(volatile int64* data, int64 val, int64 comparand);
		
		/** @brief CAS */
		inline static size_t	compareAndSwap(volatile size_t* data, size_t val, size_t comparand)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return (size_t)compareAndSwap32( (int32*)data, (int32)val, (int32)comparand );
#else
			return (size_t)compareAndSwap64( (int64*)data, (int64)val, (int64)comparand );
#endif
		}

		/** @brief  */
		template<typename CMP>
		inline static bool		compareOPAndSwap32(volatile int32* data, int32 val, int32 comparand, CMP cmp)
		{
			int32 oldVal;
			do 
			{
				oldVal = *data;
				Lock::memoryBarrier();
				if( !cmp(oldVal,comparand) )
					return false;
			}while( Lock::compareAndSwap32( (int32*)data, val, oldVal) != oldVal );
			return true;
		}

		/** @brief  */
		template<typename CMP>
		inline static bool		compareOPAndSwap64(volatile int64* data, int64 val, int64 comparand, CMP cmp)
		{
			int64 oldVal;
			do 
			{
				oldVal = *data;
				Lock::memoryBarrier();
				if( !cmp(oldVal,comparand) )
					return false;
			}while( Lock::compareAndSwap64( (int64*)data, val, oldVal) != oldVal );
			return true;
		}

		template<typename CMP>
		inline static bool		compareOPAndSwap(volatile size_t* data, size_t val, size_t comparand, CMP cmp)
		{
#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
			return compareOPAndSwap32( (int32*)data, (int32)val, (int32)comparand, cmp );
#else
			return compareOPAndSwap64( (int64*)data, (int64)val, (int64)comparand, cmp );
#endif
		}

	protected:
		size_t	mLocked;
	};//class Lock

	//////////////////////////////////////////////////////////////////////////
	class StaticLock : public Lock
	{
	public:
		inline StaticLock() :Lock(Lock::STATIC)	{}
	};

	//////////////////////////////////////////////////////////////////////////
	class ScopedLock : public NonAllocatable, public NonAssignable, public NonCopyable
	{
	public:
		inline ScopedLock(Lock& theLock)
			:mLockRef(theLock)
		{
			mLockRef.lock();
		}

		inline ~ScopedLock()
		{
			mLockRef.unlock();
		}
	protected:
		Lock&	mLockRef;
	};//class ScopedLock

	//////////////////////////////////////////////////////////////////////////
	class ScopedPLock : public NonAllocatable, public NonAssignable, public NonCopyable
	{
	public:
		inline ScopedPLock(Lock* theLock)
			:mLockPtr(theLock)
		{
			if(mLockPtr != NULL)
				mLockPtr->lock();
		}

		inline ~ScopedPLock()
		{
			if(mLockPtr != NULL)
				mLockPtr->unlock();
		}
	protected:
		Lock*	mLockPtr;
	};//class ScopedPLock

}//namespace Blade

#endif // __Blade_Lock_h__