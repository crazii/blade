/********************************************************************
	created:	2014/06/17
	filename: 	ConsoleLogRedirector.h
	author:		Crazii
	
	purpose:	moved from DebugLogRedirector to single file
*********************************************************************/
#ifndef __Blade_ConsoleLogRedirector_h__
#define __Blade_ConsoleLogRedirector_h__
#include <interface//ILog.h>
#include <memory/BladeMemory.h>


namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class ConsoleLogRedirector : public ILogRedirector, public StaticAllocatable
	{
	public:
		ConsoleLogRedirector() :mError(false)	{}
		~ConsoleLogRedirector()					{}

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

		/** @brief  */
		bool			hasError() const		{return mError;}
		/** @brief  */
		void			setError(bool error)	{mError = error;}

		bool	mError;
	};
	
}//namespace Blade


#endif // __Blade_ConsoleLogRedirector_h__
