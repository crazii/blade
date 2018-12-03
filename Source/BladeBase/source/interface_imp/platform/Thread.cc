/********************************************************************
	created:	2014/08/03
	filename: 	Thread.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "windows/WindowsThread.h"
#include "unix/UnixThread.h"
#include <threading/Thread.h>
#include <interface/IPlatformManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	static void BladeThreadEntry(Thread::HEXECUTER& hExec)
	{
		//note IMPORTANT: thread data should hold a copy of handle for executer and pass it here
		hExec->run();
		hExec.clear();
	}

	using namespace Impl;

	//////////////////////////////////////////////////////////////////////////
	Thread::Thread()
		:mData( NULL )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	Thread::Thread(HEXECUTER exectuter, size_t stackSize/* = 0*/)
		:mData( BLADE_NEW ThreadImpl() )
	{
		mData->createThread(BladeThreadEntry, exectuter, stackSize);
	}

	//////////////////////////////////////////////////////////////////////////
	Thread::~Thread()
	{
		if (mData != NULL && mData->isJoinable())
		{
			//BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("thread is still joinable on destruction."));
			this->join();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Thread::ID	Thread::getID() const noexcept
	{
		return mData != NULL ? mData->getID() : Thread::ID();
	}


	//////////////////////////////////////////////////////////////////////////
	bool		Thread::isJionable() const noexcept
	{
		return mData != NULL && mData->isJoinable();
	}

	//////////////////////////////////////////////////////////////////////////
	void		Thread::join()
	{
		if( mData == NULL || !mData->join() )
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("thread is not joinable."));
	}

	//////////////////////////////////////////////////////////////////////////
	void		Thread::detach()
	{
		if( mData == NULL || !mData->detach() )
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("thread is not joinable."));
	}

	//////////////////////////////////////////////////////////////////////////
	void		Thread::setPriority(int priority)
	{
		if(mData == NULL)
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("thread is not joinable."));
		mData->setPriority(priority);
	}

	//////////////////////////////////////////////////////////////////////////
	int			Thread::getPriority() const
	{
		if (mData == NULL)
			BLADE_EXCEPT(EXC_INVALID_OPERATION, BTString("thread is not joinable."));
		return mData->getPriority();
	}

	/************************************************************************/
	/* static methods equivilant to std::this_thread(C++11)                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Thread::ID	Thread::getCurrentID() noexcept
	{
		return ThreadImpl::getCurrentID();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	Thread::getHardwareConcurrency()
	{
		static size_t count = 0;
		if (count == 0)
			count = IPlatformManager::getSingleton().getCPUInfo().mProcessorCount;
		return count;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Thread::yield() noexcept
	{
		return ThreadImpl::yield();
	}

	//////////////////////////////////////////////////////////////////////////
	void		Thread::sleepUntil(time_t absSec, uint millisec)
	{
		return ThreadImpl::sleepUntil(absSec, millisec);
	}

	//////////////////////////////////////////////////////////////////////////
	void		Thread::setThreadName(ID id, const TString& name)
	{
		ThreadImpl::setThreadName(id, name);
	}

}//namespace Blade