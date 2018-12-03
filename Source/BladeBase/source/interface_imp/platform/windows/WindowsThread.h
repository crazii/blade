/********************************************************************
	created:	2014/08/03
	filename: 	WindowThread.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowThread_h__
#define __Blade_WindowThread_h__
#include <BladePlatform.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM

#define NOMINMAX
#if	defined(TEXT)
#	undef TEXT
#endif

#include <Windows.h>
#include <threading/Thread.h>
#include <utility/StringHelper.h>
#include <utility/StringHelperEx.h>

namespace Blade
{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
	//////////////////////////////////////////////////////////////////////////
	//http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
	const DWORD MS_VC_EXCEPTION=0x406D1388;
#pragma pack(push,8)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;
#pragma pack(pop)

	static void BladeWindowsSetThreadName(DWORD dwThreadID, const char* threadName)
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;

		__try
		{
			::RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
#else
#error not implemented yet.
#endif//BLADE_COMPILER


	//////////////////////////////////////////////////////////////////////////
	struct WindowsThreadDesc
	{
		void(*mThreadEntry)(Thread::HEXECUTER& exec);		//entry
		Thread::HEXECUTER	mExec;							//exec object
		Lock*				mReadyLock;
	};

	//////////////////////////////////////////////////////////////////////////
	static DWORD WINAPI BladeWindowsThreadProc(LPVOID lpParameter)
	{
		WindowsThreadDesc* desc = (WindowsThreadDesc*)lpParameter;
		assert(desc != NULL);

		//IMPORTANT: copy handle
		Thread::HEXECUTER exec = desc->mExec;
		void(*threadEntry)(Thread::HEXECUTER& exec) = desc->mThreadEntry;

		//notify data ready
		desc->mReadyLock->unlock();

		threadEntry(exec);

		return 0;
	}

	namespace Impl
	{
		class ThreadImpl : public Allocatable
		{
		public:
			ThreadImpl()
			{
				mThreadHandle = INVALID_HANDLE_VALUE;
				mPriority = Thread::TP_NORMAL;
				mJoinable = true;
				mID = Thread::ID();
			}

			~ThreadImpl()
			{
				if( mThreadHandle != INVALID_HANDLE_VALUE || mJoinable )
				{
					assert(false);
					//exception is raised in Thread
				}
			}

			/** @brief  */
			bool createThread( void(*threadEntry)(Thread::HEXECUTER&), const Thread::HEXECUTER& executer, size_t stackSize)
			{
				if( mThreadHandle == INVALID_HANDLE_VALUE && mJoinable )
				{
					assert(executer != NULL);

					//IMPORTANT: copy & kept data
					mDesc.mThreadEntry = threadEntry;
					mDesc.mExec = executer;
					mDesc.mReadyLock = &mReadyLock;

					DWORD id = 0;
					mReadyLock.lock();
					mThreadHandle = ::CreateThread(NULL, stackSize, BladeWindowsThreadProc, &mDesc, 0, &id);
					if( mThreadHandle == NULL )
						mThreadHandle = INVALID_HANDLE_VALUE;
					else
					{
						mID = id;
					}

					return mThreadHandle != INVALID_HANDLE_VALUE;
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
				if( !mJoinable || mThreadHandle == INVALID_HANDLE_VALUE )
					return false;

				DWORD dwResult = ::WaitForSingleObject(mThreadHandle, INFINITE);
				assert( dwResult == WAIT_OBJECT_0 );
				BLADE_UNREFERENCED(dwResult);

				::CloseHandle(mThreadHandle);
				mThreadHandle = INVALID_HANDLE_VALUE;
				mID = 0;
				mJoinable = false;

				return true;
			}

			/** @brief  */
			bool detach()
			{
				if( !mJoinable || mThreadHandle == INVALID_HANDLE_VALUE )
					return false;

				::CloseHandle(mThreadHandle);
				mThreadHandle = INVALID_HANDLE_VALUE;
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
				return mID;
			}

			/** @brief  */
			void		setPriority(int priority)
			{
				int tp;
				switch (priority)
				{
				case Thread::TP_BACKGROUND:
					tp = THREAD_PRIORITY_LOWEST;
					break;
				case Thread::TP_LOW:
					tp = THREAD_PRIORITY_BELOW_NORMAL;
					break;
				case Thread::TP_NORMAL:
					tp = THREAD_PRIORITY_NORMAL;
					break;
				case Thread::TP_HIGH:
					tp = THREAD_PRIORITY_ABOVE_NORMAL;
					break;
				default:
					tp = THREAD_PRIORITY_NORMAL;
				}
				::SetThreadPriority(mThreadHandle, priority);
			}

			/** @brief  */
			int			getPriority() const
			{
				return mPriority;
			}

			/** @brief  */
			static inline Thread::ID getCurrentID()
			{
				return (Thread::ID)::GetCurrentThreadId();
			}
			
			/** @brief  */
			static void yield()
			{
				::SwitchToThread();
			}

			/** @brief  */
			static void sleepUntil(time_t absSec, uint millisec)
			{
				uint64 now_millisec = (uint64)std::time(NULL)*1000;
				uint64 to_millisec = (uint64)absSec*1000 + (uint64)millisec;
				if( now_millisec <= to_millisec )
					::Sleep( (DWORD)(to_millisec - now_millisec) );
			}

			/** @brief  */
			static void setThreadName(Thread::ID id, const TString& name)
			{
				TempBuffer buffer;
				const char* _name = StringConverter::TStringToString(buffer, name);
				BladeWindowsSetThreadName((DWORD)id, _name);
			}

		protected:
			HANDLE		mThreadHandle;
			Thread::ID	mID;
			Lock		mReadyLock;
			WindowsThreadDesc	mDesc;
			int			mPriority;
			bool		mJoinable;
		};
		
	}//namespace Impl
	

}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM

#endif // __Blade_WindowThread_h__
