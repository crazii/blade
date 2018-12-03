/********************************************************************
	created:	2013/05/19
	filename: 	ToolApplication.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#undef TEXT
#include <Windows.h>
#endif
#include <ToolApplication.h>
#include <interface/IFramework.h>
#include <interface/IConfigManager.h>
#include <interface/ILogManager.h>
#include <interface/IPlatformManager.h>
#include <interface/ITaskManager.h>
#include <interface/IResourceManager.h>
#include <interface/IEnvironmentManager.h>
#include <databinding/IDataSourceDataBase.h>

#include <ICmdParser.h>
#include <ConsoleLogRedirector.h>

#define TOOLAPP_CHANGE_CWD 0

namespace Blade
{
	namespace ToolLog
	{
		HLOG Error;
		HLOG Warning;
	}//namespace ToolLog

	static HLOGREDIRECTOR redirector;

	//////////////////////////////////////////////////////////////////////////
	ToolApplication::ToolApplication()
		:Application(BTString("Tool"))
	{
		//tools: write log file to app running path to avoid file pollution
		const TString& processPath = IPlatformManager::getSingleton().getProcesFilePath();
		this->setLogFile(processPath + BTString("/") + mLogFile);

#if BLADE_COMPILER == BLADE_COMPILER_MSVC && BLADE_DEBUG
		_set_error_mode(_OUT_TO_MSGBOX);
#endif
		//change CWD
		//note: DO NOT use window API, although tool applications currently are running under windows.
		//maybe they will be running on other platforms in future.

#if TOOLAPP_CHANGE_CWD
		const TString& cmd = ICmdParser::getSingleton().getCommand();
		assert( !cmd.empty() );
		mCurrentWorkingDir = IPlatformManager::getSingleton().getCurrentWorkingPath();

		TString absCmd;
		if( TStringHelper::isAbsolutePath(cmd) )
			//remove ../ or ./
			absCmd = TStringHelper::standardizePath(cmd);
		else
			absCmd = TStringHelper::getAbsolutePath(mCurrentWorkingDir, cmd);

		TString dir,filename;
		TStringHelper::getFilePathInfo(absCmd, dir, filename);
		bool ret = IPlatformManager::getSingleton().setCurrentWorkingPath(dir);
		assert(ret);
		BLADE_UNREFERENCED(ret);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	ToolApplication::~ToolApplication()
	{
		ToolLog::Error.clear();
		ToolLog::Warning.clear();

		redirector.clear();
#if TOOLAPP_CHANGE_CWD
		IPlatformManager::getSingleton().setCurrentWorkingPath(mCurrentWorkingDir);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ToolApplication::hasError() const
	{
		return static_cast<ConsoleLogRedirector*>(redirector)->hasError();
	}

	//////////////////////////////////////////////////////////////////////////
	void			ToolApplication::setError(bool error)
	{
		return static_cast<ConsoleLogRedirector*>(redirector)->setError(error);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ToolApplication::preInitialize()
	{
		IEnvironmentManager::getSingleton().setVariable( ConstDef::EnvString::WORKING_MODE, BTString("tool") );
		this->initPlugins();
		this->onPluginInstalled();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ToolApplication::postInitialize()
	{
		redirector.bind( BLADE_NEW ConsoleLogRedirector() );
		ILogManager::getSingleton().redirectLog(ILog::Error, redirector, true);
		ILogManager::getSingleton().redirectLog(ILog::Warning, redirector, true);
#if BLADE_DEBUG
		ILogManager::getSingleton().redirectLog(ILog::DebugOutput, redirector, true);
#endif

		IPlatformManager::getSingleton().initAllPlugins();
		IFramework::getSingleton().start();

		ToolLog::Error = ILogManager::getSingleton().createLog(ILog::LL_CRITICAL);
		ToolLog::Warning = ILogManager::getSingleton().createLog(ILog::LL_CAREFUL);
		ILogManager::getSingleton().redirectLog(*ToolLog::Error, redirector, true);
		ILogManager::getSingleton().redirectLog(*ToolLog::Warning, redirector, true);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	void	ToolApplication::closeImpl()
	{
		try
		{
			//Note: save config have problem on parallel running, when one process closing & saving, while another process loading configs
			//save config only on config mode (GUI popup)
			if( ICmdParser::getSingleton().getOption(BTString("config"))->getValue() == true )
			{
				HSTREAM stream = IResourceManager::getSingleton().openStream( this->Application::getCommonConfigFile(), false );
				IConfigManager::getSingleton().saveCommonConfig( stream );
				stream = IResourceManager::getSingleton().openStream( this->Application::getConfigFile(), false );
				IConfigManager::getSingleton().saveConfig( stream );
			}

			IConfigManager::getSingleton().clearAllConfigs();
			IFramework::getSingleton().terminate();
			IResourceManager::getSingleton().removeAllResources();
			ITaskManager::getSingleton().shutdown();
			IPlatformManager::getSingleton().shutdownAllPlugins();
			IDataSourceDataBase::getSingleton().shutdown();
			IPlatformManager::getSingleton().unloadAllPlugins();
		}

		catch(Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription());
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
		}
	}

}//namespace Blade