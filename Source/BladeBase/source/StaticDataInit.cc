/********************************************************************
	created:	2010/03/23
	filename: 	StaticObjects.cc
	author:		Crazii
	
	purpose:	static object register, to control static object construction/destruction order
*********************************************************************/
#include <BladePCH.h>
#include "interface_imp/pool/FixedSizePageAllocator.h"
#include "interface_imp/pool/FixedSizePool.h"
#include "interface_imp/pool/IncrementalPool.h"
#include "interface_imp/PoolManager.h"
#include "interface_imp/pool/DLMallocPool.h"
#include "interface_imp/InterfaceSingletonImpl.h"
#include "interface_imp/platform/PlatformManager.h"
#include "MemoryDebug.h"
#include <StaticHandle.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable: 4640) //construction of local static object is not thread-safe
#endif

#if BLADE_COMPILER_GNU_EXTENSION
#define HIGH_PRIORITY_INIT __attribute__((init_priority(101)))
#else
#define HIGH_PRIORITY_INIT
#endif

/************************************************************************/
/* static objects                                                                     */
/************************************************************************/
namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	StaticLock Time::msLock;
	StaticLock LocalTime::msLock;
	StaticLock FactoryUtil::msInterfaceSingletonLock;

	namespace Internal
	{
#if BLADE_MEMORY_DEBUG
		bool			lLeakDump = true;
#else
		bool			lLeakDump = false;
#endif

		//common pool
		//this should be the first because it's being destructed last (show information footer)
		HIGH_PRIORITY_INIT CommonPool		lCommonPool;

		//static pool
		HIGH_PRIORITY_INIT StaticPool		lStaticPool;

#if ENABLE_FIXEDSIZE_PAGE_ALLOC
		HIGH_PRIORITY_INIT FixedSizePageAllocator lFixedSizePageAlloc;
#endif

		//the memory pool map should be init first
		HIGH_PRIORITY_INIT SizeMap			SizePoolMap;

#if !SIZE_MAP_ALIGN
		HIGH_PRIORITY_INIT Lock				SizePoolLock;
#endif

		//this should be the last because it's being destructed first (show information header)
		HIGH_PRIORITY_INIT TempPool			lTempPool;
	}//namespace Internal

	 //resource pool
	IPool*	ResourceAllocatable::msPool;
	//static pool
	IPool*	StaticAllocatable::msPool;
	//temporary pool
	IPool*	TempAllocatable::msPool;

	class MemInit
	{
	public:
		MemInit()
		{
			ResourceAllocatable::msPool = Memory::getResourcePool();
			//static pool
			StaticAllocatable::msPool = Memory::getStaticPool();
			//temporary pool
			TempAllocatable::msPool = Memory::getTemporaryPool();
		}
	};

	HIGH_PRIORITY_INIT MemInit minit;

	//size pool list
	IPool*	Allocatable::msPoolList[Allocatable::MAX_POOLCOUNT];
	StaticLock	Allocatable::msLockList[Allocatable::MAX_POOLCOUNT];

#if BLADE_STATIC_PLUGIN
	HIGH_PRIORITY_INIT StaticPluginDB PLUGIN_DATABASE;
#endif


}//namespace Blade


/************************************************************************/
/* export (template) class                                                                     */
/************************************************************************/
#include <interface/Interface.h>
#include <interface/public/IDevice.h>
#include <interface/public/window/IWindowDevice.h>
#include <interface/public/window/IWindowMessagePump.h>
#include <interface/public/window/IWindowEventHandler.h>
#include <interface/public/graphics/IRenderDevice.h>
#include <interface/public/graphics/ITexture.h>
#include <interface/public/graphics/IRenderTarget.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/public/input/IKeyboardDevice.h>
#include <interface/public/input/IMouseDevice.h>
#include <interface/public/input/ITouchDevice.h>
#include <utility/Delegate.h>
#include <utility/DataBinding.h>
#include <utility/DataBindingEx.h>
#include <utility/DataBridge.h>
#include <memory/PoolAllocator.h>
#include "graphics/DefaultGraphicsResourceManager.h"
#include "graphics/TempGraphicsResourceManager.h"
#include "interface_imp/LogManager.h"
#include "interface_imp/Log.h"
#include "interface_imp/MemoryFileDevice.h"
#include "interface_imp/LangTable.h"
#include "interface_imp/platform/FileStream.h"
#include "interface_imp/platform/FileDevice.h"

namespace Blade
{
	template class Factory<IKeyboardDevice>;
	template class Factory<IMouseDevice>;
	template class Factory<ITouchDevice>;
	template class Factory<IWindowDevice>;
	template class Factory<IWindowMessagePump>;
	template class Factory<IDevice>;
	template class Factory<Interface>;
	template class Factory<IGraphicsResourceManager>;
	template class Factory<IImageManager>;
	template class Factory<IRenderDevice>;
	template class Factory<IPlatformManager>;
}

/************************************************************************/
/* static data                                                                     */
/************************************************************************/
#include <math/Half.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	class HalfInitializer : public Half
	{
	private:
		/** @brief  */
		static uint32 halfToFloat(uint16 y)
		{
			uint s = (y >> 15) & 0x00000001u;
			uint e = (y >> 10) & 0x0000001fu;
			uint m = y & 0x000003ffu;

			if (e == 0)
			{
				if (m == 0)
				{
					// Plus or minus zero
					return s << 31;
				}
				else
				{
					// Denormalized number -- renormalize it
					while (!(m & 0x00000400))
					{
						m <<= 1;
						e -= 1;
					}
					e += 1;
					m &= ~0x00000400;
				}
			}
			else if (e == 31)
			{
				if (m == 0)
				{
					// Positive or negative infinity
					return (s << 31) | 0x7f800000;
				}
				else
				{
					// Nan -- preserve sign and significand bits
					return (s << 31) | 0x7f800000 | (m << 13);
				}
			}

			// Normalized number
			e = e + (127 - 15);
			m = m << 13;

			// Assemble s, e and m.
			return (s << 31) | (e << 23) | m;
		}

	public:
		/** @brief  */
		static void initializeTable()
		{
			for (uint32 i = 0; i < 0x100; i++)
			{
				uint32 e = (i & 0x0ff) - (127 - 15);

				if (e <= 0 || e >= 30)
				{
					// Special case
					_eLut[i] = 0;
					_eLut[i | 0x100] = 0;
				}
				else
				{
					// Common case - normalized half, no exponent overflow possible
					_eLut[i] = uint16(e << 10);
					_eLut[i | 0x100] = uint16((e << 10) | 0x8000);
				}
			}

			const int iMax = (1 << 16);
			for (int i = 0; i < iMax; i++)
			{
				_toFloat[i].u = halfToFloat((uint16)i);
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		int tableWhiteSpace[128];
		int tableNameStarts[128];
		int tableNameChars[128];

		void initConstStringTable()
		{
			static const TString whiteSpaces = BTString(" \t\r\n");
			static const TString nameStarts = BTString("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
			static const TString nameChars = BTString("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");

			for (size_t i = 0; i < 128; ++i)
			{
				tableWhiteSpace[i] = whiteSpaces.find((tchar)i) != INVALID_INDEX;
				tableNameStarts[i] = nameStarts.find((tchar)i) != INVALID_INDEX;
				tableNameChars[i] = nameChars.find((tchar)i) != INVALID_INDEX;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const Blade::Delegate Blade::Delegate::EMPTY;
	const Blade::DataBinding Blade::DataBinding::EMPTY;
	const Blade::CollectionDataBinding Blade::CollectionDataBinding::EMPTY;
	const Blade::MapDataBinding Blade::MapDataBinding::EMPTY;

	StaticLock	Log::msSharedLock;
	TextFile	Log::msSharedLog;
	Log			Log::msDebugOutput = Log(ILog::LL_DEBUG);
	Log			Log::msInformation = Log(ILog::LL_DETAIL);
	Log			Log::msWarning = Log(ILog::LL_CAREFUL);
	Log			Log::msError = Log(ILog::LL_CRITICAL);

	ILog&	ILog::DebugOutput = Log::msDebugOutput;
	ILog&	ILog::Information = Log::msInformation;
	ILog&	ILog::Warning = Log::msWarning;
	ILog&	ILog::Error = Log::msError;

	const ILog::Manipulators ILog::endLog = ILog::flushLog;
	const ILog::Manipulators ILog::noPrefix = ILog::skipPrefix;

	FactoryUtil::SingletonList FactoryUtil::msInterfaceSingletonList;

	const TString IFileDevice::DEFAULT_FILE_TYPE = BTString("NativeFileSystem");
	//note: use OGL texture coordinate system
	const IMG_ORIENT IGraphicsResourceManager::DEFAULT_TEXDIR = IMGO_BOTTOM_UP;
	const TString IGraphicsResourceManager::TYPE_SOFT = BTString("Software");
	const TString IGraphicsResourceManager::TYPE_TEMPORARY = BTString("Temp");

	StaticTStringMap<EPixelFormat> NAME_MAP;
	StaticMap<EPixelFormat, TString> FMT_MAP;

	/************************************************************************/
	/* initialization, especially for static library build                                                                     */
	/************************************************************************/

	//the static lib build is not supported yet, but is planed to be.
	//////////////////////////////////////////////////////////////////////////
	bool	initializeFoundation()
	{
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		HalfInitializer::initializeTable();
		Impl::initConstStringTable();

		/************************************************************************/
		/* class registration                                                                     */
		/************************************************************************/
		//this should be after the memory pool init
		FactoryUtil::msInterfaceSingletonList.reserve(FactoryUtil::SINGLETON_INTERFACE_COUNT);

		//init platform data & iconv
		PlatformData::getSingleton().initialize();

		RegisterSingleton(PlatformManager, IPlatformManager);

		//factory's singleton init when those call happen
		NameRegisterFactory(FixedSizePool, IPool, BTString("FixedSize"));	//in-accessible by client
		NameRegisterFactory(IncrementalPool, IPool, BTString("Incremental"));
		NameRegisterFactory(DLMallocPool, IPool, BTString("Resource"));
		NameRegisterFactory(DLMallocPool, IPool, BTString("Misc"));
		NameRegisterFactory(DefaultFileDevice, IFileDevice, IFileDevice::DEFAULT_FILE_TYPE);
		NameRegisterFactory(MemoryFileDevice, IFileDevice, MemoryFileDevice::MEMORY_FILE_TYPE);

		RegisterSingleton(LogManager, ILogManager);
		RegisterSingleton(LangTableManager, ILangTableManager);
		RegisterSingleton(DefaultGraphicsResourceManager, IGraphicsResourceManager);
		NameRegisterSingleton(DefaultGraphicsResourceManager, IGraphicsResourceManager, IGraphicsResourceManager::TYPE_SOFT);
		NameRegisterSingleton(TempGraphicsResourceManager, IGraphicsResourceManager, IGraphicsResourceManager::TYPE_TEMPORARY);
		return true;
	}

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
	void BladeAssert(const char* file, int line, const char* func, const char* msg)
	{
		BLADE_LOG(Error, file << BString("(") << line << BString("): assert failed in function ") << func << BString(", expression: ") << msg);
		::raise(SIGTRAP);
	}
#endif

}//namespace Blade
