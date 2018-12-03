/********************************************************************
	created:	2014/08/04
	filename: 	UnixMutex.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UnixMutex_h__
#define __Blade_UnixMutex_h__
#include <BladePlatform.h>
#if BLADE_IS_UNIX_CLASS_SYSTEM
#include <threading/Mutex.h>
#include <pthread.h>
#include <errno.h>

namespace Blade
{

	namespace Impl
	{
		class MutexImpl : public Allocatable
		{
		public:
			MutexImpl()
			{
				pthread_mutexattr_t attr;
				int result = ::pthread_mutexattr_init(&attr);
				assert(result == 0);

				result = ::pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE); 
				assert(result == 0);

				result = ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
				assert(result == 0);

				result = ::pthread_mutex_init(&mMutex, &attr);
				assert(result == 0);

				result = ::pthread_mutexattr_destroy(&attr);
				assert(result == 0);
			}

			~MutexImpl()
			{
				int result = ::pthread_mutex_destroy(&mMutex);
				assert(result == 0); BLADE_UNREFERENCED(result);
			}

			/** @brief  */
			void lock(uint spin)
			{
				uint i = 0;
				while( i++ <= spin)
				{
					int result = ::pthread_mutex_trylock(&mMutex);
					if( result == 0 )
						return;
					else
						assert( result == EBUSY );
				}
				
				int result = ::pthread_mutex_lock(&mMutex);
				assert(result == 0); BLADE_UNREFERENCED(result);
			}

			/** @brief  */
			bool tryLock()
			{
				return ::pthread_mutex_trylock(&mMutex) == 0;
			}

			/** @brief  */
			void unlock()
			{
				int result = ::pthread_mutex_unlock(&mMutex);
				assert(result == 0); BLADE_UNREFERENCED(result);
			}

			/** @brief  */
			bool	tryLockUntil(time_t absSec, uint millisec)
			{
				if( this->tryLock() )
					return true;

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID && 0 //BLADE_COMPILER == BLADE_COMPILER_GNUC
				timespec ts;
				ts.tv_nsec =
					int result = ::pthread_mutex_timedlock(&mMutex, unsigned(diff));
#else
				timespec abstime = { absSec + (time_t)(millisec/1000u), (time_t)((millisec%1000u)*1000u) };
				int result = ::pthread_mutex_timedlock(&mMutex, &abstime);
#endif
				assert( result == 0 || result == ETIMEDOUT);
				return result == 0;
			}

		protected:
			pthread_mutex_t mMutex;
			friend class ConditionVariableImpl;
		};
	}//namespace Impl
	
}//namespace Blade

#endif//BLADE_IS_UNIX_CLASS_SYSTEM

#endif // __Blade_UnixMutex_h__