/********************************************************************
	created:	2010/09/04
	filename: 	DebugLogRedirector.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	undef TEXT
#	include <Windows.h>
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#	include <android/log.h>
#endif
#include <DebugLogRedirector.h>

namespace Blade
{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
	//////////////////////////////////////////////////////////////////////////
	DebugLogRedirector::DebugLogRedirector()
	{
		mSwitch[0] = true;
		mSwitch[1] = true;
		mSwitch[2] = true;
		mSwitch[3] = true;
		mSwitch[4] = true;
	}

	//////////////////////////////////////////////////////////////////////////
	DebugLogRedirector::~DebugLogRedirector()
	{

	}

	/************************************************************************/
	/* ILogRedirector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void DebugLogRedirector::outputLog(ILog::ELogLevel level,const tchar* content)
	{
		if( level >= ILog::LL_CRITICAL && level <= ILog::LL_DEBUG && mSwitch[level] )
		{
			::OutputDebugString(content);
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		DebugLogRedirector::enableLogLevelOutput(ILog::ELogLevel level,bool bEnable)
	{
		if( level >= ILog::LL_CRITICAL && level <= ILog::LL_DEBUG )
			mSwitch[level] = bEnable;
	}

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

	//////////////////////////////////////////////////////////////////////////
	DebugLogRedirector::DebugLogRedirector()
	{
		mSwitch[0] = true;
		mSwitch[1] = true;
		mSwitch[2] = true;
		mSwitch[3] = true;
	}

	//////////////////////////////////////////////////////////////////////////
	DebugLogRedirector::~DebugLogRedirector()
	{

	}

	/************************************************************************/
	/* ILogRedirector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void DebugLogRedirector::outputLog(ILog::ELogLevel level,const tchar* content)
	{
		if( level >= ILog::LL_CRITICAL && level <= ILog::LL_DEBUG && mSwitch[level] )
		{
			TempBuffer buffer;
			const char* logBuffer = StringConverter::TStringToString(buffer, content);

			if( level == ILog::LL_DETAIL )
				::__android_log_print(ANDROID_LOG_INFO, "blade", "%s", logBuffer);
			else if( level == ILog::LL_DEBUG )
				::__android_log_print(ANDROID_LOG_VERBOSE, "blade", "%s", logBuffer);
			else if( level == ILog::LL_CAREFUL )
				::__android_log_print(ANDROID_LOG_WARN, "blade", "%s", logBuffer);
			else if( level == ILog::LL_CRITICAL )
				::__android_log_print(ANDROID_LOG_ERROR, "blade", "%s", logBuffer);
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		DebugLogRedirector::enableLogLevelOutput(ILog::ELogLevel level,bool bEnable)
	{
		if( level >= ILog::LL_CRITICAL && level <= ILog::LL_DEBUG )
			mSwitch[level] = bEnable;
	}

#else

#error not implemented!

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM
	
}//namespace Blade
