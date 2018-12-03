/********************************************************************
	created:	2014/08/21
	filename: 	ApkFileDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <file/android/ApkFileDevice.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <android/native_activity.h>
#include <android/asset_manager.h>

namespace Blade
{
	const TString	ApkFileDevice::APK_FILE_TYPE = BTString("APK File");
	const TString	ApkFileDevice::APK_FILE_EXT = BTString("apk");

	//////////////////////////////////////////////////////////////////////////
	//note: qsort on TString is in-consistent(maybe dangerous if some changes applied to TString) but now is OK.
	static int compareApkFileEntry(const void* lhs, const void* rhs)
	{
		const ApkFileEntry* l = (const ApkFileEntry*)lhs;
		const ApkFileEntry* r = (const ApkFileEntry*)rhs;

		if( l->mName.size() < r->mName.size() )
			return -1;
		else if(l->mName.size() > r->mName.size())
			return 1;
		else
			return l->mName.compare(r->mName);
	}

	//////////////////////////////////////////////////////////////////////////
	ApkFileDevice::ApkFileDevice()
		:mFiles(NULL)
		,mCount(0)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	ApkFileDevice::~ApkFileDevice()
	{
		this->close();
	}

	/************************************************************************/
	/* IDevice interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	ApkFileDevice::open()
	{
		if( mPathName == TString::EMPTY )
			return false;

		this->close();

		ANativeActivity* activity = PlatformData::getSingleton().getPlatformAppData();
		if( activity == NULL )
		{
			assert(false);
			return false;
		}

		struct stat apkst;
		mUTF8Path = StringConverter::TStringToUTF8String(mPathName);
		int apkFd = ::open(mUTF8Path.c_str(), S_IRUSR);
		if( apkFd == -1 || ::fstat(apkFd, &apkst) != 0 )
			return false;
		mFileDescriptors[0] = apkFd;
		for(int i = 1; i < CONCURRENT_FILE_COUNT; ++i)
		{
			mFileDescriptors[i] = ::open(mUTF8Path.c_str(), S_IRUSR);
			assert( mFileDescriptors[i] != -1);
		}

		AAssetManager* manager = activity->assetManager;
		AAssetDir* dir = ::AAssetManager_openDir(manager, "");
		if( dir == NULL )
			return false;

		time_t curTime = std::time(NULL);

		//calculate file count and prepare data
		size_t count = 0;
		const char* file = ::AAssetDir_getNextFileName(dir);
		while( file != NULL ) {
			++count;
			file = ::AAssetDir_getNextFileName(dir);
		}
		mCount = count;
		mFiles = BLADE_NEW ApkFileEntry[count];

		//read file offset & size into file table
		::AAssetDir_rewind(dir);
		file = ::AAssetDir_getNextFileName(dir);
		size_t index = 0;
		while( file != NULL ){
			AAsset* asset = ::AAssetManager_open(manager, file, AASSET_MODE_UNKNOWN);
			int fd = -1;
			if( asset != NULL )
				fd = ::AAsset_openFileDescriptor64(asset, (off64_t*)&mFiles[index].mOffset, (off64_t*)&mFiles[index].mSize);

			mFiles[index].mName = StringConverter::UTF8StringToTString(file, ::strlen(file) );

			//compressed file cannot have fd, skip
			if( fd < 0 ) {
				BLADE_LOG(Warning, BString("compressed file found in APK:") + file );
				mFiles[index].mCTime = curTime;
				mFiles[index].mATime = curTime;
				mFiles[index].mMTime = curTime;
				mFiles[index].mOffset = apkst.st_size;
				mFiles[index].mSize = -1;
			} else {
				struct stat st;
				if( ::fstat(fd, &st) == 0 ) {
					//they must be the same physical file
					assert(st.st_dev == apkst.st_dev);
					assert(st.st_ino == apkst.st_ino);

					mFiles[index].mCTime = st.st_ctime;
					mFiles[index].mATime = st.st_atime;
					mFiles[index].mMTime = st.st_mtime;
				} else {
					mFiles[index].mCTime = curTime;
					mFiles[index].mATime = curTime;
					mFiles[index].mMTime = curTime;
					mFiles[index].mOffset = apkst.st_size;
					mFiles[index].mSize = -1;
				}
				//!!important: close fd
				::close(fd);
			}

			//release asset resources
			::AAsset_close(asset);
			file = ::AAssetDir_getNextFileName(dir);
			++index;
		}
		assert( index == mCount );
		::AAssetDir_close(dir);

		//sort for later binary search
		if( mFiles != NULL )
			std::qsort( mFiles, (uint)mCount, sizeof(ApkFileEntry) , &compareApkFileEntry);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	ApkFileDevice::isOpen() const
	{
		return mFiles != NULL && mCount != 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void	ApkFileDevice::close()
	{
		if( mFiles != NULL )
			BLADE_DELETE[] mFiles;
		mCount = 0;
		mUTF8Path = String::EMPTY;
	}

	/************************************************************************/
	/* IFileDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			ApkFileDevice::existFile(const TString& filepathname) const noexcept
	{
		ApkFileEntry entry;
		entry.mName = filepathname;
		ApkFileEntry* found = (ApkFileEntry*)std::bsearch(&entry, mFiles, mCount, sizeof(ApkFileEntry), &compareApkFileEntry);
		return found != NULL && found->mSize != -1;
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			ApkFileDevice::openFile(const TString& filepathname,IStream::EAccessMode mode/* = IStream::AM_READ*/) const noexcept
	{
		if( this->isOpen() )
		{
			ApkFileEntry entry;
			entry.mName = filepathname;
			ApkFileEntry* target = (ApkFileEntry*)std::bsearch(&entry, mFiles, mCount, sizeof(ApkFileEntry), &compareApkFileEntry);
			if( target != NULL && target->mSize != -1)
			{
				//Note: since pread64 is not available on Android,
				//we need now shared several multiple fds for concurrency
				//TODO: remove duplicated fd after Android NDK update.
				int index = this->findFreeIndexFD();
				assert( index < CONCURRENT_FILE_COUNT && mFileDescriptors[index] != -1);
				ApkFileStream* stream = BLADE_NEW ApkFileStream(target, mFileDescriptors[index], &mFileLock[index]);
				return HSTREAM(stream);
			}
		}

		assert(false);
		return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	void			ApkFileDevice::findFile(TStringParam& result, const TString& pattern, int /*findFlag*//* = FF_DIR|FF_FILE*/)
	{
		if( !this->isOpen() )
			return;

		// note: for simplicity, only one level of folder supported, no cascaded sub folders
		for(size_t i = 0; i < mCount; ++i)
		{
			if( TStringHelper::wildcardMatch(pattern, mFiles[i].mName) )
				result.push_back(mFiles[i].mName);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int				ApkFileDevice::findFreeIndexFD() const
	{
		for(int i = 0; i < CONCURRENT_FILE_COUNT; i = (i+1)%CONCURRENT_FILE_COUNT )
		{
			//note: lock will be freed by ApkFileStream
			//if there's dead lock, then it's probably ApkFileStream objects are leaking
			if( mFileLock[i].tryLock() )
				return i;
		}
		return 0;
	}
	
}//namespace Blade

#endif //BLADE_PLATFORM == BLADE_PLATFORM_ANDROID