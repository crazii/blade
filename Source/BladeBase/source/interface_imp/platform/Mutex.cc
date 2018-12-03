/********************************************************************
	created:	2014/08/03
	filename: 	Mutex.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "windows/WindowsMutex.h"
#include "unix/UnixMutex.h"
#include <threading/Mutex.h>

namespace Blade
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Mutex::Mutex()
		:mData( BLADE_NEW Impl::MutexImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Mutex::~Mutex()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void	Mutex::lock(uint spin/* = 0x200*/)
	{
		return mData->lock(spin);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Mutex::tryLock()
	{
		return mData->tryLock();
	}

	//////////////////////////////////////////////////////////////////////////
	void	Mutex::unlock()
	{
		return mData->unlock();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Mutex::tryLockUntil(time_t absSec, uint millisec)
	{
		return mData->tryLockUntil(absSec, millisec);
	}
	
}//namespace Blade