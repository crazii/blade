/********************************************************************
	created:	2009/05/23
	filename: 	PlatformManager.h
	author:		Crazii
	
	purpose:	platform manager implementation
*********************************************************************/
#ifndef __Blade_PlatformManager_h__
#define __Blade_PlatformManager_h__
#include <interface/IPlatformManager.h>
#include "DynamicLib.h"
#include <utility/BladeContainer.h>

namespace Blade
{
#if BLADE_STATIC_PLUGIN
	typedef StaticList<IPlugin*> StaticPluginDB;
	extern StaticPluginDB PLUGIN_DATABASE;
#endif

	class PlatformManager : public IPlatformManager, public Singleton<PlatformManager>
	{
	public:
		PlatformManager();
		virtual ~PlatformManager();

		/************************************************************************/
		/* IPlatformManager specs interface                                                                     */
		/************************************************************************/
		/*
		@describe load all plugins in the plugin directory
		@param folder the folder in which plugin files be placed, if NULL, the archive is guessed as current dir on local disk
		@return result
		*/
		virtual bool	initialize(const TString& path = TString::EMPTY);

		/**
		@describe
		@param configStream the stream of the config file\
		this method is meant to load config files either in package or disk
		change notice 4/23/2013: remove internal dependency:
		use config file to de-couple from "resource manager" for HSTREAM creation
		@return
		*/
		virtual bool	initialize(const HSTREAM& configStream);

		/*
		@describe
		@param [in] pluginlist: string to bool pair list to indicates plugins to load and whether enable it immediately
		@param [in] path: if it is empty, use process image path (getProcesFilePath()).
		@return
		*/
		virtual bool	initialize(ParamList& pluginlist, const TString& path = TString::EMPTY);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual const CPUINFO& getCPUInfo() const;

		/*
		@describe
		@param
		@return
		*/
		virtual void	sleep(uint32 millisec);

		/*
		@describe get supported screen resolution list
		@param widthList & heightList must both be NULL or both not NULL
		@param current current screen resolution index in list
		@return count of resolutions, if widthList & heightList is NULL, return count only. 
		@remarks returned list are sorted in ascending order of width
		*/
		virtual size_t	getScreenResolutions(size_t& current, size_t* widthList = NULL, size_t* heightList = NULL) const;

		/*
		* @describe load a plugin
		* @param 
		* @return 
		*/
		virtual bool	loadPlugin(const TString& pathname, bool enable = true);

		/*
		* @describe unload a plugin
		* @param 
		* @return 
		*/
		virtual void	unloadPlugin(const TString& pathname);

		/*
		@describe get a plugin by name
		@param name the name of the plugin that is being retrieved
		@return pointer to the plugin
		@remark if name not found, return NULL
		*/
		virtual IPlugin*getPlugin(const TString& name) const;


		/*
		@describe install a plugin
		@param plugin to install
		@return install status result
		*/
		virtual bool	installPlugin(IPlugin* plugin);

		/*
		@describe uninstall a plugin
		@param plugin to uninstall
		@return 
		*/
		virtual bool	uninstallPlugin(IPlugin* plugin);


		/*
		@describe initialize all plugins
		@param none
		@return none
		*/
		virtual void	initAllPlugins();

		/*
		@describe shutdown all plugin
		@param none
		@return none
		*/
		virtual void	shutdownAllPlugins();


		/*
		@describe unload all plugins
		@param 
		@return 
		*/
		virtual void	unloadAllPlugins();


		/*
		@describe
		@param
		@return
		*/
		virtual MBRESULT	showMessageBox(const wchar* content,const wchar* caption = NULL,MBTYPE type = MBT_OK);

		/*
		@describe
		@param
		@return
		*/
		virtual MBRESULT	showMessageBox(const char* content,const char* caption = NULL,MBTYPE type = MBT_OK);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual MBRESULT	showMessageBox(MBTYPE type, const char* caption, const char* fmt, ...);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual MBRESULT	showMessageBox(MBTYPE type, const wchar* caption, const wchar* fmt, ...);

		/*
		@describe
		@param
		@return
		*/
		virtual const TString&	getCurrentWorkingPath() const;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool		setCurrentWorkingPath(const TString& path);

		/*
		@describe 
		@param
		@return
		*/
		virtual const TString&	getProcesFilePath() const { return mProcessPath; }

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getPluginPath() const { return mPluginPath; }

	protected:

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe check the dependency
		@param [in] depend the dependency to check
		@param [out] needIndex if check failed,this is the check failure index in dependency
		@return check result
		*/
		bool			checkDenpendency(TStringParam& depend,size_t& needIndex);

		static void		initCPUInfo(CPUINFO& outCpuInfo);

	protected:
		typedef StaticList<DynamicLib>			DynLibList;
		typedef TStringPointerMap<IPlugin,StaticTStringPointerPairAllocator >	PluginMap;

		typedef struct SDependency
		{
			RefCount	mSupportCount;
			RefCount	mDependCount;
		}Dependency;

		typedef StaticTStringMap<Dependency>	DependencyMap;
		typedef StaticList<IPlugin*>			PluginList;

		CPUINFO		mCPUInfo;

		/** @brief store loaded dynamic libs in order */
		DynLibList	mDynamicLibs;

		/** @brief list all plugins by order */
		PluginList	mPluginList;

		/** @brief map to store all plugin pointers */
		PluginMap	mInstalledPlugins;

		/** @brief map to store plugin dependency counting */
		DependencyMap	mDenpendencyMap;

		/** @brief plugin path */
		TString		mPluginPath;

		/** @brief current working path */
		TString		mCurrentPath;

		/** @brief current working path */
		TString		mProcessPath;

		/** @brief init flag */
		bool		mInitialized;

		/** @brief  */
		bool		mRaiseException;
	};// class PlatformManager
	
}//namespace Blade


#endif // __Blade_PlatformManager_h__

