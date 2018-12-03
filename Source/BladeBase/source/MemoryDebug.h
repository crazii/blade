/********************************************************************
	created:	2010/06/19
	filename: 	MemoryDebug.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MemoryDebug_h__
#define __Blade_MemoryDebug_h__
#include <map>
#include <StaticHandle.h>
#include <memory/BladeMemory.h>
#include <memory/StaticAllocator.h>
#include <memory/PoolAllocator.h>
#include "interface_imp/pool/IncrementalPool.h"
#include "interface_imp/pool/DLMallocPool.h"


namespace Blade
{
#define SIZE_MAP_ALIGN 0

#if SIZE_MAP_ALIGN
	//////////////////////////////////////////////////////////////////////////
	class SizeMap
	{
	public:
		SizeMap();
		~SizeMap();
		//0-256		align: 8
		//256-512	align: 16
		//512-1024	align: 32
		//1024-2048 align: 64
		//2048-4096 align: 128
		//...

		enum
		{
			SMALL_ALIGN		= 8,
			SMALL_ALLOC		= 256,
			SMALL_COUNT		= SMALL_ALLOC/SMALL_ALIGN,

			//change this exp value if you want handle larger/smaller request (MAX_ALLOC)
			MAX_ALLOC_EXP	= 4,
			MULTIPLE_COUNT	= MAX_ALLOC_EXP + 1,	//1 small + exp count
			MAX_ALLOC		= SMALL_ALLOC*( (1<<MAX_ALLOC_EXP) ),
		};

		size_t	mAligns[MULTIPLE_COUNT];
		SPOOL	mFixedSizePool[MULTIPLE_COUNT][SMALL_COUNT];
	};
#else
	class SizeMap : public std::map<size_t,SPOOL,std::less<size_t>,StaticAllocator< std::pair<const size_t,SPOOL> > >
	{
	public:
		~SizeMap();
	};
#endif

	typedef intptr_t buffer_type;
	static const int BUFFER_ALIGN = sizeof(buffer_type);

	//////////////////////////////////////////////////////////////////////////
	class BLADE_ALIGNED(64) StaticPool : public SPOOL
	{
	public:
		StaticPool();
		~StaticPool();

		void	construct();
		void	destruct();

		buffer_type buffer [ (sizeof(IncrementalPool) + BUFFER_ALIGN -1 )/BUFFER_ALIGN ];
	};


	//////////////////////////////////////////////////////////////////////////
	class BLADE_ALIGNED(64) TempPool : public SPOOL
	{
	public:
		TempPool();
		~TempPool();

		void	construct();
		void	destruct();

		buffer_type buffer [ (sizeof(IncrementalPool) + BUFFER_ALIGN -1 )/BUFFER_ALIGN ];
	};

	//////////////////////////////////////////////////////////////////////////
	class BLADE_ALIGNED(64) CommonPool : public SPOOL
	{
	public:
		CommonPool();
		~CommonPool();

		void	construct();
		void	destruct();

		buffer_type buffer [ (sizeof(DLMallocPool) + BUFFER_ALIGN -1 )/BUFFER_ALIGN ];
	};
	
}//namespace Blade


#endif //__Blade_MemoryDebug_h__