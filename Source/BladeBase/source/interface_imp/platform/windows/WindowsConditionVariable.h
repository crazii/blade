/********************************************************************
	created:	2014/08/03
	filename: 	WindowsConditionVariable.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowsConditionVariable_h__
#define __Blade_WindowsConditionVariable_h__
#include <BladePlatform.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "../../../BladeWin32API.h"
#include <threading/ConditionVariable.h>
#include <threading/Thread.h>
#include <threading/Mutex.h>
#include <limits>

namespace Blade
{
	namespace Impl
	{
		class ConditionVariableImpl : public Allocatable
		{
		public:
			ConditionVariableImpl()
			{
				mSemaphore = ::CreateSemaphore(NULL, 0, std::numeric_limits<LONG>::max(), NULL);
				mWaitCount = 0;
			}

			~ConditionVariableImpl()
			{
				ScopedLock lock(mLock);

				while( mWaitCount.count() != mWaitEndCount.count() )
					this->notifyAll();

				if( mSemaphore != NULL )
					::CloseHandle(mSemaphore);
			}

			/** @brief  */
			void	wait(UniqueLock& _lock)
			{
				assert( _lock.ownsLock() );

				//"Critical waiting point":
				{
					ScopedLock lock(mLock);
					mWaitCount.safeIncrement();
				}
				_lock.unlock();
				DWORD result = ::WaitForSingleObject(mSemaphore, INFINITE);
				assert( result == WAIT_OBJECT_0 );
				BLADE_UNREFERENCED(result);
				mWaitEndCount.safeIncrement();
				_lock.lock();
			}

			/** @brief  */
			ConditionVariable::EStatus waitUntil(UniqueLock& _lock, time_t absSec, uint millisec)
			{
				assert( _lock.ownsLock() );

				//"Critical waiting point":
				{
					ScopedLock lock(mLock);
					mWaitCount.safeIncrement();
				}

				uint64 now = (uint64)std::time(NULL)*1000;
				uint64 to = (uint64)absSec*1000 + (uint64)millisec;

				int time = int(to - now);

				_lock.unlock();
				DWORD result = 0;
				//try at least once
				if( time <= 0 )
					result = ::WaitForSingleObject(mSemaphore, 0);
				else
					result = ::WaitForSingleObject(mSemaphore, DWORD(time) );

				if( result == WAIT_OBJECT_0 )
					mWaitEndCount.safeIncrement();
				else
				{
					//ScopedLock lock(mLock);
					mWaitCount.safeDecrement();
				}

				_lock.lock();
				assert( result == WAIT_TIMEOUT || result == WAIT_OBJECT_0);
				return result == WAIT_OBJECT_0 ? ConditionVariable::CV_NO_TIMEOUT : ConditionVariable::CV_TIMEOUT;
			}

			/** @brief  */
			void notifyOne() noexcept
			{
				ScopedLock lock(mLock);
				mWaitEndCount.safeSet(0);
				if( mWaitCount.count() > 0 )
				{
					LONG prevCount = 0xFFFFFFFF;
					BOOL result = ::ReleaseSemaphore(mSemaphore, 1, &prevCount);	//semaphore+=1
					assert(result != FALSE);
					BLADE_UNREFERENCED(result);
					assert(prevCount == 0);
					//note: to avoid notify some thread just RUNNING on "Critical waiting point": {ScopedLock lock(mLock);++mWaitCount;}
					//that thread will get notified(wait done) if it continue running after here leaves mLock,
					//and left the real waiting threads still in the queue.
					//we need to receive "wait end" signal from the real waiting threads before leaving mLock,
					//to confirm the right thread is notified

					while(mWaitEndCount.count() == 0 
						&& mWaitCount.count() > 0 /*time out*/ )
					{
						Thread::yield();
					}

					if( mWaitCount.count() > 0 )
					{
						//succeeded
						assert(mWaitEndCount.count() == 1);
						mWaitCount.safeDecrement();
					}
					else
					{
						//all time out. all waits canceled, revert count to cancel notifying
						assert(mWaitEndCount.count() == 0);
						//cancel notifying, this should return immediately. and after this call, semaphore should be non-signaled. (internal count == 0)
						DWORD cancel = ::WaitForSingleObject(mSemaphore, 0);	//semaphore-=1
						assert( cancel == WAIT_OBJECT_0);
						BLADE_UNREFERENCED(cancel);
					}
				}
				mWaitEndCount.safeSet(0);
			}
		
			/** @brief  */
			void notifyAll() noexcept
			{
				ScopedLock lock(mLock);
				mWaitEndCount.safeSet(0);
				if( mWaitCount.count() > 0 )
				{
					LONG prevCount = 0xFFFFFFFF;
					LONG count = (LONG)mWaitCount.count();
					BOOL result = ::ReleaseSemaphore(mSemaphore, count, &prevCount);	//semaphore+=count
					assert(result != FALSE);
					BLADE_UNREFERENCED(result);
					assert(prevCount == 0);
					//note: to avoid notify some thread just RUNNING on "Critical waiting point": {ScopedLock lock(mLock);++mWaitCount;}
					//that thread will get notified(wait done) if it continue running after here leaves mLock,
					//and left the real waiting threads still in the queue.
					//we need to receive "wait end" signal from the real waiting threads before leaving mLock,
					//to confirm the right thread is notified

					while( mWaitEndCount != mWaitCount)
					{
						if( mWaitEndCount.count() < mWaitCount.count()/* * 3 / 4 */ )
							Thread::yield();
					}

					//skipped waiting because of timeout.
					LONG timeoutCount = count - (LONG)mWaitCount.count();
					assert(timeoutCount >= 0 );
					//cancel notifying. revert count back
					//and after this loop, semaphore should be non-signaled. (internal count == 0)
					while(timeoutCount > 0 )
					{
						DWORD result2 = ::WaitForSingleObject(mSemaphore, 0); //semaphore-=1. this should return immediately.
						assert(result2 == WAIT_OBJECT_0);
						BLADE_UNREFERENCED(result2);
						--timeoutCount;
					}
					mWaitCount.safeSet(0);
				}
				mWaitEndCount.safeSet(0);
			}

		protected:
			HANDLE		mSemaphore;
			Lock		mLock;
			RefCount	mWaitCount;
			RefCount	mWaitEndCount;
		};//class ConditionVariableImpl

	}//namespace Impl
	
}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM

#endif // __Blade_WindowsConditionVariable_h__