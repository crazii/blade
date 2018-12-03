/********************************************************************
	created:	2011/08/27
	filename: 	D3D9Helper.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "D3D9Helper.h"
#include <interface/ILog.h>
#include <interface/IEventManager.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	D3D9Helper::D3D9Helper()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	D3D9Helper::~D3D9Helper()
	{
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void D3D9Helper::outputString(const TString& str, ILog::ELogLevel level/* = ILog::LL_CRITICAL*/)
	{
		ILog* logs[] = { NULL, &ILog::Error, &ILog::Warning, &ILog::Information, &ILog::DebugOutput };
		if (level >= ILog::LL_BEGIN && level < ILog::LL_COUNT)
		{
			if(logs[level] != NULL)
				*logs[level] << ILog::noPrefix << str << ILog::endLog;
		}
	}

}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS