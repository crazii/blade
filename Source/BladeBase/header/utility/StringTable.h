/********************************************************************
	created:	2014/02/24
	filename: 	StringTable.h
	author:		Crazii
	purpose:	utility to read/write strings from/into a block in stream.
				better NOT include this file in public headers
				and DON'T use StringTable as public class's member or public functions' parameters
*********************************************************************/
#ifndef __Blade_StringTable_h__
#define __Blade_StringTable_h__
#include "String.h"
#include "BladeContainer.h"
#include "utility/StringList.h"
#include <interface/public/file/IStream.h>

namespace Blade
{
	class IOBuffer;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class StringTable : public Vector<TString>, public Allocatable
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		struct SStringInfo : public TempAllocatable
		{
			index_t	start;
			size_t	size;
		};
		class StringInfoList : public TempAllocatable
		{
		public:
			inline StringInfoList()		{mInfo = NULL; mCount = 0;}
			inline ~StringInfoList()	{this->destroy();}

			/** @brief  */
			inline size_t	size() const{return mCount;}

			/** @brief  */
			inline void create(size_t count)
			{
				this->destroy();
				mInfo = BLADE_NEW SStringInfo[count];
				if( mInfo != NULL )
					mCount = count;
			}

			/** @brief  */
			inline void destroy()
			{
				BLADE_DELETE[] mInfo;
				mInfo = NULL;
				mCount = 0;
			}

			/** @brief  */
			SStringInfo&	operator[](index_t index)
			{
				assert( mInfo != NULL );
				if( index < mCount )
					return mInfo[index];
				else
					return *(SStringInfo*)(NULL);	//generate a error intentionally
			}

			SStringInfo*	mInfo;
			size_t			mCount;
		};

		/*
		@describe 
		@param 
		@return 
		*/
		static BLADE_BASE_API tchar* readStrings(IOBuffer& buffer, StringInfoList& info, uint32& count, uint32& bytes, IPool* pool = Memory::getResourcePool() );

		/*
		@describe 
		@param 
		@return 
		*/
		static BLADE_BASE_API tchar* readStrings(const HSTREAM& stream, StringInfoList& info, uint32& count, uint32& bytes, IPool* pool = Memory::getResourcePool() );

		/*
		@describe 
		@param
		@return
		*/
		static BLADE_BASE_API bool writeStrings(IOBuffer& buffer, const TString* stringArray, uint32 count);

		/** @brief  */
		static inline bool writeStrings(IOBuffer& buffer, const TStringParam& list)
		{
			return writeStrings(buffer, list.getBuffer(), (uint32)list.size());
		}

		/** @brief  */
		inline static bool writeStrings(IOBuffer& buffer, const StringTable& table)
		{
			if( table.size() > 0 )
				return writeStrings(buffer, &table[0], (uint32)table.size());
			else
				return writeStrings(buffer, NULL, (uint32)table.size());
		}


		/**
		@describe 
		@param 
		@return 
		*/
		static BLADE_BASE_API bool writeStrings(const HSTREAM& stream, const TString* stringArray, uint32 count);

		/** @brief  */
		static inline bool writeStrings(const HSTREAM& stream, const TStringParam& list)
		{
			return writeStrings(stream, list.getBuffer(), (uint32)list.size());
		}

		/** @brief  */
		inline static bool writeStrings(const HSTREAM& stream, const StringTable& table)
		{
			if( table.size() > 0 )
				return writeStrings(stream, &table[0], (uint32)table.size());
			else
				return writeStrings(stream, NULL, (uint32)table.size());
		}

		/** @brief  */
		static inline TString	getString(const tchar* buffer, const SStringInfo& info)
		{
			return TString( buffer + info.start, info.size, TString::CONST_HINT);
		}

		//////////////////////////////////////////////////////////////////////////
		StringTable()	{mBuffer = NULL; mPool = NULL;}
		~StringTable()	{ this->clear(); }

		/** @brief  */
		inline void clear()
		{
			if( mBuffer != NULL )
			{
				assert( mPool != NULL );
				mPool->deallocate(mBuffer);
				mBuffer = NULL;
				mPool = NULL;
			}
			Vector<TString>::clear();
		}

		/** @brief  */
		inline static bool readStrings(const HSTREAM& stream, StringTable& table, IPool* pool = Memory::getResourcePool() )
		{
			table.clear();
			StringInfoList info;
			uint32 count = 0, bytes = 0;
			table.mBuffer = StringTable::readStrings(stream, info, count, bytes, pool);
			if( table.mBuffer != NULL )
			{
				assert( info.size() == count );
				table.reserve(count);
				for(size_t i = 0; i < info.size(); ++i )
					table.push_back( StringTable::getString(table.mBuffer, info[i]) );
				table.mPool = pool;
				return true;
			}
			return false;
		}

		/** @brief  */
		inline static bool readStrings(IOBuffer& buffer, StringTable& table, IPool* pool = Memory::getResourcePool() )
		{
			table.clear();
			StringInfoList info;
			uint32 count = 0, bytes = 0;
			table.mBuffer = StringTable::readStrings(buffer, info, count, bytes, pool);
			if( table.mBuffer != NULL )
			{
				assert( info.size() == count );
				table.reserve(count);
				for(size_t i = 0; i < info.size(); ++i )
					table.push_back( StringTable::getString(table.mBuffer, info[i]) );
				table.mPool = pool;
				return true;
			}
			return false;
		}

		
	protected:
		tchar*	mBuffer;
		IPool*	mPool;
	};//class StringTable


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class StringTableCache : public NonAllocatable
	{
	public:
		TStringParam mList;
		typedef TempTStringMap<index_t> IndexMap;
		IndexMap mMap;

		StringTableCache()	{}

		StringTableCache(const StringTable& src)
		{
			for(size_t i = 0; i < src.size(); ++i)
			{
				index_t index = this->add(src[i]);
				//string table MUST have unique strings, each add operation adding a new string
				if( index != mList.size() -1 )
					assert(false);
			}
		}


		/** @brief  */
		inline void clear()
		{
			mList.clear();
			mMap.clear();
		}

		/** @brief  */
		inline void reserve(size_t count)
		{
			mList.reserve(count);
		}

		/** @brief  */
		index_t	add(const TString& str)
		{
			std::pair<IndexMap::iterator, bool> ret = mMap.insert( std::make_pair(str, (index_t)0 ) );
			if( ret.second )
			{
				mList.push_back(str);
				ret.first->second = mList.size() - 1;
			}
			return ret.first->second;
		}

		/** @brief sort the string list(NOT in alphabet order). index will be changed.
		if you use/store the index previously, you need to update it by calling add() */
		void sort()
		{
			index_t index = 0;
			for(IndexMap::iterator i = mMap.begin(); i != mMap.end(); ++i)
			{
				i->second = index;
				mList[ index++ ] = i->first;
			}
		}

		/** @brief  */
		bool	persist(StringTable& table, IPool* pool = Memory::getResourcePool())
		{
			return StringTableCache::persist(this->mList, table, pool);
		}

		/** @brief  */
		static BLADE_BASE_API bool persist(const TStringParam& params, StringTable& table, IPool* pool = Memory::getResourcePool());
	};
	
}//namespace Blade

#endif //  __Blade_StringTable_h__