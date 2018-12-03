/********************************************************************
	created:	2014/06/17
	filename: 	UnixTimeDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "UnixTimeDevice.h"
#if BLADE_IS_UNIX_CLASS_SYSTEM


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TimeDevice::TimeDevice()
	{
		this->reset();
	}

	//////////////////////////////////////////////////////////////////////////
	TimeDevice::~TimeDevice()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				TimeDevice::resetImpl()
	{
		::clock_gettime(CLOCK_MONOTONIC, &mStart);
	}

	//////////////////////////////////////////////////////////////////////////
	void				TimeDevice::updateImpl()
	{
		::timespec now;
		::clock_gettime(CLOCK_MONOTONIC, &now);
		mElapsedMilliSecondFP64 = fp64(now.tv_sec-mStart.tv_sec)*fp64(1000) + fp64(now.tv_nsec-mStart.tv_nsec)/fp64(1e6);
		mElapsedMilliseconds = uint64( mElapsedMilliSecondFP64 );
	}
	
}//namespace Blade


#endif