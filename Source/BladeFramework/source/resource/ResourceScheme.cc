/********************************************************************
	created:	2010/04/10
	filename: 	ResourceScheme.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "ResourceScheme.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	ResourceScheme::ResourceScheme(const TString& name,const HFILEDEVICE& hFolder)
		:mName(name)
		,mFolder(hFolder)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	ResourceScheme::~ResourceScheme()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	ResourceScheme::getName() const
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	const HFILEDEVICE&	ResourceScheme::getDeviceFolder() const
	{
		return mFolder;
	}


	//////////////////////////////////////////////////////////////////////////
	HSTREAM		ResourceScheme::loadFile(const TString& pathname,IStream::EAccessMode mode) const
	{
		return mFolder->openFile(pathname, mode);
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM		ResourceScheme::createFile(const TString& pathname, IStream::EAccessMode mode)
	{
		return mFolder->createFile(pathname, mode);
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM		ResourceScheme::loadOrCreateFile(const TString& pathname, bool create_dir, IStream::EAccessMode mode)
	{
		TString dir,filename;
		TStringHelper::getFilePathInfo(pathname,dir,filename);

		//make sure each directory of the directory tree exist.
		if( dir != TString::EMPTY && create_dir)
		{
			if( !mFolder->createDirectory(dir, true) )
				return HSTREAM::EMPTY;
		}

		HSTREAM ret = mFolder->openFile(pathname , mode);
		if( ret == NULL )
			ret = this->createFile(pathname, mode);

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceScheme::deleteFile(const TString& pathname)
	{
		return mFolder->deleteFile(pathname);
	}


	//////////////////////////////////////////////////////////////////////////
	bool		ResourceScheme::existFile(const TString& pathname) const
	{
		return mFolder->existFile(pathname);
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ResourceScheme::existDirectory(const TString& pathname) const
	{
		return mFolder->existDirectory(pathname);
	}
	
}//namespace Blade