/********************************************************************
	created:	2010/03/28
	filename: 	LogManager.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_LogManager_h__
#define __Blade_LogManager_h__
#include <interface/ILogManager.h>
#include <Singleton.h>
#include <interface/public/file/IFileDevice.h>

namespace Blade
{
	class LogManager : public ILogManager ,public Singleton<LogManager>
	{
	public:
		LogManager();
		~LogManager();

		/*
		* @describe 
		* @param 
		* @return 
		*/
		virtual bool			initialize(const TString& logfile,ILog::ELogLevel level = ILog::LL_DEFAULT);

		/*
		@describe
		@param
		@return
		*/
		virtual bool			initialize(const HSTREAM& logstream,ILog::ELogLevel level = ILog::LL_DEFAULT);

		/*
		@describe create a new log
		@param [in] logfile: the output file for the log. if it is empty, the log will share streams with default log file
		@return 
		*/
		virtual	HLOG			createLog(ILog::ELogLevel level, const TString& logfile = TString::EMPTY);

		/*
		* @describe 
		* @param 
		* @return 
		*/
		virtual bool			setLogLevel(ILog::ELogLevel level);

		/*
		@describe redirect log
		@param
		@return
		*/
		virtual bool			redirectLog(ILog& log, const HLOGREDIRECTOR& redirection, bool preserve = true);

	protected:
		bool					mInitialized;
	};//class LogManager


	
}//namespace Blade


#endif //__Blade_LogManager_h__