/********************************************************************
	created:	2014/01/13
	filename: 	ZipFileDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ZipFileDevice_h__
#define __Blade_ZipFileDevice_h__
#include "ZipStream.h"
#if BLADE_ENABLE_ZIPFILE
#include <interface/public/file/IFileDevice.h>
#include <utility/BladeContainer.h>

#include <zzip/zzip.h>
#include <zzip/plugin.h>

namespace Blade
{

	class ZipFileDevice : public IFileDevice, public Allocatable
	{
	public:
		static const TString ZIP_FILE_EXT;
		static const TString ZIP_FILE_TYPE;
	public:
		ZipFileDevice();
		~ZipFileDevice();

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
		/* IFileDevice interface                                                                     */
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
		virtual HSTREAM			openFile(const TString& filepathname,IStream::EAccessMode mode = IStream::AM_READ) const noexcept;

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
		virtual void			findFile(TStringParam& result,const TString& pattern, int findFlag = FF_DIR|FF_FILE);

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

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/

		/** @brief  */
		void		checkError(int zzipError, const TString& source);

		//note: sort in alpha order
		typedef Set<TString>	FileList;

		ZZIP_DIR*		mZZipDir;
		mutable Lock	mLock;
		FileList		mFiles;
	};//class ZipFileDevice
	
}//namespace Blade
#endif//BLADE_ENABLE_ZIPFILE

#endif // __Blade_ZipFileDevice_h__
