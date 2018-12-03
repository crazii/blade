/********************************************************************
	created:	2011/05/03
	filename: 	Version.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Version_h__
#define __Blade_Version_h__
#include "String.h"

namespace Blade
{
	class  Version : public NonAllocatable
	{
	public:
		Version()
			:mMajor(0)
			, mMinor(0)
			, mSuffix(' ')
		{

		}

		BLADE_BASE_API Version(ushort major,uchar minor,uchar suffix = ' ');
		explicit BLADE_BASE_API Version(const TString& verString);

		~Version() {}

		/**
		@describe 
		@param 
		@return 
		*/
		BLADE_BASE_API TString		getVersionString() const;

		/**
		@describe 
		@param 
		@return 
		*/
		BLADE_BASE_API bool		operator<(const Version& rhs) const;

		/**
		@describe 
		@param 
		@return 
		*/
		inline bool		operator==(const Version& rhs) const
		{
			return mMajor == rhs.mMajor && mMinor == rhs.mMinor && mSuffix == rhs.mSuffix;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline Version		operator++(int)
		{
			Version ver = *this;
			++(*this);
			return ver;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		BLADE_BASE_API Version&	operator++();

		/**
		@describe 
		@param 
		@return 
		*/
		ushort		getMajor() const { return mMajor; }

		/**
		@describe 
		@param 
		@return 
		*/
		uchar		getMinor() const { return mMinor; }

		/**
		@describe 
		@param 
		@return 
		*/
		uchar		getSuffix() const{ return mSuffix; }

		//add class keyword to avoid weird symbol conflicts (known from max sdk)
		static const BLADE_BASE_API class Version MAX;
		static const BLADE_BASE_API class Version MIN;
	protected:
		uint16	mMajor;
		uint8	mMinor;
		uint8	mSuffix;
	};
	

}//namespace Blade



#endif // __Blade_Version_h__