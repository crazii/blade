/********************************************************************
	created:	2014/02/24
	filename: 	StringTable.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/StringTable.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	tchar* StringTable::readStrings(IOBuffer& buffer, StringInfoList& info, uint32& count, uint32& bytes, IPool* pool/* = Memory::getResourcePool() */)
	{
		if( pool == NULL )
		{
			assert(false);
			return NULL;
		}

		count = 0;
		bytes = 0;

		buffer.read(&count);
		buffer.read(&bytes);
		if( count == 0 || bytes == 0)
			return NULL;

		info.create(count);
		IOBuffer tbuffer;
		for(uint32 i = 0; i < count; ++i)
		{
			//convert utf8 string to tstring
			TString tstr;
			buffer.readTString(tstr);
			tbuffer.writeTStringRAW(tstr);
			info[i].start = (i == 0 ? 0: info[i-1].start + info[i-1].size+1);
			info[i].size = (uint32)tstr.size();
		}
		assert(buffer.eof());
		bytes = (uint32)tbuffer.tell();
		tchar* stringBuffer = (tchar*)pool->allocate( (size_t)bytes );
		std::memcpy(stringBuffer, tbuffer.getData(), (size_t)bytes );
		return stringBuffer;
	}


	//////////////////////////////////////////////////////////////////////////
	tchar* StringTable::readStrings(const HSTREAM& stream, StringInfoList& info, uint32& count, uint32& bytes, IPool* pool/* = Memory::getResourcePool()*/)
	{
		if( stream != NULL && pool != NULL && (stream->getAccesMode()&IStream::AM_READ) )
		{
			count = 0;
			bytes = 0;

			stream->readAtom(count);
			stream->readAtom(bytes);
			if( count == 0 || bytes == 0)
				return NULL;

			IOBuffer buffer;
			buffer.reserve( (size_t)bytes + sizeof(uint32)*2 );
			buffer.write(&count);
			buffer.write(&bytes);
			stream->readData(buffer.getCurrentData(), bytes);

			buffer.rewind();
			return StringTable::readStrings(buffer, info, count, bytes, pool);
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool StringTable::writeStrings(IOBuffer& buffer, const TString* stringArray, uint32 count)
	{
		if( stringArray == NULL && count != 0)
		{
			assert(false);
			return false;
		}

		//convert to utf8 string
		IOBuffer cache;
		for(size_t i = 0; i < count; ++i)
			cache.writeTString( stringArray[i] );

		buffer.write(&count);
		uint32 bytes = (uint32)cache.tell();
		buffer.write(&bytes);
		buffer.writeData(cache.getData(), bytes);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool StringTable::writeStrings(const HSTREAM& stream, const TString* stringArray, uint32 count)
	{
		IOBuffer buffer;
		if( stream != NULL && (stream->getAccesMode()&IStream::AM_WRITE) && StringTable::writeStrings(buffer, stringArray, count) )
			return buffer.saveToStream(stream);
		return false;
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool StringTableCache::persist(const TStringParam& params, StringTable& table, IPool* pool/* = Memory::getResourcePool()*/)
	{
		IOBuffer buffer;
		if( !StringTable::writeStrings(buffer, params) )
			return false;

		buffer.rewind();

		return StringTable::readStrings(buffer, table, pool);
	}
	
}//namespace Blade