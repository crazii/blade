/********************************************************************
	created:	2010/09/08
	filename: 	StringStream.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "InternalString.h"
#include <stdarg.h>

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4127)		//while(true) warning
#pragma warning( disable:4996)		//disable MSVC deprecation warning
// inheritance via dominance (for VS2012 stl class, don't know whether it is critical, turn off for now)
#pragma warning( disable:4250)
#pragma warning( disable:4774)		//'sscanf' : format string expected in argument 2 is not a string literal
#endif

#define BUFFER64SIZE 256

namespace Blade
{
	namespace Impl
	{
		class StringStreamImpl : public InternalStringStream, public TempAllocatable
		{
		public:
			typedef TmpString ContentType;
			ContentType	mContent;
		};
		class WStringStreamImpl : public InternalWStringStream, public TempAllocatable
		{
		public:
			typedef TmpWString ContentType;
			TmpWString mContent;
		};
	}//namespace Impl
	using namespace Impl;
	using namespace std;

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	StringStream::StringStream()
		:mImpl( BLADE_NEW StringStreamImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	StringStream::~StringStream()
	{
		this->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const String& text)
	{
		//note: text maybe a readonly substring without '\0'
		char* s = BLADE_TMP_ALLOCT(char, text.size() + 1);
		std::memcpy(s, text.c_str(), text.size()*sizeof(char));
		s[text.size()] = '\0';
		(*mImpl) << s;
		BLADE_TMP_FREE(s);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const char* text)
	{
		(*mImpl) << text;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const bool	yesno)
	{
		(*mImpl) << yesno;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const int32	integer)
	{
		(*mImpl) << integer;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const uint32 uinteger)
	{
		(*mImpl) << uinteger;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const fp32 real)
	{
		(*mImpl) << real;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const fp64 real)
	{
		(*mImpl) << real;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const void* ptr)
	{
		(*mImpl) << ptr;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const long ld)
	{
		(*mImpl) << ld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const unsigned long uld)
	{
		(*mImpl) << uld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const short sd)
	{
		(*mImpl) << sd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const unsigned short usd)
	{
		(*mImpl) << usd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const int64 i64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) << i64d;
#else
		char buffer[BUFFER64SIZE];
		int n = Char_Traits<char>::from_i64(buffer, BUFFER64SIZE-1, i64d);
		assert(n != -1 && n < BUFFER64SIZE-1);
		buffer[n+1] = '\0';
		(*mImpl) << buffer;
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const uint64 ui64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) << ui64d;
#else
		char buffer[BUFFER64SIZE];
		int n = Char_Traits<char>::from_u64(buffer, BUFFER64SIZE-1, ui64d);
		assert(n != -1 && n < BUFFER64SIZE-1);
		buffer[n+1] = '\0';
		(*mImpl) << buffer;
#endif
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const char cd)
	{
		(*mImpl) << cd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator<<(const unsigned char ucd)
	{
		(*mImpl) << ucd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(String& text)
	{
		text = this->getContent();
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(bool&	yesno)
	{
		(*mImpl) >> yesno;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(int32&	integer)
	{
		(*mImpl) >> integer;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(uint32& uinteger)
	{
		(*mImpl) >> uinteger;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(fp32& real)
	{
		(*mImpl) >> real;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(fp64& real)
	{
		(*mImpl) >> real;
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(void*& ptr)
	{
		(*mImpl) >> ptr;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(long& ld)
	{
		(*mImpl) >> ld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(unsigned long& uld)
	{
		(*mImpl) >> uld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(short& sd)
	{
		(*mImpl) >> sd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(unsigned short& usd)
	{
		(*mImpl) >> usd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(char& cd)
	{
		(*mImpl) >> cd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(unsigned char& ucd)
	{
		(*mImpl) >> ucd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(int64&	i64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) >> i64d;
#else
		const String& s = this->getContent();
		size_t bits = 0;
		while (bits < s.size() && s[bits] >= '0' && s[bits] <= '9')
			++bits;
		assert(bits < BUFFER64SIZE);
		char buffer[BUFFER64SIZE];
		std::memcpy(buffer, s.c_str(), bits * sizeof(char));
		buffer[bits + 1] = '\0';
		i64d = Char_Traits<char>::to_i64(buffer, NULL, 10);
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringStream&	StringStream::operator>>(uint64&	ui64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) >> ui64d;
#else
		const String& s = this->getContent();
		size_t bits = 0;
		while (bits < s.size() && s[bits] >= '0' && s[bits] <= '9')
			++bits;
		assert(bits < BUFFER64SIZE);
		char buffer[BUFFER64SIZE];
		std::memcpy(buffer, s.c_str(), bits*sizeof(char));
		buffer[bits + 1] = '\0';
		ui64d = Char_Traits<char>::to_u64(buffer, NULL, 10);
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringStream::clear()
	{
		mImpl->str( "" );
		mImpl->clear();
		mImpl->mContent = StringStreamImpl::ContentType();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringStream::format(const char* fmt,...)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i ) )
				return false;

			char* dest = (char*)buffer.getData();
			va_list v;
			va_start(v, fmt);

			int n = ::vsnprintf( dest,i-1,fmt,v);
			if( n != -1 )
			{
				dest[n] = '\0';
				break;
			}
		}

		this->clear();
		(*this) << (const char*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringStream::vformat(const char* fmt, va_list varg_ptr)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i ) )
				return false;

			char* dest = (char*)buffer.getData();
			int n = ::vsnprintf( dest, i-1, fmt, varg_ptr);
			if( n != -1 )
			{
				dest[n] = '\0';
				break;
			}
		}
		this->clear();
		(*this) << (const char*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringStream::appendFormat(const char* fmt,...)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i ) )
				return false;

			char* dest = (char*)buffer.getData();
			va_list v;
			va_start(v, fmt);

			int n = ::vsnprintf( dest,i-1,fmt,v);
			if( n != -1 )
			{
				dest[n] = '\0';
				break;
			}
		}
		(*this) << (const char*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			StringStream::appendVFormta(const char* fmt, va_list varg_ptr)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i ) )
				return false;

			char* dest = (char*)buffer.getData();
			int n = ::vsnprintf( dest, i-1, fmt, varg_ptr);
			if( n != -1 )
			{
				dest[n] = '\0';
				break;
			}
		}
		(*this) << (const char*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	int			StringStream::scanFormat(const char* fmt,...)
	{
		if( mImpl->str().size() == 0 )
			return false;

		va_list v;
		va_start(v,fmt);

		return vsscanf( this->getContent().c_str(),fmt,v);
	}

	//////////////////////////////////////////////////////////////////////////
	int			StringStream::scanFormatOffset(size_t offset,const char* fmt, ...)
	{
		if( mImpl->str().size() < offset )
			return false;

		va_list v;
		va_start(v,fmt);
		return vsscanf( this->getContent().c_str()+offset, fmt, v);
	}

	//////////////////////////////////////////////////////////////////////////
	const String&	StringStream::getContent() const
	{
		mImpl->mContent = StringStreamImpl::ContentType(  mImpl->str().c_str() );
		return reinterpret_cast<const String&>(mImpl->mContent);
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringStream::str(const String& src)
	{
		(*mImpl).str("");
		(*this) << src;
	}

	//////////////////////////////////////////////////////////////////////////
	void			StringStream::str(const char* src)
	{
		(*mImpl).str( src );
	}

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	WStringStream::WStringStream()
		:mImpl( BLADE_NEW WStringStreamImpl() )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream::~WStringStream()
	{
		this->clear();
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const WString& text)
	{
		//note: text maybe a read only substring without '\0'
		wchar* s = BLADE_TMP_ALLOCT(wchar, text.size() + 1);
		std::memcpy(s, text.c_str(), text.size()*sizeof(wchar));
		s[text.size()] = L'\0';
		(*mImpl) << s;
		BLADE_TMP_FREE(s);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const wchar* text)
	{
		(*mImpl) << text;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const bool	yesno)
	{
		(*mImpl) << yesno;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const int32	integer)
	{
		(*mImpl) << integer;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const uint32 uinteger)
	{
		(*mImpl) << uinteger;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const fp32 real)
	{
		(*mImpl) << real;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const fp64 real)
	{
		(*mImpl) << real;
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const void* ptr)
	{
		(*mImpl) << ptr;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const long ld)
	{
		(*mImpl) << ld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const unsigned long uld)
	{
		(*mImpl) << uld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const short sd)
	{
		(*mImpl) << sd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const unsigned short usd)
	{
		(*mImpl) << usd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const char cd)
	{
		(*mImpl) << cd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const unsigned char ucd)
	{
		(*mImpl) << ucd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const wchar ucd)
	{
		(*mImpl) << ucd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const int64 i64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) << i64d;
#else
		wchar buffer[BUFFER64SIZE];
		int n = Char_Traits<wchar>::from_i64(buffer, BUFFER64SIZE - 1, i64d);
		assert(n != -1 && n < BUFFER64SIZE - 1);
		buffer[n + 1] = L'\0';
		(*mImpl) << buffer;
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator<<(const uint64 ui64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) << ui64d;
#else
		wchar buffer[BUFFER64SIZE];
		int n = Char_Traits<wchar>::from_u64(buffer, BUFFER64SIZE - 1, ui64d);
		assert(n != -1 && n < BUFFER64SIZE - 1);
		buffer[n + 1] = L'\0';
		(*mImpl) << buffer;
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(WString& text)
	{
		text = this->getContent();
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(bool&	yesno)
	{
		(*mImpl) >> yesno;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(int32&	integer)
	{
		(*mImpl) >> integer;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(uint32& uinteger)
	{
		(*mImpl) >> uinteger;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(fp32& real)
	{
		(*mImpl) >> real;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(fp64& real)
	{
		(*mImpl) >> real;
		return *this;
	}


	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(void*& ptr)
	{
		(*mImpl) >> ptr;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(long& ld)
	{
		(*mImpl) >> ld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(unsigned long& uld)
	{
		(*mImpl) >> uld;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(short& sd)
	{
		(*mImpl) >> sd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(unsigned short& usd)
	{
		(*mImpl) >> usd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(char& cd)
	{
		short sd;
		(*mImpl) >> sd;
		cd = (char)sd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(unsigned char& ucd)
	{
		unsigned short usd;
		(*mImpl) >> usd;
		ucd = (unsigned char)usd;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(int64& i64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) >> i64d;
#else
		const WString& s = this->getContent();
		size_t bits = 0;
		while (bits < s.size() && s[bits] >= L'0' && s[bits] <= L'9')
			++bits;
		assert(bits < BUFFER64SIZE);
		wchar buffer[BUFFER64SIZE];
		std::memcpy(buffer, s.c_str(), bits*sizeof(wchar));
		buffer[bits + 1] = L'\0';
		i64d = Char_Traits<wchar>::to_i64(buffer, NULL, 10);
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(uint64& ui64d)
	{
#if BLADE_STDCPP >= BLADE_STDCPP11
		(*mImpl) >> ui64d;
#else
		const WString& s = this->getContent();
		size_t bits = 0;
		while (bits < s.size() && s[bits] >= L'0' && s[bits] <= L'9')
			++bits;
		assert(bits < BUFFER64SIZE);
		wchar buffer[BUFFER64SIZE];
		std::memcpy(buffer, s.c_str(), bits * sizeof(wchar));
		buffer[bits + 1] = L'\0';
		ui64d = Char_Traits<wchar>::to_u64(buffer, NULL, 10);
#endif
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringStream&	WStringStream::operator>>(wchar& wc)
	{
		(*mImpl) >> wc;
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringStream::clear()
	{
		mImpl->str( L"" );
		mImpl->clear();
		mImpl->mContent =  WStringStreamImpl::ContentType();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringStream::format(const wchar* fmt, ...)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i*sizeof(wchar) ) )
				return false;

			wchar* dest = (wchar*)buffer.getData();
			va_list v;
			va_start(v, fmt);

			int n = ::vswprintf( dest,i-1,fmt,v);
			if( n != -1 )
			{
				dest[n] = L'\0';
				break;
			}
		}

		this->clear();
		(*this) << (const wchar*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringStream::vformat(const wchar* fmt, va_list varg_ptr)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i*sizeof(wchar) ) )
				return false;

			wchar* dest = (wchar*)buffer.getData();

			int n = ::vswprintf( dest, i-1, fmt, varg_ptr);
			if( n != -1 )
			{
				dest[n] = L'\0';
				break;
			}
		}

		this->clear();
		(*this) << (const wchar*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringStream::appendFormat(const wchar* fmt,...)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i*sizeof(wchar) ) )
				return false;

			wchar* dest = (wchar*)buffer.getData();
			va_list v;
			va_start(v, fmt);

			int n = ::vswprintf( dest,i-1,fmt,v);
			if( n != -1 )
			{
				dest[n] = L'\0';
				break;
			}
		}
		(*this) << (const wchar*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			WStringStream::appendVFormat(const wchar* fmt, va_list varg_ptr)
	{
		//how to handle the buffer overflow..?
		TempBuffer buffer;

		for( size_t i = 1024;/*i < 4096*4096*/; i*=2 )
		{
			if ( !buffer.reserve( i*sizeof(wchar) ) )
				return false;

			wchar* dest = (wchar*)buffer.getData();

			int n = ::vswprintf( dest, i-1, fmt, varg_ptr);
			if( n != -1 )
			{
				dest[n] = L'\0';
				break;
			}
		}

		(*this) << (const wchar*)buffer.getData();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	int			WStringStream::scanFormat(const wchar* fmt,...)
	{
		if( mImpl->str().size() == 0 )
			return false;

		va_list v;
		va_start(v,fmt);

		return std::vswscanf(this->getContent().c_str(),fmt,v);
	}

	//////////////////////////////////////////////////////////////////////////
	int			WStringStream::scanFormatOffset(size_t offset,const wchar* fmt,...)
	{
		if( mImpl->str().size() < offset )
			return false;

		va_list v;
		va_start(v,fmt);
		return std::vswscanf(this->getContent().c_str()+offset,fmt,v);
	}

	//////////////////////////////////////////////////////////////////////////
	const WString&	WStringStream::getContent() const
	{
		mImpl->mContent = WStringStreamImpl::ContentType( mImpl->str().c_str() );
		return reinterpret_cast<const WString&>(mImpl->mContent);
	}


	//////////////////////////////////////////////////////////////////////////
	void			WStringStream::str(const wchar* src)
	{
		(*mImpl).str( src );
	}

	//////////////////////////////////////////////////////////////////////////
	void			WStringStream::str(const WString& src)
	{
		(*mImpl).str(L"");
		*this << src;
	}
	
}//namespace Blade