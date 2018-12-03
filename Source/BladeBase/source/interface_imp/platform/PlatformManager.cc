/********************************************************************
	created:	2009/05/23
	filename: 	PlatformManager.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	undef TEXT
#	include <Windows.h>
#	include <tchar.h>
#	include <direct.h>
#elif BLADE_IS_UNIX_CLASS_SYSTEM
#	include <time.h>
#	include <unistd.h>
#	if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#		include <android/native_activity.h>
#		include "android/JniHelper.h"
#	endif
#else
#	error not implemented.
#endif

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	include<mmintrin.h>
#endif

#include <BladeBase_blang.h>
#include "PlatformManager.h"

namespace Blade
{
	namespace Impl
	{
		static const TString getPluginFileExt()
		{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
			return BTString("dll");
#elif BLADE_PLATFORM == BLADE_PLATFORM_MAC || BLADE_PLATFORM == BLADE_PLATFORM_IOS
			return BTString("dylib");
#elif BLADE_IS_UNIX_CLASS_SYSTEM
			return BTString("so");
#else
#error not imeplemented.
#endif
		}
	}//namespace Impl

	//////////////////////////////////////////////////////////////////////////
	PlatformManager::PlatformManager()
		:mPluginPath(BTString("."))
		,mInitialized(false)
		,mRaiseException(true)
	{
		//init platform CPU info
		PlatformManager::initCPUInfo(mCPUInfo);

		//get current working path
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		tchar path[MAX_PATH+1];
		::GetCurrentDirectory(MAX_PATH, path);
		mCurrentPath = path;

		::GetModuleFileName(NULL, path, MAX_PATH);
		mProcessPath = path;

#elif BLADE_IS_UNIX_CLASS_SYSTEM
		char path[PATH_MAX+1];
		std::memset(path,0, PATH_MAX);
		mCurrentPath = StringConverter::StringToTString( ::getcwd(path, PATH_MAX) );

		if ( ::readlink("/proc/self/exe", path, PATH_MAX) != -1)
			mProcessPath = StringConverter::StringToTString( ::getcwd(path, PATH_MAX) );
		else
			mProcessPath = mCurrentPath;
#else
#error not implemented.
#endif

		TString dir, file; 
		TStringHelper::getFilePathInfo(mProcessPath, dir, file);
		mProcessPath = dir;
	}

	//////////////////////////////////////////////////////////////////////////
	PlatformManager::~PlatformManager()
	{
		//shutdownAllPlugins();
		mDenpendencyMap.clear();
		mInstalledPlugins.clear();
	}

	/************************************************************************/
	/* IPlatformManager specs interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	static tchar SIMDString[][11] ={TEXT("None"),TEXT("MMX"),TEXT("SSE"),TEXT("SSE2"),TEXT("SSE3"),TEXT("SSE4.1"),TEXT("SSE4.2"), TEXT("NEON/ASIMD") };

	//////////////////////////////////////////////////////////////////////////
	bool	PlatformManager::initialize(const TString& path/* = TString::EMPTY*/)
	{
		if( mInitialized )
			return false;

		bool archiveCreated = false;
		
		IFileDevice* folder = NULL;
		folder = BLADE_FACTORY_CREATE(IFileDevice, IFileDevice::DEFAULT_FILE_TYPE );
		if (folder->load(path.empty() ? BTString(".") : path) )
		{
			archiveCreated = true;
			mPluginPath = folder->getName();
		}

		if( mPluginPath == TString::EMPTY )
			return false;
	
		TString plugintypes = BTString("*.") + Impl::getPluginFileExt();

		TStringParam filelist;
		folder->findFile(filelist,plugintypes,false);

		if( archiveCreated )
			BLADE_DELETE folder;

		ParamList list;
		for (size_t i = 0; i < filelist.size(); ++i)
			list[filelist[i]] = true;

		this->initialize(list);
		mInitialized = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	PlatformManager::initialize(const HSTREAM& configStream)
	{
		if( mInitialized )
		{
			assert(false);
			return false;
		}

		ConfigFile platformConfig;
		if (!platformConfig.open(configStream))
			return false;

		const CFGFILE_SECTION* PluginSection = platformConfig.getSection(BTString("Plugins"));
		if( PluginSection == NULL )
			return false;

		//find the directory
		const TString DIRECTORY_KEY = BTString("PluginDirectory");
		const TString& dirValue = PluginSection->getValue( DIRECTORY_KEY );

		//generate a list and try all plugins in chaos order
		ParamList pluginlist;

		for( size_t i = 0; i < PluginSection->getEntryCount(); ++i )
		{
			const CFGFILE_ENTRY& entry = PluginSection->getEntry(i);
			const TString& key = entry.getName();
			const TString& value = entry.getValue();
			assert( key != TString::EMPTY );
			if( key != DIRECTORY_KEY )
				pluginlist[key] = value.empty() ? true : TStringHelper::toBool(value);
		}

		this->initialize(pluginlist, dirValue);
		mInitialized = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PlatformManager::initialize(ParamList& pluginlist, const TString& path/* = TString::EMPTY*/)
	{
		if( mInitialized )
		{
			assert(false);
			return false;
		}

		BLADE_LOG(Information, TEXT("PlateformManger initialization - Getting System information")
			<< TEXT("\n\t") << mCPUInfo.mCPUString
			<< TEXT("\n\t\t") << TEXT("Number of processors:\t") << (int)mCPUInfo.mProcessorCount
			<< TEXT("\n\t\t") << TEXT("SIMD instruction    :\t") << SIMDString[ mCPUInfo.mSIMD ]);

		mRaiseException = false;

		mPluginPath = path;
		if( mPluginPath.empty() )
			mPluginPath = this->getProcesFilePath();

		TString cwd = this->getCurrentWorkingPath();
		this->setCurrentWorkingPath(mPluginPath);

		index_t loadFailIndex;
		size_t	loadCountPerPass;
		while( pluginlist.size() > 0 )
		{
			loadFailIndex = INVALID_INDEX;
			loadCountPerPass = 0;

			for( size_t i = 0; i < pluginlist.size() ; ++i)
			{
				const NamedVariant item = pluginlist.at(i);
				const TString& realPath = item.getName();
				bool enable = item.getValue();
				if( this->loadPlugin(realPath, enable) )
				{
					pluginlist.erase(i--);
					++loadCountPerPass;
				}
				else
					loadFailIndex = i;
			}

			//there is some plugin that cannot be loaded
			if( loadCountPerPass == 0 )
			{
				assert( loadFailIndex != INVALID_INDEX );
				if( mRaiseException == false ) //raise exception,and do the final pass right with throw
					mRaiseException = true;
				else
					break;
			}
		}

		this->setCurrentWorkingPath(cwd);
		mRaiseException = true;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const CPUINFO& PlatformManager::getCPUInfo() const
	{
		return mCPUInfo;
	}

	//////////////////////////////////////////////////////////////////////////
	void	PlatformManager::sleep(uint32 millisec)
	{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		::Sleep(millisec);
#elif BLADE_IS_UNIX_CLASS_SYSTEM
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = millisec*1000;
		::nanosleep(&ts, NULL);
#else
#	error not implemented.
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	struct SResolution
	{
		size_t width;
		size_t height;
		bool operator<(const SResolution& rhs) const	{return width < rhs.width;}
		bool operator==(const SResolution& rhs) const {return width == rhs.width && height == rhs.height;}
	};
	size_t	PlatformManager::getScreenResolutions(size_t& current, size_t* widthList/* = NULL*/, size_t* heightList/* = NULL*/) const
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		typedef TempVector<SResolution> ResolutionList;

		ResolutionList list;
		::DEVMODE mode;
		::ZeroMemory(&mode, sizeof(mode));
		DWORD i = 0;
		while( ::EnumDisplaySettings(NULL, i++, &mode) )
		{
			if( mode.dmBitsPerPel != 16 && mode.dmBitsPerPel != 32 
				|| mode.dmPelsWidth < 1024 || mode.dmPelsHeight < 768 )
				continue;
			
			SResolution res = {(size_t)mode.dmPelsWidth, (size_t)mode.dmPelsHeight};
			if( std::find(list.begin(), list.end(), res) == list.end() )
				list.push_back(res);
		}

		current = 0;
		if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode))
		{
			for(size_t j = 0; j < list.size(); ++j)
			{
				if(list[j].width == mode.dmPelsWidth && list[j].height == mode.dmPelsHeight )
				{
					current = j;
					break;
				}
			}
		}

		if( widthList != NULL && heightList != NULL )
		{
			std::sort(list.begin(), list.end());
			for(size_t j = 0; j < list.size(); ++j)
			{
				widthList[j] = list[j].width;
				heightList [j] = list[j].height;
			}
		}
		else 
			assert(widthList == NULL && heightList == NULL);
		
		return list.size();
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#	if 0
		//Android doesn't support multiple resolutions, and this is not used.
		::ANativeWindow* window = PlatformData::getSingleton().getAndroidApp()->window;
		if( window != NULL )
		{
			if(widthList != NULL)
				widthList[0] = ::ANativeWindow_getWidth(window);
			if(heightList != NULL)
				heightList[0] = ::ANativeWindow_getHeight(window);
			return 1;
		}
#	endif
		return 0;
#else
#	error not implemented.
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PlatformManager::loadPlugin(const TString& pathname, bool enable/* = true*/)
	{
#if BLADE_STATIC_PLUGIN
		bool result = false;
		for (StaticPluginDB::iterator i = PLUGIN_DATABASE.begin(); i != PLUGIN_DATABASE.end(); ++i)
		{
			IPlugin* plugin = *i;
			//note: plugin name now must match .dll/.so file name
			//if multiple plugin exist, plugin names should start with the file name
			TString pluginName = plugin->getName();
#if BLADE_IS_UNIX_CLASS_SYSTEM
			pluginName = BTString("lib")+pluginName;
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
			pluginName += BTString("_Android");
#endif
#endif
			if ( TStringHelper::isStartWith(pluginName, pathname) )
			{
				//plugin->install();
				this->installPlugin(plugin);
				plugin->setEnable(enable);
				result = true;
			}
		}
		return result;
#else
		DynamicLib lib(pathname);

		DynLibList::const_iterator it = std::find(mDynamicLibs.begin(), mDynamicLibs.end(), lib);
		if(  it == mDynamicLibs.end() )
		{
			TString pathExt = pathname;
			TStringHelper::appendFileExtension(pathExt, Impl::getPluginFileExt());
			/* @note: 
			in debug mode,the memory allocation routine need extra information like source file name,(compiler built in __FILE__ macro)
			but those strings are generated in separated library module,when libs are unloaded,the string buffer no longer valid.
			thus we should load the library and never unload it until process terminated.
			*/
			bool bNoUnload = Memory::isLeakDumpEnabled();
			lib.load(pathExt, false, bNoUnload);
		}
		else
			//same lib already loaded
			//but maybe just the file name is the same,or last time failed,do it again
			lib = *it;

		BladePluginCountFn countFn = (BladePluginCountFn)lib.getExport("BladePluginCount");
		BladePluginEntryFn entryFn = (BladePluginEntryFn)lib.getExport("BladeGetPlugin");

		if (countFn == NULL || entryFn == NULL)
		{
#if !BLADE_DEBUG
			lib.unload();
#endif
			return false;
		}

		bool result = true;
		size_t pluginCount = countFn();

		for (size_t i = 0; i < pluginCount; ++i)
		{
			IPlugin* plugin = entryFn(i);
			if (plugin != NULL)
			{
				if (!this->installPlugin(plugin))
				{
					result = false;
					break;
				}
				plugin->setEnable(enable);
			}
		}
						
		if( result )
			mDynamicLibs.push_back( lib );

		return result;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void		PlatformManager::unloadPlugin(const TString& pathname)
	{
#if !BLADE_STATIC_PLUGIN
		DynamicLib lib(pathname);
		DynLibList::iterator it = std::find(mDynamicLibs.begin(), mDynamicLibs.end(), lib);
		if(  it != mDynamicLibs.end() )
		{
			BladePluginCountFn countFn = (BladePluginCountFn)it->getExport("BladePluginCount");
			BladePluginEntryFn entryFn = (BladePluginEntryFn)it->getExport("BladeGetPlugin");

			if (countFn != NULL && entryFn != NULL)
			{
				size_t pluginCount = countFn();
				for (size_t i = 0; i < pluginCount; ++i)
				{
					IPlugin* plugin = entryFn(i);
					this->uninstallPlugin(plugin);
				}
			}

			it->unload();
			mDynamicLibs.erase(it);
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	IPlugin*	PlatformManager::getPlugin(const TString& name) const
	{
		PluginMap::const_iterator it = mInstalledPlugins.find( name );
		if( it != mInstalledPlugins.end() )
			return it->second;
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PlatformManager::installPlugin(IPlugin* plugin)
	{
		if (plugin == NULL)
			return false;

		std::pair<PluginMap::iterator,bool> insert_result = mInstalledPlugins.insert( PluginMap::value_type(plugin->getName(), (IPlugin*)NULL) );
		//already exist the same name
		//name conflict or same plugin
		if( !insert_result.second )
		{
			if( mRaiseException )
				BLADE_EXCEPT(EXC_REDUPLICATE,TEXT("Plugin not loaded, a plugin named \"") + plugin->getName() +  TEXT("\" already exist.") );
			else
				//log
				BLADE_LOG(Warning, TEXT("Plugin not loaded, a plugin named \"") << plugin->getName() << TEXT("\" already exist."));
			return false;
		}

		//log
		//BLADE_LOG(Information, TEXT("Plugin's Unique name is \"") << plugin->getName() <<TEXT("\""));

		IPlugin*& empty = insert_result.first->second;

		TStringParam dependency;
		plugin->getDependency(dependency);
		size_t stopIndex = 0;
		if( this->checkDenpendency(dependency, stopIndex) )
			//succeed
		{
			plugin->install();
			empty = plugin;
			TStringParam supports;
			plugin->getSupportList(supports);

			//add plugin support
			for( size_t i = 0; i < supports.size(); ++i )
				mDenpendencyMap[ supports[i] ].mSupportCount.increment();

			//store plugins in install order so that they could be un-installed reversely
			mPluginList.push_back(plugin);

			//add dependency count
			for( size_t i = 0; i < dependency.size(); ++i )
			{
				const TString& dependName = dependency.at(i);
				mDenpendencyMap[ dependName ].mDependCount.increment();
			}
			BLADE_LOG(Information, TEXT("Plugin loaded - ") << plugin->getName() );
			return true;
		}
		else
		{
			mInstalledPlugins.erase( insert_result.first );

			if( mRaiseException )
			{
				BLADE_EXCEPT(EXC_INTERNAL_ERROR,
					BTString("Plugin '") + plugin->getName() + BTString("' cannot be installed. It depend on one plugin that support \"") 
					+ dependency[stopIndex]
					+  TEXT("\", get a plugin supportting \"")+ dependency[stopIndex] + TEXT("\" installed first.")
					);
			}
			else if( false )
			{
				//log
				BLADE_LOG(Warning,
					BTString("Plugin '") + plugin->getName() + BTString("' cannot be installed. It depend on one plugin that support \"")
					+ dependency[stopIndex] << TEXT("\""));
			}

			return false;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	bool		PlatformManager::uninstallPlugin(IPlugin* plugin)
	{
		if( plugin == NULL )
			return false;

		PluginMap::iterator it = mInstalledPlugins.find( plugin->getName() );
		if( it == mInstalledPlugins.end() )
			return true;

		else if( it->second != plugin )
		{
			mInstalledPlugins.erase(it);
			return true;
		}

		//check if this plugin depended on by others
		TStringParam supports;
		plugin->getSupportList(supports);

		// note: there is one situation that two plugin have the same support,and another plugin depend on it,
		// because support/dependency are just conceptional,so we cannot decide it actually depend on which one,so both cannot be uninstalled

		//				plugin[C] 
		//					|
		//					|depend on
		//				   \|/
		//				support[S]
		//				/		|
		//				/		|
		//      plugin[A]   plugin[B]
		//
		//both A and B cannot be un installed, if C depend on support S

		for( size_t i = 0; i < supports.size(); ++i )
		{
			DependencyMap::iterator iter = mDenpendencyMap.find( supports[i] );
			if( iter != mDenpendencyMap.end() )
			{
				//some other installed plugins still depend on it , cannot uninstall
				if( iter->second.mDependCount != 0 )
				{
					if( mRaiseException )
					{
						BLADE_EXCEPT(EXC_INTERNAL_ERROR,
							BTString("Plugin \"")+plugin->getName()+TEXT("\" cannot be uninstalled. Other plugins depend on it,uninstall other plugins first"));
					}
					else
						BLADE_LOG(Warning, BTString("Plugin \"")+plugin->getName()+TEXT("\" cannot be uninstalled.Other plugins depend on it,uninstall other plugins first"));
					return false;
				}
				else
					mDenpendencyMap.erase(iter);
			}
		}

		//remove this one's dependency
		TStringParam dependency;
		plugin->getDependency(dependency);
		for( size_t  i = 0; i < dependency.size(); ++i)
		{
			const TString& dpendName = dependency[i];
			DependencyMap::iterator iter = mDenpendencyMap.find( dpendName );

			if( iter != mDenpendencyMap.end() )
				iter->second.mDependCount.decrement();
		}
		plugin->uninstall();
		mInstalledPlugins.erase( it );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void	PlatformManager::initAllPlugins()
	{
		//init order now connected with dependency
		for( PluginList::const_iterator i = mPluginList.begin(); i != mPluginList.end(); ++i )
		{
			IPlugin* plugin = *i;
			bool enabled = plugin->isEnabled();
			if (enabled)
			{
				//check if all dependencies are enabled
				TStringParam dependency;
				plugin->getDependency(dependency);
				for (size_t j = 0; j < dependency.size(); ++j)
				{
					bool dependEnabled = false;
					const TString& dpendName = dependency[j];
					//find at least one enabled plugin for this dependency
					for (PluginList::const_iterator k = mPluginList.begin(); k != mPluginList.end(); ++k)
					{
						TStringParam supports;
						(*k)->getSupportList(supports);
						if (supports.find(dpendName) != INVALID_INDEX)
						{
							dependEnabled = (*k)->isEnabled();
							if (dependEnabled)
								break;
						}
					}

					if(!dependEnabled)
						enabled = dependEnabled;
				}
				
				if (enabled)
					plugin->initialize();
				else
					BLADE_LOG(Warning, BTString("plugin '") << plugin->getName() << BTString("' not initialized."));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	PlatformManager::shutdownAllPlugins()
	{
		//shutdown order connected with dependency
		//shutdown plugins in reverse order.
		for( PluginList::reverse_iterator ri = mPluginList.rbegin(); ri != mPluginList.rend(); ++ri )
		{
			(*ri)->shutdown();
		}
		//mPluginList.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void	PlatformManager::unloadAllPlugins()
	{
#if BLADE_STATIC_PLUGIN
		for (PluginList::reverse_iterator ri = mPluginList.rbegin(); ri != mPluginList.rend(); ++ri)
		{
			(*ri)->uninstall();
			mInstalledPlugins.erase((*ri)->getName());
		}
#else
		//bug fix 09.17.2011: unload lib from the end (from on top of the dependency list)
		while( !mDynamicLibs.empty() )
		{
			DynLibList::reverse_iterator i = mDynamicLibs.rbegin();
			DynamicLib& lib = *i;
			this->unloadPlugin( lib.getName() );
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	PlatformManager::MBRESULT	PlatformManager::showMessageBox(const wchar* content, const wchar* caption/* = NULL*/, MBTYPE type/* = MBT_OK*/)
	{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		UINT bmtype;
		if( type == MBT_YESNO )
			bmtype = MB_YESNO;
		if( type == MBT_YESNOCANCEL )
			bmtype = MB_YESNOCANCEL;
		else
			bmtype = MB_OK;

		int ret = ::MessageBoxW(::GetActiveWindow(), content, caption, bmtype);
					
		if( ret == IDCANCEL )
			return MBR_CANCEL;
		else if( ret == IDYES)
			return MBR_YES;
		else if( ret == IDNO )
			return MBR_NO;
		else
			return MBR_OK;

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		const TString& strOk = BTLang(BLANG_OK);
		const TString& strCancel = BTLang(BLANG_CANCEL);
		const TString& strYes = BTLang(BLANG_YES);
		const TString& strNo = BTLang(BLANG_YES);

		JniHelper::JavaU16String u16content, u16caption, u16Cancel, u16Yes, u16No;
		JniHelper::convertString(content, u16content);
		JniHelper::convertString(caption, u16caption);
		JniHelper::convertString(strNo.c_str(), u16No);
		JniHelper::convertString(strCancel.c_str(), u16Cancel);
		if(type == MBT_OK )
			JniHelper::convertString(strOk.c_str(), u16Yes);
		else
			JniHelper::convertString(strYes.c_str(), u16Yes);

		return JniHelper::showSystemDialog(u16caption, u16content, u16Yes, u16No, u16Cancel, type);
#else
#error not implemented.
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	PlatformManager::MBRESULT	PlatformManager::showMessageBox(const char* content,const char* caption/* = NULL*/,MBTYPE type/* = MBT_OK*/)
	{
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		UINT bmtype;
		if( type == MBT_YESNO )
			bmtype = MB_YESNO;
		if( type == MBT_YESNOCANCEL )
			bmtype = MB_YESNOCANCEL;
		else
			bmtype = MB_OK;

		int ret = ::MessageBoxA(NULL,content,caption,bmtype);

		if( ret == IDCANCEL )
			return MBR_CANCEL;
		else if( ret == IDYES)
			return MBR_YES;
		else if( ret == IDNO )
			return MBR_NO;
		else
			return MBR_OK;
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		const TString& strOk = BTLang(BLANG_OK);
		const TString& strCancel = BTLang(BLANG_CANCEL);
		const TString& strYes = BTLang(BLANG_YES);
		const TString& strNo = BTLang(BLANG_NO);

		JniHelper::JavaU16String u16content, u16caption, u16Cancel, u16Yes, u16No;
		JniHelper::convertString(content, u16content);
		JniHelper::convertString(caption, u16caption);
		JniHelper::convertString(strNo.c_str(), u16No);
		JniHelper::convertString(strCancel.c_str(), u16Cancel);
		if(type == MBT_OK )
			JniHelper::convertString(strOk.c_str(), u16Yes);
		else
			JniHelper::convertString(strYes.c_str(), u16Yes);

		return JniHelper::showSystemDialog(u16caption, u16content, u16Yes, u16No, u16Cancel, type);
#else
#error not implemented.
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	PlatformManager::MBRESULT	PlatformManager::showMessageBox(MBTYPE type, const char* caption, const char* fmt, ...)
	{
		StringStream ss;
		va_list va;
		va_start(va, fmt);
		ss.vformat(fmt, va);
		return this->showMessageBox(ss.getContent().c_str(), caption, type);
	}

	//////////////////////////////////////////////////////////////////////////
	PlatformManager::MBRESULT	PlatformManager::showMessageBox(MBTYPE type, const wchar* caption, const wchar* fmt, ...)
	{
		WStringStream ss;
		va_list va;
		va_start(va, fmt);
		ss.vformat(fmt, va);
		return this->showMessageBox(ss.getContent().c_str(), caption, type);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	PlatformManager::getCurrentWorkingPath() const
	{
		return mCurrentPath;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			PlatformManager::setCurrentWorkingPath(const TString& path)
	{
		bool ret = true;
		if( mCurrentPath != path )
		{
			ret = false;
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
			ret = (::_tchdir( path.c_str() ) == 0);
#elif BLADE_IS_UNIX_CLASS_SYSTEM
			TempBuffer buffer;
			const char* utf8Path = StringConverter::TStringToUTF8String(buffer, path);
			ret = (::chdir(utf8Path) == 0);
#else
#error not implemented.
#endif
			if(ret)
				mCurrentPath = path;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		PlatformManager::checkDenpendency(TStringParam& depend, size_t& needIndex)
	{
		for( size_t i = 0; i < depend.size(); ++i)
		{
			DependencyMap::iterator di = mDenpendencyMap.find( depend[i] );
			if( di == mDenpendencyMap.end() || di->second.mSupportCount == 0 )
			{
				needIndex = i;
				return false;
			}
		}

		return true;
	}

}//namespace Blade