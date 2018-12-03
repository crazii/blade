/********************************************************************
	created:	2014/02/11
	filename: 	BPKData.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BPKData_h__
#define __Blade_BPKData_h__
#include <utility/IOBuffer.h>
#include <utility/StringHelper.h>

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const String BPK_HEAD_MAGIC = BString("BPKH");
	const String BPK_TAIL_MAGIC = BString("BPKT");
	const String BPK_STRING_MAGIC = BString("BPKS");
	const String BPK_DIR_INDEX_MAGIC = BString("BPKI");
	const String BPK_ENTRY_TABLE_MAGIC = BString("BPKE");

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		typedef Vector<TString> PathList;

		class BPKData : public Allocatable
		{
		public:
			BPKData()	
			{
				mEntries = NULL; mCapacity = 0; mEntryCount = 0;
				mIndices = NULL; mIndexSize = 0; mIndexCount = 0;
				mPathBuffer = NULL;
				mTailOffset = 0; mWritable = false; mDirty = false;
			}
			~BPKData()	{this->clear();}

			/** @brief  */
			inline bool		isWritable() const	{return mWritable;}

			/** @brief  */
			inline bool		isValid() const		{return mEntries != NULL && mEntryCount != 0 && mPathBuffer != NULL;}

			/** @brief  */
			inline IStream::Off	getTableOffset() const {return mTailOffset;}

			/** @brief  */
			inline void		clear()
			{
				if( mEntries != NULL )
				{
					assert(mEntryCount > 0);
					for(uint32 i = 0; i < mEntryCount; ++i)
					{
						if( !mEntries[i].isDirectory() || mEntries[i].mDir.mIndices == NULL)
							continue;

						if( !this->isUsingSharedIndices( &mEntries[i] ) )
							BLADE_RES_FREE(mEntries[i].mDir.mIndices);
					}
				}

				mEntryCount = mCapacity = 0;
				BLADE_RES_FREE(mEntries); mEntries = NULL;
				mIndexSize = mIndexCount = 0;
				BLADE_RES_FREE(mIndices); mIndices = NULL;
				BLADE_RES_FREE(mPathBuffer);mPathBuffer = NULL; mPathTable.clear();
				mTailOffset = 0; mWritable = false; mDirty = false;
			}

			/** @brief  */
			inline const TString&	getPath(index_t name) const
			{
				assert( mPathBuffer != NULL && mPathTable.size() > 0 );
				if( name < mPathTable.size() )
					return mPathTable[name];
				else
				{
					assert(false);
					return TString::EMPTY;
				}
			}

			/* @note: folder use shared indices on reading a package
			if any modification applied(i.e. and sub entries to the folder)
			it will use own indices data to expand easily.
			so for a read only package, folders use shared indices */
			inline bool			isUsingSharedIndices(BPK_ENTRY* folder) const
			{
				assert(folder != NULL && folder->isDirectory() );
				return mIndices != NULL && folder->mDir.mIndices != NULL &&
					folder->mDir.mIndices >= mIndices && folder->mDir.mIndices < mIndices + mIndexSize;
			}

			/** @brief  */
			BPK_ENTRY*			getRoot() const					{return mEntries;}

			/** @brief  */
			inline uint32		getEntryCount() const			{return mEntryCount;}

			/** @brief  */
			inline BPK_ENTRY*	findSubEntry(BPK_ENTRY* parent, const TString& name) const;

			/** @brief  */
			inline bool			addIndexToParent(BPK_ENTRY* parent, uint32 index);

			/** @brief find parent node and return filename without path */
			/* @note: return NULL if parent node not found, or exist but is not a directory  */
			inline BPK_ENTRY*	locateParent(const TString& path, TString& name, TString* prefix = NULL) const;

			/** @brief  */
			inline BPK_ENTRY*	getChild(BPK_ENTRY* parent, uint32 index) const;

			/** @brief  */
			inline BPK_ENTRY*	findEntry(const TString& path) const;

			/** @brief  */
			inline bool			removeEntry(const TString& destPath);

			/** @brief  */
			inline bool			reserveEntries(uint32 entryCount);

			/** @brief  */
			inline bool			addIndex(BPK_ENTRY* parent, uint32 index);

			/** @brief  */
			inline BPK_ENTRY*	addFolder(BPK_ENTRY* parent, const TString& name, uint32 attrib, const FILE_TIME& time, const HSTREAM& package, bool bWriteTail);

			/** @brief  */
			inline BPK_ENTRY*	addFile(BPK_ENTRY* parent, const TString& name, uint32 attrib, const FILE_TIME& time, const HSTREAM& fileData, const HSTREAM& package, bool bWriteTail);

			/** @brief  */
			bool		initialize(const HSTREAM& stream);

			/** @brief  */
			bool		reinit();

			/** @brief  */
			bool		readStringTable(const HSTREAM& stream);

			/** @brief  */
			bool		readFolderContent(const HSTREAM& stream);

			/** @brief  */
			bool		writeTail(const HSTREAM& package);

			/** @brief  */
			static bool	copyStream(IStream* src, IStream* dest, IStream::Off from, IStream::Off to, IStream::Size bytes);

		protected:
			uint32		mEntryCount;
			uint32		mCapacity;
			BPK_ENTRY*	mEntries;

			uint32		mIndexCount;	//current count may be larger than size of mIndices
			uint32		mIndexSize;		//allocated size, size of mIndices
			uint32*		mIndices;

			tchar*		mPathBuffer;
			PathList	mPathTable;

			//dirty data record
			IStream::Off mTailOffset;
			bool		mWritable;
			bool		mDirty;
		};
	}//namespace Impl
	using namespace Impl;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	struct FnIndexFinder : public NonAssignable
	{
		const PathList&		mPathListRef;
		const TString&		mNameRef;
		const BPK_ENTRY*	mEntryList;
		size_t				mEntryCount;
		FnIndexFinder(const PathList& listRef, const TString& target, const BPK_ENTRY* entryBase, size_t count)
			:mPathListRef(listRef)
			,mNameRef(target)
			,mEntryList(entryBase)
			,mEntryCount(count)
		{
		}

		//find index by indexed entry's name
		bool operator()(const uint32& lhs, const uint32& /*rhs*/) const
		{
			assert( lhs < mEntryCount );
			const BPK_ENTRY& l = mEntryList[lhs];
			assert( (size_t)l.mName < mPathListRef.size() );
			return FnTStringFastLess::compare(mPathListRef[l.mName], mNameRef);
		}
		//
		bool lessThan(const uint32& val) const
		{
			assert( val < mEntryCount );
			const BPK_ENTRY& v = mEntryList[val];
			assert( (size_t)v.mName < mPathListRef.size() );
			return FnTStringFastLess::compare(mNameRef, mPathListRef[v.mName]);
		}
	};
	//////////////////////////////////////////////////////////////////////////
	inline BPK_ENTRY*	BPKData::findSubEntry(BPK_ENTRY* parent, const TString& name) const
	{
		FnIndexFinder finder(mPathTable, name, mEntries, mEntryCount);
		uint32* index = std::lower_bound(parent->mDir.mIndices, parent->mDir.mIndices + parent->mDir.mSize, 0u, finder);
		//lower bound: name <= [index].name && 
		//!(name < [index].name)
		if( index != parent->mDir.mIndices + parent->mDir.mSize && !finder.lessThan(*index) )
		{
			assert( *index < mEntryCount );
			return mEntries + *index;
		}
		else
			return NULL;
	}


	//////////////////////////////////////////////////////////////////////////
	struct FnIndexSorter : public NonAssignable
	{
		const PathList&		mPathListRef;
		const BPK_ENTRY*	mEntryList;
		size_t				mEntryCount;
		FnIndexSorter(const PathList& listRef, const BPK_ENTRY* entryBase, size_t count)
			:mPathListRef(listRef)
			,mEntryList(entryBase)
			,mEntryCount(count)
		{
		}

		//compare/sort index by indexed entry's name
		bool operator()(const uint32& lhs, const uint32& rhs) const
		{
			assert( lhs < mEntryCount );
			assert( rhs < mEntryCount );
			const BPK_ENTRY& l = mEntryList[lhs];
			const BPK_ENTRY& r = mEntryList[rhs];
			assert( (size_t)l.mName < mPathListRef.size() );
			assert( (size_t)r.mName < mPathListRef.size() );
			return FnTStringFastLess::compare(mPathListRef[l.mName], mPathListRef[r.mName]);
		}
	};
	//////////////////////////////////////////////////////////////////////////
	inline bool			BPKData::addIndexToParent(BPK_ENTRY* parent, uint32 index)
	{
		assert(parent != NULL);
		this->addIndex(parent, index);
		assert(parent->mDir.mSize <= parent->mDir.mCapacitcy);

		//re-sort
		std::sort(parent->mDir.mIndices, parent->mDir.mIndices + parent->mDir.mSize, FnIndexSorter(mPathTable, mEntries, mEntryCount) );
		++mIndexCount;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	inline BPK_ENTRY*	BPKData::locateParent(const TString& path, TString& name, TString* prefix/* = NULL*/) const
	{
		if( path == TString::EMPTY )
			return NULL;

		TStringTokenizer tokenizer;
		tokenizer.tokenize(path, TEXT("/\\"), true);
		if( tokenizer.size() == 0 )
			return NULL;

		BPK_ENTRY* entry = &mEntries[0];

		//find parent node:
		for(size_t i = 0; i < tokenizer.size(); ++i)
		{
			if( tokenizer.size() == 1 || i == tokenizer.size() - 1 )
				break;

			const TString& subNodeName = tokenizer[i];
			if( entry->isDirectory() )
			{
				//find sub node by subNodeName
				entry = this->findSubEntry(entry, subNodeName);
				if( entry == NULL )
					return NULL;
			}
			else
				return NULL;
		}

		if( !entry->isDirectory() )
			return NULL;

		TString entryName = tokenizer[tokenizer.size() - 1];
		entryName.make_const_temp();
		name = entryName;

		if( prefix != NULL )
		{
			TStringConcat concat( tokenizer[0] );
			for(size_t i = 1; i < tokenizer.size()-1; ++i)
				concat += BTString("/")+ tokenizer[i];
			*prefix = concat;
		}

		return entry;
	}

	//////////////////////////////////////////////////////////////////////////
	inline BPK_ENTRY*	BPKData::getChild(BPK_ENTRY* parent, uint32 index) const
	{
		if( parent == NULL || !parent->isDirectory() || parent->mDir.mIndices == NULL)
		{
			assert(false);
			return NULL;
		}
		
		if( this->isUsingSharedIndices(parent) )
		{
			assert( parent->mDir.mIndices == &mIndices[parent->mDir.mIndex] );
			assert( parent->mDir.mIndex + parent->mDir.mSize <= mIndexCount );
			assert( index < parent->mDir.mSize );
		}
		else
			assert( index < parent->mDir.mSize );

		uint32 realIndex = parent->mDir.mIndices[ index ];
		if( realIndex < mEntryCount )
			return &mEntries[ realIndex ];
		else
		{
			assert(false);
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	inline BPK_ENTRY*	BPKData::findEntry(const TString& path) const
	{
		TString name;
		TString stdPath = TStringHelper::standardizePath(path);
		BPK_ENTRY* parent = this->locateParent(stdPath, name);
		if( parent == NULL )
		{
			if( stdPath == BTString("/") )
				return this->getRoot();
			else
				return NULL;
		}
		return this->findSubEntry(parent, name);
	}

	//////////////////////////////////////////////////////////////////////////
	inline bool	BPKData::removeEntry(const TString& destPath)
	{
		if( !mWritable )
		{
			assert(false);
			return false;
		}

		BPK_ENTRY* entry = this->findEntry(destPath);
		if( entry != NULL )
		{
			entry->mAttribute |= BPKA_DELETED;
			mDirty = true;
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	inline bool		BPKData::reserveEntries(uint32 entryCount)
	{
		if( this->isValid() )
		{
			if( entryCount < mEntryCount )
			{
				assert(false);
				return false;
			}

			BPK_ENTRY* cache = NULL;
			if( mCapacity < entryCount )
			{
				cache = BLADE_TMP_ALLOCT(BPK_ENTRY, mEntryCount);
				std::memcpy(cache, mEntries, (mEntryCount)*sizeof(BPK_ENTRY) );
				mCapacity = entryCount;
			}
			if( cache != NULL )
			{
				BLADE_RES_FREE(mEntries);
				mEntries = BLADE_RES_ALLOCT(BPK_ENTRY, mCapacity);
				std::memcpy(mEntries, cache, (mEntryCount)*sizeof(BPK_ENTRY) );
				BLADE_TMP_FREE(cache);
			}
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	inline bool			BPKData::addIndex(BPK_ENTRY* parent, uint32 index)
	{
		assert(parent != NULL && parent->isDirectory() );

		//use custom indices
		if( parent->mDir.mIndices == NULL )
		{
			parent->mDir.mCapacitcy = 32;
			parent->mDir.mSize = 0;
			parent->mDir.mIndices = BLADE_RES_ALLOCT(uint32, parent->mDir.mCapacitcy);
			assert(!this->isUsingSharedIndices(parent));
		}
		else if( this->isUsingSharedIndices(parent) )
		{
			//use custom indices
			assert(parent->mDir.mSize != 0);
			size_t capacity = parent->mDir.mSize*2;
			uint32* indices = BLADE_RES_ALLOCT(uint32, capacity);
			std::memcpy(indices, parent->mDir.mIndices, parent->mDir.mSize*sizeof(uint32) );

			parent->mDir.mIndices = indices;
			parent->mDir.mCapacitcy = (uint32)capacity;
			assert(!this->isUsingSharedIndices(parent));
		}
		else
		{
			assert(parent->mDir.mSize != 0);
			assert(parent->mDir.mSize <= parent->mDir.mCapacitcy);
			if( parent->mDir.mSize == parent->mDir.mCapacitcy )
			{
				uint32* old = parent->mDir.mIndices;
				uint32 newCap = parent->mDir.mCapacitcy + parent->mDir.mCapacitcy/2;
				parent->mDir.mIndices = BLADE_RES_ALLOCT(uint32, newCap);
				std::memcpy(parent->mDir.mIndices, old, parent->mDir.mSize*sizeof(uint32));
				BLADE_RES_FREE(old);
				parent->mDir.mCapacitcy = newCap;
			}
		}
		parent->mDir.mIndices[ parent->mDir.mSize ] = index;
		++parent->mDir.mSize;
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	inline BPK_ENTRY*	BPKData::addFolder(BPK_ENTRY* parent, const TString& name, uint32 attrib, const FILE_TIME& time, const HSTREAM& package, bool bWriteTail)
	{
		assert( parent != NULL );
		BPK_ENTRY* existing = this->findSubEntry(parent, name);
		if( existing != NULL )
		{
			//assert(false);
			return NULL;
		}

		if( mEntryCount >= mCapacity )
		{
			assert( mEntryCount == mCapacity );
			//note: entries maybe relocated, so parent changes
			indexdiff_t idx = parent - &mEntries[0];
			this->reserveEntries(mEntryCount+(mEntryCount+1)/2);
			parent = mEntries + idx;
		}
		assert( mEntryCount < mCapacity );

		index_t index = mEntryCount;
		++mEntryCount;

		//time, name, attrib:
		std::memset(&mEntries[index], 0, sizeof(BPK_ENTRY));
		mEntries[index].mLastTime = time;
		mPathTable.push_back(name);
		mEntries[index].mName = (uint32)mPathTable.size()-1;
		mEntries[index].mAttribute = attrib | BPKA_DIR;
		//directory content
		mEntries[index].mDir.mIndex = uint32(-1);
		mEntries[index].mDir.mSize = 0;
		mEntries[index].mDir.mIndices = NULL;

		bool ret = this->addIndexToParent(parent, (uint32)index);
		assert(ret);
		BLADE_UNREFERENCED(ret);

		mDirty = true;
		if(bWriteTail)
			this->writeTail(package);

		return &mEntries[index];
	}

	//////////////////////////////////////////////////////////////////////////
	inline BPK_ENTRY*	BPKData::addFile(BPK_ENTRY* parent, const TString& name, uint32 attrib, const FILE_TIME& time, const HSTREAM& fileData, const HSTREAM& package, bool bWriteTail)
	{
		assert( parent != NULL );
		assert( fileData != package );
		BPK_ENTRY* existing = this->findSubEntry(parent, name);
		if( existing != NULL )
		{
			assert(false);
			return NULL;
		}

		if( mEntryCount >= mCapacity )
		{
			assert( mEntryCount == mCapacity );
			indexdiff_t idx = parent - &mEntries[0];
			this->reserveEntries(mEntryCount+(mEntryCount+1)/2);
			parent = mEntries + idx;
		}
		size_t count = mEntryCount;
		assert( count < mCapacity );

		index_t index = count;
		++mEntryCount;

		//time, name, attrib:
		std::memset(&mEntries[index], 0, sizeof(BPK_ENTRY));
		mEntries[index].mLastTime = time;
		mPathTable.push_back(name);
		mEntries[index].mName = (uint32)mPathTable.size()-1;
		mEntries[index].mAttribute = attrib & (~BPKA_DIR);
		//file info
		mEntries[index].mFile.mSize = fileData->getSize();
		mEntries[index].mFile.mOffset = mTailOffset;

		mTailOffset += mEntries[index].mFile.mSize;

		BPKData::copyStream(fileData, package, 0, mEntries[index].mFile.mOffset, mEntries[index].mFile.mSize);
		bool ret = this->addIndexToParent(parent, (uint32)index);
		assert(ret);
		BLADE_UNREFERENCED(ret);

		mDirty = true;
		if(bWriteTail)
			this->writeTail(package);
		return &mEntries[index];
	}
	
}//namespace Blade

#endif //  __Blade_BPKData_h__
