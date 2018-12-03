/********************************************************************
	created:	2013/03/10
	filename: 	Profiling.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Profiling_h__
#define __Blade_Profiling_h__

#if defined(BLADE_LW_DISABLED_FILE_SCOPE) || defined(BLADE_LW_DISABLED)

class FrameAccumulateProfiling
{
public:
	static void onLoopInit() {}
	static void onLoopFinish() {}
};

#	define BLADE_LW_PROFILING_FUNCTION_NAME( _TSTR_FUNCTION, _NAME) 
#	define BLADE_LW_PROFILING(_NAME) 
#	define BLADE_LW_PROFILING_FUNCTION() 

#	define BLADE_LW_PROFILING_FUNCTION_NAME_SWITCH( _TSTR_FUNCTION, _NAME, _switch) 
#	define BLADE_LW_PROFILING_SWITCH(_NAME, _switch) 
#	define BLADE_LW_PROFILING_FUNCTION_SWITCH(_switch) 


#	define BLADE_FAPROFILING_FUNCTION_NAME( _TSTR_FUNCTION, _NAME) 
#	define BLADE_FA_PROFILING(_NAME) 
#	define BLADE_FA_PROFILING_FUNCTION() 

#	define BLADE_FA_PROFILING_FUNCTION_NAME_SWITCH( _TSTR_FUNCTION, _NAME, _switch) 
#	define BLADE_FA_PROFILING_SWITCH(_NAME, _switch) 
#	define BLADE_FA_PROFILING_FUNCTION_SWITCH(_switch) 
#else


#include <utility/String.h>

namespace Blade
{
	///class for profiling once for each function call\n
	///and just for one frame, don't track/save the long-term data for statistics
	class ITimeDevice;
	class BLADE_BASE_API LightWeightProfiling : public NonCopyable, public NonAllocatable, public NonAssignable
	{
	public:
		LightWeightProfiling(const TString& Function, const TString& info = TString::EMPTY, bool enabled = true);
		~LightWeightProfiling();
	protected:
		const TString	mFunction;
		const TString	mInfo;
		ptrdiff_t		mMemory;
		ITimeDevice*	mTimer;
	};

	class BLADE_BASE_API FrameAccumulateProfiling : public NonCopyable, public NonAllocatable, public NonAssignable
	{
	public:
		FrameAccumulateProfiling(const TString& Function, const TString& info = TString::EMPTY, bool enabled = true);
		~FrameAccumulateProfiling();
	protected:
		const TString	mFunction;
		const TString	mInfo;
		ptrdiff_t		mMemory;
		ITimeDevice*	mTimer;
	public:
		static void onLoopInit();
		static void onLoopFinish();
	};


#	define BLADE_LW_PROFILING_FUNCTION_NAME(_TSTR_FUNCTION, _NAME) LightWeightProfiling lwpf##_NAME(_TSTR_FUNCTION, BTString(#_NAME) )
#	define BLADE_LW_PROFILING(_NAME) BLADE_LW_PROFILING_FUNCTION_NAME( BLADE_TFUNCTION, _NAME)
#	define BLADE_LW_PROFILING_FUNCTION() BLADE_LW_PROFILING(DEFAULT)

#	define BLADE_LW_PROFILING_FUNCTION_NAME_TAG(_TSTR_FUNCTION, _NAME, _TAGSTR) LightWeightProfiling lwpf##_NAME(_TSTR_FUNCTION, BTString(#_NAME) + BTString(":") + _TAGSTR )
#	define BLADE_LW_PROFILING_TAG(_NAME, _TAGSTR) BLADE_LW_PROFILING_FUNCTION_NAME_TAG( BLADE_TFUNCTION, _NAME, _TAGSTR)

#	define BLADE_LW_PROFILING_FUNCTION_NAME_SWITCH( _TSTR_FUNCTION, _NAME, _switch) LightWeightProfiling lwpf##_NAME(_TSTR_FUNCTION, BTString(#_NAME), _switch)
#	define BLADE_LW_PROFILING_SWITCH(_NAME, _switch) BLADE_LW_PROFILING_FUNCTION_NAME_SWITCH( BLADE_TFUNCTION, _NAME, _switch)
#	define BLADE_LW_PROFILING_FUNCTION_SWITCH(_switch) BLADE_LW_PROFILING_SWITCH(DEFAULT, _switch)



#	define BLADE_FA_PROFILING_FUNCTION_NAME(_TSTR_FUNCTION, _NAME) FrameAccumulateProfiling fapf##_NAME(_TSTR_FUNCTION, BTString(#_NAME) )
#	define BLADE_FA_PROFILING(_NAME) BLADE_FA_PROFILING_FUNCTION_NAME( BLADE_TFUNCTION, _NAME)
#	define BLADE_FA_PROFILING_FUNCTION() BLADE_FA_PROFILING(DEFAULT)

#	define BLADE_FA_PROFILING_FUNCTION_NAME_TAG(_TSTR_FUNCTION, _NAME, _TAGSTR) FrameAccumulateProfiling fapf##_NAME(_TSTR_FUNCTION, BTString(#_NAME) + BTString(":") + _TAGSTR )
#	define BLADE_FA_PROFILING_TAG(_NAME, _TAGSTR) BLADE_FA_PROFILING_FUNCTION_NAME_TAG( BLADE_TFUNCTION, _NAME, _TAGSTR)

#	define BLADE_FA_PROFILING_FUNCTION_NAME_SWITCH( _TSTR_FUNCTION, _NAME, _switch) FrameAccumulateProfiling fapf##_NAME(_TSTR_FUNCTION, BTString(#_NAME), _switch)
#	define BLADE_FA_PROFILING_SWITCH(_NAME, _switch) BLADE_FA_PROFILING_FUNCTION_NAME_SWITCH( BLADE_TFUNCTION, _NAME, _switch)
#	define BLADE_FA_PROFILING_FUNCTION_SWITCH(_switch) BLADE_FA_PROFILING_SWITCH(DEFAULT, _switch)

#endif
	
}//namespace Blade

#endif//__Blade_Profiling_h__