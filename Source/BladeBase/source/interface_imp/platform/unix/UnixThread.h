/********************************************************************
	created:	2014/08/04
	filename: 	UnixThread.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UnixThread_h__
#define __Blade_UnixThread_h__
#include <BladePlatform.h>
#include <math/BladeMath.h>
#if BLADE_IS_UNIX_CLASS_SYSTEM
#include <utility/StringHelper.h>
#include <utility/StringHelperEx.h>
#include <threading/Thread.h>

#include <pthread.h>
#include <errno.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	struct UnixThreadDesc
	{
		void(*mThreadEntry)(Thread::HEXECUTER& exec);		//entry
		Thread::HEXECUTER	mExec;							//exec object
		Lock*				mReadyLock;

		UnixThreadDesc()
		{
			mThreadEntry = NULL;
			mReadyLock = NULL;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	static void* BladeUnixThreadProc(void* arg)
	{
		UnixThreadDesc* desc = (UnixThreadDesc*)arg;
		assert(desc != NULL);

		//IMPORTANT: copy handle
		Thread::HEXECUTER exec = desc->mExec;
		void(*threadEntry)(Thread::HEXECUTER& exec) = desc->mThreadEntry;

		//notify data ready
		desc->mReadyLock->unlock();

		threadEntry(exec);

		return NULL;
	}


	namespace Impl
	{
		
		class ThreadImpl : public Allocatable
		{
		public:
			ThreadImpl()
				:mID(Thread::INVALID_ID)
				,mPriority(Thread::TP_NORMAL)
				,mJoinable(true)
			{

			}

			~ThreadImpl()
			{

			}

			/** @brief  */
			bool createThread( void(*threadEntry)(Thread::HEXECUTER&), const Thread::HEXECUTER& executer, size_t stackSize)
			{
				if( mID == Thread::INVALID_ID && mJoinable )
				{
					assert(executer != NULL);

					//IMPORTANT: copy & kept data
					mDesc.mThreadEntry = threadEntry;
					mDesc.mExec = executer;
					mDesc.mReadyLock = &mReadyLock;

					mReadyLock.lock();
					pthread_attr_t threadAttr;
					sched_param param;

					int result = ::pthread_attr_init(&threadAttr);
					assert( result == 0 );

					if( stackSize > 0 )
					{
						assert( stackSize > PTHREAD_STACK_MIN );
						result = ::pthread_attr_setstacksize(&threadAttr, stackSize);
						assert( result == 0 );
					}

					result = ::pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
					assert( result == 0 );

					std::memset( &param, 0, sizeof(param) );
					param.sched_priority = SCHED_NORMAL;
					result = ::pthread_attr_setschedparam(&threadAttr, &param);
					assert( result == 0 );

					result = ::pthread_create(&mID, &threadAttr, BladeUnixThreadProc, &mDesc);
					::pthread_attr_destroy(&threadAttr);
					bool ret = result == 0;
					assert( ret );
					if( !ret )
						mID = 0;
					return result == 0;
				}
				else
				{
					mID = 0;
					assert(false);
					return false;
				}
			}

			/** @brief  */
			bool join()
			{
				if( !mJoinable || mID == Thread::INVALID_ID)
					return false;

				void* retVal = NULL;
				int result = ::pthread_join(mID, &retVal);
				assert( result == 0); BLADE_UNREFERENCED(result);

				mID = 0;
				mJoinable = false;

				return true;
			}

			/** @brief  */
			bool detach()
			{
				if( !mJoinable || mID == 0 )
					return false;

				::pthread_detach(mID);
				mID = 0;
				mJoinable = false;

				//wait detach ready.
				mReadyLock.lock();

				return true;
			}

			/** @brief  */
			inline bool isJoinable() const
			{
				return mJoinable;
			}

			/** @brief  */
			inline Thread::ID getID() const
			{
				return (Thread::ID)mID;
			}

			/** @brief  */
			void		setPriority(int priority)
			{
				int tp;
				int min = sched_get_priority_min(SCHED_RR);
				int max = ::sched_get_priority_max(SCHED_RR);
				int middle = (min + max + 1) / 2;

				switch (priority)
				{
				case Thread::TP_BACKGROUND:
					tp = Math::Clamp(middle-2, min, max);
					break;
				case Thread::TP_LOW:
					tp = Math::Clamp(middle - 1, min, max);
					break;
				case Thread::TP_NORMAL:
					tp = Math::Clamp(middle, min, max);
					break;
				case Thread::TP_HIGH:
					tp = Math::Clamp(middle+1, min, max);
					break;
				default:
					tp = Math::Clamp(middle, min, max);
				}

				sched_param sch_params;
				sch_params.sched_priority = tp;
				if (pthread_setschedparam(mID, SCHED_RR, &sch_params))
					BLADE_EXCEPT(EXC_API_ERROR, BTString("set thread priority failed."));
			}

			/** @brief  */
			int			getPriority() const
			{
				return mPriority;
			}

			/** @brief  */
			static inline Thread::ID getCurrentID()
			{
				return (Thread::ID)::pthread_self();
			}

			/** @brief  */
			static void yield()
			{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
				int result = ::sched_yield();
#else
				int result = ::pthread_yield();
#endif
				assert(result == 0); BLADE_UNREFERENCED(result);
			}

			/** @brief  */
			static void sleepUntil(time_t absSec, uint millisec)
			{
				int64 diff = std::difftime(absSec, std::time(NULL))*1000 + millisec;
				if( diff > 0 )
				{
					timespec time = { time_t(diff/1000), long((diff%1000)*1000)};
					timespec left;

					int result = ::nanosleep(&time, &left);
					while( result != 0 && errno == EINTR )
					{
						time = left;
						result = ::nanosleep( &time, &left );
					}
				}
			}

			/** @brief  */
			static void setThreadName(Thread::ID id, const TString& name)
			{
				TempBuffer buffer;
				const char* _name = StringConverter::TStringToString(buffer, name);
				::pthread_setname_np(id, _name);
			}

		protected:
			UnixThreadDesc	mDesc;
			pthread_t		mID;
			Lock			mReadyLock;
			int				mPriority;
			bool			mJoinable;
		};

	}//namespace Impl
	
}//namespace Blade

#endif //BLADE_IS_UNIX_CLASS_SYSTEM

#endif // __Blade_UnixThread_h__
