/********************************************************************
	created:	2014/06/17
	filename: 	FileDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_FileDevice_h__
#define __Blade_FileDevice_h__
#include <interface/public/file/IFileDevice.h>
#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>

namespace Blade
{

	class FileDevice : public IFileDevice
	{
	public:
		FileDevice();
		virtual ~FileDevice();

		/************************************************************************/
		/* IDevice interfaces                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const;

		/** @brief close the device */
		virtual void	close();

		/** @brief reset the device */
		virtual bool	reset()			{return true;}

		/** @brief update data(I/O) */
		virtual bool	update()		{return true;}

		/************************************************************************/
		/* IFileDevice interfaces                                                                     */
		/************************************************************************/
		/*
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReadOnly() const noexcept;

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
		virtual HSTREAM			openFile(const TString& filepathname,IStream::EAccessMode mode = IStream::AM_READ ) const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			createFile(const TString& filepathname, IStream::EAccessMode mode) noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteFile(const TString& filepathname) noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			findFile(TStringParam& result,const TString& pattern, int findFlag = FF_DIR|FF_FILE) = 0;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			createDirectory(const TString& name, bool bRecursive = false);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteDirectory(const TString& name);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			existDirectory(const TString& name) const noexcept;

	protected:
		bool		mLoaded;
		bool		mReadOnly;
	};//class FileDevice
	
}//namespace Blade


#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	include "windows/WindowsFileDevice.h"
#elif BLADE_IS_UNIX_CLASS_SYSTEM
#	include "unix/UnixFileDevice.h"
#else
#	error un-implemented platform.
#endif


#endif // __Blade_FileDeviceBase_h__

