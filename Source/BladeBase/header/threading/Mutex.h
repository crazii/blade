/********************************************************************
	created:	2014/08/02
	filename: 	Mutex.h
	author:		Crazii
	
	purpose:	cross platform class for mutex
*********************************************************************/
#ifndef __Blade_Mutex_h__
#define __Blade_Mutex_h__
#include <Pimpl.h>
#include <ctime>

namespace Blade
{
	namespace Impl
	{
		class MutexImpl;
	}//namespace Impl

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API Mutex : public Allocatable, public NonCopyable, public NonAssignable
	{
	public:
		Mutex();
		~Mutex();

		/**
		@describe 
		@param
		@return
		*/
		void	lock(uint spin = 0x200);

		/**
		@describe 
		@param
		@return
		*/
		bool	tryLock();

		/**
		@describe 
		@param
		@return
		*/
		void	unlock();

		/**
		@describe try locking to a time point
		@param [in] absSec should have the same origin as std::time()
		@return
		*/
		bool	tryLockUntil(time_t absSec, uint millisec);

		/**
		@describe try locking for a time span
		@param
		@return
		*/
		bool	tryLockFor(uint millisec)
		{
			time_t time = std::time(NULL);
			return this->tryLockUntil(time, millisec);
		}
		
	protected:

	private:
		Pimpl<Impl::MutexImpl>	mData;
	};//class Mutex


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class UniqueLock : public Allocatable, public NonAssignable, public NonCopyable
	{
	public:
		UniqueLock() :mMutex(NULL),mOwner(false)	{}

		explicit UniqueLock(Mutex& _mutex)
			:mMutex(&_mutex)
		{
			mMutex->lock();
			mOwner = true;
		}

		explicit UniqueLock(Mutex* _mutex)
			:mMutex(_mutex)
		{
			mMutex->lock();
			mOwner = true;
		}

		UniqueLock(Mutex& _mutex, uint millisec)
			:mMutex(&_mutex)
			,mOwner(false)
		{
			mOwner = this->tryLockFor(millisec);
		}

		UniqueLock(Mutex& _mutex, time_t absSec, uint millisec)
			:mMutex(&_mutex)
			,mOwner(false)
		{
			mOwner = this->tryLockUntil(absSec, millisec);
		}

		~UniqueLock()
		{
			if( mOwner && mMutex != NULL )
				mMutex->unlock();
		}

		/** @brief  */
		void	lock()
		{
			if (this->ownsLock())
				BLADE_EXCEPT(EXC_REDUPLICATE, BTString("cannot lock for a second time."));

			mMutex->lock();
			mOwner = true;
		}

		/** @brief  */
		bool	tryLock()
		{
			if (this->ownsLock())
				BLADE_EXCEPT(EXC_REDUPLICATE, BTString("cannot lock for a second time."));

			mOwner = mMutex->tryLock();
			return mOwner;
		}

		/** @brief  */
		void	unlock()
		{
			if (!this->ownsLock())
				BLADE_EXCEPT(EXC_REDUPLICATE, BTString("cannot unlock on a non-owned lock."));

			if (mOwner)
			{
				assert(mMutex != NULL);
				mMutex->unlock();
				mOwner = false;
			}
		}

		/** @brief  */
		bool	tryLockUntil(time_t absSec, uint millisec)
		{
			if (this->ownsLock())
				BLADE_EXCEPT(EXC_REDUPLICATE, BTString("cannot lock for a second time."));

			mOwner = mMutex->tryLockUntil(absSec, millisec);
			return mOwner;
		}

		/** @brief  */
		inline bool	tryLockFor(uint millisec)
		{
			time_t time = std::time(NULL);
			return this->tryLockUntil(time + millisec/1000, millisec%1000);
		}

		/** @brief  */
		inline bool	ownsLock() const
		{
			assert( !mOwner || mMutex != NULL );
			return mOwner && mMutex != NULL;
		}

		/** @brief  */
		inline operator bool () const
		{
			return this->ownsLock();
		}

		/** @brief  */
		inline Mutex*	mutex() const noexcept
		{
			return mMutex;
		}

		/** @brief  */
		inline Mutex*	release() noexcept
		{
			if( mMutex != NULL && mOwner )
			{
				mMutex->unlock();
				mOwner = false;
			}
			return mMutex;
		}

	protected:

		Mutex*	mMutex;
		bool	mOwner;
	};

	class MutexAutoLock : public NonCopyable, public NonAllocatable, public NonAssignable
	{
	public:
		inline MutexAutoLock(Mutex& m)
			:mMutexRef(m)
		{
			mMutexRef.lock();
		}

		inline ~MutexAutoLock()
		{
			mMutexRef.unlock();
		}
		Mutex& mMutexRef;
	};

	
}//namespace Blade


#endif // __Blade_Mutex_h__
