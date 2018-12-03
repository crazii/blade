/********************************************************************
	created:	2010/09/10
	filename: 	TimeSourceManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TimeSourceManager.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TimeSourceManager::TimeSourceManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TimeSourceManager::~TimeSourceManager()
	{
		//ILog::DebugOutput << uint( ((scalar)1.0/mTimeSource.getTimePerLoop()) ) << ILog::endLog;
	}

	/************************************************************************/
	/* ITimerService interface                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const TimeSource&		TimeSourceManager::getTimeSource() const
	{
		return mTimeSource;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TimeSourceManager::resetTimeSource()
	{
		mTimeSource.reset();
	}

	//////////////////////////////////////////////////////////////////////////
	void					TimeSourceManager::setLoopTimeLimit(scalar fLoopTime/* = 0.016667f*/)
	{
		mTimeSource.setLoopTimeLimit(fLoopTime);
	}


	/************************************************************************/
	/* custom method internal                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TimeSource&				TimeSourceManager::getTimeSource()
	{
		return mTimeSource;
	}

	
}//namespace Blade