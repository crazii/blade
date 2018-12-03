/********************************************************************
	created:	2014/02/16
	filename: 	BPKStream.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "BPKStream.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	BPKStream::BPKStream(const TString& name, IStream::EAccessMode mode, const BPKFile* package, BPKFile::FILE* fp)
		:mName(name)
		,mPackage(package)
		,mFile(fp)
		,mAccessMode(mode)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BPKStream::~BPKStream()
	{
		this->close();
	}

	/************************************************************************/
	/* IStream interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const TString& BPKStream::getName()
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		BPKStream::isValid() const
	{
		return mFile != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void		BPKStream::close()
	{
		if( mPackage != NULL && mFile != NULL )
		{
			mPackage->fclose(mFile);

			mName = TString::EMPTY;
			mFile = NULL;
			mPackage = NULL;
			mAccessMode = AM_READ;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off			BPKStream::seek(EStreamPos origin, Off off)
	{
		if( mPackage != NULL && mFile != NULL )
		{
			mPackage->fseek(mFile, off, origin);
			return mPackage->ftell(mFile);
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Off			BPKStream::tell() const
	{
		if( mPackage != NULL && mFile != NULL )
			return mPackage->ftell(mFile);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		BPKStream::eof() const
	{
		if( mPackage != NULL && mFile != NULL )
			return mPackage->feof(mFile) != 0;
		else
		{
			assert(false);
			return false;
		}	
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size		BPKStream::readData(void* destBuffer, Size size)
	{
		if( mPackage != NULL && mFile != NULL )
			return mPackage->fread(destBuffer, size, mFile); 
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size		BPKStream::writeData(const void* /*srcBuffer*/,Size /*size*/)
	{
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IStream::Size		BPKStream::getSize() const
	{
		if( mPackage != NULL && mFile != NULL )
			return mPackage->fsize(mFile);
		else
		{
			assert(false);
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int			BPKStream::getAccesMode() const
	{
		return mAccessMode;
	}

	//////////////////////////////////////////////////////////////////////////
	void		BPKStream::flush(bool /*hardFlush = false*/)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool		BPKStream::truncate(Size /*size*/)
	{
		return false;
	}
	
}//namespace Blade