/********************************************************************
	created:	2014/08/03
	filename: 	WindowsMutex.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowsMutex_h__
#define __Blade_WindowsMutex_h__
#include <BladePlatform.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "../../../BladeWin32API.h"
#include <threading/Mutex.h>
#include <threading/Thread.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)
#endif

namespace Blade
{
	namespace Impl
	{
		class MutexImpl : public Allocatable
		{
		public:
			MutexImpl()
			{
				mWaitCount = 0;
				mSemaphore = ::CreateSemaphore(NULL, 0, 1, NULL);
				mThreadID = Thread::ID();
			}

			~MutexImpl()
			{
				//lock self
				this->lock(0);
				ScopedLock countGuard(mWaitCountGuard);
				assert( mWaitCount == 0 );

				//clean up data
				if( mSemaphore != NULL )
					::CloseHandle(mSemaphore);
			}

			/** @brief  */
			void lock(uint spin)
			{
				Thread::ID threadID = Thread::getCurrentID();
				uint i = 0;
				while( i++ <= spin )
				{
					if( this->tryLock(threadID) )
						return;
				}

				while(true)
				{
					{
						ScopedLock countGuard(mWaitCountGuard);
						if( mLock.tryLock() )
							break;
						//enable waiting: add waiting count
						++mWaitCount;
					}

					//wait: wait other threads to release semaphore
					DWORD result = ::WaitForSingleObject(mSemaphore, INFINITE);
					assert( result == WAIT_OBJECT_0); //must be signaled here
					BLADE_UNREFERENCED(result);
				}
				mThreadID = threadID;
				mRecursion.safeIncrement();
			}

			/** @brief  */
			bool tryLock(Thread::ID threadID = Thread::ID() )
			{
				if( threadID == Thread::ID() )
					threadID = Thread::getCurrentID();

				//recursive lock
				if( mLock.isLocked() && mThreadID == threadID )
				{
					mRecursion.safeIncrement();
					return true;
				}

				bool ret = mLock.tryLock();
				if( ret )
				{
					mThreadID = threadID;
					mRecursion.safeIncrement();
				}
				return ret;
			}

			/** @brief  */
			void unlock()
			{
				if( mLock.isLocked() )
				{
					if( mRecursion.safeDecrement() == 0 )
					{
						mThreadID = 0;

						//there's other threads waiting
						//release semaphore
						ScopedLock countGuard(mWaitCountGuard);
						if( mWaitCount > 0 )
						{
							--mWaitCount;
							BOOL result = ::ReleaseSemaphore(mSemaphore, 1, NULL);
							assert(result != FALSE);
							BLADE_UNREFERENCED(result);
						}
						mLock.unlock();
					}
				}
				else
				{
					assert(mThreadID == 0);
					assert(false);
				}
			}

			/** @brief  */
			bool	tryLockUntil(time_t absSec, uint millisec)
			{
				Thread::ID threadID = Thread::getCurrentID();
				if( this->tryLock(threadID) )
					return true;

				uint64 now = (uint64)std::time(NULL)*1000;
				uint64 to = (uint64)absSec*1000 + (uint64)millisec;
				if( now >= to )
					return false;

				DWORD timeOut = DWORD(to - now);
				DWORD timeBase = ::GetTickCount();
				DWORD lastResult = WAIT_OBJECT_0;
				while(true)
				{
					DWORD elapsed = ::GetTickCount() - timeBase;
					int timeToWait = int(timeOut - elapsed);

					{
						ScopedLock countGuard(mWaitCountGuard);
						if( mLock.tryLock() )
							break;

						if( lastResult != WAIT_OBJECT_0 )
						{
							assert( lastResult == WAIT_TIMEOUT );
							assert( timeToWait <= 0 );
							--mWaitCount; //cancel waiting
							return false;
						}

						//enable waiting: add waiting count
						++mWaitCount;
					}

					//wait: wait other threads to release semaphore
					assert( timeToWait >= 0 );
					lastResult = ::WaitForSingleObject(mSemaphore, (DWORD)timeToWait);
				}//while

				mThreadID = threadID;
				return true;
			}

		protected:
			Lock			mLock;		//simple lock
			RefCount		mRecursion;	//recursion count
			Thread::ID		mThreadID;	//current locked thread id for recursive lock
			HANDLE			mSemaphore;	//wait semaphore used when simple lock fails

			volatile size_t	mWaitCount;			//how many threads waiting on the lock. semaphore will be signaled if mWaitCount > 0
			Lock			mWaitCountGuard;	//safe guard on mWaitCount
		};
		
	}//namespace Impl
	
}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM

#endif // __Blade_WindowsMutex_h__