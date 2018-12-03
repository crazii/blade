/********************************************************************
	created:	2014/02/16
	filename: 	BPKFileDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BPKFileDevice_h__
#define __Blade_BPKFileDevice_h__
#include <interface/public/file/IFileDevice.h>
#include <utility/BPKFile.h>

namespace Blade
{
	class BPKFileDevice : public IFileDevice, public Allocatable
	{
	public:
		static const TString BPK_FILE_EXT;
		static const TString BPK_FILE_TYPE;
	public:
		BPKFileDevice();
		~BPKFileDevice();

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

		BPKFile	mFile;
	};//class BPKFileDevice
	
}//namespace Blade

#endif //  __Blade_BPKFileDevice_h__