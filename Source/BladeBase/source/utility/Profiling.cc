/********************************************************************
	created:	2013/03/10
	filename: 	Profiling.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "../BladeWin32API.h"
#include <utility/Profiling.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	LightWeightProfiling::LightWeightProfiling(const TString& funcionName, const TString& info/* = TString::EMPTY*/, bool enabled/* = true*/)
		:mFunction(funcionName)
		,mInfo(info)
	{
		if (enabled)
		{
			mTimer = ITimeDevice::create(ITimeDevice::TP_TEMPORARY);
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
			MEMORYSTATUSEX statex;
			statex.dwLength = sizeof(statex);
			::GlobalMemoryStatusEx(&statex);
			mMemory = (ptrdiff_t)(statex.ullAvailPhys / 1024 / 1024);
#else
			mMemory = 0;
#endif
			mTimer->reset();
		}
		else
		{
			mTimer = NULL;
			mMemory = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	LightWeightProfiling::~LightWeightProfiling()
	{
		if (mTimer == NULL)
			return;

		//output information to log
		mTimer->update();

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		::GlobalMemoryStatusEx(&statex);
		mMemory -= (ptrdiff_t)(statex.ullAvailPhys / 1024 / 1024);
#else
		mMemory = 0;
#endif

		ILog::DebugOutput << BTString("LW Profiling - ")
			<< mFunction
			<< (mInfo != TString::EMPTY ? TEXT("::") : TEXT(""))
			<< mInfo
			<< TEXT(" : ")
			<< mTimer->getMillisecondsHP()
			<< TEXT("ms, ")
			<< (int64)mMemory
			<< TEXT("Mbytes")
			<< ILog::endLog;

		BLADE_DELETE mTimer;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	struct TimeAndMemroy
	{
		TString		name;
		size_t		callCount;
		fp64		time;
		ptrdiff_t	memory;

		TimeAndMemroy()
		{
			time = 0;
			memory = 0;
		}
	};
	typedef StaticTStringMap<TimeAndMemroy> FrameStat;
	static FrameStat msFameStat;
	static StaticLock msLock;
	//////////////////////////////////////////////////////////////////////////
	FrameAccumulateProfiling::FrameAccumulateProfiling(const TString& funcionName, const TString& info/* = TString::EMPTY*/, bool enabled/* = true*/)
		:mFunction(funcionName)
		, mInfo(info)
	{
		if (enabled)
		{
			mTimer = ITimeDevice::create(ITimeDevice::TP_TEMPORARY);
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
			MEMORYSTATUSEX statex;
			statex.dwLength = sizeof(statex);
			::GlobalMemoryStatusEx(&statex);
			mMemory = (ptrdiff_t)(statex.ullAvailPhys / 1024 / 1024);
#else
			mMemory = 0;
#endif
			mTimer->reset();
		}
		else
		{
			mTimer = NULL;
			mMemory = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	FrameAccumulateProfiling::~FrameAccumulateProfiling()
	{
		if (mTimer == NULL)
			return;

		//output information to log
		mTimer->update();

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		::GlobalMemoryStatusEx(&statex);
		mMemory -= (ptrdiff_t)(statex.ullAvailPhys / 1024 / 1024);
#else
		mMemory = 0;
#endif

		{
			ScopedLock sl(msLock);
			TimeAndMemroy& tam = msFameStat[mFunction];
			tam.time += mTimer->getMillisecondsHP();
			tam.memory += mMemory;
			++tam.callCount;
		}

		BLADE_DELETE mTimer;
	}

	//////////////////////////////////////////////////////////////////////////
	void FrameAccumulateProfiling::onLoopInit()
	{
		for (FrameStat::iterator i = msFameStat.begin(); i != msFameStat.end(); ++i)
		{
			i->second.time = 0;
			i->second.memory = 0;
			i->second.callCount = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void FrameAccumulateProfiling::onLoopFinish()
	{
		for (FrameStat::iterator i = msFameStat.begin(); i != msFameStat.end(); ++i)
		{
			ILog::DebugOutput << BTString("FA Profiling - ")
				<< i->first
				<< ((i->second.name != TString::EMPTY) ? TEXT("::") : TEXT(""))
				<< i->second.name
				<< TEXT(" : ")
				<< i->second.time
				<< TEXT("ms, ")
				<< (int64)i->second.memory
				<< TEXT("Mbytes, ")
				<< TEXT("Called count:")
				<< (uint64)i->second.callCount
				<< ILog::endLog;
		}
	}
	
}//namespace Blade