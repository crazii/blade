/********************************************************************
	created:	2010/03/28
	filename: 	ILogManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ILogManager_h__
#define __Blade_ILogManager_h__
#include <interface/ILog.h>
#include <interface/InterfaceSingleton.h>
#include <interface/public/file/IStream.h>

namespace Blade
{
	class ILogManager : public InterfaceSingleton<ILogManager>
	{
	public:
		virtual ~ILogManager()	{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			initialize(const TString& logfile, ILog::ELogLevel level = ILog::LL_DEFAULT) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool			initialize(const HSTREAM& logstream, ILog::ELogLevel level = ILog::LL_DEFAULT) = 0;

		/**
		@describe create a new log
		@param [in] logfile: the output file for the log. if it is empty, the log will share streams with default log file
		@return 
		*/
		virtual	HLOG			createLog(ILog::ELogLevel level, const TString& logfile = TString::EMPTY) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			setLogLevel(ILog::ELogLevel level) = 0;

		/**
		@describe redirect log
		@param [in] preserve: redirect log output but preserve original output to log file.
		@return
		*/
		virtual bool			redirectLog(ILog& log, const HLOGREDIRECTOR& redirection, bool preserve = true) = 0;

		inline bool	redirectLogs(const HLOGREDIRECTOR& redirection, bool preserve = true)
		{
			bool ret = this->redirectLog(ILog::DebugOutput, redirection, preserve);
			ret = this->redirectLog(ILog::Information, redirection, preserve) && ret;
			ret = this->redirectLog(ILog::Warning, redirection, preserve) && ret;
			ret = this->redirectLog(ILog::Error, redirection, preserve) && ret;
			return ret;
		}

	};//class ILogManager

	extern template class BLADE_BASE_API Factory<ILogManager>;
	
}//namespace Blade


#endif //__Blade_ILogManager_h__