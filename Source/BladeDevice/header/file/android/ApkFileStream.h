/********************************************************************
	created:	2014/08/21
	filename: 	ApkFileStream.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ApkFileStream_h__
#define __Blade_ApkFileStream_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <interface/public/file/IStream.h>
#include <ctime>

#include <unistd.h>
#include <fcntl.h>

namespace Blade
{
	struct ApkFileEntry : public ResourceAllocatable
	{
		TString		mName;
		int64		mOffset;
		int64		mSize;
		std::time_t mCTime;
		std::time_t mATime;
		std::time_t mMTime;
	};

	class ApkFileStream : public IStream, public Allocatable
	{
	public:
		ApkFileStream(ApkFileEntry* entry, int apkFD, Lock* fileLock);
		~ApkFileStream();

		/************************************************************************/
		/* IStream interface                                                                     */
		/************************************************************************/
		/** @brief get the stream name: path of the stream */
		virtual const TString& getName()
		{
			return mEntry != NULL ? mEntry->mName : TString::EMPTY;
		}

		/** @brief  */
		virtual bool		isValid() const
		{
			return mEntry != NULL && mFD != -1;
		}

		/** @brief  */
		virtual void		close()
		{
			mEntry = NULL;
			mFD = -1;
			mOffset = 0;
			if( mFileLock != NULL )
			{
				mFileLock->unlock();
				mFileLock = NULL;
			}
		}

		/** @brief  */
		virtual Off			seek(EStreamPos origin, Off off);

		/** @brief  */
		virtual Off			tell() const;

		/** @brief  */
		virtual bool		eof() const;

		/** @brief  */
		virtual Size		readData(void* destBuffer,Size size);

		/** @brief  */
		virtual Size		writeData(const void* /*srcBuffer*/,Size /*size*/)	{return 0;}

		/** @brief  */
		virtual Size		getSize() const
		{
			return mEntry != NULL ? mEntry->mSize : 0;
		}

		/** @brief  */
		virtual int			getAccesMode() const				{return AM_READ;}

		/** @brief  */
		virtual void		flush(bool /*hardFlush = false*/)	{}

		/** @brief  */
		virtual bool		truncate(Size /*size*/)				{return false;}

	protected:
		ApkFileEntry*	mEntry;
		int				mFD;
		Off				mOffset;
		Lock*			mFileLock;
	};
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#endif // __Blade_ApkFileStream_h__
