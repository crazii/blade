/********************************************************************
	created:	2014/01/13
	filename: 	ZipStream.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ZipStream.h"
#if BLADE_ENABLE_ZIPFILE

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ZipStream::ZipStream(const TString& name, ZZIP_FILE* zipFile, IStream::EAccessMode mode, IStream::Size decompressSize)
		:mName(name)
		,mZipFile(zipFile)
		,mDecompressSize(decompressSize)
		,mMode(mode)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ZipStream::~ZipStream()
	{
		this->close();
	}

	/************************************************************************/
	/* IStream interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString& ZipStream::getName()
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ZipStream::isValid() const
	{
		return mZipFile != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void		ZipStream::close()
	{
		if (mZipFile != NULL)
		{
			::zzip_file_close(mZipFile);
			mZipFile = NULL;
		}

		mName = TString::EMPTY;
		mDecompressSize = 0;
		mMode = IStream::AM_UNDEFINED;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off	ZipStream::seek(EStreamPos origin,Off off)
	{
		int dir = origin == SP_CURRENT ? SEEK_CUR : (origin ==SP_BEGIN ? SEEK_SET : SEEK_END );
		return ::zzip_seek(mZipFile, (zzip_off_t)off, dir);
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off	ZipStream::tell() const
	{
		zzip_off_t pos = ::zzip_tell(mZipFile);
		return pos;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ZipStream::eof() const
	{
		return (this->ZipStream::tell() >= this->ZipStream::getSize() );
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	ZipStream::readData(void* destBuffer,Size size)
	{
		zzip_ssize_t ret = ::zzip_file_read(mZipFile, (char*)destBuffer, (zzip_size_t)size);
		if (ret < 0 )
		{
			ZZIP_DIR *dir = ::zzip_dirhandle(mZipFile);
			TString msg = StringConverter::StringToTString( ::zzip_strerror_of(dir) );
			BLADE_EXCEPT(EXC_FILE_READ, msg);
		}
		return (Size)ret;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	ZipStream::writeData(const void* /*srcBuffer*/,Size /*size*/)
	{
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	ZipStream::getSize() const
	{
		return mDecompressSize;
	}

	//////////////////////////////////////////////////////////////////////////
	int			ZipStream::getAccesMode() const
	{
		return mMode;
	}

	//////////////////////////////////////////////////////////////////////////
	void		ZipStream::flush(bool /*hardFlush = false*/)
	{
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ZipStream::truncate(Size /*size*/)
	{
		return false;
	}
	
}//namespace Blade
#endif//BLADE_ENABLE_ZIPFILE
