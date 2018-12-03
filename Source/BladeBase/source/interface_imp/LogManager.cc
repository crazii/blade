/********************************************************************
	created:	2010/03/28
	filename: 	LogManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "LogManager.h"
#include "Log.h"

namespace Blade
{
	template class Factory<ILogManager>;

	//////////////////////////////////////////////////////////////////////////
	LogManager::LogManager()
		:mInitialized(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	LogManager::~LogManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	bool			LogManager::initialize(const TString& logfile,ILog::ELogLevel level/* = ILog::LL_DEFAULT*/)
	{
		if( mInitialized )
			return true;

		Log* LogList[] = {&Log::msDebugOutput,&Log::msInformation,&Log::msWarning,&Log::msError };

		for(size_t i = 0; i < sizeof(LogList)/sizeof(void*); ++i )
		{
			bool enable =  LogList[i]->getLevel() <= level;
			LogList[i]->setEnable(enable);
		}

		if(level != ILog::LL_NONE )
		{
			ScopedLock lock(Log::msSharedLock);
			if( Log::msSharedLog.open(logfile,IStream::AM_OVERWRITE) == false )
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("unable to creat/write log file:" ) + logfile  );
		}

		if( level >= ILog::LL_DETAIL )
			ILog::Information << TEXT("Log Started.") << ILog::endLog;

		mInitialized = true;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LogManager::initialize(const HSTREAM& logstream,ILog::ELogLevel level/* = ILog::LL_DEFAULT*/)
	{
		if( mInitialized )
			return true;

		Log*	LogList[] = {&Log::msDebugOutput,&Log::msInformation,&Log::msWarning,&Log::msError };
		for(size_t i = 0; i < sizeof(LogList)/sizeof(void*); ++i )
		{
			bool enable =  LogList[i]->getLevel() <= level;
			LogList[i]->setEnable(enable);
		}


		{
			ScopedLock lock(Log::msSharedLock);
			if( Log::msSharedLog.open(logstream) == false )
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("unable to creat/write log file:" )+ logstream->getName()  );
		}

		ILog::Information << TEXT("Log Started.") << ILog::endLog;
		mInitialized = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	HLOG		LogManager::createLog(ILog::ELogLevel level, const TString& logfile/* = TString::EMPTY*/)
	{
		bool shared = logfile.empty();
		Log* log = BLADE_NEW Log(level, shared);

		if( !shared )
		{
			ScopedLock( log->mOwnLock );
			if( log->mOwnLog.open(logfile,IStream::AM_WRITE) )
				BLADE_EXCEPT(EXC_FILE_WRITE,BTString("unable to creat/write log file:" )+ logfile);
		}
		return HLOG(log);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LogManager::setLogLevel(ILog::ELogLevel level)
	{
		Log*	LogList[] = {&Log::msDebugOutput,&Log::msInformation,&Log::msWarning,&Log::msError };
		for(size_t i = 0; i < sizeof(LogList)/sizeof(void*); ++i )
		{
			bool enable =  LogList[i]->getLevel() > level;
			LogList[i]->setEnable(enable);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			LogManager::redirectLog(ILog& log, const HLOGREDIRECTOR& redirection, bool preserve/* = true*/)
	{
		Log& l = static_cast<Log&>(log);
		l.redirectLog(redirection, preserve);
		return true;
	}
	
}//namespace Blade