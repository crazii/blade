/********************************************************************
	created:	2014/02/09
	filename: 	BPKFile.h
	author:		Crazii
	purpose:	Blade PacKage file
*********************************************************************/
#ifndef __Blade_BPKFile_h__
#define __Blade_BPKFile_h__
#include <memory/BladeMemory.h>
#include <interface/public/file/IFileDevice.h>
#include <interface/public/file/IStream.h>
#include <interface/public/time/Time.h>
#include <utility/TList.h>

namespace Blade
{
	/** @brief  */
	typedef enum EBPKAttribute
	{
		BPKA_FILE		= 0x0000,
		BPKA_DIR		= 0x0001,
		BPKA_READONLY	= 0x0002,
		BPKA_DELETED	= 0x0004,
	}BPK_ATTR;

	/** @brief entry of a directory or file */
	typedef struct SBPKEntry
	{
		///file info
		struct SFileDesc
		{
			IStream::Off	mOffset;
			IStream::Size	mSize;
		};

		///directory info
		struct SDirDesc
		{
			uint32		mSize;
			union
			{
				uint32		mIndex;		//start index sub entries indices array
				uint32		mCapacitcy;
			};
			uint32*		mIndices;		//runtime data: this is not a good way to save data of platform dependent size
										//fortunately it's in a union and we can make sure of the consistent size
		};

		uint32		mName;			//index to name table
		uint32		mAttribute;		//BPK_ATTR
		union
		{
			///for files
			SFileDesc mFile;
			///for directories
			SDirDesc mDir;
		};
		FILE_TIME	mLastTime;		//last modified time

		/** @brief  */
		inline bool isDirectory() const	{return (mAttribute&BPKA_DIR) != 0;}
		/** @brief  */
		inline bool	isReadOnly() const	{return (mAttribute&BPKA_READONLY) != 0;}
		/** @brief  */
		inline bool	isDeleted() const	{return (mAttribute&BPKA_DELETED) != 0;}
	}BPK_ENTRY;
	//ensure union in consistent size
	static_assert( sizeof(BPK_ENTRY::SFileDesc)>=sizeof(BPK_ENTRY::SDirDesc), "inconsistent size for serialization");

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	/** @brief  */
	class IBPKCallback
	{
	public:
		/** @brief  */
		virtual void onProgress(const TString& name, scalar percent) = 0;
	};//class IBPKCallback
	class IBPKFilter
	{
	public:
		/** @brief return true if filter passed and file will be packed */
		virtual bool filter(const TString& file) { BLADE_UNREFERENCED(file);return true; }
	};

	typedef TPointerParam<BPK_ENTRY>	BPKEntryList;

	namespace Impl
	{
		class BPKData;
	}//namespace Impl

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class BLADE_BASE_API BPKFile : public Allocatable
	{
	public:
		BPKFile();
		~BPKFile();
		
		//max opening file count
		static const uint MAX_PARALLEL_COUNT = 4;

		struct Stream;
		typedef struct Stream FILE;

		/************************************************************************/
		/* package manipulations(NOT thread safe)                                                                     */
		/************************************************************************/

		/** @brief open native file */
		bool	openFile(const HSTREAM& package);

		/** @brief open stream as BPK file. used for embedded mode(package in another package) */
		/* @remark count must be MAX_PARALLEL_COUNT.
		for write mode, stream[0] should not be NULL and the left streams must be NULL,
		for read-only mode, all streams in array should be valid */
		bool	openFile(const HSTREAM* stream, size_t count);

		/** @brief  */
		bool	closeFile();

		/** @brief  */
		bool	isOpen() const;

		/** @brief write BPK file. file must be writable */
		bool	writeFile();

		/** @brief erase all content and write BPK file */
		bool	refillFile(const HFILEDEVICE& folder, IBPKFilter* flt = NULL, IBPKCallback* callback = NULL);

		/** @brief add a new abstract folder to internal data and write BPK file */
		bool	addEntry(const HFILEDEVICE& path, const TString& destPath, IBPKFilter* flt = NULL, IBPKCallback* callback = NULL);

		/** @brief add a new file to internal data and write BPK file */
		/* @param overwrite: overwrite exist file */
		BPK_ENTRY*addEntry(const HSTREAM& stream, uint32 attrib, const TString& destPath, bool overwrite = true, IBPKFilter* flt = NULL, IBPKCallback* callback = NULL);

		/** @brief remove internal file/folder (mark entry as deleted) */
		bool	removeEntry(const TString& destPath);

		/** @brief erase all content directly */
		bool	eraseAll();

		/** @brief count waste space, return total waste bytes & deleted entries(file & dirs), sorted by file offset */
		bool	countDeletedSpace(IStream::Size& bytes, BPKEntryList& entryList) const;

		/** @brief remove (deleted file) gaps between files to de-fragment */
		/* @remarks: deleted files are marked as delete and not actually removed from package,
		and the delete file data cannot be re-use,
		that will cause a large number of deleted files with huge waste along with time.
		when updating package with huge changes, it's better to delete all files first and perform re-organization, then add new files  */
		bool	reorganizeFile(IStream::Size threshold);

		/** @brief find entries with path & wildcard (NO recursion) * */
		/* @note: wildcard will be split into path & name in prefix & suffix */
		void	findEntries(const TString& wildcard, BPKEntryList& entries) const;

		/** @brief find entry with path. DO NOT delete returned value */
		BPK_ENTRY*	findEntry(const TString& path) const;

		/** @brief  */
		const tchar*getEntryName(const BPK_ENTRY* entry) const;

		/** @brief create internal stream via file table entry */
		/* @note: use fclose to close returned value  */
		FILE*		entryToStream(BPK_ENTRY* entry) const;

		/** @brief get BPK file path */
		const TString&	getBPKName() const;

		/************************************************************************/
		/* I/O operations (thread safe)                                                                     */
		/************************************************************************/

		/** @brief read-only open, use fclose to close opened file, un-closed stream will cause memory leak & dead lock  */
		FILE*	fopen(const TString& path) const;

		/** @brief  */
		int		fclose(FILE* stream) const;

		/** @brief  */
		int		fseek(FILE* stream, IStream::Off offset, IStream::EStreamPos origin) const;

		/** @brief  */
		IStream::Off ftell(FILE* stream) const;

		/** @brief  */
		size_t fread(void* ptr, size_t size, size_t count, FILE* stream) const;

		/** @brief  */
		int		feof(FILE* stream) const;

		/** @brief  */
		int		ferror(FILE* stream) const;

		/** @brief */
		void	clearerr(FILE* stream) const;

		/************************************************************************/
		/* none std functions                                                                     */
		/************************************************************************/

		/** @brief stream size */
		IStream::Size fsize(FILE* stream) const;

		/** @brief  */
		IStream::Size fread(void* ptr, IStream::Size bytes, FILE* stream) const;

	private:
		Pimpl<Impl::BPKData>	mData;
	protected:

		/** @brief  */
		index_t		fetchStream() const;

		HSTREAM			mStream[MAX_PARALLEL_COUNT];
		mutable Lock	mStreamLock[MAX_PARALLEL_COUNT];	//parallel lock for fopen/fclose
	};//class BPKFile
	
}//namespace Blade

#endif //  __Blade_BPKFile_h__