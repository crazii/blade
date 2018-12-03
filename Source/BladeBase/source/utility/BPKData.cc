/********************************************************************
	created:	2014/02/11
	filename: 	BPKData.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/BPKFile.h>
#include <utility/StringTable.h>
#include "BPKData.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	namespace Impl
	{
		//////////////////////////////////////////////////////////////////////////
		bool		BPKData::initialize(const HSTREAM& stream)
		{
			if( this->isValid() || stream == NULL )
				return false;
			
			this->clear();

			//reset writable flag
			mWritable = (stream->getAccesMode()&IStream::AM_WRITE) != 0;

			//empty file
			if( stream->getSize() == 0 )
				return true;
			
			if( stream != NULL && stream->isValid() )
			{
				union
				{
					char magic[4];
					char mstring[5];
				};
				std::memset(mstring,0,sizeof(mstring));

				stream->rewind();

				//header magic
				stream->readData(magic, sizeof(magic));
				if( magic != BPK_HEAD_MAGIC )
				{
					assert(false);
					return false;
				}

				//tail magic
				stream->seek(IStream::SP_END, -4);
				stream->readData(magic, sizeof(magic));
				if( magic != BPK_TAIL_MAGIC )
				{
					assert(false);
					return false;
				}

				//tail offset
				IStream::Off tailOff;
				stream->seek(IStream::SP_END, -(IStream::Off)(sizeof(tailOff)+4) );
				stream->readAtom(tailOff);

				mTailOffset = tailOff;
				stream->seek(IStream::SP_BEGIN, mTailOffset);

				//read string tables
				if( !this->readStringTable(stream) )
				{
					assert(false);
					return false;
				}

				//read entries
				mEntryCount = 0;
				stream->readData(magic, sizeof(magic));
				if( magic != BPK_ENTRY_TABLE_MAGIC )
					return false;
				stream->readAtom(mEntryCount);

				mCapacity = mEntryCount;
				mEntries = BLADE_RES_ALLOCT(BPK_ENTRY, (size_t)mCapacity);
				stream->read(mEntries, mCapacity);

				//read indices table for directory
				if( !this->readFolderContent(stream) )
				{
					assert(false);
					return false;
				}
				//apply runtime data: share indices
				for(uint32 i = 0; i < mEntryCount; ++i)
				{
					if( mEntries[i].isDirectory() )
					{
						assert( mEntries[i].mDir.mIndices == NULL);
						if( mEntries[i].mDir.mSize != 0 )
						{
							assert( mEntries[i].mDir.mIndex + mEntries[i].mDir.mSize <= mIndexCount );
							mEntries[i].mDir.mIndices = &mIndices[ mEntries[i].mDir.mIndex ];
						}
					}
				}

				//check tail magic again
				stream->readAtom(tailOff);
				assert(tailOff == mTailOffset);
				stream->readData(magic, sizeof(magic));
				if( magic != BPK_TAIL_MAGIC )
					return false;

				return true;
			}
			assert(false);
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		BPKData::reinit()
		{
			if( !this->isValid() && !this->isWritable() )
			{
				assert(false);
				return false;
			}

			this->clear();
			mPathBuffer = BLADE_RES_ALLOCT(tchar, 2);
			mPathBuffer[0] = TEXT('/');
			mPathBuffer[1] = TEXT('\0');
			mPathTable.push_back( TString(mPathBuffer, 1, TString::CONST_HINT) );

			mEntryCount = mCapacity = 1;
			mEntries = BLADE_RES_ALLOCT(BPK_ENTRY, 1);
			std::memset(mEntries, 0, sizeof(BPK_ENTRY)*mEntryCount);
			mEntries[0].mName = 0;
			mEntries[0].mAttribute = BPKA_DIR;
			mEntries[0].mLastTime = FILE_TIME(Time());

			mEntries[0].mDir.mIndex = uint32(-1);
			mEntries[0].mDir.mSize = 0;
			mEntries[0].mDir.mIndices = NULL;

			mTailOffset = (IStream::Off)BPK_HEAD_MAGIC.size();	//header magic
			mWritable = true;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		BPKData::readStringTable(const HSTREAM& stream)
		{
			union
			{
				char magic[4];
				char mstring[5];
			};
			std::memset(mstring,0,sizeof(mstring));

			stream->readData(magic, sizeof(magic));
			if( magic != BPK_STRING_MAGIC )
				return false;

			uint32 count = 0;
			uint32 bytes = 0;

			StringTable::StringInfoList info;
			mPathBuffer = StringTable::readStrings(stream, info, count, bytes);

			mPathTable.reserve(count);
			for(uint32 i = 0; i < count; ++i)
				mPathTable.push_back( StringTable::getString(mPathBuffer, info[i]) );
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		BPKData::readFolderContent(const HSTREAM& stream)
		{
			union
			{
				char magic[4];
				char mstring[5];
			};
			std::memset(mstring,0,sizeof(mstring));

			stream->readData(magic, sizeof(magic));
			if( magic != BPK_DIR_INDEX_MAGIC )
				return false;

			uint32 indexCount = 0;
			stream->readAtom(indexCount);
			assert( indexCount != 0);
			mIndexSize = mIndexCount = indexCount;
			mIndices = BLADE_RES_ALLOCT(uint32,mIndexCount);
			stream->read(mIndices, mIndexCount);

			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		BPKData::writeTail(const HSTREAM& package)
		{
			if( mWritable )
			{
				if( mDirty )
				{
					//write tail:
					package->seek(IStream::SP_BEGIN, mTailOffset);

					//string table
					package->writeData( BPK_STRING_MAGIC.c_str(), (IStream::Size)BPK_STRING_MAGIC.size() );
					{
						typedef TempTStringMap<index_t> StringIDTable;
						StringIDTable table;
						uint32 id = 0;
						TStringParam stringList;

						for(size_t i = 0; i < mEntryCount; ++i)
						{
							const TString& name = mPathTable[ mEntries[i].mName ];
							assert(name != TString::EMPTY );
							std::pair<StringIDTable::iterator,bool> ret = table.insert( std::make_pair(name, id) );
							//insert successfully
							if( ret.second )
							{
								mEntries[i].mName = id;
								stringList.push_back( name );
								++id;
								assert( stringList.size() == id );
							}
							else
								//use existing id
								mEntries[i].mName = (uint32)ret.first->second;
						}

						StringTable::writeStrings(package, stringList);
					}

					//file & directory entry

					//merge indices
					size_t indexCount = 0;
					uint32* cacheIndices = BLADE_TMP_ALLOCT(uint32, mIndexCount);
					for(uint32 i = 0; i < mEntryCount; ++i)
					{
						if( mEntries[i].isDirectory() && mEntries[i].mDir.mSize != 0 )
						{
							assert( indexCount + mEntries[i].mDir.mSize <= mIndexCount );
							assert( mEntries[i].mDir.mIndices != NULL );
							std::memcpy(cacheIndices+indexCount, mEntries[i].mDir.mIndices, mEntries[i].mDir.mSize*sizeof(uint32) );
							
							if( !this->isUsingSharedIndices(&mEntries[i]) )
								BLADE_RES_FREE(mEntries[i].mDir.mIndices);

							mEntries[i].mDir.mIndices = NULL;
							mEntries[i].mDir.mIndex = (uint32)indexCount;

							indexCount += mEntries[i].mDir.mSize;
						}
						else
							assert( !mEntries[i].isDirectory() || (mEntries[i].mDir.mIndices == NULL && mEntries[i].mDir.mIndex == uint32(-1)) );
					}
					assert( indexCount == mIndexCount );

					package->writeData( BPK_ENTRY_TABLE_MAGIC.c_str(), (IStream::Size)BPK_ENTRY_TABLE_MAGIC.size() );
					package->write(&mEntryCount);
					package->write(mEntries, mEntryCount);

					//directory content indices
					package->writeData( BPK_DIR_INDEX_MAGIC.c_str(), (IStream::Size)BPK_DIR_INDEX_MAGIC.size() );
					package->write(&mIndexCount);
					package->write(cacheIndices, mIndexCount);
					BLADE_TMP_FREE(cacheIndices);

					//tail magic
					package->write(&mTailOffset);
					package->writeData( BPK_TAIL_MAGIC.c_str(), (IStream::Size)BPK_TAIL_MAGIC.size() );
					package->truncate( package->tell() );
					mDirty = false;

					//reload data: format data memory
					this->clear();
					this->initialize(package);
					return true;
				}
			}
			else
				assert( !mDirty );
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		bool		BPKData::copyStream(IStream* src, IStream* dest, IStream::Off from, IStream::Off to, IStream::Size bytes)
		{
			if( src == NULL || dest == NULL || bytes <= 0 )
				return false;

			IOBuffer buffer;
			buffer.reserve(4*1024*1024);

			IStream::Size fromSize = src->getSize();
			IStream::Size bufferSize = (IStream::Size)buffer.getCapacity();

			if( src != dest )
			{
				src->seek(IStream::SP_BEGIN, from);
				dest->seek(IStream::SP_BEGIN, to);
			}

			if( fromSize <= bufferSize )
			{
				if( src == dest )
					src->seek(IStream::SP_BEGIN, from);

				src->readData( buffer.getData(), fromSize);

				if( src == dest )
					dest->seek(IStream::SP_BEGIN, to);

				dest->writeData( buffer.getData(), fromSize );
				return true;
			}

			size_t loopCount = (size_t)(fromSize/bufferSize);
			for(size_t i = 0; i < loopCount; ++i)
			{
				if( src == dest )
					src->seek(IStream::SP_BEGIN, from + (IStream::Size)loopCount*bufferSize);

				src->readData( buffer.getData(), bufferSize);

				if( src == dest )
					dest->seek(IStream::SP_BEGIN, to + (IStream::Size)loopCount*bufferSize);

				dest->writeData( buffer.getData(), bufferSize );
			}

			IStream::Size leftSize = fromSize - bufferSize*(IStream::Size)loopCount;

			if( src == dest )
				src->seek(IStream::SP_BEGIN, from + bufferSize*(IStream::Size)loopCount);
			src->readData( buffer.getData(), leftSize);

			if( src == dest )
				dest->seek(IStream::SP_BEGIN, to  + bufferSize*(IStream::Size)loopCount);
			dest->writeData( buffer.getData(), leftSize );
			return true;
		}

	}//namespace Impl
	
}//namespace Blade