/********************************************************************
	created:	2014/06/17
	filename: 	ConsoleLogRedirector.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
# undef TEXT
#	include <Windows.h>
#endif
#include <ConsoleLogRedirector.h>
#include <BladeStdTypes.h>

namespace Blade
{

	/************************************************************************/
	/* ILogRedirector interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void ConsoleLogRedirector::outputLog(ILog::ELogLevel level, const tchar* content)
	{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		::OutputDebugString(content);
#endif

		if( level == ILog::LL_CRITICAL )
		{
			std::tcerr << content;
			mError = true;
		}
		else if(level == ILog::LL_CAREFUL)
		{
			std::tcout << content;
		}
	}

	
}//namespace Blade
