/********************************************************************
	created:	2014/07/10
	filename: 	UnixFileStream.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_UnixFileStream_h__
#define __Blade_UnixFileStream_h__
#include <BladePlatform.h>

#if BLADE_IS_UNIX_CLASS_SYSTEM
#include "../FileStream.h"

namespace Blade
{
	class UnixFileStream : public FileStream, public Allocatable
	{
	public:
		UnixFileStream(const TString& name, int mode);
		~UnixFileStream();

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
		
		int mFileDescriptor;
	};

	typedef UnixFileStream DefaultFileStream;
	
}//namespace Blade
#endif//BLADE_IS_UNIX_CLASS_SYSTEM

#endif // __Blade_UnixFileStream_h__
