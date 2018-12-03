/********************************************************************
	created:	2011/05/03
	filename: 	Version.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
	const Version Version::MAX(255,255,'Z');
	const Version Version::MIN(0, 1, ' ');

	//////////////////////////////////////////////////////////////////////////
	Version::Version(ushort major,uchar minor,uchar suffix/* = ' '*/)
		:mMajor(major)
		,mMinor(minor)
	{
		if( std::isalpha(suffix) )
			mSuffix = (uchar)::toupper( (int)suffix);
		else
		{
			assert(suffix == ' ');
			mSuffix = ' ';
		}
	}

	//////////////////////////////////////////////////////////////////////////
	Version::Version(const TString& verString)
	{
		TStringStream ss;
		int major = 0,minor = 0;
		int suffix = ' ';
		ss.str(verString);
		ss.scanFormat(TEXT("%d.%d%c"),&major,&minor,&suffix);
		mMajor = (uchar)major;
		mMinor = (uchar)minor;
		mSuffix = (uchar)suffix;
	}

	//////////////////////////////////////////////////////////////////////////
	TString		Version::getVersionString() const
	{
		TStringStream ss;
		int suffix = mSuffix;
		ss.format(TEXT("%d.%02d%c"),mMajor,mMinor,suffix);
		return ss.getContent();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Version::operator<(const Version& rhs) const
	{
		if( this->mMajor < rhs.mMajor )
			return true;
		else if( this->mMajor > rhs.mMajor )
			return false;

		if( this->mMinor < rhs.mMinor )
			return true;
		else if( this->mMinor > rhs.mMinor )
			return false;

		if( this->mSuffix < rhs.mSuffix )
			return true;
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	Version&	Version::operator++()
	{
		if( mSuffix == ' ')
			mSuffix = 'A';
		else
		{
			assert( std::isalpha(mSuffix) );
			if( mSuffix == 'Z' )
			{
				if( mMinor == 255 )
				{
					++mMajor;
					mMinor = 0;
				}
				else
					++mMinor;
				mSuffix = ' ';
			}
			else
				++mSuffix;
		}
		return *this;
	}
	

}//namespace Blade
