/********************************************************************
	created:	2009/03/09
	filename: 	ConstDef.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ConstDef_h__
#define __Blade_ConstDef_h__
#include <BladeFramework.h>

namespace Blade
{

	namespace ConstDef
	{
		/************************************************************************/
		/* Environment variables                                                                     */
		/************************************************************************/
		namespace EnvBool
		{
		
			extern const BLADE_FRAMEWORK_API TString	WORK_TERMINATE;	//termination flag to exit main loop
			extern const BLADE_FRAMEWORK_API TString	DEVELOPER_MODE;	//develop mode switch, some options are visible in dev mode
		
		}//namespace EnvVarName

		namespace EnvString
		{
		
			extern const BLADE_FRAMEWORK_API TString	WORKING_MODE;	//mode string shared between plugins
			extern const BLADE_FRAMEWORK_API TString	STARTUP_IMAGE;	//startup image shared between plugins, mostly for UIs
		
		}//namespace EnvString
	
	}//namespace ConstDef

}//namespace Blade

#endif // __blade_constdef_h__