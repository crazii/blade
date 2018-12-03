/********************************************************************
	created:	2014/08/21
	filename: 	ApkFileStream.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <file/android/ApkFileStream.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ApkFileStream::ApkFileStream(ApkFileEntry* entry, int apkFD, Lock* fileLock)
		:mEntry(entry)
		,mFD(apkFD)
		,mOffset(0)
		,mFileLock(fileLock)
	{
		assert( mEntry != NULL && mFD != -1);
	}

	//////////////////////////////////////////////////////////////////////////
	ApkFileStream::~ApkFileStream()
	{
		this->close();
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off ApkFileStream::seek(EStreamPos origin, Off off)
	{
		if( this->isValid() )
		{
			Off offset = 0;

			if( origin == SP_BEGIN )
				offset = off;
			else if( origin == SP_CURRENT )
				offset = this->tell() + off;
			else
				offset = this->getSize() + off;

			if( offset > this->getSize() )
			{
				assert(false);
				offset = this->getSize();
			}
			mOffset = offset;
			return offset;
		}
		else
			BLADE_EXCEPT(EXC_FILE_READ,BTString("cannot seek()\"") + this->getName() + TEXT("\".not opened."));
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off ApkFileStream::tell() const
	{
		return mOffset;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ApkFileStream::eof() const
	{
		return mOffset == this->getSize();
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size ApkFileStream::readData(void* destBuffer, Size size)
	{
		Off realOff = mEntry->mOffset + mOffset;
		//it's thread safe because ApkFileDevice opens duplicated fd on each opening stream
		Off off = ::lseek64(mFD, realOff, SEEK_SET);
		if( off == realOff )
		{
			Size retSize = ::read(mFD, destBuffer, size);
			mOffset += retSize;
			return retSize;
		}
		else
			return 0;
	}
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_ANDROID