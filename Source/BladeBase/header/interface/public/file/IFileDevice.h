/********************************************************************
	created:	2014/06/19
	filename: 	IFileDevice.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IFileDevice_h__
#define __Blade_IFileDevice_h__
#include <interface/public/IDevice.h>
#include "IStream.h"

namespace Blade
{
	class BLADE_BASE_API IFileDevice : public IDevice
	{
	public:
		static const TString DEFAULT_FILE_TYPE;
	public:
		enum EFindFlag
		{
			FF_DIR			= 0x00000001,
			FF_FILE			= 0x00000002,
			FF_RECURSIVE	= 0x00000004,
		};

		virtual ~IFileDevice()		{}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual const TString&	getType() const noexcept = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual inline const TString&	getName() const noexcept
		{
			return mPathName;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			getFullPath(const TString& subPath, TString& outFullPath) const noexcept;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			load(const TString& name) noexcept;

		inline bool			isLoaded() const noexcept
		{
			return this->isOpen();
		}

		inline bool			unload() noexcept
		{
			this->close();
			return true;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			isReadOnly() const noexcept = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			existFile(const TString& filepathname) const noexcept = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			openFile(const TString& filepathname,IStream::EAccessMode mode = IStream::AM_READ) const noexcept = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual HSTREAM			createFile(const TString& filepathname, IStream::EAccessMode mode) noexcept = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteFile(const TString& filepathname) noexcept = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual void			findFile(TStringParam& result, const TString& pattern, int findFlag = FF_DIR|FF_FILE) = 0;

		inline void	listFile(TStringParam& result, int findFlag/* = FF_FILE*/)
		{
			this->findFile(result, BTString("*"), findFlag);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			createDirectory(const TString& name, bool bRecursive = false) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			deleteDirectory(const TString& name) = 0;

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool			existDirectory(const TString& name) const noexcept = 0;

	protected:

		/** @brief  */
		virtual void	getDeviceOption(ParamList& defVal, Options& outOption);

		/** @brief  */
		virtual bool	applyDeviceOption(const ParamList& result);

		TString		mPathName;
	};

	extern template class BLADE_BASE_API Factory<IFileDevice>;

	typedef Factory<IFileDevice>	FileDeviceFactory;
	typedef Handle<IFileDevice>		HFILEDEVICE;
	
}//namespace Blade


#endif // __Blade_IFileDevice_h__
