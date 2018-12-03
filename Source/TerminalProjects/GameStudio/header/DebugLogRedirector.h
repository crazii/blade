/********************************************************************
	created:	2010/09/04
	filename: 	DebugLogRedirector.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_DebugLogRedirector_h__
#define __Blade_DebugLogRedirector_h__
#include <interface//ILog.h>
#include <memory/BladeMemory.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class DebugLogRedirector : public ILogRedirector, public StaticAllocatable
	{
	public:
		DebugLogRedirector();
		~DebugLogRedirector();

		/************************************************************************/
		/* ILogRedirector interface                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		virtual void outputLog(ILog::ELogLevel level,const tchar* content);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/*
		@describe
		@param
		@return
		*/
		void		enableLogLevelOutput(ILog::ELogLevel level,bool bEnable);


	private:
		bool		mSwitch[5];
	};
	
}//namespace Blade


#endif //  __Blade_DebugLogRedirector_h__
