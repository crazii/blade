/********************************************************************
	created:	2014/06/17
	filename: 	WindowsTimeDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "WindowsTimeDevice.h"
#if BLADE_IS_WINDOWS_CLASS_SYSTEM

namespace Blade
{
	LARGE_INTEGER TimeDevice::msFrequency;

	//////////////////////////////////////////////////////////////////////////
	TimeDevice::TimeDevice()
	{
		TimeDevice::staticInit();
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	TimeDevice::~TimeDevice()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				TimeDevice::resetImpl()
	{
		std::memset(&mStartTime, 0, sizeof(mStartTime));
		::QueryPerformanceCounter( &mStartTime );
		mLastTime = 0;
		mLastTick = ::GetTickCount();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TimeDevice::updateImpl()
	{
		LARGE_INTEGER CurrentTime;
		::QueryPerformanceCounter( &CurrentTime );

		LONGLONG time = CurrentTime.QuadPart - mStartTime.QuadPart;

		mElapsedMilliSecondFP64 = (fp64)1000 * (fp64)time / (fp64)msFrequency.QuadPart;
		mElapsedMilliseconds = (uint64)mElapsedMilliSecondFP64;

		//check leap jump
		//http://support.microsoft.com/kb/274323/en-us
		//this also fix the different counter among multi-processors (See MSDN:QueryPerformanceCounter Remarks)
		LONGLONG TimeFromLast = time - mLastTime;
		unsigned long	MillisecFromLast = (unsigned long) (1000 * TimeFromLast / msFrequency.QuadPart);

		unsigned long tick = ::GetTickCount();
		unsigned long check = tick - mLastTick;
		signed long msecOff = (signed long)(MillisecFromLast - check);
		if (msecOff < -32 || msecOff > 32 || TimeFromLast < 0 )
		{
			int64 adjust = std::min(msecOff * msFrequency.QuadPart / 1000,TimeFromLast);
			mStartTime.QuadPart += adjust;
			time -= adjust;

			mElapsedMilliSecondFP64 = (fp64)1000 * (fp64)time / (fp64)msFrequency.QuadPart;
			mElapsedMilliseconds = (uint64)mElapsedMilliSecondFP64;
		}

		mLastTime = time;
		mLastTick = tick;
	}

}//namespace Blade


#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM