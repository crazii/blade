/********************************************************************
	created:	2014/01/13
	filename: 	SubFileDevice.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_SubFileDevice_h__
#define __Blade_SubFileDevice_h__
#include <interface/public/file/IFileDevice.h>

namespace Blade
{

	class SubFileDevice : public IFileDevice, public Allocatable
	{
	public:
		SubFileDevice(const HFILEDEVICE& folder, const TString& subPath);
		~SubFileDevice();

		/************************************************************************/
		/* IDevice interfaces                                                                     */
		/************************************************************************/
		/** @brief open a device */
		virtual bool	open();

		/** @brief check if it is open */
		virtual bool	isOpen() const	{return mLoaded;}

		/** @brief close the device */
		virtual void	close()			{mLoaded = false;}

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
		virtual const TString&	getType() const noexcept	{return mParent->getType();}

		/*
		@describe 
		@param 
		@return 
		*/
		virtual void			getFullPath(const TString& subPath, TString& outFullPath) const noexcept;

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReadOnly() const noexcept	{return !this->isLoaded() || mParent->isReadOnly();}

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
		virtual void			findFile(TStringParam& result,const TString& pattern = BTString("*"), int findFlag = FF_DIR|FF_FILE) noexcept;

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
		HFILEDEVICE	mParent;
		TString		mSubPath;
		bool		mLoaded;
	};//class SubFileDevice

	
}//namespace Blade

#endif //  __Blade_SubFileDevice_h__