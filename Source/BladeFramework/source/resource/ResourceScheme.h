/********************************************************************
	created:	2010/04/10
	filename: 	ResourceScheme.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ResourceScheme_h__
#define __Blade_ResourceScheme_h__
#include <BladeFramework.h>
#include <interface/public/file/IFileDevice.h>

namespace Blade
{
	class ResourceScheme : public Allocatable
	{
	public:
		ResourceScheme(const TString& name,const HFILEDEVICE& hFolder);
		~ResourceScheme();

		/*
		@describe 
		@param 
		@return 
		*/
		const TString&	getName() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HFILEDEVICE&	getDeviceFolder() const;

		/*
		@describe 
		@param 
		@return 
		*/
		HSTREAM		loadFile(const TString& pathname,IStream::EAccessMode mode) const;

		/*
		@describe 
		@param 
		@return 
		*/
		HSTREAM		createFile(const TString& pathname, IStream::EAccessMode mode);

		/*
		@describe load the file if it exist, or create a new one
		@param 
		@return 
		*/
		HSTREAM		loadOrCreateFile(const TString& pathname, bool create_dir, IStream::EAccessMode mode);

		/*
		@describe 
		@param 
		@return 
		*/
		bool		deleteFile(const TString& pathname);

		/*
		@describe 
		@param 
		@return 
		*/
		bool		existFile(const TString& pathname) const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool		existDirectory(const TString& pathname) const;


	protected:
		TString		mName;
		HFILEDEVICE	mFolder;
	};
	
}//namespace Blade


#endif //__Blade_ResourceScheme_h__