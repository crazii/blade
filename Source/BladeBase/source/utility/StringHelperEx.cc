/********************************************************************
	created:	2013/12/26
	filename: 	StringHelperEx.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	const wchar*			StringConverter::StringToWString(TempBuffer& buffer, const char* mbcString,size_t len/* = 0*/, size_t* count/* = NULL*/)
	{
		size_t localCount = 0;
		if( count == NULL )
			count = &localCount;

		if( len == 0 )
			len = ::strlen(mbcString);
		if( len == 0 )
		{
			*count = 0;
			return WString::EMPTY.c_str();
		}

		buffer.reserve( (len+1)*sizeof(wchar) );
		wchar* dest = (wchar*)buffer.getData();
#if BLADE_UNICODE
		*count = CodeConverter::OtherStringToTString<char>(mbcString,len,(wchar*)buffer.getData(),buffer.getCapacity() );
#else
		*count = CodeConverter::TStringToOtherString<wchar>(mbcString,len,(wchar*)buffer.getData(),buffer.getCapacity() );
#endif
		assert( *count + 1 <= buffer.getCapacity() );
		dest[*count] = wchar();
		return (wchar*)buffer.getData();
	}

	//////////////////////////////////////////////////////////////////////////
	const char*			StringConverter::WStringToString(TempBuffer& buffer, const wchar* wcString,size_t len/* = 0*/, size_t* count/* = NULL*/)
	{
		size_t localCount = 0;
		if( count == NULL )
			count = &localCount;

		if( len == 0 )
			len = ::wcslen(wcString);
		if( len == 0 )
		{
			*count = 0;
			return String::EMPTY.c_str();
		}

		buffer.reserve( len+1 );
		char* dest = (char*)buffer.getData();
#if BLADE_UNICODE
		*count = CodeConverter::TStringToOtherString<char>(wcString,len,(char*)buffer.getData(),buffer.getCapacity() );
#else
		*count = CodeConverter::OtherStringToTString<wchar>(wcString,len,(char*)buffer.getData(),buffer.getCapacity() );
#endif
		assert( *count + 1 <= buffer.getCapacity() );
		dest[ *count ] = char();
		return (char*)buffer.getData();
	}

	//////////////////////////////////////////////////////////////////////////
	const char*		StringConverter::StringToUTF8String(TempBuffer& buffer, const char* mbcString, size_t len/* = 0*/, size_t* count/* = NULL*/)
	{
		size_t localCount = 0;
		if( count == NULL )
			count = &localCount;

		if( len == 0)
			len = ::strlen(mbcString);

		buffer.reserve( (len+1)*3 );
		char* dest = (char*)buffer.getData();
		*count = CodeConverter::StringtoUTF8<char>(mbcString, len, (char*)buffer.getData(), buffer.getCapacity() );
		assert( *count + 1 <= buffer.getCapacity() );
		dest[ *count ] = char();
		return (const char*)buffer.getData();
	}

	//////////////////////////////////////////////////////////////////////////
	const char*		StringConverter::WStringToUTF8String(TempBuffer& buffer, const wchar* wcString, size_t len/* = 0*/, size_t* count/* = NULL*/)
	{
		size_t localCount = 0;
		if( count == NULL )
			count = &localCount;

		if( len == 0)
			len = ::wcslen(wcString);

		buffer.reserve( (len+1)*3 );
		char* dest = (char*)buffer.getData();
		*count = CodeConverter::StringtoUTF8<wchar>(wcString, len, (char*)buffer.getData(), buffer.getCapacity() );
		assert( *count + 1 <= buffer.getCapacity() );
		dest[ *count ] = char();
		return (const char*)buffer.getData();
	}

	//////////////////////////////////////////////////////////////////////////
	const char*		StringConverter::UTF8StringToString(TempBuffer& buffer, const char* utf8String, size_t bytes/* = 0*/, size_t* count/* = NULL*/)
	{
		size_t localCount = 0;
		if( count == NULL )
			count = &localCount;

		if( bytes == 0)
			bytes = ::strlen(utf8String);

		buffer.reserve( (bytes+1) );
		char* dest = (char*)buffer.getData();
		*count = CodeConverter::UTF8toString<char>(utf8String, bytes, (char*)buffer.getData(), buffer.getCapacity() );
		assert( *count + 1 <= buffer.getCapacity() );
		dest[ *count ] = char();
		return (const char*)buffer.getData();
	}

	//////////////////////////////////////////////////////////////////////////
	const wchar*		StringConverter::UTF8StringToWString(TempBuffer& buffer, const char* utf8String, size_t bytes/* = 0*/, size_t* count/* = NULL*/)
	{
		size_t localCount = 0;
		if( count == NULL )
			count = &localCount;

		if( bytes == 0)
			bytes = ::strlen(utf8String);

		buffer.reserve( (bytes+1)*sizeof(wchar) );
		wchar* dest = (wchar*)buffer.getData();
		*count = CodeConverter::UTF8toString<wchar>(utf8String, bytes, (wchar*)buffer.getData(), buffer.getCapacity()/sizeof(wchar) );
		assert( *count + 1<= buffer.getCapacity()/sizeof(wchar) );
		dest[*count] = wchar();
		return (const wchar*)buffer.getData();
	}
	
}//namespace Blade