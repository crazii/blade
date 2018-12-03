/********************************************************************
	created:	2013/03/23
	filename: 	Lock.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#include <intrin.h>
#pragma warning(disable:4127)	//while(true)
#pragma warning(disable:4505)	//unreferenced local function has been removed
#elif BLADE_COMPILER_GNU_EXTENSION
//empty
#endif

#include "BladeWin32API.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	//static methods
	//////////////////////////////////////////////////////////////////////////
	const Lock::Static Lock::STATIC;

	//////////////////////////////////////////////////////////////////////////
	void		Lock::memoryBarrier()
	{
		//full memory barrier
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		::MemoryBarrier();
#elif BLADE_COMPILER_GNU_EXTENSION
		__sync_synchronize();
#else
#error not implemented.
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int32	Lock::increment32(volatile int32* data)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedIncrement( (volatile long*)data);
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_add_and_fetch(data, 1);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int32	Lock::decrement32(volatile int32* data)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedDecrement( (volatile long*)data);
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_sub_and_fetch(data,1);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int32	Lock::set32(volatile int32* data, int32 val)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		volatile long oldVal;
		while ( _InterlockedCompareExchange( (volatile long*)data, val, (oldVal=*data) ) != oldVal );
		return oldVal;
#elif BLADE_COMPILER_GNU_EXTENSION
		volatile int32 oldVal;
		while( !__sync_bool_compare_and_swap(data, (oldVal=*data), val) );
		return oldVal;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int32	Lock::add32(volatile int32* data, int32 val)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedExchangeAdd( (volatile long*)data, (long)val );
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_fetch_and_add(data,val);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int32	Lock::sub32(volatile int32* data, int32 val)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedExchangeAdd( (volatile long*)data, -(long)val );
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_fetch_and_sub(data,val);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int64	Lock::increment64(volatile int64* data)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		int64 oldVal;
		do
		{
			oldVal = *data;
		}while ( _InterlockedCompareExchange64( data, oldVal+1, oldVal ) != oldVal );
		return oldVal+1;
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_add_and_fetch(data, 1);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int64	Lock::decrement64(volatile int64* data)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		int64 oldVal;
		do
		{
			oldVal = *data;
		}while ( _InterlockedCompareExchange64( data, oldVal-1, oldVal ) != oldVal );
		return oldVal-1;
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_sub_and_fetch(data,1);
#endif
	}

#if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT
	//////////////////////////////////////////////////////////////////////////
	int64	Lock::get64(const volatile int64* data)
	{
#	if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedCompareExchange64( (volatile int64*)data, 0, 0);
#	elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_val_compare_and_swap( (volatile int64*)data, 0, 0);
#	endif
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	int64	Lock::set64(volatile int64* data, int64 val)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		int64 oldVal;
		while ( _InterlockedCompareExchange64( data, val, (oldVal=*data) ) != oldVal );
		return oldVal;
#elif BLADE_COMPILER_GNU_EXTENSION
		int64 oldVal;
		while( !__sync_bool_compare_and_swap(data, (oldVal=*data), val) );
		return oldVal;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int64	Lock::add64(volatile int64* data, int64 val)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		int64 oldVal;
		do
		{
			oldVal = *data;
		}while ( _InterlockedCompareExchange64( data, oldVal+val, oldVal ) != oldVal );
		return oldVal;
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_fetch_and_add(data,val);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int64	Lock::sub64(volatile int64* data, int64 val)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		int64 oldVal;
		do
		{
			oldVal = *data;
		}while ( _InterlockedCompareExchange64( data, oldVal-val, oldVal ) != oldVal );
		return oldVal;
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_fetch_and_sub(data,val);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int32	Lock::compareAndSwap32(volatile int32* data, int32 val, int32 comparand)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedCompareExchange( (volatile long*)data, (long)val, (long)comparand );
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_val_compare_and_swap(data, comparand, val);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	int64	Lock::compareAndSwap64(volatile int64* data, int64 val, int64 comparand)
	{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
		return _InterlockedCompareExchange64( (volatile int64*)data, (long)val, (long)comparand );
#elif BLADE_COMPILER_GNU_EXTENSION
		return __sync_val_compare_and_swap(data, comparand, val);
#endif
	}
}//namespace Blade