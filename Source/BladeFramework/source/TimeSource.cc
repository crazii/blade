/********************************************************************
	created:	2010/09/08
	filename: 	TimeSource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <TimeSource.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TimeSource::TimeSource()
	{
		mTimer = ITimeDevice::create();

		mLoopCount = 0;
		mLoopTime = 0;
		mAverageLoopTime = 0;
		mLoopTimeLimit = 0;
		mLastTime = mTimer->getSeconds();
	}

	//////////////////////////////////////////////////////////////////////////
	TimeSource::~TimeSource()
	{
		BLADE_DELETE mTimer;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TimeSource::reset()
	{
		mTimer->reset();
		mLoopCount = 0;
		mAverageLoopTime = 0;
		mLoopTime = 0;
		mLastTime = mTimer->getSeconds();
	}

	//////////////////////////////////////////////////////////////////////////
	void					TimeSource::update()
	{
		mTimer->update();
		mLoopTime = mTimer->getSeconds() - mLastTime;
		mLastTime = mTimer->getSeconds();

		if( mLoopTime < 0 )
			mLoopTime = 0.0;

		if( mLoopTimeLimit != 0 && mLoopTime < mLoopTimeLimit )
			mLoopTime = mLoopTimeLimit;

		mLoopCount++;
		if( mTimer->getMilliseconds() > 1000 * 1000 )//floating point precision
		{
			mTimer->reset();
			mLastTime = 0;
			mLoopCount = 0;
			mAverageLoopTime = mLoopTime;
			//ILog::DebugOutput << TEXT("timer reset.") << ILog::endLog;
		}
		else
			mAverageLoopTime = mTimer->getSeconds() / mLoopCount;

		//ILog::DebugOutput << uint( ((scalar)1.0/mLoopTime) ) << ILog::endLog;
	}
	
}//namespace Blade