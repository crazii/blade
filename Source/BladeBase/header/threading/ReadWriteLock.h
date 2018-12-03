/********************************************************************
	created:	2014/04/03
	filename: 	ReadWriteLock.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ReadWriteLock_h__
#define __Blade_ReadWriteLock_h__
#include <Lock.h>
#include <RefCount.h>
#include <Handle.h>
#include <threading/Mutex.h>

namespace Blade
{
	class BLADE_BASE_API ReadWriteLock : public Allocatable
	{
	public:
		ReadWriteLock()		{}
		~ReadWriteLock()	{}

		/** @brief shared lock if shared is true, otherwise exclusive lock */
		void	lock(bool shared);

		/** @brief  */
		bool	tryLock(bool shared);

		/** @brief  */
		bool	tryLockUntil(bool shared, time_t time, uint millisec);

		/** @brief  */
		bool	tryLockFor(bool shared, uint millisec);

		/** @brief  */
		void	unlock(bool shared);

	protected:
		Mutex		mWriteLock;
		Mutex		mReadLock;
		RefCount	mShareCount;
	};

	class BLADE_BASE_API RWLOCK : public Handle<ReadWriteLock>
	{
	public:
		/** @brief  */
		inline const RWLOCK& constructInstance(void) throw()
		{
			Handle<ReadWriteLock>::constructInstance<ReadWriteLock>();
			return *this;
		}
	};
	
}//namespace Blade


#endif // __Blade_ReadWriteLock_h__
