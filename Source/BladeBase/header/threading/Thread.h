/********************************************************************
	created:	2014/08/02
	filename: 	Thread.h
	author:		Crazii
	
	purpose:	cross platform class for thread
				TODO: internal pooling for threads, or add hi level pooling based on thread class.
*********************************************************************/
#ifndef __Blade_Thread_h__
#define __Blade_Thread_h__
#include <Handle.h>
#include <utility/String.h>
#include <ctime>

namespace Blade
{
	namespace Impl
	{
		class ThreadImpl;
	}//namespace Impl

	class BLADE_BASE_API Thread : public Allocatable, public NonCopyable
	{
	public:
		typedef uintptr_t ID;
		static const uintptr_t INVALID_ID = (ID)(unsigned)-1;

		enum EThreadPriority
		{
			TP_BACKGROUND = -1,
			TP_LOW = 0,
			TP_NORMAL,
			TP_HIGH,
		};

		///note: HEXECUTER is kept in Thread object until join/detach/destruction.
		///if HEXECUTER's reference count becomes 1, 
		///Executer will be destroyed at detached thread after run()
		class BLADE_BASE_API IExecuter
		{
		public:
			virtual ~IExecuter() {}

			/** @brief  */
			virtual void run() = 0;
		};
		typedef Handle<IExecuter> HEXECUTER;
		
	public:
		Thread();
		Thread(HEXECUTER exectuter, size_t stackSize = 0);
		~Thread();

		/** @brief  */
		ID			getID() const noexcept;

		/** @brief  */
		bool		isJionable() const noexcept;

		/** @brief  */
		void		join();

		/** @brief  */
		void		detach();

		/** @brief  */
		void		setPriority(int priority);

		/** @brief  */
		int			getPriority() const;

		/************************************************************************/
		/* static methods equivilant to std::this_thread::XXX (C++11)                                                                     */
		/************************************************************************/

		/** @brief get current running thread id */
		static ID	getCurrentID() noexcept;

		/** @brief get concurrent threads supported by hardware */
		static size_t	getHardwareConcurrency();

		/** @brief yield to another thread */
		static void	yield() noexcept;

		/** @brief absSec should have the same origin as std::time() */
		static void	sleepUntil(time_t absSec, uint millisec);

		static void setThreadName(ID id, const TString& name);

		/** @brief  */
		static inline void	sleepFor(uint millisec)
		{
			time_t time = std::time(NULL);
			return Thread::sleepUntil(time + millisec/1000, millisec%1000);
		}

	private:
		Pimpl<Impl::ThreadImpl>	mData;
	};
	typedef Handle<Thread> HTHREAD;
	
}//namespace Blade


#endif // __Blade_Thread_h__
