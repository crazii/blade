/********************************************************************
	created:	2014/08/04
	filename: 	UnixConditionVariable.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UnixConditionVariable_h__
#define __Blade_UnixConditionVariable_h__
#include <BladePlatform.h>
#if BLADE_IS_UNIX_CLASS_SYSTEM
#include <threading/ConditionVariable.h>
#include "UnixMutex.h"

namespace Blade
{
	namespace Impl
	{
		class ConditionVariableImpl : public Allocatable
		{
		public:
			ConditionVariableImpl()
			{
				pthread_condattr_t attr;
				int result = ::pthread_condattr_init(&attr);
				assert(result == 0);
				result = ::pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
				assert(result == 0);

				result = ::pthread_cond_init(&mCondition, &attr);
				assert(result == 0);

				result = ::pthread_condattr_destroy(&attr);
				assert(result == 0);
			}

			~ConditionVariableImpl()
			{
				int result = ::pthread_cond_destroy(&mCondition);
				assert( result == 0 ); BLADE_UNREFERENCED(result);
			}


			/** @brief  */
			void	wait(UniqueLock& _lock)
			{
				assert( _lock.ownsLock() );
				Mutex* m = _lock.mutex();
				Pimpl<Impl::MutexImpl>& data = reinterpret_cast<Pimpl<Impl::MutexImpl>&>(*m);
				int result = ::pthread_cond_wait(&mCondition, &data->mMutex);
				assert(result == 0); BLADE_UNREFERENCED(result);
			}

			/** @brief  */
			ConditionVariable::EStatus waitUntil(UniqueLock& _lock, time_t absSec, uint millisec)
			{
				assert( _lock.ownsLock() );
				Mutex* m = _lock.mutex();
				LazyPimpl<Impl::MutexImpl>& data = reinterpret_cast<LazyPimpl<Impl::MutexImpl>&>(*m);

				timespec abstime = { time_t(absSec + millisec/1000), long((millisec%1000)*1000) };
				int result = ::pthread_cond_timedwait(&mCondition, &data->mMutex, &abstime);
				assert(result == 0 || result == ETIMEDOUT);
				return result == 0 ? ConditionVariable::CV_NO_TIMEOUT : ConditionVariable::CV_TIMEOUT;
			}

			/** @brief  */
			void notifyOne() noexcept
			{
				int result = ::pthread_cond_signal(&mCondition);
				assert( result == 0 ); BLADE_UNREFERENCED(result);
			}

			/** @brief  */
			void notifyAll() noexcept
			{
				int result = ::pthread_cond_broadcast(&mCondition);
				assert(result == 0); BLADE_UNREFERENCED(result);
			}
		protected:
			pthread_cond_t mCondition;
		};
		
	}//namespace Impl
	
}//namespace Blade

#endif//BLADE_IS_UNIX_CLASS_SYSTEM


#endif // __Blade_UnixConditionVariable_h__

