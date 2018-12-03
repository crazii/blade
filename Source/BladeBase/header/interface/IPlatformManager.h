/********************************************************************
	created:	2009/03/29
	filename: 	IPlatformManager.h
	author:		Crazii
	
	purpose:	interface for platform manager
*********************************************************************/
#ifndef __Balde_IPlatformManager_h__
#define __Balde_IPlatformManager_h__
#include <interface/InterfaceSingleton.h>
#include <utility/Variant.h>
#include <interface/public/IPlugin.h>
#include <interface/public/file/IStream.h>
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#include <xmmintrin.h>
#endif

namespace Blade
{
	/************************************************************************/
	/* platform CPU info                                                                     */
	/************************************************************************/
	typedef struct SCPUInfo
	{
		typedef enum ESIMDInstructionSet
		{
			SIMDS_NONE	= 0,

			SIMDS_MMX	= 1,
			SIMDS_SSE,
			SIMDS_SSE2,
			SIMDS_SSE3,

			//SSE 4.1
			SIMDS_SSE41,
			SIMDS_SSE42,

			//ARM SIMD
			SIMDS_NEON,
		}SIMDIS;

		TString	mCPUString;
		size_t	mProcessorCount;
		size_t	mCacheLineSize;
		SIMDIS	mSIMD;
		bool	mHardwareFP;
	}CPUINFO;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	enum EPrefetechMode
	{
		PM_READ = 0,
		PM_WRITE = 1,
	};
	class IPlatformManager : public InterfaceSingleton<IPlatformManager>
	{
	public:	
		virtual ~IPlatformManager()		{};

		typedef enum EMessageBoxResult
		{
			MBR_OK		= 0x01,
			MBR_YES		= 0x02,
			MBR_NO		= 0x04,
			MBR_CANCEL	= 0x08
		}MBRESULT;

		typedef enum EMessageBoxType
		{
			MBT_OK			= 0x01,
			MBT_YESNO		= 0x06,
			MBT_YESNOCANCEL	= 0x0e,
		}MBTYPE;
		
		/**
		@describe load all plugins in the plugin directory
		@param path the folder in which plugin files be placed,\
		if EMPTY, the folder is guessed as current dir on local disk\
		this parameter is meant to load plugins in an package
		@return result
		*/
		virtual bool	initialize(const TString& path = TString::EMPTY) = 0;

		/**
		@describe 
		@param configStream the stream of the config file\
		this method is meant to load config files either in package or disk
		change notice 4/23/2013: remove internal dependency:
		use config file to de-couple from "resource manager" for HSTREAM creation
		@return 
		*/
		virtual bool	initialize(const HSTREAM& configStream) = 0;

		/**
		@describe 
		@param [in] pluginlist: string to bool pair list to indicates plugins to load and whether enable it immediately
		@param [in] path: if it is empty, use process image path (getProcesFilePath()).
		@return 
		*/
		virtual bool	initialize(ParamList& pluginlist, const TString& path = TString::EMPTY) = 0;

		//////////////////////////////////////////////////////////////////////////
		//utility functions that do not need initialize
		//////////////////////////////////////////////////////////////////////////
		/**
		@describe 
		@param 
		@return 
		*/
		virtual const CPUINFO& getCPUInfo() const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void	sleep(uint32 millisec) = 0;

		/**
		@describe get supported screen resolution list
		@param widthList & heightList must both be NULL or both not NULL
		@param current current screen resolution index in list
		@return count of resolutions, if widthList & heightList is NULL, return count only. 
		@remarks returned list are sorted in ascending order of width
		*/
		virtual size_t	getScreenResolutions(size_t& current, size_t* widthList = NULL, size_t* heightList = NULL) const = 0;

		/**
		@describe load a plugin
		@param 
		@return 
		*/
		virtual bool	loadPlugin(const TString& pathname, bool enable = true) = 0;

		/**
		@describe unload a plugin
		@param 
		@return 
		*/
		virtual void	unloadPlugin(const TString& pathname) = 0;
		
		/**
		@describe get a plugin by name
		@param name the name of the plugin that is being retrieved
		@return pointer to the plugin
		@remark if name not found, return NULL
		*/
		virtual IPlugin*getPlugin(const TString& name) const = 0;

		/**
		@describe install a plugin
		@param plugin to install
		@return install status result
		*/
		virtual bool	installPlugin(IPlugin* plugin) = 0;

		/**
		@describe uninstall a plugin
		@param plugin to uninstall
		@return 
		*/
		virtual bool	uninstallPlugin(IPlugin* plugin) = 0;


		/**
		@describe initialize all plugins
		@param none
		@return none
		*/
		virtual void	initAllPlugins() = 0;

		/**
		@describe shutdown all plugin
		@param none
		@return none
		*/
		virtual void	shutdownAllPlugins() = 0;

		/**
		@describe unload all plugins
		@param 
		@return 
		*/
		virtual void	unloadAllPlugins() = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual MBRESULT	showMessageBox(const wchar* content,const wchar* caption = NULL, MBTYPE type = MBT_OK) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual MBRESULT	showMessageBox(const char* content,const char* caption = NULL, MBTYPE type = MBT_OK) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual MBRESULT	showMessageBox(MBTYPE type, const char* caption, const char* fmt, ...) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual MBRESULT	showMessageBox(MBTYPE type, const wchar* caption, const wchar* fmt, ...) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual const TString&	getCurrentWorkingPath() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool		setCurrentWorkingPath(const TString& path) = 0;

		/**
		@describe executable directory of current process
		@param
		@return
		*/
		virtual const TString&	getProcesFilePath() const = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getPluginPath() const = 0;

		///note: prefetch may not have improvement on some situations( may have slightly unnoticeable drawbacks, only noticeable on profiling)
		///even if the code is same, prefetch will have different result on different environments (threading, memory usage, other processes' CPU usage)
		///so on average, we better use prefetch to prevent cache miss
		template<EPrefetechMode pm>
		static BLADE_ALWAYS_INLINE void prefetch(const void* data, size_t bytes = 1)
		{
			//note: use constance if possible, for compiler's optimization
			//note: armv7 has 64byte cache line.
#if BLADE_PROCESSOR == BLADE_PROCESSOR_X86 || BLADE_PROCESSOR == BLADE_PROCESSOR_ARM
			//compiling time constant optimization
			//static const size_t cacheLine = 64;	//disable inline will have linking error of cacheLine
			#define cacheLine (64)
#else
			static const size_t cacheLine = IPlatformManager::getSingleton().getCPUInfo().mCacheLineSize;
#endif
			const size_t count = (bytes + cacheLine - 1) / cacheLine;
			const char* addr = (const char*)data;
			addr = (const char*)( uintptr_t(addr)/cacheLine*cacheLine);
#if BLADE_COMPILER_GNU_EXTENSION
			for(size_t i = 0; i < count; ++i)
			{
				__builtin_prefetch(addr, pm, 0);
				addr += cacheLine;
			}
#elif BLADE_COMPILER == BLADE_COMPILER_MSVC
			BLADE_UNREFERENCED(pm);
			//prefetch data to L3 cache: for x86 systems, L3 cache is large(in Megabytes)
			//prefetch data to L0 (_MM_HINT_T0) is not a good option, it may cause cache pollution, and prefetch to it is mostly like load/access data directly so it is slower.
			for(size_t i = 0; i < count; ++i)
			{
				_mm_prefetch(addr, _MM_HINT_T2);
				addr += cacheLine;
			}
#else
#error not implemented.
#endif
		}

	};//class IPlatformManager

	extern template class BLADE_BASE_API Factory<IPlatformManager>;
	
}//namespace Blade


#endif // __Balde_IPlatformManager_h__