/********************************************************************
	created:	2014/01/13
	filename: 	ZipFileDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ZipFileDevice.h"
#if BLADE_ENABLE_ZIPSTREAM

namespace Blade
{
	const TString ZipFileDevice::ZIP_FILE_EXT = BTString("zip");
	const TString ZipFileDevice::ZIP_FILE_TYPE = BTString("Zip Archive");


	//////////////////////////////////////////////////////////////////////////
	static void* blade_zzip_malloc(size_t bytes, void* data)
	{
		return ((IPool*)data)->allocate(bytes);
	}
	//////////////////////////////////////////////////////////////////////////
	static void blade_zzip_free(void* ptr, void* data)
	{
		((IPool*)data)->deallocate(ptr);
	}

	static zzip_plugin_mem_handlers blade_zzip_plugin_mem = 
	{
		blade_zzip_malloc,
		blade_zzip_free,
		Memory::getResourcePool(),
		Memory::getTemporaryPool(),
	};

	//////////////////////////////////////////////////////////////////////////
	ZipFileDevice::ZipFileDevice()
		:mZZipDir(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ZipFileDevice::~ZipFileDevice()
	{
		this->unload();
	}

	/************************************************************************/
	/* IDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ZipFileDevice::open()
	{
		this->unload();
		if( mPathName == TString::EMPTY )
			return false;

		ScopedLock lock(mLock);
		TempBuffer buffer;
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		const char* path = StringConverter::TStringToString(buffer, mPathName);
#else
		const char* path = StringConverter::TStringToUTF8String(buffer, mPathName);
#endif

		zzip_error_t zzipError;
		mZZipDir = ::zzip_dir_open_ext_io(path, &zzipError, 0, NULL, &blade_zzip_plugin_mem);
		this->checkError(zzipError, BLADE_TFUNCTION );

		ZZIP_DIRENT zzipEntry;
		size_t entryCount = 0;
		while ( ::zzip_dir_read(mZZipDir, &zzipEntry) )
			++entryCount;
		::zzip_rewinddir(mZZipDir);

		while ( ::zzip_dir_read(mZZipDir, &zzipEntry) )
			mFiles.insert( StringConverter::StringToTString(zzipEntry.d_name) );

		::zzip_rewinddir(mZZipDir);

		return zzipError == ZZIP_NO_ERROR;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ZipFileDevice::isOpen() const
	{
		return mZZipDir != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ZipFileDevice::close()
	{
		ScopedLock lock(mLock);
		if (mZZipDir != NULL)
		{
			mPathName = TString::EMPTY;
			::zzip_dir_close(mZZipDir);
			mZZipDir = NULL;
			mFiles.clear();
		}
	}

	/************************************************************************/
	/* IFileDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString&	ZipFileDevice::getType() const noexcept
	{
		return ZIP_FILE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ZipFileDevice::isReadOnly() const noexcept
	{
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ZipFileDevice::existFile(const TString& filepathname) const noexcept
	{
		ScopedLock lock(mLock);
		//standard path to a file/folder doesn't end with '/'
		if( !TStringHelper::isEndWith(filepathname, BTString("/")) && !TStringHelper::isEndWith(filepathname, BTString("\\")) )
			return mFiles.find( filepathname ) != mFiles.end();
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			ZipFileDevice::openFile(const TString& filepathname, IStream::EAccessMode mode/* = IStream::AM_READ*/) const noexcept
	{
		HSTREAM stream;
		//only support binary read mode
		if( !(mode&IStream::AM_READ) || (mode&IStream::AM_OVERWRITE) || filepathname == TString::EMPTY )
			return stream;

		//early out: zzip_file_open will traverse all files in linear order (link list)
		if( !this->existFile(filepathname) )
			return stream;

		ScopedLock lock(mLock);

		TempBuffer buffer;
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
		const char* path = StringConverter::TStringToString(buffer, filepathname);
#else
		const char* path = StringConverter::TStringToUTF8String(buffer, filepathname);
#endif
		ZZIP_FILE* zzipFile = ::zzip_file_open(mZZipDir, path, ZZIP_ONLYZIP | ZZIP_CASELESS);

		if ( zzipFile == NULL )
		{
			BLADE_LOG(Warning, BTString("unable to open file stream:") << filepathname);
			return stream;
		}

		ZZIP_STAT zstat;
		::zzip_dir_stat(mZZipDir, path, &zstat, ZZIP_CASEINSENSITIVE);
		IStream::Size decomressedBytes = static_cast<IStream::Size>(zstat.st_size);

		stream.bind( BLADE_NEW ZipStream(filepathname, zzipFile, mode, decomressedBytes) );
		return stream;
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			ZipFileDevice::createFile(const TString& /*filepathname*/, IStream::EAccessMode /*mode*/) noexcept
	{
		return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ZipFileDevice::deleteFile(const TString& /*filepathname*/) noexcept
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ZipFileDevice::findFile(TStringParam& result, const TString& pattern, int findFlag/* = FF_DIR|FF_FILE*/)
	{
		if( (findFlag&(FF_DIR|FF_FILE)) == 0 )
			return;

#define IS_RELATIVE_DIR(dir) ( (dir)[0] == '.' && (dir)[1] == '.' && (dir)[2] == '\0' || (dir)[0] == '.' && (dir)[1] == '\0' )

		index_t pos1 = pattern.rfind (TEXT('/'));
		index_t pos2 = pattern.rfind (TEXT('\\'));
		if (pos1 == pattern.npos || ((pos2 != pattern.npos) && (pos1 < pos2)))
			pos1 = pos2;
		bool hasPrefix = (pos1 != pattern.npos);

		TString directory;
		TString	subPattern;
		if(hasPrefix)
		{
			directory = pattern.substr(0, pos1 + 1);
			subPattern = pattern.substr(pos1 + 1);
			assert( directory.find(TEXT('*') ) == TString::npos );
			//standardize path
			directory = TStringHelper::standardizePath(directory);
		}
		else
			subPattern = pattern;

		for(FileList::const_iterator i = mFiles.begin(); i != mFiles.end(); ++i)
		{
			const TString& entry = *i;
			bool dirtype = TStringHelper::isEndWith(entry, BTString("/") );
			if( dirtype && !(findFlag&FF_DIR) || !dirtype && !(findFlag&FF_FILE) )
				continue;

			size_t pos;
			if( dirtype )
				pos = entry.find_last_of(TEXT('/'), entry.size()-1 );
			else
				pos = entry.find_last_of(TEXT('/'));

			TString prefix = entry.substr_nocopy(0, pos);
			TString entryName = entry.substr_nocopy(pos+1);

			if( (!(findFlag&FF_RECURSIVE) && (prefix == directory))
				|| ((findFlag&FF_RECURSIVE) && TStringHelper::isStartWith(prefix, directory) )
				)
			{
				if( TStringHelper::wildcardMatch(subPattern,entryName) )
				{
					if( dirtype )
						//remove the last slash '/'(id of folder) in zip
						result.push_back(entry.substr(0, entry.size()-1) );
					else
						result.push_back(entry);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ZipFileDevice::createDirectory(const TString& /*name*/, bool /*bRecursive = false*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ZipFileDevice::deleteDirectory(const TString& /*name*/)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			ZipFileDevice::existDirectory(const TString& name) const noexcept
	{
		ScopedLock lock(mLock);

		//Note: '/' suffix is an folder in zip
		// name/ is a folder
		// name is a file 

		//standard path to a file/folder doesn't end with '/'
		if( !TStringHelper::isEndWith(name, BTString("/")) && !TStringHelper::isEndWith(name, BTString("\\")) )
			return mFiles.find( name + BTString("/") ) != mFiles.end();
		else
			return false;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void		ZipFileDevice::checkError(int zzipError, const TString& source)
	{
		switch (zzipError)
		{
		case ZZIP_NO_ERROR:
			break;
		case ZZIP_OUTOFMEM:
			BLADE_EXCEPT_SOURCE(EXC_OUT_OF_MEMORY, BTString("out of memory"), source);
			break;            
		case ZZIP_DIR_OPEN:
		case ZZIP_DIR_STAT: 
		case ZZIP_DIR_SEEK:
		case ZZIP_DIR_READ:
			BLADE_EXCEPT_SOURCE(EXC_FILE_READ, BTString("Unable to read zip file."), source);
			break;            
		case ZZIP_UNSUPP_COMPR:
			BLADE_EXCEPT_SOURCE(EXC_FILE_READ, BTString("Unsupported compression format."), source);
			break;            
		case ZZIP_CORRUPTED:
			BLADE_EXCEPT_SOURCE(EXC_FILE_READ, BTString("Corrupted archive."), source);
			break;            
		default:
			BLADE_EXCEPT_SOURCE(EXC_UNKNOWN, BTString("unable to read archive."), source);
			break;            
		};
	}
	
}//namespace Blade
#endif//BLADE_ENABLE_ZIPFILE