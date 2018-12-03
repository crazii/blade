/********************************************************************
	created:	2014/07/10
	filename: 	WindowsFileStream.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "WindowsFileStream.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	WindowsFileStream::WindowsFileStream(const TString& name, int mode)
		:FileStream(name, mode)
		,mFileHandle(INVALID_HANDLE_VALUE)
		,mReadEvent(INVALID_HANDLE_VALUE)
		,mWriteEvent(INVALID_HANDLE_VALUE)
	{
		this->open(name, mode);
	}

	//////////////////////////////////////////////////////////////////////////
	WindowsFileStream::~WindowsFileStream()
	{
		this->close();
	}

	//////////////////////////////////////////////////////////////////////////
	bool	WindowsFileStream::isValid() const
	{
		BY_HANDLE_FILE_INFORMATION info;
		return mFileHandle != INVALID_HANDLE_VALUE && ::GetFileInformationByHandle(mFileHandle,&info) == TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	WindowsFileStream::eof() const
	{
		assert(mFileHandle != INVALID_HANDLE_VALUE);

		OVERLAPPED overlapped = {0};
		overlapped.Offset = (DWORD)(mOffset&0xFFFFFFFF);
		overlapped.OffsetHigh = (DWORD)( mOffset >> 32 );
		overlapped.hEvent = mReadEvent;
		BOOL bResult = FALSE;
		DWORD dwError = 0;


		::WaitForSingleObject(mReadEvent, INFINITE);
		::ResetEvent(mReadEvent);
		if( (mAccessMode&AM_WRITE) )
		{
			overlapped.hEvent = mWriteEvent;
			assert(mWriteEvent != INVALID_HANDLE_VALUE);
			::WaitForSingleObject(mWriteEvent, INFINITE);
		}

		DWORD data;
		DWORD readBytes;
		bResult = ::ReadFile(mFileHandle, &data, sizeof(data), NULL, &overlapped);
		dwError = ::GetLastError();
		if(!bResult && dwError == ERROR_IO_PENDING)
		{ 
			//Wait until the I/O is complete, that is: the overlapped event is signaled.
			bResult = ::GetOverlappedResult(mFileHandle, &overlapped, &readBytes, TRUE);
			dwError = ::GetLastError();
		}
		if( dwError == 0 )
			return false;
		else if(dwError == ERROR_HANDLE_EOF)
			return true;
		else
			BLADE_EXCEPT(EXC_API_ERROR, BTString("read file error."));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	WindowsFileStream::openImpl(const TString& path, int mode)
	{
		if( mFileHandle == INVALID_HANDLE_VALUE )
		{
			DWORD dwDesiredAccess = 0;
			DWORD dwShareMode = 0;
			DWORD dwCreationDisposition = 0;
			
			if( (mode&AM_READ) )
			{
				dwDesiredAccess |= GENERIC_READ;
				dwShareMode |= FILE_SHARE_READ;
				dwCreationDisposition = OPEN_EXISTING;
			}

			if( (mode&AM_WRITE) )
			{
				dwDesiredAccess |= GENERIC_WRITE;
				dwShareMode |= FILE_SHARE_WRITE;
				dwCreationDisposition = OPEN_ALWAYS;
			}	

			assert(dwCreationDisposition != 0);

			if( (mode&AM_OVERWRITE) == AM_OVERWRITE )
				dwCreationDisposition = TRUNCATE_EXISTING;

			mFileHandle = ::CreateFile(path.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_FLAG_OVERLAPPED/*|FILE_FLAG_NO_BUFFERING*/, NULL);

			DWORD dwError = ::GetLastError();
			if( dwError == ERROR_FILE_NOT_FOUND && (dwCreationDisposition == TRUNCATE_EXISTING) )
			{
				dwCreationDisposition = OPEN_ALWAYS;
				mFileHandle = ::CreateFile(path.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_FLAG_OVERLAPPED/*|FILE_FLAG_NO_BUFFERING*/, NULL);
			}
			bool ret = mFileHandle != INVALID_HANDLE_VALUE;
			if( ret )
			{
				if( (mode&AM_READ) )
					mReadEvent = ::CreateEvent(NULL, TRUE, TRUE, NULL);
				if( (mode&AM_WRITE) )
					mWriteEvent = ::CreateEvent(NULL, TRUE, TRUE, NULL);
			}
			return ret;
		}
		else
		{
			assert(false);
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	WindowsFileStream::closeImpl()
	{
		if(mFileHandle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}

		if( mReadEvent != INVALID_HANDLE_VALUE )
		{
			::CloseHandle(mReadEvent);
			mReadEvent = INVALID_HANDLE_VALUE;
		}

		if( mWriteEvent != INVALID_HANDLE_VALUE )
		{
			::CloseHandle(mWriteEvent);
			mWriteEvent = INVALID_HANDLE_VALUE;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	WindowsFileStream::readDataImpl(void* destBuffer, Size size, Off absOff)
	{
		assert(mFileHandle != INVALID_HANDLE_VALUE);
		assert( size <= DWORD(-1) && "overflow");

		OVERLAPPED overlapped = {0};
		overlapped.Offset = (DWORD)(absOff&0xFFFFFFFF);
		overlapped.OffsetHigh = (DWORD)(absOff >> 32);
		overlapped.hEvent = mReadEvent;
		BOOL bResult = FALSE;
		DWORD dwError = 0;

		::WaitForSingleObject(mReadEvent, INFINITE);
		::ResetEvent(mReadEvent);
		if( (mAccessMode&AM_WRITE) )
		{
			assert(mWriteEvent != INVALID_HANDLE_VALUE);
			::WaitForSingleObject(mWriteEvent, INFINITE);
		}

		DWORD readBytes = 0;
		bResult = ::ReadFile(mFileHandle, destBuffer, (DWORD)size, NULL, &overlapped);
		dwError = ::GetLastError();
		if(!bResult || dwError == ERROR_IO_PENDING) 
		{
			bResult = ::GetOverlappedResult(mFileHandle, &overlapped, &readBytes, TRUE);
			dwError = ::GetLastError();
		}
		else
			return size;

		if( !bResult )
			BLADE_EXCEPT(EXC_FILE_READ, BTString("read file error."));
		return (Size)readBytes;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	WindowsFileStream::writeDataImpl(const void* srcBuffer, Size size, Off absOff)
	{
		assert(mFileHandle != INVALID_HANDLE_VALUE);
		assert( size <= DWORD(-1) && "overflow");

		OVERLAPPED overlapped = {0};
		overlapped.Offset = (DWORD)(absOff&0xFFFFFFFF);
		overlapped.OffsetHigh = (DWORD)(absOff >> 32);
		overlapped.hEvent = mWriteEvent;
		BOOL bResult = FALSE;
		DWORD dwError = 0;

		::WaitForSingleObject(mWriteEvent, INFINITE);
		::ResetEvent(mWriteEvent);
		if( (mAccessMode&AM_READ) )
		{
			assert(mReadEvent != INVALID_HANDLE_VALUE);
			::WaitForSingleObject(mReadEvent, INFINITE);
		}

		DWORD writtenBytes = 0;
		bResult = ::WriteFile(mFileHandle, srcBuffer, (DWORD)size, NULL, &overlapped);
		dwError = ::GetLastError();
		if(!bResult || dwError == ERROR_IO_PENDING)
		{
			bResult = ::GetOverlappedResult(mFileHandle, &overlapped, &writtenBytes, TRUE);
			dwError = ::GetLastError();
		}
		else
			return size;

		if( !bResult)
			BLADE_EXCEPT(EXC_FILE_WRITE, BTString("write file error."));
		return (Size)writtenBytes;
	}

	//////////////////////////////////////////////////////////////////////////
	void	WindowsFileStream::flushImpl()
	{
		assert( mFileHandle != INVALID_HANDLE_VALUE );
		BOOL ret = ::FlushFileBuffers(mFileHandle);
		if( !ret )
			BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to flush file."));
	}

	//////////////////////////////////////////////////////////////////////////
	bool	WindowsFileStream::truncateImpl(Size size)
	{
		assert( mFileHandle != INVALID_HANDLE_VALUE );

		LONG m = (LONG)size;
		LONG hm = (LONG)(size >> 32);
		BOOL ret = ::SetFilePointer(mFileHandle, m, &hm, FILE_BEGIN) != INVALID_SET_FILE_POINTER || ::GetLastError() == NO_ERROR;
		assert(ret);
		ret = ret && ::SetEndOfFile(mFileHandle);
		if( ret )
			mSize = size;
		return ret != 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size	WindowsFileStream::updateFileSize() const
	{
		assert(mFileHandle != INVALID_HANDLE_VALUE);
		LONG high = 0;
		DWORD low = ::SetFilePointer(mFileHandle, high, NULL, FILE_END);
		if ( low == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR )
			BLADE_EXCEPT(EXC_API_ERROR, BTString("unable to set seek pointer."));
		return Size( ((Size)high<<32) | (Size)low );
	}

}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM