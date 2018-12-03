/********************************************************************
	created:	2010/08/12
	filename: 	Event.h
	author:		Crazii
	purpose:	
	2015/12/10: change name from StateData to Event
*********************************************************************/
#ifndef __Blade_Event_h__
#define __Blade_Event_h__
#include <BladeFramework.h>
#include <utility/String.h>

namespace Blade
{

	/** @brief  */
	class Event
	{
	public:
		Event() {}
		Event(const TString& name) :mName(name) {}
		TString		mName;
	};

	/** @brief  */
	typedef void (*pfnEventHandler) (const Event& state);


	/************************************************************************/
	/* framework predefined events, dispatched in main sync state                                                                     */
	/************************************************************************/
	namespace ConstDef
	{
		extern const BLADE_FRAMEWORK_API TString EVENT_MAIN_LOOP;
		extern const BLADE_FRAMEWORK_API TString EVENT_LOOP_DATA_INIT;
		extern const BLADE_FRAMEWORK_API TString EVENT_LOOP_DATA_CLEAR;

	}//namespace ConstDef
	
}//namespace Blade


#endif //__Blade_Event_h__