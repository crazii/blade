/********************************************************************
	created:	2015/09/06
	filename: 	ToolLog.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ToolLog_h__
#define __Blade_ToolLog_h__
#include <BladeApplication.h>
#include <interface/ILog.h>

namespace Blade
{
	namespace ToolLog
	{
		extern BLADE_APPLICATION_API HLOG Error;
		extern BLADE_APPLICATION_API HLOG Warning;
	}//namespace ToolLog
	
}//namespace Blade


#endif // __Blade_ToolLog_h__