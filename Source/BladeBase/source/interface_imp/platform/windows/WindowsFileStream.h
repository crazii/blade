/********************************************************************
	created:	2014/07/10
	filename: 	WindowsFileStream.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_WindowsFileStream_h__
#define __Blade_WindowsFileStream_h__
#include <BladePlatform.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#include "../FileStream.h"

namespace Blade
{

	class WindowsFileStream : public FileStream, public Allocatable
	{
	public:
		WindowsFileStream(const TString& name, int mode);
		~WindowsFileStream();

		virtual bool	isValid() const;

		virtual bool	eof() const;

	protected:
		virtual bool	openImpl(const TString& path, int mode);

		virtual void	closeImpl();

		virtual Size	readDataImpl(void* destBuffer, Size size, Off absOff);

		virtual Size	writeDataImpl(const void* srcBuffer, Size size, Off absOff);

		virtual void	flushImpl();

		virtual bool	truncateImpl(Size size);

		virtual Size	updateFileSize() const;

		HANDLE mFileHandle;
		HANDLE mReadEvent;
		HANDLE mWriteEvent;
	};

	typedef WindowsFileStream DefaultFileStream;
	
}//namespace Blade

#endif//BLADE_IS_WINDOWS_CLASS_SYSTEM


#endif // __Blade_WindowsFileStream_h__