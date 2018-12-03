/********************************************************************
	created:	2014/06/17
	filename: 	FileDevice.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "FileDevice.h"
#include "FileStream.h"

namespace Blade
{
	template class Factory<IFileDevice>;

	//////////////////////////////////////////////////////////////////////////
	void	IFileDevice::getDeviceOption(ParamList& defVal, Options& /*outOption*/)
	{
		defVal[ BTString("PATH") ] = (mPathName == TString::EMPTY ? BTString(".") : mPathName);
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IFileDevice::applyDeviceOption(const ParamList& result)
	{
		bool ret = false;
		const Variant& path = result[ BTString("PATH") ];
		if(path.isValid())
		{
			mPathName = path;
			ret = true;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void	IFileDevice::getFullPath(const TString& subPath, TString& outFullPath) const noexcept
	{
		const TString& name = this->getName();
		assert( name != TString::EMPTY );
		if( subPath == TString::EMPTY )
			outFullPath = TStringHelper::standardizePath( name.substr_nocopy(0, name.find_last_not_of(TEXT("\\/"))+1 ) );
		else
			outFullPath = TStringHelper::standardizePath( name.substr_nocopy(0, name.find_last_not_of(TEXT("\\/"))+1 ) + BTString("/") + subPath );
	}

	//////////////////////////////////////////////////////////////////////////
	bool	IFileDevice::load(const TString& name) noexcept
	{
		ParamList list;
		list[BTString("PATH")] = name;
		return this->applyDeviceOption(list) && this->open();
	}


	//////////////////////////////////////////////////////////////////////////
	FileDevice::FileDevice()
		:mLoaded(false)
		,mReadOnly(true)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	FileDevice::~FileDevice()
	{

	}

	/************************************************************************/
	/* IDevice interfaces                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::open()
	{
		this->unload();

		if( mPathName == TString::EMPTY )
		{
			tchar path[PATH_MAX];
			blade_getcwd(path, PATH_MAX);
			mPathName = TString(path);
		}

		struct blade_stat_t s;
		std::memset(&s, 0, sizeof(s));

		if( blade_stat(mPathName.c_str(),&s) != 0 )
			return false;

		if( (s.st_mode&S_IFDIR) && (s.st_mode&S_IREAD) )
		{
			mReadOnly = (s.st_mode&S_IWRITE) == 0;
			mLoaded = true;
			//make path standard
			mPathName = TStringHelper::standardizePath(mPathName);
			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::isOpen() const
	{
		return mLoaded;
	}

	//////////////////////////////////////////////////////////////////////////
	void			FileDevice::close()
	{
		mLoaded = false;
	}

	/************************************************************************/
	/* IFileDevice interfaces                                                                     */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	const TString&	FileDevice::getType() const  noexcept
	{
		return DEFAULT_FILE_TYPE;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::isReadOnly() const noexcept
	{
		return mReadOnly;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::existFile(const TString& filepathname) const noexcept
	{
		struct blade_stat_t s;
		TString fullPath;
		this->getFullPath(filepathname, fullPath);
		if( blade_stat(fullPath.c_str(), &s) == 0
			&& (s.st_mode&S_IFMT) == S_IFREG )
			return true;
		else
			return false;
	}


	//////////////////////////////////////////////////////////////////////////
	HSTREAM			FileDevice::openFile(const TString& filepathname,IStream::EAccessMode mode/* = IStream::AM_READ*/) const noexcept
	{
		if( mPathName == TString::EMPTY || filepathname == TString::EMPTY )
		{
			assert(false);
			return HSTREAM::EMPTY;
		}

		TString fullPath;
		this->getFullPath(filepathname,fullPath);

		HSTREAM hstream;
		DefaultFileStream* stream = BLADE_NEW DefaultFileStream(fullPath, mode);
		if( stream->isValid() )
			hstream.bind( stream );
		else
			BLADE_DELETE stream;

		return hstream;
	}


	//////////////////////////////////////////////////////////////////////////
	HSTREAM			FileDevice::createFile(const TString& filepathname, IStream::EAccessMode mode) noexcept
	{
		HSTREAM hstream;
		if( mReadOnly )
			return hstream;

		TString fullPath;
		this->getFullPath(filepathname,fullPath);

		DefaultFileStream* stream = BLADE_NEW DefaultFileStream(fullPath, mode);
		if( stream->isValid() )
			hstream.bind( stream );
		else
			BLADE_DELETE stream;
		return hstream;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::deleteFile(const TString& filepathname) noexcept
	{
		if( mReadOnly )
			return false;

		TStringStream stream;
		stream << mPathName.c_str() << TEXT("/") << filepathname.c_str();
		blade_remove( stream.str().c_str() );
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::createDirectory(const TString& name, bool bRecursive/* = false*/)
	{
		if( !bRecursive )
		{
			TString path = mPathName + name;
			return blade_mkdir( path.c_str() ) == 0;
		}
		else
		{
			TStringTokenizer tokenizer;
			TStringConcat concat(mPathName);
			tokenizer.tokenize(name, TEXT("\\/"), true);
			for(size_t i = 0; i < tokenizer.size(); ++i)
			{
				concat += BTString("/") + tokenizer[i];
				TString path = concat;
				//check to see whether sub directory already exist
				struct blade_stat_t s;
				if( blade_stat(path.c_str(),&s) == 0 && (s.st_mode&S_IFDIR) && (s.st_mode&S_IREAD) )
					continue;

				//non-exist
				int ret = blade_mkdir(path.c_str());
				if( !(ret == 0 ) )
					return false;
			}
			return true;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::deleteDirectory(const TString& name)
	{
		TString path = mPathName + name;
		return blade_rmdir(path.c_str()) == 0;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			FileDevice::existDirectory(const TString& name) const noexcept
	{
		struct blade_stat_t s;
		TString fullPath;
		this->getFullPath(name, fullPath);
		if( blade_stat(fullPath.c_str(), &s) == 0 
			&& (s.st_mode&S_IFMT) == S_IFDIR )
			return true;
		else
			return false;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	
	
}//namespace Blade
