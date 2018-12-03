/********************************************************************
	created:	2010/03/24
	filename: 	FileStream.h
	author:		Crazii
	
	purpose:	default file stream implementation
*********************************************************************/
#ifndef __Blade_FileStream_h__
#define __Blade_FileStream_h__

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning( disable : 4996 )
#endif
#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>
#include <interface/public/file/IStream.h>
#include <utility/String.h>
#include <utility/IOBuffer.h>

namespace Blade
{
	class FileStream : public IStream
	{
	public:
		FileStream(const TString& name, int mode);
		virtual ~FileStream();

		virtual const TString& getName();

		inline virtual void	close()
		{
			this->flush();
			this->closeImpl();
			this->freeBuffer();
		}

		virtual Off		seek(EStreamPos origin,Off off);

		virtual Off		tell() const;

		virtual bool	eof() const = 0;

		virtual Size	readData(void* destBuffer,Size size);

		virtual Size	writeData(const void* srcBuffer,Size size);

		virtual Size	getSize() const;

		virtual int		getAccesMode() const;

		virtual void	flush(bool hardFlush = false);

		virtual bool	truncate(Size size)
		{
			this->flush();
			return this->truncateImpl(size);
		}

	protected:
		inline bool		open(const TString& path, int mode)
		{
			bool ret = this->openImpl(path, mode);
			if( ret )
			{
				mSize = this->updateFileSize();
				this->prepareBuffer();
			}
			return ret;
		}


		virtual bool	openImpl(const TString& path, int mode) = 0;

		virtual void	closeImpl() = 0;

		virtual Size	readDataImpl(void* destBuffer, Size size, Off absOff) = 0;

		virtual Size	writeDataImpl(const void* srcBuffer, Size size, Off absOff) = 0;

		virtual Size	updateFileSize() const = 0;

		virtual void	flushImpl() = 0;

		virtual bool	truncateImpl(Size size) = 0;

		void			prepareBuffer();
		void			freeBuffer();

		TString					mFileNamePath;
		mutable Size			mSize;
		mutable Off				mOffset;
		int						mAccessMode;
		IOBuffer				mBuffer;
		bool					mWriteDirty;	//indicates mBuffer is dirty for write and need flush
	};
	
}//namespace Blade


#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	include "windows/WindowsFileStream.h"
#elif BLADE_IS_UNIX_CLASS_SYSTEM
#	include "unix/UnixFileStream.h"
#else
#	error un-implemented platform.
#endif

#endif //__Blade_FileStream_h__