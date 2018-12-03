/********************************************************************
	created:	2010/03/28
	filename: 	Exception.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	Exception::Exception(const tchar* name,uint code,const TString& description,const TString& src)
		:mName(name)
		,mCode(code)
		,mDescription(description)
		,mSource(src)
		,mFile(TEXT(""))
		,mLine(0)
	{
		_what[0] = '\0';
	}

	//////////////////////////////////////////////////////////////////////////
	Exception::Exception(const tchar* name,uint code,const TString& description,const TString& src,const TString& file,ulong line)
		:mName(name)
		,mCode(code)
		,mDescription(description)
		,mSource(src)
		,mFile(file)
		,mLine(line)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	Exception::Exception(const Exception& src)
		:mName(src.mName)
		,mCode(src.mCode)
		,mDescription(src.mDescription)
		,mSource(src.mSource)
		,mFile(src.mFile)
		,mLine(src.mLine)
		,_what(src._what)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	void Exception::operator = (const Exception& rhs)
	{
		mName = rhs.mName;
		mCode = rhs.mCode;
		mDescription = rhs.mDescription;
		mSource = rhs.mSource;
		mFile = rhs.mFile;
		mLine = rhs.mLine;
		_what = rhs._what;
	}

	//////////////////////////////////////////////////////////////////////////
	/// std::exception::what override
	const char*		Exception::what() const noexcept
	{
		if( _what.size() == 0 )
		{
			TStringStream stream;
			stream << TEXT("Blade Exception:") << mName << TEXT("\n")
				<< TEXT("Code:") << mCode << TEXT("\n")
				<< TEXT("Description:") << mDescription.c_str() << TEXT("\n")
				<< TEXT("in:") << mSource << TEXT("\n")
				<< TEXT("Source File:") << TEXT("\n") << mFile 
				<< TEXT("(") << mLine << TEXT(")") << TEXT("\n");

#if !BLADE_UNICODE
			const tchar* desc = stream.str().c_str();
			_what =  String(desc);
#else
			_what = StringConverter::WStringToString(stream.str().c_str(),stream.str().size());
#endif
		}
		return _what.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	const tchar*	Exception::getTypeName() const throw()
	{
		return mName;
	}

	//////////////////////////////////////////////////////////////////////////
	uint			Exception::getCode() const throw()
	{
		return mCode;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Exception::getDescription() const
	{
		return mDescription;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Exception::getSource() const throw()
	{
		return mSource;
	}

	//////////////////////////////////////////////////////////////////////////
	const TString&	Exception::getFile() const throw()
	{
		return mFile;
	}

	//////////////////////////////////////////////////////////////////////////
	ulong			Exception::getLine() const throw()
	{
		return mLine;
	}

	const tchar*	Exception::getFullDescription() const
	{
		if( mFullDescription.size() == 0 )
		{
			TStringStream stream;
			stream << TEXT("Blade Exception:") << mName << TEXT("(Code:") << mCode  << TEXT(")") << TEXT("\n")
				<< TEXT("Description:") << mDescription.c_str() << TEXT("\n")
				<< TEXT("in:") << mSource << TEXT("\n")
				<< TEXT("Source File:") << TEXT("\n") << mFile
				<< TEXT("(") << mLine << TEXT(")") << TEXT("\n");

			mFullDescription = stream.str().c_str();
		}
		return mFullDescription.c_str();
	}


	
}//namespace Blade