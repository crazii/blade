/********************************************************************
	created:	2014/01/13
	filename: 	SubFileDevice.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "SubFileDevice.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	SubFileDevice::SubFileDevice(const HFILEDEVICE& folder, const TString& subPath)
		:mParent(folder)
		,mSubPath(TStringHelper::standardizePath(subPath))
	{
		mLoaded = mParent->existDirectory(mSubPath);
		if( mLoaded )
			mParent->getFullPath(mSubPath, mPathName);
	}

	//////////////////////////////////////////////////////////////////////////
	SubFileDevice::~SubFileDevice()
	{
		this->unload();
	}

	/************************************************************************/
	/* IDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool	SubFileDevice::open()
	{
		mLoaded = mParent->isLoaded() && mParent->existDirectory(mPathName);
		if( mLoaded )
		{
			mSubPath = mPathName;
			mParent->getFullPath(mSubPath, mPathName);
		}
		return mLoaded;
	}

	/************************************************************************/
	/* IFileDevice interfaces                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	void			SubFileDevice::getFullPath(const TString& subPath, TString& outFullPath) const noexcept
	{
		outFullPath = TStringHelper::standardizePath(mPathName + BTString("/") + subPath);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SubFileDevice::existFile(const TString& filepathname) const noexcept
	{
		return this->isLoaded() && mParent->existFile( mSubPath + BTString("/") + filepathname );
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			SubFileDevice::openFile(const TString& filepathname,IStream::EAccessMode mode/* = IStream::AM_READ*/) const noexcept
	{
		if( this->isLoaded() )
			return mParent->openFile( mSubPath + BTString("/") + filepathname, mode );
		else
			return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	HSTREAM			SubFileDevice::createFile(const TString& filepathname, IStream::EAccessMode mode) noexcept
	{
		if( this->isLoaded() )
			return mParent->createFile( mSubPath + BTString("/") + filepathname, mode );
		else
			return HSTREAM::EMPTY;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SubFileDevice::deleteFile(const TString& filepathname) noexcept
	{
		return this->isLoaded() &&
			mParent->deleteFile( mSubPath + BTString("/") + filepathname );
	}

	//////////////////////////////////////////////////////////////////////////
	void			SubFileDevice::findFile(TStringParam& result, const TString& pattern/* = BTString("*")*/, int findFlag/* = FF_DIR|FF_FILE*/) noexcept
	{
		if( this->isLoaded() )
		{
			TStringParam temp;
			TString path = mSubPath + BTString("/");
			mParent->findFile( temp, path + pattern, findFlag );
			for(size_t i = 0; i < temp.size(); ++i)
			{
				const TString& str = temp[i];
				assert( TStringHelper::isStartWith(str, path) );
				result.push_back( str.substr( path.size() ) ) ;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SubFileDevice::createDirectory(const TString& name, bool bRecursive/* = false*/)
	{
		return this->isLoaded() && mParent->createDirectory( mSubPath + BTString("/") + name, bRecursive);
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SubFileDevice::deleteDirectory(const TString& name)
	{
		return this->isLoaded() && mParent->deleteDirectory( mSubPath + BTString("/") + name );
	}

	//////////////////////////////////////////////////////////////////////////
	bool			SubFileDevice::existDirectory(const TString& name) const noexcept
	{
		return this->isLoaded() && mParent->existDirectory( mSubPath + BTString("/") + name );
	}
	
}//namespace Blade