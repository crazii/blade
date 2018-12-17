/********************************************************************
	created:	2010/08/10
	filename: 	String.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "InternalStringHelper.h"

namespace Blade
{
	using namespace Impl;

	/************************************************************************/
	/* StringConcat                                                                    */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	StringConcat::StringConcat(const char* origin)
		:mImpl( BLADE_NEW StringConcatImpl() )
	{
		if( origin != NULL )
		{
			StringConcatImpl::string_type const_string(origin, traits_type::length(origin), StringConcatImpl::string_type::CONST_HINT);
			mImpl->add( const_string );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat::StringConcat(const string_type& origin)
		:mImpl( BLADE_NEW StringConcatImpl() )
	{
		mImpl->add( StringConcatImpl::string_type(origin.c_str(),origin.size(), StringConcatImpl::string_type::CONST_HINT) );
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+(const string_type& rhs)
	{
		mImpl->add( StringConcatImpl::string_type(rhs.c_str(),rhs.size(), StringConcatImpl::string_type::CONST_HINT) );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+(const char* rhs)
	{
		StringConcatImpl::string_type const_string(rhs, traits_type::length(rhs), StringConcatImpl::string_type::CONST_HINT);
		mImpl->add(const_string);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+(const Impl::Char rhs)
	{
		char data[2] = {rhs,0};
		StringConcatImpl::string_type stub;
		stub = data;
		mImpl->add( stub );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat operator+(const Impl::Char lhs, const StringConcat& rhs)
	{
		char data[2] = {lhs,0};
		StringConcatImpl::string_type stub;
		stub = data;
		StringConcat ret(NULL);
		ret.mImpl->add(stub);
		return ret + rhs;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat StringConcat::operator+(const StringConcat& rhs)
	{
		StringConcat ret(string_type::EMPTY);
		for(size_t i = 0; i < mImpl->size(); ++i )
			ret.mImpl->add( (*mImpl)[i] );

		for( size_t i = 0; i < rhs.mImpl->size(); ++i )
			ret.mImpl->add( (*rhs.mImpl)[i] );
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+=(const string_type& rhs)
	{
		mImpl->add( StringConcatImpl::string_type(rhs.c_str(), rhs.size(), StringConcatImpl::string_type::CONST_HINT) );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+=(const char* rhs)
	{
		StringConcatImpl::string_type const_string(rhs, traits_type::length(rhs), StringConcatImpl::string_type::CONST_HINT);
		mImpl->add(const_string);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+=(const Impl::Char rhs)
	{
		char data[2] = {rhs,0};
		StringConcatImpl::string_type stub;
		stub = data;
		mImpl->add(stub);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat& StringConcat::operator+=(const StringConcat& rhs)
	{
		for( size_t i = 0; i < rhs.mImpl->size(); ++i )
			mImpl->add( (*rhs.mImpl)[i] );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat::StringConcat(const StringConcat& src)
		:mImpl( BLADE_NEW StringConcatImpl() )
	{
		*mImpl = *src.mImpl;
	}

	//////////////////////////////////////////////////////////////////////////
	StringConcat::~StringConcat()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	size_t	StringConcat::size() const
	{
		return mImpl->count();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	StringConcat::total_bytes() const
	{
		return mImpl->total_size();
	}

	//////////////////////////////////////////////////////////////////////////
	const StringConcat::string_type& StringConcat::at(index_t index) const
	{
		return reinterpret_cast<const string_type&>( mImpl->at(index) );
	}

	/************************************************************************/
	/* WStringConcat                                                                    */
	/************************************************************************/

	//////////////////////////////////////////////////////////////////////////
	WStringConcat::WStringConcat(const wchar* origin)
		:mImpl( BLADE_NEW WStringConcatImpl() )
	{
		if( origin != NULL )
		{
			WStringConcatImpl::string_type const_string(origin, traits_type::length(origin), WStringConcatImpl::string_type::CONST_HINT);
			mImpl->add( const_string );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat::WStringConcat(const string_type& origin)
		:mImpl( BLADE_NEW WStringConcatImpl() )
	{
		mImpl->add( WStringConcatImpl::string_type(origin.c_str(),origin.size(), WStringConcatImpl::string_type::CONST_HINT) );
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+(const string_type& rhs)
	{
		mImpl->add( WStringConcatImpl::string_type(rhs.c_str(), rhs.size(), WStringConcatImpl::string_type::CONST_HINT) );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+(const wchar* rhs)
	{
		WStringConcatImpl::string_type const_string(rhs, traits_type::length(rhs), WStringConcatImpl::string_type::CONST_HINT);
		mImpl->add(const_string);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+(const Impl::WChar rhs)
	{
		wchar data[2] = {rhs,0};
		WStringConcatImpl::string_type stub;
		stub = data;
		mImpl->add(stub);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat operator+(const Impl::WChar lhs, const WStringConcat& rhs)
	{
		wchar data[2] = {lhs,0};
		WStringConcatImpl::string_type stub;
		stub = data;
		WStringConcat ret(NULL);
		ret.mImpl->add(stub);
		return ret + rhs;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat WStringConcat::operator+(const WStringConcat& rhs)
	{
		WStringConcat ret(string_type::EMPTY);
		for(size_t i = 0; i < mImpl->size(); ++i )
			ret.mImpl->add( (*mImpl)[i] );

		for( size_t i = 0; i < rhs.mImpl->size(); ++i )
			ret.mImpl->add( (*rhs.mImpl)[i] );
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+=(const string_type& rhs)
	{
		mImpl->add( WStringConcatImpl::string_type(rhs.c_str(),rhs.size(), WStringConcatImpl::string_type::CONST_HINT) );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+=(const wchar* rhs)
	{
		WStringConcatImpl::string_type const_string(rhs,traits_type::length(rhs), WStringConcatImpl::string_type::CONST_HINT);
		mImpl->add(const_string);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+=(const Impl::WChar rhs)
	{
		wchar data[2] = {rhs, 0};
		WStringConcatImpl::string_type stub;
		stub = data;
		mImpl->add(stub);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat& WStringConcat::operator+=(const WStringConcat& rhs)
	{
		for( size_t i = 0; i < rhs.mImpl->size(); ++i )
			mImpl->add( (*rhs.mImpl)[i] );
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat::WStringConcat(const WStringConcat& src)
		:mImpl( BLADE_NEW WStringConcatImpl() )
	{
		*mImpl = *src.mImpl;
	}

	//////////////////////////////////////////////////////////////////////////
	WStringConcat::~WStringConcat()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	size_t	WStringConcat::size() const
	{
		return mImpl->count();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	WStringConcat::total_bytes() const
	{
		return mImpl->total_size();
	}

	//////////////////////////////////////////////////////////////////////////
	const WStringConcat::string_type& WStringConcat::at(index_t index) const
	{
		return reinterpret_cast<const string_type&>( mImpl->at(index) );
	}


#if BLADE_COMPILER_GNU_EXTENSION
	TLiteralHelper::TLiteralHelper(const String& str)
		:mString( StringConverter::StringToTString(str) )
	{

	}
#endif


	/************************************************************************/
	/* StringConverter                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	WString			StringConverter::StringToWString(const char* mbcString, size_t len/* = 0*/)
	{
		if (len == 0)
			len = ::strlen(mbcString);
		if (len == 0)
			return WString();

		TempBuffer	buffer;
		buffer.reserve((len + 1) * sizeof(wchar));
		wchar* dest = (wchar*)buffer.getData();
#if BLADE_UNICODE
		size_t count = CodeConverter::OtherStringToTString<char>(mbcString, len, dest, buffer.getCapacity());
#else
		size_t count = CodeConverter::TStringToOtherString<wchar>(mbcString, len, dest, buffer.getCapacity());
#endif
		assert(count != size_t(-1));
		assert(count + sizeof(wchar) <= buffer.getCapacity());
		//code converter doesn't add \0, we need to append it
		dest[count] = wchar();
		return WString((wchar*)buffer.getData());
	}


	//////////////////////////////////////////////////////////////////////////
	String			StringConverter::WStringToString(const wchar* wcString, size_t len/* = 0*/)
	{
		if (len == 0)
			len = ::wcslen(wcString);
		if (len == 0)
			return String();

		TempBuffer	buffer;
		buffer.reserve(len + 1);
		char* dest = (char*)buffer.getData();
#if BLADE_UNICODE
		size_t count = CodeConverter::TStringToOtherString<char>(wcString, len, dest, buffer.getCapacity());
#else
		size_t count = CodeConverter::OtherStringToTString<wchar>(wcString, len, dest, buffer.getCapacity());
#endif
		assert(count + 1 <= buffer.getCapacity());
		dest[count] = char();
		return String((char*)buffer.getData());
	}

	//////////////////////////////////////////////////////////////////////////
	String		StringConverter::StringToUTF8String(const char* mbcString, size_t len/* = 0*/)
	{
		if (len == 0)
			len = ::strlen(mbcString);

		TempBuffer	buffer;
		buffer.reserve((len + 1) * 3);
		char* utf8Dest = (char*)buffer.getData();
		size_t count = CodeConverter::StringtoUTF8<char>(mbcString, len, utf8Dest, buffer.getCapacity());
		assert(count + 1 <= buffer.getCapacity());
		utf8Dest[count] = char();
		return String((const char*)buffer.getData());
	}

	//////////////////////////////////////////////////////////////////////////
	String		StringConverter::WStringToUTF8String(const wchar* wcString, size_t len/* = 0*/)
	{
		if (len == 0)
			len = ::wcslen(wcString);

		TempBuffer	buffer;
		buffer.reserve((len + 1) * 3);
		char* utf8Dest = (char*)buffer.getData();
		size_t count = CodeConverter::StringtoUTF8<wchar>(wcString, len, utf8Dest, buffer.getCapacity());
		assert(count + 1 <= buffer.getCapacity());
		utf8Dest[count] = char();
		return String((const char*)buffer.getData());
	}

	//////////////////////////////////////////////////////////////////////////
	String		StringConverter::UTF8StringToString(const char* utf8String, size_t bytes/* = 0*/)
	{
		if (bytes == 0)
			bytes = ::strlen(utf8String);

		TempBuffer	buffer;
		buffer.reserve((bytes + 1));
		char* dest = (char*)buffer.getData();
		size_t count = CodeConverter::UTF8toString<char>(utf8String, bytes, dest, buffer.getCapacity());
		dest[count] = char();
		assert(count + 1 <= buffer.getCapacity());
		return String((const char*)buffer.getData());
	}

	//////////////////////////////////////////////////////////////////////////
	WString		StringConverter::UTF8StringToWString(const char* utf8String, size_t bytes/* = 0*/)
	{
		if (bytes == 0)
			bytes = ::strlen(utf8String);

		TempBuffer	buffer;
		buffer.reserve((bytes + 1) * sizeof(wchar));
		wchar* dest = (wchar*)buffer.getData();
		size_t count = CodeConverter::UTF8toString<wchar>(utf8String, bytes, dest, buffer.getCapacity() / sizeof(wchar));
		assert(count + 1 <= buffer.getCapacity() / sizeof(wchar));
		dest[count] = wchar();
		return WString((const wchar*)buffer.getData());
	}
	
}//namespace Blade


#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID && !_GLIBCXX_USE_C99

//////////////////////////////////////////////////////////////////////////
long long wcstoll(const wchar_t* begwstr, wchar_t** endwstr, int base)
{
	Blade::TempBuffer buffer;
	char* cend;
	char** pcend = NULL;
	const char* cstart = Blade::StringConverter::WStringToString(buffer, begwstr, 0);
	if( endwstr != NULL )
	{
		cend = ( (char*)cstart + (*endwstr - begwstr) );
		pcend = &cend;
	}
	return strtoll(cstart, pcend, base);
}

//////////////////////////////////////////////////////////////////////////
unsigned long long wcstoull(const wchar_t* begwstr, wchar_t** endwstr, int base)
{
	Blade::TempBuffer buffer;
	char* cend;
	char** pcend = NULL;
	const char* cstart = Blade::StringConverter::WStringToString(buffer, begwstr, 0);
	if( endwstr != NULL )
	{
		cend = ( (char*)cstart + (*endwstr - begwstr) );
		pcend = &cend;
	}
	return strtoull(cstart, pcend, base);
}
#endif

#if BLADE_STDCPP < BLADE_STDCPP11
#include <stdarg.h>
using namespace Blade;

namespace std
{
#if (defined(_MSC_VER) && (_MSC_VER < 1800/*before VS2013*/))
int vsscanf(const char* strbuffer, const char* fmt, va_list varg_ptr)
{
#if	BLADE_COMPILER == BLADE_COMPILER_MSVC

#	if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
	//note: this piece of code is from: http://www.flipcode.com/archives/vsscanf_for_Win32.shtml
	//this is quite simple implementation by copy the stack and call the std sscanf function

	size_t count = 0;
	{
		const char *p = fmt;
		while (true)
		{
			char c = *(p++);
			if (c == 0)
				break;
			if (c == '%' && (p[0] != '*' && p[0] != '%')) ++count;
		}
	}
	if (count == 0)
		return 0;

#		if BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT

	//the original code has a bug : the stack space is not enough for function calls
	const int ADDITIONAL_STACK_SIZE = 512;
	const int MINIMAL_STACK_SIZE = 1024;
	// Make a local stack
	size_t stackSize = (2 + count) + ADDITIONAL_STACK_SIZE < MINIMAL_STACK_SIZE ? MINIMAL_STACK_SIZE : (2 + count) + ADDITIONAL_STACK_SIZE;
	void **newStack = (void**)BLADE_TMP_ALLOC(stackSize * sizeof(void*));
	if (newStack == NULL)
	{
		assert(false);
		return 0;
	}

	// Fill local stack the way sscanf likes it
	void** stackTop = newStack + stackSize - count;
	std::memcpy(stackTop, varg_ptr, count * sizeof(void*));
	*(--stackTop) = (void*)fmt;
	*(--stackTop) = (void*)strbuffer;

	void* savedESP;

	int result;
	__asm
	{
		mov     savedESP, esp;
		mov     esp, stackTop;
#			if _DLL	//MD or MDd
		call    dword ptr[sscanf];
#			else		//MT or MTd
		call	sscanf;
#			endif
		mov     esp, savedESP;
		mov     result, eax;
	}
	BLADE_TMP_FREE(newStack);
	return result;

#		else//BLADE_ARCHITECTURE
	//HACK for C++98
	#define MAX_COUNT 64
	if (count > MAX_COUNT)
		BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("buffer overrun."));

	size_t bytes = sizeof(void*) * MAX_COUNT;
	void** ptr = (void**)BLADE_TMP_ALLOC(bytes);
	std::memset(ptr, 0, bytes);

	for (size_t i = 0; i < count; ++i)
		ptr[i] = va_arg(varg_ptr, void*);

	int scanCount = sscanf(strbuffer, fmt, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
		ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15],
		ptr[16], ptr[17], ptr[18], ptr[19], ptr[20], ptr[21], ptr[22], ptr[23],
		ptr[24], ptr[25], ptr[26], ptr[27], ptr[28], ptr[29], ptr[30], ptr[31],
		ptr[32], ptr[33], ptr[34], ptr[35], ptr[36], ptr[37], ptr[38], ptr[39],
		ptr[40], ptr[41], ptr[42], ptr[43], ptr[44], ptr[45], ptr[46], ptr[47],
		ptr[48], ptr[49], ptr[50], ptr[51], ptr[52], ptr[53], ptr[54], ptr[55],
		ptr[56], ptr[57], ptr[58], ptr[59], ptr[60], ptr[61], ptr[62], ptr[63]);

	assert(scanCount <= (int)count);
	BLADE_TMP_FREE(ptr);

	return scanCount;
#		endif//BLADE_ARCHITECTURE

#	else//BLADE_PLATFORM
	//MSVC compiler on Non Window desktop target?
#		error not implemented.
	return 0;
#	endif

#else//BLADE_COMPILER
#	error not implemented.
	return 0;
#endif//BLADE_COMPILER
}
#endif

#if (defined(_MSC_VER) && (_MSC_VER < 1800/*before VS2013*/)) || (BLADE_COMPILER_GNU_EXTENSION && !_GLIBCXX_HAVE_VSWSCANF)
int vswscanf(const wchar_t* strbuffer, const wchar_t* fmt, va_list varg_ptr)
{
#if	BLADE_COMPILER == BLADE_COMPILER_MSVC || BLADE_COMPILER_GNU_EXTENSION

	//note: this piece of code is from: http://www.flipcode.com/archives/vsscanf_for_Win32.shtml
	//this is quite simple implementation by copy the stack and call the std sscanf function

	size_t count = 0;
	{
		const wchar *p = fmt;
		while (true)
		{
			wchar c = *(p++);
			if (c == 0)
				break;
			if (c == L'%' && (p[0] != L'*' && p[0] != L'%')) ++count;
		}
	}
	if (count == 0)
		return 0;

#	if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS && BLADE_ARCHITECTURE == BLADE_ARCHITECTURE_32BIT && BLADE_COMPILER == BLADE_COMPILER_MSVC
	//the original code has a bug : the stack space is not enough for function calls
	const int ADDITIONAL_STACK_SIZE = 512 * 8;
	const int MINIMAL_STACK_SIZE = 1024 * 16;
	// Make a local stack
	size_t stackSize = (2 + count) + ADDITIONAL_STACK_SIZE < MINIMAL_STACK_SIZE ? MINIMAL_STACK_SIZE : (2 + count) + ADDITIONAL_STACK_SIZE;
	void **newStack = (void**)BLADE_TMP_ALLOC(stackSize * sizeof(void*));
	if (newStack == NULL)
	{
		assert(false);
		return 0;
	}

	// Fill local stack the way sscanf likes it
	void** stackTop = newStack + stackSize - count;
	std::memcpy(stackTop, varg_ptr, count * sizeof(void*));
	*(--stackTop) = (void*)fmt;
	*(--stackTop) = (void*)strbuffer;

	void* savedESP;

	int result;
	__asm
	{
		mov     savedESP, esp;
		mov     esp, stackTop;
#		if _DLL	//MD or MDd
		call    dword ptr[swscanf];
#		else		//MT or MTd
		call	swscanf;
#		endif
		mov     esp, savedESP;
		mov     result, eax;
	}
	BLADE_TMP_FREE(newStack);
	return result;

#	else //BLADE_PLATFORM & BLADE_ARCHITECTURE - x64 or android
	//TODO: swscanf on android is not working properly

	//HACK for C++98
#define MAX_COUNT 64
	if (count > MAX_COUNT)
		BLADE_EXCEPT(EXC_INTERNAL_ERROR, BTString("buffer overrun."));

	size_t bytes = sizeof(void*) * MAX_COUNT;
	void** ptr = (void**)BLADE_TMP_ALLOC(bytes);
	std::memset(ptr, 0, bytes);

	for (size_t i = 0; i < count; ++i)
		ptr[i] = va_arg(varg_ptr, void*);

	int scanCount = swscanf(strbuffer, fmt, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
		ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15],
		ptr[16], ptr[17], ptr[18], ptr[19], ptr[20], ptr[21], ptr[22], ptr[23],
		ptr[24], ptr[25], ptr[26], ptr[27], ptr[28], ptr[29], ptr[30], ptr[31],
		ptr[32], ptr[33], ptr[34], ptr[35], ptr[36], ptr[37], ptr[38], ptr[39],
		ptr[40], ptr[41], ptr[42], ptr[43], ptr[44], ptr[45], ptr[46], ptr[47],
		ptr[48], ptr[49], ptr[50], ptr[51], ptr[52], ptr[53], ptr[54], ptr[55],
		ptr[56], ptr[57], ptr[58], ptr[59], ptr[60], ptr[61], ptr[62], ptr[63]);

	assert(scanCount <= (int)count);
	BLADE_TMP_FREE(ptr);

	return scanCount;
#	endif

#else//BLADE_COMPILER
#	error not implemented.
	return 0;
#endif//BLADE_COMPILER
}
#endif

}//namespace std
#endif//BLADE_STDCPP < BLADE_STDCPP11