/********************************************************************
	created:	2014/02/16
	filename: 	BPKFileDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BPKFileDevice.h"
#include "BPKStream.h"
#include <interface/IResourceManager.h>

namespace Blade
{
	const TString BPKFileDevice::BPK_FILE_EXT = BTString("bpk");
	const TString BPKFileDevice::BPK_FILE_TYPE = BTString("BPK Package");

	//////////////////////////////////////////////////////////////////////////
	BPKFileDevice::BPKFileDevice()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BPKFileDevice::~BPKFileDevice()
	{
	}

	/************************************************************************/
	/* IDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	BPKFileDevice::open()
	{
		this->unload();
		if( mPathName == TString::EMPTY )
			return false;

		HSTREAM stream[BPKFile::MAX_PARALLEL_COUNT];
		for(size_t i = 0; i < BPKFile::MAX_PARALLEL_COUNT; ++i)
		{
			stream[i] = IResourceManager::getSingleton().loadStream(mPathName, IStream::AM_READ);
			if( stream[i] == NULL )
				return false;
		}
		return mFile.openFile(stream, BPKFile::MAX_PARALLEL_COUNT);

	}

	//////////////////////////////////////////////////////////////////////////
	bool	BPKFileDevice::isOpen() const
	{
		return mFile.isOpen();
	}

	//////////////////////////////////////////////////////////////////////////
	void	BPKFileDevice::close()
	{
		mFile.closeFile();
	}

		
	/************************************************************************/
	/* IFileDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	BPKFileDevice::getType() const noexcept
	{
		return BPK_FILE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BPKFileDevice::isReadOnly() const noexcept
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BPKFileDevice::existFile(const TString& filepathname) const noexcept
	{
		BPK_ENTRY* entry = mFile.findEntry(filepathname);
		return entry != NULL && !entry->isDirectory();
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			BPKFileDevice::openFile(const TString& filepathname, IStream::EAccessMode mode/* = IStream::AM_READ*/) const noexcept
	{
		if( !(mode&IStream::AM_WRITE) && (mode&IStream::AM_READ) )
		{
			BPKFile::FILE* fp = mFile.fopen(filepathname);
			if( fp != NULL )
				return HSTREAM(BLADE_NEW BPKStream(mPathName + BTString("/") + filepathname, mode, &mFile, fp));
			return HSTREAM::EMPTY;
		}
		else
		{
			assert(false);
			return HSTREAM::EMPTY;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			BPKFileDevice::createFile(const TString& /*filepathname*/, IStream::EAccessMode /*mode*/) noexcept
	{
		return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BPKFileDevice::deleteFile(const TString& /*filepathname*/) noexcept
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			BPKFileDevice::findFile(TStringParam& result,const TString& pattern, int findFlag/* = FF_DIR|FF_FILE*/)
	{
		if( (findFlag&(FF_DIR|FF_FILE)) == 0 )
			return;

		BPKEntryList list;
		TString directory;
		TString fileWildcard;
		TStringHelper::getFilePathInfo(pattern, directory, fileWildcard);
		mFile.findEntries( directory + BTString("/*"), list);

		for(size_t i = 0; i < list.size(); ++i)
		{
			BPK_ENTRY* entry = list[i];
			bool dir = entry->isDirectory();
			const tchar* name = mFile.getEntryName(entry);
			TString fileName(name, Char_Traits<tchar>::length(name), TString::CONST_HINT);
			TStringConcat nodeFullPath = directory + BTString("/") + fileName; 

			if( (dir && (findFlag&FF_DIR)) || (!dir && (findFlag&FF_FILE) && TStringHelper::wildcardMatch(fileWildcard, fileName)) )
				result.push_back( nodeFullPath );

			if( dir && (findFlag&FF_RECURSIVE) )
				this->findFile(result, nodeFullPath + BTString("/") + fileWildcard, findFlag);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BPKFileDevice::createDirectory(const TString& /*name*/, bool /*bRecursive = false*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BPKFileDevice::deleteDirectory(const TString& /*name*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			BPKFileDevice::existDirectory(const TString& name) const noexcept
	{
		BPK_ENTRY* entry = mFile.findEntry(name);
		return entry != NULL && entry->isDirectory();
	}
	
}//namespace Blade