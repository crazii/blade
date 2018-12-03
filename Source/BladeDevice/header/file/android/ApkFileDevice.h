/********************************************************************
	created:	2014/08/21
	filename: 	ApkFileDevice.h
	author:		Crazii
	
	purpose:	some platforms(i.e.) Amazon doesn't support APK expansion (OBB), 
				so reading from APK directly is needed.

				this implementation read APK files for Android, But have some constrains:
				APK is a common ZIP format, but files in APk should NOT be compressed.
				if files are compressed in APK, ApkFileDevice simply won't work.
	
				disable compression for resources is done at APK/custom_rules.xml
*********************************************************************/
#ifndef __Blade_ApkFileDevice_h__
#define __Blade_ApkFileDevice_h__
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#include <interface/public/file/IFileDevice.h>
#include <linux/limits.h>
#include "ApkFileStream.h"

#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>

namespace Blade
{
	class ApkFileDevice : public IFileDevice, public Allocatable
	{
	public:
		static const TString APK_FILE_TYPE;
		static const TString APK_FILE_EXT;
	public:
		ApkFileDevice();
		~ApkFileDevice();
		
		/************************************************************************/
		/* IDevice interface                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset()
		{
			return true;
		}

		/** @brief update data(I/O) */
		virtual bool	update()
		{
			return true;
		}

		/************************************************************************/
		/* IFileDevice interfaces                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const noexcept
		{
			return APK_FILE_TYPE;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReadOnly() const noexcept
		{
			return true;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			existFile(const TString& filepathname) const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			openFile(const TString& filepathname,IStream::EAccessMode mode = IStream::AM_READ) const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			createFile(const TString& /*filepathname*/, IStream::EAccessMode /*mode*/) noexcept
		{
			return HSTREAM::EMPTY;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteFile(const TString& /*filepathname*/) noexcept
		{
			return false;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			findFile(TStringParam& result, const TString& pattern, int findFlag = FF_DIR|FF_FILE);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			createDirectory(const TString& /*name*/, bool /*bRecursive = false*/)
		{
			return false;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteDirectory(const TString& /*name*/)
		{
			return false;
		}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			existDirectory(const TString& name) const noexcept
		{
			return false;
		}

	protected:

		static const int CONCURRENT_FILE_COUNT = 4;
		int				mFileDescriptors[CONCURRENT_FILE_COUNT];
		mutable Lock	mFileLock[CONCURRENT_FILE_COUNT];

		/** @brief  */
		int				findFreeIndexFD() const;

		ApkFileEntry*	mFiles;
		size_t			mCount;
		String			mUTF8Path;
	};
	
}//namespace Blade

#endif//BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#endif // __Blade_ApkFileDevice_h__


