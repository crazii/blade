/********************************************************************
	created:	2014/02/09
	filename: 	BPKFile.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/BPKFile.h>
#include "BPKData.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127) //conditional expression is constant
#endif

namespace Blade
{
	/*
	note: BPK file layout:
	+-------------+-----------+-----------+------+--------------------------+-----------------+-------------+-------+
	| 4byte magic | file0data  | file1data | ...  | string table0: node name  | dir index table | entry table  | magic |
	+-------------+-----------+-----------+------+--------------------------+-----------------+--------------+------+
	| head        |          data                 |    tail                                                            |
	*/

	//////////////////////////////////////////////////////////////////////////
	struct BPKFile::Stream : public TempAllocatable
	{
		IStream::Off	mPos;
		BPK_ENTRY*		mEntry;
		index_t			mStreamIndex;
		Lock			mLock;
		uint16			mStatus;
		uint16			mEOF;

		~Stream()
		{
			//lock before delete
			mLock.lock();
		}
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	BPKFile::BPKFile()
		:mData( BLADE_NEW BPKData() )
	{
	}

	//////////////////////////////////////////////////////////////////////////
	BPKFile::~BPKFile()
	{
		this->closeFile();
	}

	/************************************************************************/
	/* package manipulations(NOT thread safe)                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::openFile(const HSTREAM& package)
	{
		if( package != NULL && package->isValid() && (package->getAccesMode()&IStream::AM_READ) != 0 )
		{
			HSTREAM hStream[MAX_PARALLEL_COUNT];

			for(uint i = 0; i < MAX_PARALLEL_COUNT; ++i)
			{	
				hStream[i] = package;
				//write mode: only one stream
				if(package->getAccesMode()&IStream::AM_WRITE)
					break;
			}
			return this->openFile(hStream, MAX_PARALLEL_COUNT);
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::openFile(const HSTREAM* stream, size_t count)
	{
		if( stream != NULL && count == MAX_PARALLEL_COUNT )
		{
			this->closeFile();

			for(uint i = 0; i < count; ++i)
				mStream[i] = stream[i];

			if( mData->initialize(stream[0]) )
				return true;
			mData->clear();
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::closeFile()
	{
		for(uint i = 0; i < MAX_PARALLEL_COUNT; ++i)
		{
			mStream[i].clear();
			assert( !mStreamLock[i].isLocked() && "opened file not closed, use BPKFile::fclose to close it.");
			mStreamLock[i].lock();	//wait other(threads) to closed files (may cause dead lock if fclose() is missing  )
			mStreamLock[i].unlock();
		}
		mData->clear();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::isOpen() const
	{
		if( mData->isValid() )
		{
			bool ret = true;
			for(uint i = 0; i < MAX_PARALLEL_COUNT; ++i)
				ret = (ret && ((mStream[i] != NULL && mStream[i]->isValid()) || mData->isWritable() ) );
			return ret;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::writeFile()
	{
		if( this->isOpen() && mData->isWritable() && !mStreamLock[0].isLocked() )
		{
			ScopedLock lock(mStreamLock[0]);
			return mData->writeTail(mStream[0]);
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::refillFile(const HFILEDEVICE& folder, IBPKFilter* flt/* = NULL*/, IBPKCallback* callback/* = NULL*/)
	{
		if( (this->isOpen() || (mStream[0] != NULL && mStream[0]->getSize() == 0)) && mData->isWritable() && !mStreamLock[0].isLocked())
		{
			//clean old data
			mStream[0]->rewind();
			mStream[0]->writeData( BPK_HEAD_MAGIC.c_str(), (IStream::Size)BPK_HEAD_MAGIC.size() );
			mData->reinit();

			bool ret = this->addEntry(folder, TString::EMPTY, flt, callback);

			//re-init(restore) data on failure
			if( !ret )
			{
				mData->clear();
				mData->initialize(mStream[0]);
			}
			return ret;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::addEntry(const HFILEDEVICE& folder, const TString& destPath, IBPKFilter* flt/* = NULL*/, IBPKCallback* callback/* = NULL*/)
	{
		if(!folder->isOpen())
		{
			assert(false);
			return false;
		}

		if( mStream[0] == NULL || !(mStream[0]->getAccesMode()&IStream::AM_WRITE) || mStreamLock[0].isLocked() )
		{
			assert(false);
			return false;
		}

		//source path is a directory
		ScopedLock lock(mStreamLock[0]);

		//destPath cannot be empty
		if( destPath == TString::EMPTY || !TStringHelper::isStartWith(destPath, BTString("/") ) )
			return false;

		if( !this->isOpen() && destPath != BTString("/") )
			return false;

		if( this->isOpen() )
		{
			//BPK_ENTRY* entry = this->findEntry(destPath);
			//if( entry != NULL )
			//	return false;
		}
		else
		{
			assert( destPath == TString::EMPTY );
			assert( mData->getTableOffset() == 4 );
			//skip magic
			mStream[0]->seek(IStream::SP_BEGIN, (IStream::Off)BPK_HEAD_MAGIC.size() );
		}

		if(true)
		{
			FILE_TIME fileTime = FILE_TIME(Time());

			TStringTokenizer destTokenizer;
			destTokenizer.tokenize(destPath, TEXT("/\\"));

			//1.count all new files & dirs, reserve enough space.
			//2.add folders & files without write tail
			//3.write tail
			//TODO: optimization - use recursion for each directory level, avoid find shared parent for files in the same directory
			TStringParam dirList;
			folder->listFile(dirList, IFileDevice::FF_DIR|IFileDevice::FF_RECURSIVE);
			uint32 dirCount = (uint32)dirList.size();

			TStringParam fileList;
			folder->listFile(fileList, IFileDevice::FF_FILE|IFileDevice::FF_RECURSIVE);
			uint32 fileCount = (uint32)fileList.size();

			//reserve space to avoid frequent re-allocation
			mData->reserveEntries(mData->getEntryCount() + dirCount + fileCount + (uint32)destTokenizer.size() );

			//create dest path
			{
				TStringConcat conPath(TString::EMPTY);
				for(size_t i = 0; i < destTokenizer.size(); ++i)
				{
					conPath += BTString("/") + destTokenizer[i];
					TString dirName;
					BPK_ENTRY* parent = mData->locateParent(conPath, dirName);
					assert(parent != NULL);
					mData->addFolder(parent, dirName, BPKA_DIR, fileTime, mStream[0], false);
				}
			}

			//add folders
			for(size_t i = 0; i < dirCount; ++i)
			{
				TString dir = destPath + BTString("/") + dirList[i];
				TString dirName;
				BPK_ENTRY* parent = mData->locateParent(dir, dirName);
				assert(parent != NULL);
				mData->addFolder(parent, dirName, BPKA_DIR, fileTime, mStream[0], false);
			}

			//add files
			size_t total = fileCount;
			size_t counting = 0;
			for(size_t i = 0; i < fileCount; ++i)
			{
				const TString& file = fileList[i];
				if(flt != NULL && !flt->filter(file))
					continue;

				TString destFile = destPath + BTString("/") + file;
				TString fileName;
				BPK_ENTRY* parent = mData->locateParent(destFile, fileName);
				assert(parent != NULL);
				mData->addFile(parent, fileName, BPKA_FILE, fileTime, folder->openFile(file), mStream[0], false);
				if( callback != NULL )
				{
					TString srcFile;
					folder->getFullPath(fileList[i], srcFile);
					callback->onProgress(srcFile + BTString(" => ") + destFile, (scalar)(counting++)/(scalar)total );
				}
			}
			if( callback != NULL )
				callback->onProgress(TString::EMPTY, 1.0f );

			mData->writeTail(mStream[0]);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	BPK_ENTRY*	BPKFile::addEntry(const HSTREAM& stream, uint32 attrib, const TString& destPath, bool overwrite/* = true*/, IBPKFilter* flt/* = NULL*/, IBPKCallback* callback/* = NULL*/)
	{
		if( (attrib&BPKA_DIR) || (attrib&BPKA_DELETED) || !this->isOpen() || !mData->isWritable() || mStreamLock[0].isLocked() )
		{
			assert(false);
			return NULL;
		}

		if( stream == NULL || !stream->isValid() || stream->getSize() == 0 )
		{
			assert(false);
			return NULL;
		}

		const TString& fileName = stream->getName();
		if (flt != NULL && flt->filter(fileName))
			return NULL;

		attrib |= BPKA_FILE;

		TString name;
		BPK_ENTRY* parent = mData->locateParent(destPath, name);
		if( parent == NULL )
			return NULL;

		if (callback != NULL)
			callback->onProgress(fileName, 0.0f);

		BPK_ENTRY* existing = mData->findSubEntry(parent, name);
		FILE_TIME curTime = FILE_TIME( Time() );

		//already exist?
		if( existing != NULL )
		{
			//only overwrite files, no directories
			if( overwrite && !existing->isDirectory() )
			{
				IStream::Size size = existing->mFile.mSize;
				if( size == stream->getSize() )
				{
					existing->mAttribute = attrib;
					existing->mLastTime = curTime;
					//overwrite data
					//lock stream 0
					ScopedLock lock(mStreamLock[0]);
					BPKData::copyStream(stream, mStream[0], 0, existing->mFile.mOffset, size);
					return existing;
				}
				else
					//delete old entry
					existing->mAttribute |= BPKA_DELETED;
			}
			else
				return NULL;
		}

		if (callback != NULL)
			callback->onProgress(fileName, 0.4f);

		BPK_ENTRY* ret = NULL;
		//lock stream 0
		{
			ScopedLock lock(mStreamLock[0]);
			ret = mData->addFile(parent, name, attrib, curTime, stream, mStream[0], true);
		}

		if (callback != NULL)
			callback->onProgress(fileName, 1.0f);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::removeEntry(const TString& destPath)
	{
		if( !this->isOpen() || mStreamLock[0].isLocked() )
		{
			assert(false);
			return false;
		}

		return mData->removeEntry(destPath);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::eraseAll()
	{
		if( (this->isOpen() || (mStream[0] != NULL && mStream[0]->getSize() == 0)) && mData->isWritable() && !mStreamLock[0].isLocked())
		{
			//clean old data
			mStream[0]->rewind();
			mStream[0]->writeData( BPK_HEAD_MAGIC.c_str(), (IStream::Size)BPK_HEAD_MAGIC.size() );
			bool ret = mData->reinit();
			assert(ret);
			return ret;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	struct EntrySortOffset
	{
		bool operator()(const void* left,const void* right) const
		{
			const BPK_ENTRY* lhs = (const BPK_ENTRY*)left;
			const BPK_ENTRY* rhs = (const BPK_ENTRY*)right;
			if( lhs->isDirectory() && !rhs->isDirectory() )
				return true;
			else if( !lhs->isDirectory() && rhs->isDirectory() )
				return false;
			else if( lhs->isDirectory() && rhs->isDirectory() )
				return lhs < rhs;
			else
				return lhs->mFile.mOffset < rhs->mFile.mOffset;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	struct BPKDeleteInfo
	{
		BPK_ENTRY*	mEntry;
		bool		mRecursiveDel;

		BPKDeleteInfo(){}
		BPKDeleteInfo(BPK_ENTRY* entry) :mEntry(entry),mRecursiveDel(false)			{}
		BPKDeleteInfo(BPK_ENTRY* entry, bool del) :mEntry(entry),mRecursiveDel(del)	{}
	};
	typedef TempStack<BPKDeleteInfo> EntryStack;
	bool	BPKFile::countDeletedSpace(IStream::Size& bytes, BPKEntryList& entryList) const
	{
		if( this->isOpen() )
		{
			bytes = 0;
			entryList.clear();

			EntryStack stack;
			stack.push(mData->getRoot());

			while( !stack.empty() )
			{
				BPKDeleteInfo info = stack.top();
				stack.pop();
				BPK_ENTRY* entry = info.mEntry;
				assert( entry->isDirectory() );
				for(uint32 i = 0; i < entry->mDir.mSize; ++i)
				{
					BPK_ENTRY* child = mData->getChild(entry, i);
					if( child->isDeleted() || info.mRecursiveDel )
					{
						entryList.push_back(child);
						if( !child->isDirectory() )
							bytes += child->mFile.mSize;
						else
						{
							//put all sub entries/descendants in list
							stack.push( BPKDeleteInfo(child, true) );
						}
					}
					else if( child->isDirectory() )
						stack.push(child);
				}
			}

			//sort by offset
			std::sort(entryList.begin(), entryList.end(), EntrySortOffset());
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFile::reorganizeFile(IStream::Size threshold)
	{
		if( this->isOpen() )
		{
			if( !mData->isWritable() )
				return false;
		}

		//find all deleted files and order them by offset
		//for each gap, move data block that is before next gap(between gaps) to overwrite this gap.
		//write tail with new offset.
		IStream::Size totalbytes = 0;
		BPKEntryList list;
		this->countDeletedSpace(totalbytes, list);

		if( totalbytes >= threshold )
		{
			for(size_t i = 0; i < list.size(); ++i)
			{
				BPK_ENTRY* entry = list[i];
				assert( entry->isDeleted() );
				if( entry->isDirectory() )
					continue;

			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void	BPKFile::findEntries(const TString& wildcard, BPKEntryList& list) const
	{
		list.clear();

		TString suffix;
		BPK_ENTRY* parent = mData->locateParent(wildcard, suffix);
		if( parent == NULL )
			return;

		for(uint32 i = 0; i < parent->mDir.mSize; ++i)
		{
			BPK_ENTRY* child = mData->getChild(parent, i);
			const TString& childName = mData->getPath( child->mName );
			if( TStringHelper::wildcardMatch(suffix, childName ) )
				list.push_back( child );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	BPK_ENTRY*	BPKFile::findEntry(const TString& path) const
	{
		return mData->findEntry(path);
	}

	//////////////////////////////////////////////////////////////////////////
	const tchar*BPKFile::getEntryName(const BPK_ENTRY* entry) const
	{
		if( entry != NULL )
			return mData->getPath( entry->mName ).c_str();
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	BPKFile::FILE*	BPKFile::entryToStream(BPK_ENTRY* entry) const
	{
		if( this->isOpen() && entry != NULL && !entry->isDirectory() )
		{
			if( mData->isWritable() )
			{
				//only one writable stream, and it's in use, return empty
				assert( MAX_PARALLEL_COUNT == 1 || mStream[MAX_PARALLEL_COUNT-1] == NULL );
				if( mStreamLock[0].isLocked() )
				{
					assert(false && "close the other opened FILE stream first." );
					return NULL;
				}
			}

			BPKFile::FILE* fp = BLADE_NEW BPKFile::FILE();
			fp->mEntry = entry;
			fp->mPos = 0;
			fp->mStatus = 0;
			fp->mStreamIndex = this->fetchStream();
			mStream[fp->mStreamIndex]->seek(IStream::SP_BEGIN, entry->mFile.mOffset );
			return fp;
		}
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	BPKFile::getBPKName() const
	{
		if( mStream[0] != NULL )
			return mStream[0]->getName();
		else
			return TString::EMPTY;
	}

	/************************************************************************/
	/* I/O operations (thread safe)                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	BPKFile::FILE*	BPKFile::fopen(const TString& path) const
	{
		BPK_ENTRY* entry = this->findEntry(path);
		if( entry != NULL )
		{
			if( !entry->isDirectory() )
			{
				BPKFile::FILE* fp = this->entryToStream(entry);
				return fp;
			}
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	int		BPKFile::fclose(BPKFile::FILE* stream) const
	{
		if( stream != NULL )
		{
			ScopedLock lock(stream->mLock);
			index_t index = stream->mStreamIndex;
			assert( mStreamLock[index].isLocked() );
			mStreamLock[index].unlock();
		}
		BLADE_DELETE stream;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	int		BPKFile::fseek(BPKFile::FILE* stream, IStream::Off offset, IStream::EStreamPos origin) const
	{
		if( stream != NULL )
		{
			assert( stream->mEntry != NULL && !stream->mEntry->isDirectory() );
			ScopedLock lock(stream->mLock);

			IStream::Off begin = 0;
			IStream::Off current = stream->mPos;
			IStream::Off end = stream->mEntry->mFile.mSize;

			if( !(current >= begin && current <= end) )
			{
				assert(false);
				stream->mStatus = 1;
			}

			switch(origin)
			{
			case IStream::SP_BEGIN:
				stream->mPos = begin + offset;
				break;
			case IStream::SP_CURRENT:
				stream->mPos = current + offset;
				break;
			case IStream::SP_END:
				stream->mPos = end + offset;
				break;
			case IStream::SP_EOF:
				assert(false);
				break;
			}

			if( stream->mPos < begin )
				stream->mPos = begin;

			if( stream->mPos > end )
			{
				stream->mPos = end;
				stream->mEOF = 1;
			}

			if( stream->mPos != current )
				mStream[stream->mStreamIndex]->seek(IStream::SP_BEGIN, stream->mPos + stream->mEntry->mFile.mOffset );
			return 0;
		}
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off BPKFile::ftell(BPKFile::FILE* stream) const
	{
		if( stream != NULL )
		{
			assert( stream->mEntry != NULL && !stream->mEntry->isDirectory() );
			ScopedLock lock(stream->mLock);

			IStream::Off size = stream->mEntry->mFile.mSize;
			IStream::Off pos = stream->mPos;

			if( pos  < 0 )
			{
				assert(false);
				stream->mStatus = 1;
				stream->mPos = 0;
			}
			if( pos > size )
			{
				assert(false);
				stream->mStatus = 1;
				stream->mPos = size;
			}
			return stream->mPos;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t 	BPKFile::fread(void* ptr, size_t size, size_t count, BPKFile::FILE* stream) const
	{
		if( stream != NULL )
		{
			assert( stream->mEntry != NULL && !stream->mEntry->isDirectory() );
			ScopedLock lock(stream->mLock);

			IStream::Size streamSize = stream->mEntry->mFile.mSize;
			IStream::Size bytes = (IStream::Size)size*(IStream::Size)count;

			if( stream->mPos + bytes >= streamSize )
			{
				stream->mEOF = true;
				bytes = streamSize - stream->mPos;
				if( bytes == 0)
					return 0;
			}
			IStream::Size readByes = mStream[stream->mStreamIndex]->readData(ptr, bytes);
			stream->mPos += readByes;

			return (size_t)( readByes / (IStream::Size)size);
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	int		BPKFile::feof(FILE* stream) const
	{
		if( stream != NULL )
		{
			ScopedLock lock(stream->mLock);
			return stream->mEOF;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	int		BPKFile::ferror(FILE* stream) const
	{
		if( stream != NULL )
		{
			ScopedLock lock(stream->mLock);
			return stream->mStatus;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void	BPKFile::clearerr(FILE* stream) const
	{
		if( stream != NULL )
		{
			ScopedLock lock(stream->mLock);
			stream->mEOF = 0;
			stream->mStatus = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size BPKFile::fsize(FILE* stream) const
	{
		if( stream != NULL )
		{
			assert( stream->mEntry != NULL && !stream->mEntry->isDirectory() );
			ScopedLock lock(stream->mLock);
			return stream->mEntry->mFile.mSize;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size BPKFile::fread(void* ptr, IStream::Size bytes, FILE* stream) const
	{
		if( stream != NULL )
		{
			assert( stream->mEntry != NULL && !stream->mEntry->isDirectory() );
			ScopedLock lock(stream->mLock);
			IStream::Size streamSize = stream->mEntry->mFile.mSize;
			if( stream->mPos + bytes >= streamSize )
			{
				stream->mEOF = true;
				bytes = streamSize - stream->mPos;
				if( bytes == 0)
					return 0;
			}
			IStream::Size readBytes = mStream[stream->mStreamIndex]->readData(ptr, bytes);
			stream->mPos += readBytes;
			return readBytes;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	index_t		BPKFile::fetchStream() const
	{
		uint i = 0;

		while(true)
		{
			if( mStreamLock[i].tryLock() )
				return i;
			else if( ++i >= MAX_PARALLEL_COUNT )
				i = 0;
		}
		return INVALID_INDEX;
	}

	
}//namespace Blade