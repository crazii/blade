/********************************************************************
	created:	2011/08/29
	filename: 	AppConfig.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_AppConfig_h__
#define __Blade_AppConfig_h__

#include <IAppConfig.h>

namespace Blade
{

	class BLADE_APPLICATION_API AppConfig : public IAppConfig
	{
	public:
		//default command arguments
		static const TString CONFIG_MODE;
		static const TString DATA_PATH;
		static const TString CONFIG_PATH;
		static const TString PLUGIN_PATH;

	public:
		AppConfig(const TString& name, const TString& desc, const Version ver);
		~AppConfig();

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getAppName()		{return mAppName;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getDescription()	{return mDesc;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual Version			getVersion()		{return mVersion;}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			setupAppCmdArgument();

		/** @brief set data path, default "../Data/" */
		inline void		setDataPath(const TString& path)	{mDataPath = path;}
		inline void		setDataPath(const tchar* path)		{mDataPath = path;}
		inline const TString& getDataPath() const			{return mDataPath;}
		/** @brief set config path, default "../Config/" */
		inline void		setConfigPath(const TString& path)	{mConfigPath = path;}
		inline void		setConfigPath(const tchar* path)	{mConfigPath = path;}
		inline const TString& getConfigPath() const			{return mConfigPath;}
		/** @brief set plugin path, default "./Plugins/" */
		inline void		setPluginPath(const TString& path)	{mPluginPath = path;}
		inline void		setPluginPath(const tchar* path)	{mPluginPath = path;}
		inline const TString& getPluginPath() const			{return mPluginPath;}
		/** @brief set cwd: data path of resource manager, default ".", this is NOT the real cwd of platform manager */
		inline void		setCurrentPath(const TString& path)	{mCurrentPath = path;}
		inline void		setCurrentPath(const tchar* path)	{mCurrentPath = path;}
		inline const TString& getCurrentPath() const		{return mCurrentPath;}

	protected:
		TString	mAppName;
		TString	mDesc;
		Version mVersion;

		TString	mDataPath;
		TString	mConfigPath;
		TString	mPluginPath;
		TString	mCurrentPath;
	};

	

}//namespace Blade



#endif // __Blade_AppConfig_h__