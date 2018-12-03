/********************************************************************
	created:	2010/03/20
	filename: 	RefCount.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_RefCount_h__
#define __Blade_RefCount_h__
#include <memory/BladeMemory.h>
#include <Lock.h>

namespace Blade
{

	class RefCount : public Allocatable
	{
	public:
		RefCount() :mCount(0) {}
		RefCount(size_t value) :mCount(value) {}
		~RefCount() {}

		/**
		* thread safe increment counting
		@param none
		@return self reference
		*/
		inline size_t		safeIncrement()
		{
			return Lock::increment(&mCount);
		}


		/**
		* thread safe decrement counting
		@param none
		@return self reference
		*/
		inline size_t		safeDecrement()
		{
			return Lock::decrement(&mCount);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline size_t		safeSet(size_t val)
		{
			return Lock::set(&mCount, val);
		}

		/*
		@describe 
		@param 
		@return 
		*/
		inline size_t		safeSubstract(size_t val)
		{
			return Lock::sub(&mCount, val);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline size_t		safeAdd(size_t val)
		{
			return Lock::add(&mCount, val);
		}

		/**
		* simple increment counting
		@param none
		@return self reference
		*/
		inline size_t		increment()
		{
			return ++mCount;
		}

		/**
		* simple decrement counting
		@param none
		@return self reference
		*/
		inline size_t		decrement()
		{
			return --mCount;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		size_t		set(size_t val)
		{
			return (mCount = val);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline size_t		substract(size_t val)
		{
			return (mCount -= val);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline size_t		add(size_t val)
		{
			return (mCount += val);
		}

		/**
		@describe
		@param
		@return
		*/
		inline size_t		count() const volatile
		{
			return mCount;
		}

		/**
		* compare refcount value with integer value
		@param [in] integer value to be compared
		@return true / false
		*/
		inline bool	operator==(size_t rhs) const volatile
		{
			return mCount == rhs;
		}

		/**
		* compare with another refcount 
		@param [in] another refcount to be compared
		@return true / false
		*/
		inline bool	operator==(const volatile RefCount& rhs) const volatile
		{
			return mCount == rhs.mCount;
		}

		/**
		@describe
		@param
		@return
		*/
		inline bool	operator!=(size_t rhs) const volatile
		{
			return mCount != rhs;
		}

		/**
		@describe
		@param
		@return
		*/
		inline bool	operator!=(const volatile RefCount& rhs) const volatile
		{
			return mCount != rhs.mCount;
		}

	protected:
		volatile size_t	mCount;
	};

}//namespace Blade

#endif //__Blade_RefCount_h__