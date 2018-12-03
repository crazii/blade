/********************************************************************
	created:	2010/06/18
	filename: 	BladeMemory.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "../BladeWin32API.h"
#include "MemoryDebug.h"
#include "interface_imp/pool/FixedSizePool.h"
#include "interface_imp/pool/IncrementalPool.h"
#include "interface_imp/pool/DLMallocPool.h"

#if BLADE_DEBUG
#include <fstream>
#include <iomanip>
#endif

#if BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wattributes" //always_inline function might not be inlinable 
#endif

#define BLADE_ALLOC_THREAD_COUNT (2)

namespace Blade
{

	/**
	@describe
	@param
	@return
	*/
	static BLADE_ALWAYS_INLINE void* GlobalNewImpl(size_t size, const char* sourcefile, int lineNO, Blade::IPool::ALLOC_TYPE type)
	{
		//note: the allocation impl must match default delete, so it should call default new
		//on Windows, new is implemented by malloc, or _malloc_dbg if _DEBUG defined
		//this relies on impl of msvc runtime, but it is safe for now.
#	if BLADE_IS_WINDOWS_CLASS_SYSTEM && BLADE_COMPILER == BLADE_COMPILER_MSVC && defined _DEBUG && BLADE_MEMORY_DEBUG
		BLADE_UNREFERENCED(type);
		return ::_malloc_dbg(size, _NORMAL_BLOCK, sourcefile, lineNO);
#	else
		BLADE_UNREFERENCED(sourcefile); BLADE_UNREFERENCED(lineNO);
		return type == IPool::PAT_NEW_ARRAY ? ::operator new[](size) : ::operator new(size);
#	endif
	}

	/**
	@describe
	@param0
	@return
	*/
	static BLADE_ALWAYS_INLINE void GlobalDeleteImpl(void* ptr, Blade::IPool::ALLOC_TYPE type)
	{
#	if BLADE_IS_WINDOWS_CLASS_SYSTEM && BLADE_COMPILER == BLADE_COMPILER_MSVC && defined _DEBUG && BLADE_MEMORY_DEBUG
		BLADE_UNREFERENCED(type);
		::_free_dbg(ptr, _NORMAL_BLOCK);
#	else
		return type == IPool::PAT_NEW_ARRAY ? ::operator delete[](ptr) : ::operator delete(ptr);
#	endif
	}

	//////////////////////////////////////////////////////////////////////////
#if BLADE_DEBUG

#	if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
	static size_t allocTotal = 0;
	static int BladeAllocHook( int allocType, void *userData, size_t size, int blockType, long requestNumber, const unsigned char *filename, int lineNumber)
	{
		BLADE_UNREFERENCED(allocType);
		BLADE_UNREFERENCED(userData);
		BLADE_UNREFERENCED(blockType);
		BLADE_UNREFERENCED(requestNumber);
		BLADE_UNREFERENCED(filename);
		BLADE_UNREFERENCED(lineNumber);
		size_t n = size;
		if( allocType == _HOOK_ALLOC )
			allocTotal += n;
		else if(allocType == _HOOK_FREE )
			allocTotal -= n;
		//realloc ignored
		return TRUE;
	}

#	elif BLADE_PLATFORM == BLADE_PLATFORM_LINUX

#	endif

#endif//BLADE_DEBUG

#if SIZE_MAP_ALIGN
	SizeMap::SizeMap()
	{
		for(int i = 0; i < MULTIPLE_COUNT; ++i)
		{
			mAligns[i] = (i+1)*SMALL_ALIGN;
		}
	}
#endif


	//////////////////////////////////////////////////////////////////////////
	SizeMap::~SizeMap()
	{
#if BLADE_MEMORY_DEBUG
		std::ofstream ofs;
		ofs.open("memory_statistics.log");
		ofs << std::setw(16) << "Size" << std::setw(32) << "Allocation Count" << std::endl;

#	if SIZE_MAP_ALIGN
		for(int i = 0; i < SizeMap::MULTIPLE_COUNT; ++i)
		{
			for(int j = 0; j < SizeMap::SMALL_COUNT; ++j)
			{
				FixedSizePool* pPool = (FixedSizePool*)(IPool*)mFixedSizePool[i][j];
				ofs << std::setw(16) << pPool->getUnitSize() << std::setw(32) << pPool->getMaxAllocatedCount() << std::endl;
			}
		}
		ofs.flush();
#	else //SIZE_MAP_ALIGN
		for( iterator i = begin(); i != end(); ++i)
		{
			FixedSizePool* pPool = (FixedSizePool*)(IPool*)i->second;
			ofs << std::setw(16) << pPool->getUnitSize() << std::setw(32) << pPool->getMaxAllocatedCount() << std::endl;
		}
		ofs.flush();

#		if BLADE_IS_WINDOWS_CLASS_SYSTEM
		if(Memory::isLeakDumpEnabled())
			::OutputDebugString(TEXT("----- fixed size memory leaks -----\n") );
		// fixed size pools will be destructed, in clear()
		//let the fixed size pool to output leak report
#		elif BLADE_PLATFORM == BLADE_PLATFORM_LINUX
#		endif

#	endif//SIZE_MAP_ALIGN

#endif//BLADE_DEBUG
	}

	//////////////////////////////////////////////////////////////////////////
	StaticPool::StaticPool()
		:SPOOL(SPOOL::Static())
	{
	}

	//////////////////////////////////////////////////////////////////////////
	StaticPool::~StaticPool()
	{
#if BLADE_DEBUG
#	if BLADE_IS_WINDOWS_CLASS_SYSTEM
		if(Memory::isLeakDumpEnabled())
			::OutputDebugString(TEXT("----- static     memory leaks -----\n") );

		// static pools will be destructed, in clear()
		//let the static pool to output leak report
		destruct();

#	elif BLADE_PLATFORM == BLADE_PLATFORM_LINUX
		destruct();
#	endif
#else	//BLADE_DEBUG
		destruct();
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void StaticPool::construct()
	{
		if( mPtr == NULL )
			mPtr = new(buffer) IncrementalPool();
	}

	//////////////////////////////////////////////////////////////////////////
	void StaticPool::destruct()
	{
		if( mPtr != NULL )
		{
			((IncrementalPool*)mPtr)->~IncrementalPool();
			mPtr = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	TempPool::TempPool()
		:SPOOL(SPOOL::Static())
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TempPool::~TempPool()
	{
#if BLADE_DEBUG

#	if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		//restore memory report to default OutputDebugString
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		if(Memory::isLeakDumpEnabled())
		{
			::OutputDebugString(TEXT("********** Begin Blade memory leaks dump **********\n") );
			::OutputDebugString(TEXT("----- temporary  memory leaks -----\n") );
		}

#	elif BLADE_PLATFORM == BLADE_PLATFORM_LINUX

#	endif
		// temp pools will be destructed,in clear()
		//let the temp pool to output leak report
		destruct();

#else //BLADE_DEBUG
		destruct();
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void TempPool::construct()
	{
		if( mPtr == NULL )
			mPtr = new(buffer) IncrementalPool();
	}

	//////////////////////////////////////////////////////////////////////////
	void TempPool::destruct()
	{
		if( mPtr != NULL )
		{
			((IncrementalPool*)mPtr)->~IncrementalPool();
			mPtr = NULL;
		}
	}

	CommonPool::CommonPool()
		:SPOOL(SPOOL::Static())
	{
#if BLADE_DEBUG
#	if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS && BLADE_COMPILER == BLADE_COMPILER_MSVC
		//because MFC leaks dump is not accurate. 
		//(it dumps earlier than blade memory system, thus some memories are not freed yet.)
		//we need to disable the MFC debug dump

		//set other output info to stderr,(such as MFC report)
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_INVALID_HFILE);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_INVALID_HFILE);
#		if BLADE_MEMORY_DEBUG
		_CrtSetAllocHook(BladeAllocHook);
#		endif
#	endif
#else //BLADE_DEBUG

#endif
	}

	CommonPool::~CommonPool()
	{
#if BLADE_MEMORY_DEBUG

#	if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		if(Memory::isLeakDumpEnabled())
			::OutputDebugString(TEXT("----- common     memory leaks -----\n") );

		destruct();

		if(Memory::isLeakDumpEnabled())
			::OutputDebugString(TEXT("----- default    memory leaks -----\n") );

#		if BLADE_COMPILER == BLADE_COMPILER_MSVC
		if(Memory::isLeakDumpEnabled())
			_CrtDumpMemoryLeaks();
#		endif

		if(Memory::isLeakDumpEnabled())
			::OutputDebugString(TEXT("********** End   Blade memory leaks dump **********\n") );

#		if (BLADE_COMPILER == BLADE_COMPILER_MSVC) && BLADE_DEBUG
		_CrtSetAllocHook(NULL);
#		endif

#	elif BLADE_PLATFORM == BLADE_PLATFORM_LINUX
		destruct();
#	endif
#else
		// temp pools will be destructed,in clear()
		//let the temp pool to output leak report
		destruct();
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void CommonPool::construct()
	{
		if( mPtr == NULL )
			mPtr = new(buffer) DLMallocPool();
	}

	//////////////////////////////////////////////////////////////////////////
	void CommonPool::destruct()
	{
		if( mPtr != NULL )
		{
			((DLMallocPool*)mPtr)->~DLMallocPool();
			mPtr = NULL;
		}
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace Internal
	{
		extern bool			lLeakDump;

		extern CommonPool	lCommonPool;

		//static pool
		extern StaticPool	lStaticPool;

		//the memory pool map should be init first
		extern SizeMap		SizePoolMap;

#if !SIZE_MAP_ALIGN
		extern Lock			SizePoolLock;
#endif

		extern TempPool		lTempPool;
	}
	using namespace Internal;

	/************************************************************************/
	/* MemoryUtil                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	IPool* Memory::requestPoolWithSize(size_t allocSize)
	{
#if SIZE_MAP_ALIGN
		if( allocSize >= SizeMap::MAX_ALLOC )
			return Memory::getResourcePool();

		size_t log = Math::GetHigherLog2(allocSize/SizeMap::SMALL_ALLOC);
		assert(log < SizeMap::MULTIPLE_COUNT);

		const size_t align = SizePoolMap.mAligns[ log ];
		const size_t alignIndex = (allocSize+align-1)/align;
		//size_t alignedSize = alignIndex*align;

		SPOOL& spool = SizePoolMap.mFixedSizePool[log][alignIndex];

		//double check lock
		if( spool == NULL )
		{
			spool.lock();
			if( spool == NULL )
			{
				IPool* newpool = BLADE_NEW FixedSizePool();
				newpool->initialize(BLADE_ALLOC_THREAD_COUNT, align );
				spool = newpool;
			}
			spool.unlock();
		}
		return spool;
#else
		size_t key = (allocSize + ALIGNMENT - 1)/ALIGNMENT;

		static const size_t KEY_MAP_COUNT = 32;

		if( key == 0 ) //allocSize = 0
			key = 1; //compatible to C++ standard, allocate a (minimal) block
		else if( key <= KEY_MAP_COUNT )
		{
			/*
			key align	mapped
			9	72		10
			10	80		
			11	88		12
			12	96
			13	104		16
			14	112
			16	128
			17	136		18
			18	144
			19	152		22
			20	160
			21	168
			22	176
			23	184		26
			24	192
			25	200
			26	208
			27 	216		32
			28	224
			29	232
			30	240
			31	248		
			32	256
			*/
			static size_t keyMap[KEY_MAP_COUNT] =
			{
				1,	2,	4,	4,	6,	6,	8,	8,
				10,	10,	12,	12,	16,	16,	16,	16,
				18,	18,	22,	22,	22,	22,	26,	26,
				26,	26,	32,	32,	32,	32,	32,	32,
			};
			key = keyMap[key - 1];
		}

		SizePoolLock.lock();
		SPOOL& spool = SizePoolMap[key];
		SizePoolLock.unlock();

		//double check lock
		if( spool == NULL )
		{
			spool.lock();
			if( spool == NULL )
			{
				IPool* newpool = BLADE_NEW FixedSizePool();
				newpool->initialize(BLADE_ALLOC_THREAD_COUNT, key*ALIGNMENT);
				Lock::memoryBarrier();
				spool = newpool;
			}
			spool.unlock();
		}
		return spool;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	IPool*	Memory::getTemporaryPool()
	{
		if( lTempPool == NULL )
		{
			lTempPool.lock();
			if( lTempPool == NULL )
			{
				lTempPool.construct();
				lTempPool->initialize(1/*BLADE_ALLOC_THREAD_COUNT*/,4,16*1024);
			}
			lTempPool.unlock();
		}
		return lTempPool;
	}


	//////////////////////////////////////////////////////////////////////////
	IPool*	Memory::getStaticPool()
	{
		if( lStaticPool == NULL )
		{
			lStaticPool.lock();
			if( lStaticPool == NULL )
			{
				lStaticPool.construct();
				lStaticPool->initialize(1, 4,4*1024);
			}
			lStaticPool.unlock();
		}
		return lStaticPool;
	}

	//////////////////////////////////////////////////////////////////////////
	IPool*	Memory::getResourcePool()
	{
		if( lCommonPool == NULL )
		{
			lCommonPool.lock();
			if( lCommonPool == NULL )
			{
				lCommonPool.construct();
				lCommonPool->initialize(1,0,16*1024);
			}
			lCommonPool.unlock();
		}
		return lCommonPool;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Memory::enableLeakDump(bool enable)
	{
#if BLADE_MEMORY_DEBUG
		lLeakDump = enable;
#else
		BLADE_UNREFERENCED(enable);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Memory::isLeakDumpEnabled()
	{
		return lLeakDump;
	}

	/************************************************************************/
	/* BaseAllocatable                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void BaseAllocatable::deallocate(void* ptr, IPool::ALLOC_TYPE type)
	{
		if (ptr == NULL)
			return;
		size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;

		void* p = (char*)ptr - extra;
		(*(IPool**)p)->deallocate(p, type);
	}

	//////////////////////////////////////////////////////////////////////////
	void* BaseAllocatable::allocate(IPool* pool, size_t bytes, const char* sourcefile, int lineNO, IPool::ALLOC_TYPE type)
	{
		size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
		void* p = pool->allocate(bytes + extra, sourcefile, lineNO, type);
		*((IPool**)p) = pool;
		return (char*)p + extra;
	}

	//////////////////////////////////////////////////////////////////////////
	void* BaseAllocatable::allocate(IPool* pool, size_t bytes, IPool::ALLOC_TYPE type)
	{
		size_t extra = (sizeof(void*) + Memory::ALIGNMENT - 1) / Memory::ALIGNMENT * Memory::ALIGNMENT;
		void* p = pool->allocate(bytes + extra, type);
		*((IPool**)p) = pool;
		return (char*)p + extra;
	}

}//namespace Blade

/************************************************************************/
/* ::new, ::delete                                                                     */
/************************************************************************/
#if BLADE_MEMORY_DEBUG
//////////////////////////////////////////////////////////////////////////
void*		operator new(size_t size, const char* sourcefile, int lineNO, const Blade::MEMINFO*)
{
	return GlobalNewImpl(size, sourcefile, lineNO, Blade::IPool::PAT_NEW);
}

//////////////////////////////////////////////////////////////////////////
void		operator delete(void* ptr, const char*, int, const Blade::MEMINFO*)
{
	return GlobalDeleteImpl(ptr, Blade::IPool::PAT_NEW);
}

//////////////////////////////////////////////////////////////////////////
void*		operator new[](size_t size, const char* sourcefile, int lineNO, const Blade::MEMINFO*)
{
	return GlobalNewImpl(size, sourcefile, lineNO, Blade::IPool::PAT_NEW_ARRAY);
}

//////////////////////////////////////////////////////////////////////////
void		operator delete[](void* ptr, const char*, int, const Blade::MEMINFO*)
{
	return GlobalDeleteImpl(ptr, Blade::IPool::PAT_NEW_ARRAY);
}

#else

//////////////////////////////////////////////////////////////////////////
void*		operator new(size_t size, const Blade::MEMINFO*)
{
	return GlobalNewImpl(size, NULL, 0, Blade::IPool::PAT_NEW);
}

//////////////////////////////////////////////////////////////////////////
void		operator delete(void* ptr, const Blade::MEMINFO*)
{
	return GlobalDeleteImpl(ptr, Blade::IPool::PAT_NEW);
}

//////////////////////////////////////////////////////////////////////////
void*		operator new[](size_t size, const Blade::MEMINFO*)
{
	return GlobalNewImpl(size, NULL, 0, Blade::IPool::PAT_NEW_ARRAY);
}

//////////////////////////////////////////////////////////////////////////
void		operator delete[](void* ptr, const Blade::MEMINFO*)
{
	return GlobalDeleteImpl(ptr, Blade::IPool::PAT_NEW_ARRAY);
}

#endif //BLADE_MEMORY_DEBUG