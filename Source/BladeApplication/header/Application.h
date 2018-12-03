/*********************************************************************
	created:	2011/08/28
	filename: 	Application.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Application_h__
#define __Blade_Application_h__
#include <BladeApplication.h>
#include <utility/Version.h>
#include <interface/InterfaceSingleton.h>
#include <IAppConfig.h>

namespace Blade
{
	class IConfig;

	class BLADE_APPLICATION_API Application : public InterfaceSingleton<Application>
	{
	public:
		Application(const TString& type);
		virtual ~Application();

		/**
		@describe
		@param
		@return
		*/
		const TString&	getName() const;

		/**
		@describe 
		@param
		@return
		*/
		const TString&	getType() const;

		/**
		@describe 
		@param 
		@return 
		*/
		Version			getVersion() const;

		/**
		@describe 
		@param 
		@return 
		*/
		const TString&	getDesc() const;

		/**
		@describe the common config file to load/save
		@param 
		@return
		@remark the common config file is an ini style file.\n
		the value should be file name only , no paths, path will be added from command line,\n
		default: commoncfg.cfg
		*/
		void			setCommonConfigFile(const TString& cfg);

		/**
		@describe the config file to load/save for ISerializable interfaces
		@param 
		@return 
		@remark the configurable settings file is an xml file.\n
		the value should be file name only , no paths, path will be added from command line,\n
		default: BladeConfig.xml
		*/
		void			setConfigFile(const TString& cfgsetting);

		/**
		@describe the config file to define resource schemes
		@param 
		@return 
		@remark the configurable settings file is an ini style file
		the value should be file name only , no paths, path will be added from command line,\n
		default: resource.cfg
		*/
		void			setResourceConfigFile(const TString& resCfg);


		/**
		@describe the plugin config file to load/save for IConfig interfaces
		@param 
		@return 
		@default: EMPTY string (that will make it load all plugins exist in current directory)
		*/
		void			setPluginConfigFile(const TString& plugincfg);

		/**
		@describe the plain text format log file
		@param 
		@return 
		@remark default: Blade.log
		*/
		void			setLogFile(const TString& log);

		/**
		@describe init the application
		@param none
		@return result
		@retval true init succeeded and should continue
		@retval false init failed, or succeeded but should not continue
		@remark it will call preInitialize before init \n
		and postInitialize after init
		*/
		bool			initialize(IAppConfig& cfg);

		/**
		@describe 
		@param 
		@return 
		*/
		void			close();

		/**
		@describe run the application
		@param [in] bModal whether in modal mode(internal loop) or just run for once
		@return run result
		@retval true run succeeded
		@retval false run failed
		@remark this will call beginRun at first,and endRun last, and call loopRun() once/everytime depending on whether in modal mode
		*/
		virtual bool	run(bool bModal);

	protected:

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void	closeImpl() = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	doInitialize() = 0;

		/**
		@describe 
		@param
		@return
		*/
		virtual void	setupAppConfig(IConfig& root);

		/**
		@describe 
		@param
		@return
		*/
		virtual bool	applyConfig(const IConfig& root);


		/**
		@describe called before initialize
		@param none
		@return result
		@remark if this function return false,the application will stop initializing
		*/
		virtual bool preInitialize();

		/**
		@describe called after initialize
		@param none
		@return result
		*/
		virtual bool postInitialize();

		/**
		@describe this function will be called right after subsystems are added
		@param none
		@return none
		*/
		virtual void onSubSystemInstalled()	{}

		/**
		@describe this function will be called when plugins are installed
		@param 
		@return 
		*/
		virtual void onPluginInstalled()	{}

		/**
		@describe called before run
		@param none
		@return none
		*/
		virtual void beginRun() {}

		/**
		@describe called after run
		@param none
		@return none
		*/
		virtual void endRun() {}

		/**
		  @describe called in run loop (if in modal loop)
		  @param
		  @return
		*/
		virtual void loopRun() {}

		/*************************************************************************/
		/** custom method                                                                     */
		/*************************************************************************/
		/**
		@describe get the full path of the common config file
		@param 
		@return 
		*/
		TString		getCommonConfigFile() const;

		/**
		@describe get the full path of the configurable setting file
		@param 
		@return 
		*/
		TString		getConfigFile() const;

		/**
		@describe load subsystem & its modules according to module config file
		@param 
		@return 
		@note: this is optional, you can directly add subsystem & modules instead of loading this config
		*/
		bool		loadSubSystems(const TString& moduleCfg = BTString("plugins.cfg") );

		/**
		@describe 
		@param 
		@return 
		*/
		const TString&	getConfigPath() const;

		/**
		@describe 
		@param 
		@return 
		*/
		const TString& getDataPath() const;

		/**
		@describe 
		@param 
		@return 
		*/
		const TString& getPluginPath() const;

		/**
		@describe 
		@param
		@return
		*/
		const TString& getCurrentPath() const;

		TString		mName;
		TString		mType;
		Version		mVersion;
		TString		mDescription;

		mutable TString	mConfigPath;
		TString		mCommonCfgFile;
		TString		mConfigXML;
		TString		mResourceCfgFile;
		TString		mPluginConfigFile;
		TString		mLogFile;
	};

	extern template class BLADE_APPLICATION_API Factory<Application>;
	typedef Factory<Application> AppFactory;
	
	extern "C"
	{
		BLADE_APPLICATION_API bool	initializeAppFramework();
	}

}//namespace Blade



#endif // __Blade_Application_h__