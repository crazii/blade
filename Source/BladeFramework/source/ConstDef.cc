/********************************************************************
	created:	2009/03/09
	filename: 	ConstDef.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ConstDef.h>
#include <Event.h>

namespace Blade
{

	namespace ConstDef
	{
		/************************************************************************/
		/* Environment variables                                                                     */
		/************************************************************************/
		namespace EnvBool
		{
		
			const TString		WORK_TERMINATE =	BTString("exit");	
			const TString		DEVELOPER_MODE =	BTString("developer");	
		
		}//namespace EnvBool
		
		namespace EnvString
		{
		
			const TString		WORKING_MODE =		BTString("mode");
			const TString		STARTUP_IMAGE =		BTString("startup image");
		
		}//namespace EnvString

		const TString ENTITY_RESOURCE_SUFFIX = BTString("entity");
		
		/************************************************************************/
		/* framework predefined events                                                                     */
		/************************************************************************/
		const TString EVENT_MAIN_LOOP = BTString("::MainLoopBegin");
		const TString EVENT_LOOP_DATA_INIT = BTString("::LoopDataInit");
		const TString EVENT_LOOP_DATA_CLEAR = BTString("::LoopDataClear");

	}//namespace constdef
}//namespace Blade