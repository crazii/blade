/********************************************************************
	created:	2014/08/02
	filename: 	ConditionVariable.h
	author:		Crazii
	
	purpose:	cross platform class for condition variable
*********************************************************************/
#ifndef __Blade_ConditionVariable_h__
#define __Blade_ConditionVariable_h__
#include <Pimpl.h>
#include <ctime>

namespace Blade
{
	class UniqueLock;

	namespace Impl
	{
		class ConditionVariableImpl;
	}//namespace Impl

	class BLADE_BASE_API ConditionVariable : public Allocatable
	{
	public:
		enum EStatus
		{
			CV_NO_TIMEOUT,
			CV_TIMEOUT
		};
	public:
		ConditionVariable();
		~ConditionVariable();

		/**
		@describe 
		@param
		@return
		*/
		void wait(UniqueLock& _lock);

		/** @brief  */
		template<typename Predicate>
		inline void wait(UniqueLock& _lock, Predicate _predicate)
		{
			while( !_predicate() )
				this->wait(_lock);
		}

		/**
		@describe wait to a time point
		@param [in] absSec should have the same origin as std::time()
		@return
		*/
		EStatus waitUntil(UniqueLock& _lock, time_t absSec, uint millisec);

		/** @brief  */
		template<typename Predicate>
		inline bool waitUntil(UniqueLock& _lock, time_t absSec, uint millisec, Predicate _predicate)
		{
			bool ret = true;
			while( ret && !_predicate() )
				ret = this->waitUntil(_lock, absSec, millisec) != CV_TIMEOUT;
			return _predicate();
		}


		/** @brief  */
		inline EStatus waitFor(UniqueLock& _lock, uint millisec)
		{
			time_t time = std::time(NULL);
			return this->waitUntil(_lock, time, millisec);
		}

		/** @brief  */
		template<typename Predicate>
		inline bool waitFor(UniqueLock& _lock, uint millisec, Predicate _predicate)
		{
			time_t time = std::time(NULL);
			return this->waitUntil<Predicate>(_lock, time, millisec, _predicate);
		}

		/**
		@describe 
		@param
		@return
		*/
		void notifyOne() noexcept;
		
		/**
		@describe 
		@param
		@return
		*/
		void notifyAll() noexcept;

	protected:


	private:
		Pimpl<Impl::ConditionVariableImpl>	mData;
	};//class Condition 
	
}//namespace Blade


#endif // __Blade_ConditionVariable_h__
