/********************************************************************
	created:	2014/08/03
	filename: 	ConditionalVariable.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "windows/WindowsConditionVariable.h"
#include "unix/UnixConditionVariable.h"
#include <threading/ConditionVariable.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ConditionVariable::ConditionVariable()
		:mData( BLADE_NEW Impl::ConditionVariableImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ConditionVariable::~ConditionVariable()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void ConditionVariable::wait(UniqueLock& _lock)
	{
		if( !_lock )
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("wait on a non-locked lock object."));
		return mData->wait(_lock);
	}

	//////////////////////////////////////////////////////////////////////////
	ConditionVariable::EStatus ConditionVariable::waitUntil(UniqueLock& _lock, time_t absSec, uint millisec)
	{
		if( !_lock )
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("wait on a non-locked lock object."));

		return mData->waitUntil(_lock, absSec, millisec);
	}

	//////////////////////////////////////////////////////////////////////////
	void ConditionVariable::notifyOne() noexcept
	{
		return mData->notifyOne();
	}
		
	//////////////////////////////////////////////////////////////////////////
	void ConditionVariable::notifyAll() noexcept
	{
		return mData->notifyAll();
	}
	
}//namespace Blade