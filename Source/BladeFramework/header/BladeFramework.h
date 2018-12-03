/********************************************************************
	created:	2009/03/29
	filename: 	BladeFramework.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeFramework_h__
#define __Blade_BladeFramework_h__
#include <BladeTypes.h>
#include <Handle.h>

#if !defined(BLADE_STATIC)	//dynamic link lib

#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
#		ifdef BLADE_FRAMEWORK_EXPORTS
#			define BLADE_FRAMEWORK_API __declspec( dllexport )
#		else
#			define BLADE_FRAMEWORK_API __declspec( dllimport )
#		endif
#	elif BLADE_COMPILER_GNU_EXTENSION
#		define BLADE_FRAMEWORK_API __attribute__ ((visibility("default")))
#	endif

#else

#	define BLADE_FRAMEWORK_API	//static link lib

#endif

namespace Blade
{
	class Event;

	class IConfig;
	typedef Handle<IConfig> HCONFIG;

	///generally, subsystems should make their PUBLIC data (scenes/elements through public interfaces) read only at TS_ASYNC_RUN state,
	///so that other subsystems can query them. 
	///if there are any public write operations (through public interfaces) happens in TS_ASYNC_RUN state, the operations should
	///be delayed to TS_ASYNC_UPDATE state.
	///subsystems should write/update their internal data on TS_ASYNC_UPDATE state, and MUST NOT access other subsystem's public data on TS_ASYNC_UPDATE state.
	typedef enum ETaskParallelState
	{
		TS_MAIN_SYNC,		///main synchronous state: single main task sync mode, no other tasks running parallel except the background task.
		TS_ASYNC_RUN,		///asynchronous run state: in call of ITaskManager::startAllTasks()
		TS_ASYNC_UPDATE,	///asynchronous update state: in call of ITaskManager::updateAllTasks()
	}TASK_STATE;

	///simplify usage
	extern BLADE_FRAMEWORK_API const volatile size_t* GLOBAL_TASK_STATE;

#define BLADE_TS_CHECK(_state) (*GLOBAL_TASK_STATE == _state)
#define BLADE_TS_VERIFY(_state) assert( BLADE_TS_CHECK(_state) )

#define BLADE_TS_INCLUDE(_state) (*GLOBAL_TASK_STATE == _state || *GLOBAL_TASK_STATE == TS_MAIN_SYNC)
#define BLADE_TS_VERIFY_INCLUDE(_state) assert( BLADE_TS_CHECK(_state) )

#define BLADE_TS_EXCLUDE(_state) (*GLOBAL_TASK_STATE != _state)
#define BLADE_TS_VERIFY_EXCLUDE(_state) assert( BLADE_TS_EXCLUDE(_state) )

	extern "C"
	{
		BLADE_FRAMEWORK_API bool	initializeFramework();
	}
	
}//namespace Blade

#endif // __Blade_BladeFramework_h__