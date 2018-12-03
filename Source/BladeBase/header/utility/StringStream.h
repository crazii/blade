/********************************************************************
	created:	2009/03/23
	filename: 	StringStream.h
	author:		Crazii
	
	purpose:	string streams are also temporary buffers, so DO NOT hold the data \n
				unless using the clear() method frequently
*********************************************************************/
#ifndef __Blade_StringStream_h__
#define __Blade_StringStream_h__
#include "String.h"

namespace Blade
{
	namespace Impl
	{
		class StringStreamImpl;
		class WStringStreamImpl;
	}//namespace Impl

	class BLADE_BASE_API StringStream : public NonAllocatable, public NonAssignable, public NonCopyable
	{
	public:
		StringStream();
		~StringStream();

		/**
		* @describe 
		* @param 
		* @return 
		*/
		StringStream&	operator<<(const String& text);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const char* text);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator<<(const bool	yesno);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator<<(const int32	integer);


		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const uint32 uinteger);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator<<(const fp32 real);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator<<(const fp64 real);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const void* ptr);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const long ld);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const unsigned long uld);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const short sd);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const unsigned short usd);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const char cd);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator<<(const unsigned char ucd);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator<<(const int64 i64d);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator<<(const uint64 ui64d);

		/**
		* @describe 
		* @param 
		* @return 
		*/
		StringStream&	operator>>(String& text);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator>>(bool&	yesno);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator>>(int32&	integer);


		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(uint32& uinteger);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator>>(fp32& real);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator>>(fp64& real);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(void*& ptr);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(long& ld);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(unsigned long& uld);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(short& sd);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(unsigned short& usd);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(char& cd);

		/**
		@describe
		@param
		@return
		*/
		StringStream&	operator>>(unsigned char& ucd);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator>>(int64&	i64d);

		/**
		@describe 
		@param 
		@return 
		*/
		StringStream&	operator>>(uint64&	ui64d);

		/**
		@describe
		@param
		@return
		*/
		void			clear();

		/**
		@describe C-style format ,without CPP style IO manipulators to avoid potential lib version compatibility issue
		@param
		@return
		*/
		bool			format(const char* fmt, ...);

		/**
		@describe 
		@param 
		@return 
		*/
		bool			vformat(const char* fmt, va_list varg_ptr);

		/**
		@describe
		@param
		@return
		*/
		bool			appendFormat(const char* fmt, ...);

		/**
		@describe 
		@param 
		@return 
		*/
		bool			appendVFormta(const char* fmt, va_list varg_ptr);

		/**
		@describe 
		@param 
		@return 
		*/
		int				scanFormat(const char* fmt,...);

		/**
		@describe 
		@param 
		@return 
		*/
		int				scanFormatOffset(size_t offset,const char* fmt,...);

		/**
		@describe
		@param
		@return
		*/
		const String&	getContent() const;

		/**
		@describe
		@param
		@return
		*/
		inline const String&	str() const		{return getContent();}

		/**
		@describe
		@param
		@return
		*/
		void			str(const String& src);
		void			str(const char* src);

	private:
		mutable Pimpl<Impl::StringStreamImpl>	mImpl;
	};//class StringStream




	class BLADE_BASE_API WStringStream : public NonAllocatable, public NonAssignable, public NonCopyable
	{
	public:
		WStringStream();
		~WStringStream();

		/**
		* @describe 
		* @param 
		* @return 
		*/
		WStringStream&	operator<<(const WString& text);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const wchar* text);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator<<(const bool	yesno);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator<<(const int32	integer);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const uint32 uinteger);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator<<(const fp32 real);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator<<(const fp64 real);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const void* ptr);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const long ld);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const unsigned long uld);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const short sd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const unsigned short usd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const char cd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const unsigned char ucd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator<<(const wchar ucd);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator<<(const int64 i64d);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator<<(const uint64 ui64d);

		/**
		* @describe 
		* @param 
		* @return 
		*/
		WStringStream&	operator>>(WString& text);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator>>(bool&	yesno);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator>>(int32&	integer);


		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(uint32& uinteger);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator>>(fp32& real);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator>>(fp64& real);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(void*& ptr);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(long& ld);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(unsigned long& uld);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(short& sd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(unsigned short& usd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(char& cd);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(unsigned char& ucd);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator>>(int64& i64d);

		/**
		@describe 
		@param 
		@return 
		*/
		WStringStream&	operator>>(uint64& ui64d);

		/**
		@describe
		@param
		@return
		*/
		WStringStream&	operator>>(wchar& wc);

		/**
		@describe
		@param
		@return
		*/
		void			clear();

		/**
		@describe C-style format ,without CPP style IO manipulators to avoid potential lib version compatibility issue
		@param
		@return
		*/
		bool			format(const wchar* fmt,...);

		/**
		@describe 
		@param 
		@return 
		*/
		bool			vformat(const wchar* fmt, va_list varg_ptr);

		/**
		@describe
		@param
		@return
		*/
		bool			appendFormat(const wchar* fmt,...);

		/**
		@describe 
		@param 
		@return 
		*/
		bool			appendVFormat(const wchar* fmt, va_list varg_ptr);

		/**
		@describe 
		@param 
		@return 
		*/
		int				scanFormat(const wchar* fmt,...);

		/**
		@describe 
		@param 
		@return 
		*/
		int				scanFormatOffset(size_t offset,const wchar* fmt,...);

		/**
		@describe
		@param
		@return
		*/
		const WString&	getContent() const;

		/**
		@describe
		@param
		@return
		*/
		inline const WString&	str() const		{return getContent();}

		/**
		@describe
		@param
		@return
		*/
		void			str(const WString& src);
		void			str(const wchar* src);

	private:
		mutable Pimpl<Impl::WStringStreamImpl>	mImpl;
	};//class StringStream



#if BLADE_UNICODE
	typedef WStringStream TStringStream;
#else
	typedef StringStream TStringStream;
#endif

	
}//namespace Blade

#endif // __Blade_StringStream_h__