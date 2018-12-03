/********************************************************************
	created:	2014/12/20
	filename: 	Util.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Util_h__
#define __Blade_Util_h__
#include <memory/BladeMemory.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class TempStrategy
	{
	public:
		static void* alloc(size_t bytes)
		{
			return BLADE_TMP_ALLOC(bytes);
		}
		static void free(void* ptr)
		{
			return BLADE_TMP_FREE(ptr);
		}
	};

	class DefaultStrategy
	{
	public:
		static void* alloc(size_t bytes)
		{
			return BLADE_RES_ALLOC(bytes);
		}
		static void free(void* ptr)
		{
			return BLADE_RES_FREE(ptr);
		}
	};


	template<typename Strategy>
	class SoftwareBuffer
	{
	public:
		static const size_t HEADGUARD  = 0xB1900D09;
		static const size_t TAILGUARD = 0xBAD0BABE;

	public:
		SoftwareBuffer() :mData(NULL)	{}
		~SoftwareBuffer()				{this->freeBuffer();}

		inline void	allocBuffer(size_t bytes)
		{
			this->freeBuffer();
#if BLADE_DEBUG
			//add guard bytes
			bytes += sizeof(size_t)*2;
			mData = (char*)Strategy::alloc(bytes);
			*( (size_t*)mData ) = HEADGUARD;
			*( (size_t*)( (mData+bytes)-sizeof(size_t)) ) = TAILGUARD;
#else
			mData = (char*)Strategy::alloc(bytes);
#endif	
		}

		inline void check(size_t bytes)
		{
#if BLADE_DEBUG
			assert( *((size_t*)mData ) == HEADGUARD );
			assert( *( (size_t*)( (mData+bytes)+sizeof(size_t)) ) == TAILGUARD );
#else
			BLADE_UNREFERENCED(bytes);
#endif
		}

		inline void	freeBuffer()
		{
			Strategy::free(mData);
			mData = NULL;
		}

		inline char*	getData()
		{
#if BLADE_DEBUG
			return mData == NULL ? NULL : mData + sizeof(size_t);
#else
			return mData;
#endif
		}
		char* mData;
	};

	
}//namespace Blade


#endif // __Blade_Util_h__