/********************************************************************
	created:	2011/10/23
	filename: 	LoopDataTypes.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_LoopDataTypes_h__
#define __Blade_LoopDataTypes_h__
#include <BladeFramework.h>
#include <Event.h>
#include <RefCount.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	///the LoopFlag class is a wrap for bool type,
	///and it is auto-reseted to false on each begin of the main loop
	///the alignment is used for internal implementation
	class LoopFlag
	{
	public:
		BLADE_FRAMEWORK_API LoopFlag();
		BLADE_FRAMEWORK_API explicit LoopFlag(bool true_false);
		BLADE_FRAMEWORK_API ~LoopFlag();

		/** @brief  */
		inline operator bool() const
		{
			return mFlag[0] == 1 ? true : false;
		}

		/** @brief  */
		inline bool operator!() const
		{
			return mFlag[0] == 0;
		}

		/** @brief  */
		inline bool operator[](index_t i) const
		{
			return mFlag[i] == 1;
		}

		/** @brief  */
		inline uint8& operator[](index_t i)
		{
			return mFlag[i];
		}

		/** @brief  */
		inline bool	operator==(bool rhs) const
		{
			return (mFlag[0] != 0 ) == rhs;
		}

		/** @brief  */
		LoopFlag&	operator=(bool rhs)
		{
			mFlag[0] = rhs ? 1u : 0;
			return *this;
		}

	protected:
		/** @brief  */
		BLADE_FRAMEWORK_API void			onLoopBegin(const Event& data);

		uint8	mFlag[4];
	};//LoopFlag


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	///the LoopRefCount class is a wrap for RefCount type,
	///and it is auto-reseted to 0 on each begin of the main loop
	class LoopRefCount
	{
	public:
		BLADE_FRAMEWORK_API LoopRefCount();
		BLADE_FRAMEWORK_API explicit LoopRefCount(size_t count);
		BLADE_FRAMEWORK_API ~LoopRefCount();

		/** @brief  */
		inline operator RefCount&()
		{
			return mRefCount;
		}

		/** @brief  */
		inline operator const RefCount&() const
		{
			return mRefCount;
		}

		/** @brief  */
		inline size_t		safeIncrement()
		{
			return mRefCount.safeIncrement();
		}

		/** @brief  */
		inline size_t		safeDecrement()
		{
			return mRefCount.safeDecrement(); 
		}

		/** @brief  */
		inline size_t		safeSet(size_t val)
		{
			return mRefCount.safeSet(val);
		}

		/** @brief  */
		inline size_t		safeSubstract(size_t val)
		{
			return mRefCount.safeSubstract(val);
		}

		/** @brief  */
		inline size_t		safeAdd(size_t val)
		{
			return mRefCount.safeAdd(val);
		}

		/** @brief  */
		inline size_t		increment()
		{
			return mRefCount.increment();
		}

		/** @brief  */
		inline size_t		decrement()
		{
			return mRefCount.decrement();
		}

		/** @brief  */
		inline size_t		set(size_t val)
		{
			return mRefCount.set(val);
		}

		/** @brief  */
		inline size_t		substract(size_t val)
		{
			return mRefCount.substract(val);
		}

		/** @brief  */
		inline size_t		add(size_t val)
		{
			return mRefCount.add(val);
		}


	protected:
		/** @brief  */
		BLADE_FRAMEWORK_API void			onLoopBegin(const Event& data);

		RefCount	mRefCount;
	};//class LoopRefCount

	class LoopPointer
	{
	public:
		BLADE_FRAMEWORK_API LoopPointer();
		BLADE_FRAMEWORK_API explicit LoopPointer(const void* ptr);
		BLADE_FRAMEWORK_API ~LoopPointer();

		inline const void*	get()
		{
			return mPtr;
		}

		inline bool		operator==(const void* rhs) const
		{
			return mPtr == rhs;
		}

		inline bool		operator!=(const void* rhs) const
		{
			return mPtr != rhs;
		}

		inline LoopPointer&	operator=(const void* rhs)
		{
			mPtr = rhs;
			return *this;
		}

		//unsafe cast get
		template<typename T>
		inline LoopPointer&	operator=(T* rhs)
		{
			mPtr = rhs;
			return *this;
		}

		//unsafe cast get
		template<typename T>
		inline T*		getPtr()
		{
			return (T*)mPtr;
		}

	protected:
		/** @brief  */
		BLADE_FRAMEWORK_API void	onLoopBegin(const Event& data);

		const void* mPtr;
	};
	

}//namespace Blade



#endif // __Blade_LoopDataTypes_h__