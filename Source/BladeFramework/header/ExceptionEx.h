/********************************************************************
	created:	2010/03/29
	filename: 	ExceptionEx.h
	author:		Crazii
	
	purpose:	exception extension for framework
*********************************************************************/
#ifndef __Blade_ExceptionEx_h__
#define __Blade_ExceptionEx_h__
#include <BladeException.h>
#include <BladeFramework.h>

namespace Blade
{
	enum EExcptionCodeExtension
	{
		EXCE_STATE_ERROR = EXC_COUNT,
		EXCE_CONFIG_ERROR,
	};

	BLADE_DEFINE_EXCEPT(BLADE_FRAMEWORK_API,EXCE_STATE_ERROR,ExceptionStateError,"State Error Exception");
	BLADE_DEFINE_EXCEPT(BLADE_FRAMEWORK_API,EXCE_CONFIG_ERROR,ExceptionConfigError,"Config Error Exception");
	
}//namespace Blade


#endif //__Blade_ExceptionEx_h__
