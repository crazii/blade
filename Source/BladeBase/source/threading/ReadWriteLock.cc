/********************************************************************
	created:	2014/04/03
	filename: 	ReadWriteLock.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <threading/ReadWriteLock.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	struct ReadWaitCondition
	{
		volatile RefCount* count;
		bool operator()() const
		{
			return *count == 0;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	void	ReadWriteLock::lock(bool shared)
	{
		if(!shared)
		{
			mWriteLock.lock();
			assert( mShareCount == 0 );
		}
		else
		{
			MutexAutoLock lock(mReadLock);
			if( mShareCount == 0 )
				mWriteLock.lock();
			mShareCount.safeIncrement();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ReadWriteLock::tryLock(bool shared)
	{
		if( !shared )
			return mWriteLock.tryLock();
		else
		{
			if( mReadLock.tryLock() )
			{
				if( mShareCount == 0 && mWriteLock.tryLock() )
				{
					mShareCount.safeIncrement();
					return true;
				}
			}

			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ReadWriteLock::tryLockUntil(bool shared, time_t time, uint millisec)
	{
		if( !shared )
			return mWriteLock.tryLockUntil(time, millisec);
		else
		{
			if( mReadLock.tryLockUntil(time, millisec) )
			{
				if( mShareCount == 0 && mWriteLock.tryLockUntil(time, millisec) )
				{
					mShareCount.safeIncrement();
					return true;
				}
			}

			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ReadWriteLock::tryLockFor(bool shared, uint millisec)
	{
		if( !shared )
			return mWriteLock.tryLockFor(millisec);
		else
		{
			Timer timer;	
			if( mReadLock.tryLockFor(millisec) )
			{
				timer.update();
				if( mShareCount == 0 && mWriteLock.tryLockFor(millisec - timer.getMilliseconds() ) )
				{
					mShareCount.safeIncrement();
					return true;
				}
			}

			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	ReadWriteLock::unlock(bool shared)
	{
		if( !shared )
		{
			assert( mShareCount.count() == 0 );
			mWriteLock.unlock();
		}
		else
		{
			assert( mShareCount.count() > 0 );
			MutexAutoLock lock(mReadLock);
			if( mShareCount.safeDecrement() == 0 )
				mWriteLock.unlock();
		}
	}
	
}//namespace Blade