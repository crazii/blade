/********************************************************************
	created:	2011/08/28
	filename: 	Application.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Application.h>
#include <ICmdParser.h>
#include <AppConfig.h>

#include <interface/IFramework.h>
#include <interface/IConfigManager.h>
#include <interface/ILogManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/ITaskManager.h>
#include <interface/IResourceManager.h>
#include <interface/public/IStartupOutput.h>
#include <ConfigTypes.h>
#include <BladeBase_blang.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	const TString AppConfig::CONFIG_MODE = BTString("config");
	const TString AppConfig::DATA_PATH = BTString("dpath");
	const TString AppConfig::CONFIG_PATH = BTString("cpath");
	const TString AppConfig::PLUGIN_PATH = BTString("pluginpath");


	const TString DEF_DATA_PATH = BTString("../Data");
	const TString DEF_CONFIG_PATH = BTString("../Config");
	const TString DEF_PLUGIN_PATH = BTString("./Plugins");

	//////////////////////////////////////////////////////////////////////////
	AppConfig::AppConfig(const TString& name, const TString& desc, const Version ver)
		:mAppName(name)
		,mDesc(desc)
		,mVersion(ver)
	{
		mDataPath = DEF_DATA_PATH;
		mConfigPath = DEF_CONFIG_PATH;
		mPluginPath = DEF_PLUGIN_PATH;
	}

	//////////////////////////////////////////////////////////////////////////
	AppConfig::~AppConfig()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void		AppConfig::setupAppCmdArgument()
	{
		ICmdParser& cmdOpt = ICmdParser::getSingleton();

		cmdOpt.addOption( AppConfig::CONFIG_MODE, BTString("start config mode."),TEXT('C'),CAT_BOOL, BTString("false") );
		cmdOpt.addOption( AppConfig::DATA_PATH, BTString("application data file path."),TEXT('d'),CAT_STRING, mDataPath );
		cmdOpt.addOption( AppConfig::CONFIG_PATH, BTString("application config file path."),TEXT('c'),CAT_STRING, mConfigPath );
		cmdOpt.addOption( AppConfig::PLUGIN_PATH, BTString("application plugins path."),TEXT('p'),CAT_STRING, mPluginPath );
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Application::Application(const TString& type)
		:mName( BTString("Blade Application") )
		,mType(type)
		,mVersion(0,2,'B')
		,mDescription( BTString("an application base on Blade Engine.") )
		,mCommonCfgFile(BTString("commoncfg.cfg"))
		,mConfigXML(BTString("BladeConfig.xml"))
		,mResourceCfgFile( BTString("resource.cfg") )
		,mPluginConfigFile( TString::EMPTY )
		,mLogFile(BTString("Blade.log"))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Application::~Application()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Application::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Application::getType() const
	{
		return mType;
	}

	//////////////////////////////////////////////////////////////////////////
	Version			Application::getVersion() const
	{
		return mVersion;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Application::getDesc() const
	{
		return mDescription;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Application::setCommonConfigFile(const TString& cfg)
	{
		mCommonCfgFile = cfg;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Application::setConfigFile(const TString& cfgsetting)
	{
		mConfigXML = cfgsetting;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Application::setResourceConfigFile(const TString& resCfg)
	{
		mResourceCfgFile = resCfg;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Application::setPluginConfigFile(const TString& plugincfg)
	{
		mPluginConfigFile = plugincfg;
	}

	//////////////////////////////////////////////////////////////////////////
	void			Application::setLogFile(const TString& log)
	{
		mLogFile = log;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Application::initialize(IAppConfig& cfg)
	{
		mName = cfg.getAppName();
		mDescription = cfg.getDescription();
		mVersion = cfg.getVersion();

		const TString& configpath = this->getConfigPath();
		const TString& dataPath = this->getDataPath();
		const TString& pluginPath = this->getPluginPath();
		const TString& cwdPath = this->getCurrentPath();
		const TString& appPath = IPlatformManager::getSingleton().getProcesFilePath();

		ILogManager& logManager = ILogManager::getSingleton();
		IResourceManager& resourceManager = IResourceManager::getSingleton();
		IConfigManager& configManager = IConfigManager::getSingleton();
		IFramework& framework = IFramework::getSingleton();
		ITaskManager& taskManager = ITaskManager::getSingleton();

		//initialize log manager
		logManager.initialize(mLogFile/*, ILog::LL_NONE*/);

		//initialize resource manager
		resourceManager.initialize(appPath, dataPath, cwdPath);
		//scheme path for plugins, this may NOT take effect, because this path can be overridden by plugin config file
		resourceManager.registerScheme(BTString("plugins"), pluginPath, false);
		resourceManager.registerScheme(BTString("configs"), configpath, false);

		//resource config
		HSTREAM stream = resourceManager.loadStream( BTString("configs:/") +  mResourceCfgFile );
		assert( stream != NULL );
		resourceManager.readConfig( stream );

		//plugin config file
		if( mPluginConfigFile != TString::EMPTY )
			mPluginConfigFile = BTString("configs:") + BTString("/") + mPluginConfigFile;

		//common/simple config (ini)
		stream = IResourceManager::getSingleton().loadStream(BTString("configs:/") +  mCommonCfgFile);
		assert( stream != NULL );
		configManager.loadCommonConfig( stream );

		framework.initialize();
		size_t nProcessor = IPlatformManager::getSingleton().getCPUInfo().mProcessorCount;
		nProcessor = std::min<size_t>(nProcessor+1, 4U);
		taskManager.initialize(BTString("Default"), nProcessor);

		//init app config (xml/config via UI) before loadConfig
		HCONFIG appConfig(BLADE_NEW ConfigGroup(BXLang("General Config")));
		this->setupAppConfig(*appConfig);
		configManager.addConfig(appConfig);

		if( !this->preInitialize() )
			return false;

		if( !this->doInitialize() )
			return false;

		//because a subsystem may add its config entry into the config manager,
		//config should be loaded after subsystem installed.
		stream = resourceManager.loadStream( BTString("configs:/") +  mConfigXML );
		configManager.loadConfig(stream);

		//apply config after loadConfig
		this->applyConfig(*appConfig);

		return this->postInitialize();
	}

	//////////////////////////////////////////////////////////////////////////
	void			Application::close()
	{
		this->closeImpl();
		IFramework::getSingleton().shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Application::run(bool bModal)
	{
		static IFramework& framework = IFramework::getSingleton();
		static IEnvironmentManager& envManager = IEnvironmentManager::getSingleton();
		static ITaskManager& taskManager = ITaskManager::getSingleton();
		static IResourceManager& resManager = IResourceManager::getSingleton();
		static IPlatformManager& platformManager = IPlatformManager::getSingleton();

		try
		{
			this->beginRun();

			do
			{
				bool close = envManager.getVariable(ConstDef::EnvBool::WORK_TERMINATE);
				if (close)
					break;
				//BLADE_LW_PROFILING_FUNCTION();

				ITask* task = resManager.update();
				if (task != NULL)
				{
					BLADE_LOG(DebugOutput, TEXT("ResourceManager task added:") << task->getName());
					taskManager.addTask(task);
				}

				{
					//BLADE_LW_PROFILING(RUN);
					taskManager.runAllTasks();
				}
				{
					//BLADE_LW_PROFILING(UPDATE);
					taskManager.updateAllTasks();
				}

				{
					//BLADE_LW_PROFILING(FRAMEWORK_UPDATE);
					framework.update();
				}

				this->loopRun();

				//no task running, release CPU load
				if (taskManager.getTaskCount() <= 1)
					platformManager.sleep(30);

			} while (bModal);

			this->endRun();
		}

		catch (Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription());
			platformManager.showMessageBox(e.getFullDescription(), e.getTypeName());
			return false;
		}

		catch (std::exception& e)
		{
			BLADE_LOG(Error, e.what());
			platformManager.showMessageBox(e.what());
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void Application::setupAppConfig(IConfig& root)
	{
		TStringParam langList;
		ILangTableManager::getSingleton().getSupportedLanuages(langList);

		//check if system language is supported by app, if yes, use it as default
		const TString& systemLang = ILangTableManager::getSingleton().getSystemLang();
		TString defaultLang = ILangTableManager::DEFAULT_LANG;
		for(size_t i = 0; i < langList.size(); ++i)
		{
			if( langList[i] == systemLang )
				defaultLang = systemLang;
		}

		IConfig* langConfig = BLADE_NEW ConfigAtom(BTString("Language"), langList, CAF_NORMAL);
		langConfig->setValue(defaultLang);

		root.addSubConfigPtr(langConfig);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Application::applyConfig(const IConfig& root)
	{
		//read language config
		IConfig* config = root.getSubConfigByName(BTString("Language"));
		assert(config != NULL);
		if( config == NULL )
		{
			assert(false && "impossible, someone removed lang config after it added by Application.");
			return false;
		}
		HSTREAM langFile = IResourceManager::getSingleton().loadStream( BTString("media:/gamestudio.blang") );
		return ILangTableManager::getSingleton().initialize(langFile, config->getValue());
	}

	//////////////////////////////////////////////////////////////////////////
	bool Application::preInitialize()
	{
		try
		{
			if( mPluginConfigFile == TString::EMPTY )
				IPlatformManager::getSingleton().initialize();
			else
			{
				HSTREAM stream = IResourceManager::getSingleton().loadStream(mPluginConfigFile);
				IPlatformManager::getSingleton().initialize(stream);
			}
			
			this->onPluginInstalled();

			return true;
		}


		catch(Exception& e)
		{
			BLADE_LOG(Error, e.getFullDescription() );
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool Application::postInitialize()
	{
		if( Factory<IStartupOutput>::getSingleton().getNumRegisteredClasses() > 0 )
		{
			TString softwareInfo = BTLang(BLANG_VERSION) + BTString(" ") + mVersion.getVersionString() + BTString("\n") + 
				BTLang(BLANG_COPYRIGHT);
			IStartupOutput::getSingleton().initialize(softwareInfo);
		}

		const CmdOption* option = ICmdParser::getSingleton().getOption( AppConfig::CONFIG_MODE );
		if( option != NULL && option->getValue() == true )
		{
			IConfigManager::getSingleton().showConfigDialog();
			return false;
		}

		IPlatformManager::getSingleton().initAllPlugins();
		return true;
	}

	/************************************************************************/
	/* custom method                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	TString		Application::getCommonConfigFile() const
	{
		return BTString("configs:/") + mCommonCfgFile;
	}

	//////////////////////////////////////////////////////////////////////////
	TString		Application::getConfigFile() const
	{
		return BTString("configs:/") + mConfigXML;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Application::loadSubSystems(const TString& moduleCfg/* = BTString("plugins.cfg")*/)
	{
		const TString& path = this->getConfigPath();

		TString moduleFile = path + BTString("/") + moduleCfg;
		HSTREAM stream = IResourceManager::getSingleton().loadStream( moduleFile );
		if( stream == NULL )
			return false;

		IFramework& framework = IFramework::getSingleton();

		ConfigFile file;
		file.open(stream);

		const CFGFILE_SETTING* cfgSetting = file.getAllConfigs();
		const CFGFILE_SECTION* section = cfgSetting->getSection(BTString("SubSystems"));
		for(size_t i = 0; i < section->getEntryCount(); ++i)
		{
			const CFGFILE_ENTRY& entry = section->getEntry(i);

			ISubsystem* subsystem = BLADE_FACTORY_CREATE(ISubsystem, entry.getName());
			framework.addSubSystem(subsystem);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Application::getConfigPath() const
	{
		if (mConfigPath.empty())
		{
			ICmdParser& clp = ICmdParser::getSingleton();
			Factory<Application>& appFactory = Factory<Application>::getSingleton();

			const CmdOption* option = clp.getOption(AppConfig::CONFIG_PATH);
			const TString& configPath = option->getValue();

			if (configPath == DEF_CONFIG_PATH
				&& appFactory.isClassRegistered(this->getType())	//skip tool app (not registered, and no need to)
				&& appFactory.getRegisteredClass(0) != this->getType())
				mConfigPath = DEF_CONFIG_PATH + BTString("_") + this->getType();
			else
				mConfigPath = configPath;
		}
		return mConfigPath;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Application::getDataPath() const
	{
		const CmdOption* option = ICmdParser::getSingleton().getOption( AppConfig::DATA_PATH );
		const TString& dataPath = option->getValue();
		return dataPath;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& Application::getPluginPath() const
	{
		const CmdOption* option = ICmdParser::getSingleton().getOption( AppConfig::PLUGIN_PATH );
		const TString& dataPath = option->getValue();
		return dataPath;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& Application::getCurrentPath() const
	{
		return IPlatformManager::getSingleton().getCurrentWorkingPath();
	}

}//namespace Blade
