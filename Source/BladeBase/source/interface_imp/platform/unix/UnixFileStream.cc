/********************************************************************
	created:	2014/07/10
	filename: 	UnixFileStream.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "UnixFileStream.h"

#if BLADE_IS_UNIX_CLASS_SYSTEM
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	UnixFileStream::UnixFileStream(const TString& name, int mode)
		:FileStream(name, mode)
		,mFileDescriptor(-1)
	{
		this->open(name, mode);
	}

	//////////////////////////////////////////////////////////////////////////
	UnixFileStream::~UnixFileStream()
	{
		this->close();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	UnixFileStream::isValid() const
	{
		return mFileDescriptor != -1;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	UnixFileStream::eof() const
	{
		assert( mFileDescriptor != -1 );
		int dummy = 0;
		return ::pread(mFileDescriptor, &dummy, sizeof(dummy), mOffset) == 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	UnixFileStream::openImpl(const TString& path, int mode)
	{
		if( mFileDescriptor == -1 )
		{
			int opflag = 0;
			if( (mode & AM_READOVERWRITE ) == AM_READOVERWRITE )
				opflag = O_RDWR | O_TRUNC | O_CREAT;
			else if( (mode & AM_OVERWRITE ) == AM_OVERWRITE )
				opflag = O_WRONLY | O_TRUNC | O_CREAT;
			else if( (mode& AM_WRITE) && (mode & AM_READ ) )
				opflag = O_RDWR | O_CREAT;
			else if( (mode & AM_WRITE ) )
				opflag = O_WRONLY | O_CREAT;
			else if( (mode & AM_READ ) )
				opflag |= O_RDONLY;

			TempBuffer buffer;
			const char* utf8Path = StringConverter::TStringToUTF8String(buffer, path);
			mFileDescriptor = ::open(utf8Path, opflag, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			//assert( mFileDescriptor != -1 );
			return mFileDescriptor != -1;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	UnixFileStream::closeImpl()
	{
		if( mFileDescriptor != -1 )
		{
			::close(mFileDescriptor);
			mFileDescriptor = -1;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	UnixFileStream::readDataImpl(void* destBuffer, Size size, Off absOff)
	{
		assert( mFileDescriptor != -1 );
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		Off off = ::lseek64(mFileDescriptor, absOff, SEEK_SET);
		if( off == absOff )
			return ::read(mFileDescriptor, destBuffer, size);
		else
			return 0;
#else
		return ::pread64(mFileDescriptor, destBuffer, size, absOff);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	UnixFileStream::writeDataImpl(const void* destBuffer, Size size, Off absOff)
	{
		assert( mFileDescriptor != -1 );
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		Off off = ::lseek64(mFileDescriptor, absOff, SEEK_SET);
		if( off == absOff )
			return (IStream::Size)::write(mFileDescriptor, destBuffer, size);
		else
			return 0;
#else
		return (IStream::Size)::pwrite64(mFileDescriptor, destBuffer, size, absOff);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void	UnixFileStream::flushImpl()
	{
		assert( mFileDescriptor != -1 );
		::fsync(mFileDescriptor);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	UnixFileStream::truncateImpl(Size size)
	{
		assert( mFileDescriptor != -1 );
		return ::ftruncate(mFileDescriptor, size) == 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	UnixFileStream::updateFileSize() const
	{
		assert( mFileDescriptor != -1 );
		off64_t off = ::lseek64(mFileDescriptor, 0, SEEK_END);
		assert(off != -1);
		return off;
	}
}//namespace Blade

#endif //BLADE_IS_UNIX_CLASS_SYSTEM