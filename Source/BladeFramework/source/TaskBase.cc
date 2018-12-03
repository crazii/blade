/********************************************************************
	created:	2010/04/09
	filename: 	TaskBase.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TaskBase.h"

namespace Blade
{
	const ITask::Type ITask::ANY_TYPE = BTString("ANY_TYPE");
	const ITask::Type ITask::NO_TYPE = TString::EMPTY;

	//////////////////////////////////////////////////////////////////////////
	TaskBase::TaskBase(const TString& name,ETaskPriority priority/* = TP_NORMAL*/, uint32 priorityID/* = 0*/)
		:mName(name)
		,mPriorityID(priorityID)
		,mPriority(priority)
	{

	}


	//////////////////////////////////////////////////////////////////////////
	TaskBase::~TaskBase()
	{

	}

	/************************************************************************/
	/* ITask class spec                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const TString&	TaskBase::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	ITask::ETaskPriority	TaskBase::getPriority() const
	{
		return mPriority;
	}

	//////////////////////////////////////////////////////////////////////////
	uint32					TaskBase::getPriorityID() const
	{
		return mPriorityID;
	}
	
}//namespace Blade