/********************************************************************
	created:	2013/10/29
	filename: 	LangTableFile.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "LangTableFile.h"
#include <StaticHandle.h>
#include <utility/StringTable.h>

namespace Blade
{
	const String LANGTABLE_MAGIC = BString("BLLT");

	//////////////////////////////////////////////////////////////////////////
	LangTableFile::LangTableFile()
	{
		mBuffer = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	LangTableFile::~LangTableFile()
	{
		this->close();	
	}

	//////////////////////////////////////////////////////////////////////////
	bool LangTableFile::open(const HSTREAM& stream, MultiLangTable& table)
	{
		if( stream == NULL || !stream->isValid() || mBuffer != NULL)
			return false;

		char magic[5] = {0};
		stream->readData(&magic[0], (IStream::Size)LANGTABLE_MAGIC.size());
		if( LANGTABLE_MAGIC != &magic[0] )
			return false;

		uint32 bytes = 0;

		//read section count: how many type of languages
		uint32 sectionCount = 0;
		stream->read(&sectionCount);

		//combine the string buffer of all tables
		IOBuffer buffer;
		TempVector<StringTable::StringInfoList> list(sectionCount+1);
		TempVector<size_t> sizeList(sectionCount+1);

		//read section table
		uint32 stringCount = 0;
		tchar* tdata = StringTable::readStrings( stream, list[0], stringCount, bytes, Memory::getTemporaryPool() );
		buffer.writeData(tdata, bytes);
		BLADE_TMP_FREE(tdata);
		sizeList[0] = bytes;

		if( list[0].size() != (size_t)sectionCount )
		{
			assert(false);
			return false;
		}

		//read tables for each section
		for(uint32 i = 0; i < sectionCount; ++i)
		{
			tdata = StringTable::readStrings( stream, list[i+1], stringCount, bytes, Memory::getTemporaryPool() );
			buffer.writeData(tdata, bytes);
			BLADE_TMP_FREE(tdata);
			sizeList[i+1] = bytes;
		}

		//combine buffer
		bytes = (uint32)buffer.getSize();
		mBuffer = (tchar*)BLADE_STATIC_ALLOC(bytes);
		std::memcpy(mBuffer, buffer.getData(), bytes);
		buffer.clear();
		
		//cache section table strings
		char* data = (char*)mBuffer;
		typedef TempVector<TString> SectionList;
		SectionList sectionCache;
		sectionCache.reserve( sectionCount );
		for(size_t i = 0; i < sectionCount; ++i )
			sectionCache.push_back( StringTable::getString((const tchar*)data, list[0][i]) );
		data += sizeList[0];

		//get table of strings for each section
		for(uint32 i = 0; i < sectionCount; ++i)
		{
			LangStringTable& t = table[ sectionCache[i] ];
			StringTable::StringInfoList& info = list[i+1];	//skip section table
			t.resize(info.size());

			for(size_t j = 0; j < info.size(); ++j)
				t[j] = StringTable::getString((const tchar*)data, info[j]);

			data += sizeList[i+1];
		}
		assert( (data - (char*)mBuffer) == (ptrdiff_t)bytes );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool LangTableFile::isOpen() const
	{
		return mBuffer != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void LangTableFile::close()
	{
		BLADE_STATIC_FREE(mBuffer);
		mBuffer = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool LangTableFile::save(const HSTREAM& stream, const WritableMultiLangTable& data)
	{
		if( stream == NULL || !stream->isValid() || (stream->getAccesMode()&IStream::AM_WRITE) == 0 )
			return false;

		stream->writeData( LANGTABLE_MAGIC.c_str(), (IStream::Size)LANGTABLE_MAGIC.size() );

		uint32 sectionCount = (uint32)data.size();
		stream->write(&sectionCount);

		TempVector<TString> sections;
		sections.resize(sectionCount);

		index_t index = 0;
		for(WritableMultiLangTable::const_iterator i = data.begin(); i != data.end(); ++i)
			sections[index++] = i->first;
		assert(index == sectionCount);

		if( !StringTable::writeStrings(stream, sectionCount == 0 ? NULL : &sections[0], sectionCount) )
			return false;

		for(WritableMultiLangTable::const_iterator i = data.begin(); i != data.end(); ++i)
		{
			const WritableLangStringTable& table = i->second;
			if( !StringTable::writeStrings(stream, table.size() == 0 ? NULL : &table[0], (uint32)table.size()) )
				return false;
		}
		return true;
	}
	
}//namespace Blade

