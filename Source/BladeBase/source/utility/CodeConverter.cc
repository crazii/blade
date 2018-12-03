/********************************************************************
	created:	2009/04/12
	filename: 	CodeConverter.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "InternalString.h"
#include "Base64.h"
#include "BladeUnicode.h"
#include "../interface_imp/platform/PlatformData_Private.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4819)		//character out of current code page
#pragma warning(disable:4127)		//while(true)
#pragma warning( disable:4996)		//CRT deprecated
#endif

//some platform(i.e. android) 's C lib don't have wide char version functions (wcslen, mbstowcs, etc.)
//so we have to use iconv
#define USE_ICONV 0		//iconv is GPL and will affect Blade's liberal license, disable it.

#if USE_ICONV
#include <iconv.h>
#endif

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	const String& CodeConverter::GetCodePage()
	{
		assert( !PlatformData_Private::getSingleton().mCodePage.empty() && "plz initialize PlatformData on startup" );
		return PlatformData_Private::getSingleton().mCodePage;
	}

	/************************************************************************/
	/* UTF conversions                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t CodeConverter::UTF8toMBCS(const char *utf8buffer, size_t src_bytes, char *mbcsbuffer, size_t dest_size)
	{
#if !USE_ICONV
		TempBuffer tempbuffer;
		//well, this is difficult to calculate the destination char count from UTF-8,\n
		//so make it as the same code count, and that is big enough.
		tempbuffer.reserve( (src_bytes+1) * sizeof(wchar) );
		wchar* wcsbuffer = static_cast<wchar*>( tempbuffer.getData() );

		//first,try to convert utf8 to wcs
		size_t size = UTF8toWCS(utf8buffer, src_bytes, wcsbuffer, tempbuffer.getCapacity() );

		if(  size == -1 )
			return (size_t)-1;

		//then convert wcs to mbcs
		size = std::wcstombs(mbcsbuffer,wcsbuffer,dest_size);
		if(  size == -1 )
			return (size_t)-1;

		mbcsbuffer[size] = char('\0');
		return size;
#else
		size_t size = CodeConverter::CodeConvert(TE_UTF8, TE_MBCS, utf8buffer, src_bytes, mbcsbuffer, dest_size);
		if(  size == INVALID_INDEX )
			return size;
		mbcsbuffer[size] = char('\0');
		return size;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CodeConverter::UTF8toWCS(const char* utf8buffer,size_t src_bytes, wchar* wcsbuffer, size_t dest_size)
	{
#if !USE_ICONV
		ConversionResult retval;
		wchar* pWchar = wcsbuffer;
		const UTF8 * pUtf8 = (const UTF8 *) utf8buffer;

		if( sizeof(wchar) == 2 )
		{	
			retval = ::ConvertUTF8toUTF16(
				&pUtf8, pUtf8 + src_bytes,
				(UTF16**)&pWchar, (UTF16*)pWchar + dest_size,
				lenientConversion);
		}
		else if( sizeof(wchar) == 4 )
		{
			retval = ::ConvertUTF8toUTF32(
				&pUtf8, pUtf8 + src_bytes,
				(UTF32**)&pWchar, (UTF32*)pWchar + dest_size,
				lenientConversion);
		}

		if (retval != conversionOK )
			return (size_t)-1;
		else
		{
			//end of string
			*pWchar = 0;
			return  size_t(pWchar - wcsbuffer);
		}
#else
		TEXT_ENCODING target = sizeof(wchar) == 2 ? TE_UTF16 : TE_UTF32;
		size_t size = CodeConverter::CodeConvert(TE_UTF8, target, utf8buffer, src_bytes, (char*)wcsbuffer, dest_size*sizeof(wchar)) / sizeof(wchar);
		if(  size == INVALID_INDEX )
			return size;
		wcsbuffer[size] = wchar('\0');
		return size;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CodeConverter::MBCStoUTF8(const char* mbcsbuffer,size_t src_size,char* utf8buffer,size_t dest_bytes)
	{
#if !USE_ICONV
		TempBuffer tempbuffer;

		size_t size = std::mbstowcs(NULL,mbcsbuffer,src_size);

		tempbuffer.reserve( (size+1)*sizeof(wchar) );
		wchar* wcsbuffer = static_cast<wchar*>( tempbuffer.getData() );

		size = std::mbstowcs(wcsbuffer,mbcsbuffer,size);
		wcsbuffer[size] = wchar('\0');

		size = WCStoUTF8(wcsbuffer,size+1,utf8buffer, dest_bytes);

		return size;
#else
		return CodeConverter::CodeConvert(TE_MBCS, TE_UTF8, mbcsbuffer, src_size, utf8buffer, dest_bytes);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CodeConverter::WCStoUTF8(const wchar* wcsbuffer,size_t src_size,char* utf8buffer, size_t dest_bytes)
	{
#if !USE_ICONV
		ConversionResult retval;
		const wchar* pWchar = wcsbuffer;
		UTF8* pUtf8 = (UTF8*) utf8buffer;
		if( sizeof(wchar) == 2 )
		{
			retval = ::ConvertUTF16toUTF8(
				(const UTF16**)&pWchar, (const UTF16*)pWchar + src_size,
				&pUtf8, pUtf8 + dest_bytes,
				lenientConversion);
		}
		else if( sizeof(wchar) == 4 )
		{
			retval = ::ConvertUTF32toUTF8(
				(const UTF32**)&pWchar, (const UTF32*)pWchar + src_size,
				&pUtf8, pUtf8 + dest_bytes,
				lenientConversion);
		}

		if (retval != conversionOK )
			return (size_t)-1;
		else
		{
			size_t len = size_t((char*)pUtf8 - utf8buffer);
			if( len < dest_bytes )
				*pUtf8 = char();
			return len;
		}
#else
		TEXT_ENCODING srcEncoding = sizeof(wchar) == 2 ? TE_UTF16 : TE_UTF32;
		return CodeConverter::CodeConvert(srcEncoding, TE_UTF8, (char*)wcsbuffer, src_size*sizeof(wchar), utf8buffer, dest_bytes);
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	CodeConverter::StringToTString(const char* srcbuffer,size_t src_size,tchar* tcbuffer,size_t dest_size)
	{
#if !BLADE_UNICODE
		BLADE_UNREFERENCED(srcbuffer);
		BLADE_UNREFERENCED(src_size);
		BLADE_UNREFERENCED(tcbuffer);
		BLADE_UNREFERENCED(dest_size);
		BLADE_EXCEPT(EXC_REDUPLICATE,BTString("string conversion between reduplicate types:")+TEXT("char.") );
#else
		if( ::isLegalUTF8Sequence( (const UTF8*)srcbuffer, (const UTF8*)srcbuffer+src_size) )
			return UTF8toWCS(srcbuffer, src_size, tcbuffer, dest_size);
		else
		{
#if	!USE_ICONV
			size_t size = std::mbstowcs(NULL,srcbuffer,src_size);

			if( tcbuffer == NULL )
				return size;
			else if( size > dest_size )
				return (size_t)-1;

			size = std::mbstowcs(tcbuffer,srcbuffer,size);
			tcbuffer[size] = tchar();
			return size;
#	else
			TEXT_ENCODING dstEncoding = sizeof(wchar) == 2 ? TE_UTF16 : TE_UTF32;
			return CodeConverter::CodeConvert(TE_MBCS, dstEncoding, (char*)srcbuffer, src_size, (char*)tcbuffer, dest_size*sizeof(tchar) ) / sizeof(tchar);
#	endif
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	CodeConverter::WStringToTString(const wchar* srcbuffer, size_t src_size, tchar* tcbuffer,size_t dest_size)
	{
#if BLADE_UNICODE
		BLADE_UNREFERENCED(srcbuffer);
		BLADE_UNREFERENCED(src_size);
		BLADE_UNREFERENCED(tcbuffer);
		BLADE_UNREFERENCED(dest_size);
		BLADE_EXCEPT(EXC_REDUPLICATE,BTString("string conversion between reduplicate types:")+TEXT("wchar_t."));
#else
#	if !USE_ICONV
		size_t size = std::wcstombs( NULL, srcbuffer, dest_size);
		if( tcbuffer == NULL )
			return size;
		else if( size > dest_size )
			return -1;

		size = std::wcstombs(tcbuffer,srcbuffer,size);
		tcbuffer[size] = tchar();

		return size;
#	else
		TEXT_ENCODING srcEncoding = sizeof(wchar) == 2 ? TE_UTF16 : TE_UTF32;
		return CodeConverter::CodeConvert(srcEncoding, TE_MBCS, (char*)srcbuffer, src_size*sizeof(wchar), (char*)tcbuffer, dest_size );
#	endif
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	CodeConverter::TStringToString(const tchar* tcbuffer, size_t src_size, char* destbuffer, size_t dest_size)
	{
#if !BLADE_UNICODE
		BLADE_UNREFERENCED(tcbuffer);
		BLADE_UNREFERENCED(src_size);
		BLADE_UNREFERENCED(destbuffer);
		BLADE_UNREFERENCED(dest_size);
		BLADE_EXCEPT(EXC_REDUPLICATE,BTString("string conversion between reduplicate types:")+TEXT("char.") );
#else
#	if !USE_ICONV
		BLADE_UNREFERENCED(src_size);

		size_t size = std::wcstombs(NULL, tcbuffer, dest_size);
		if( destbuffer == NULL )
			return size;
		else if( size > dest_size )
			return (size_t)-1;

		size = std::wcstombs(destbuffer,tcbuffer,dest_size);
		destbuffer[size] = char();
		return size;
#else
		TEXT_ENCODING srcEncoding = sizeof(tchar) == 2 ? TE_UTF16 : TE_UTF32;
		return CodeConverter::CodeConvert(srcEncoding, TE_MBCS, (const char*)tcbuffer, src_size*sizeof(tchar), (char*)destbuffer, dest_size );
#endif
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	CodeConverter::TStringToWString(const tchar* tcbuffer,size_t src_size, wchar* destbuffer,size_t dest_size)
	{
#if BLADE_UNICODE
		BLADE_UNREFERENCED(tcbuffer);
		BLADE_UNREFERENCED(src_size);
		BLADE_UNREFERENCED(destbuffer);
		BLADE_UNREFERENCED(dest_size);
		BLADE_EXCEPT(EXC_REDUPLICATE,BTString("string conversion between reduplicate types:")+TEXT("wchar_t."));
#else
		if( ::isLegalUTF8Sequence( (const UTF8*)tcbuffer, (const UTF8*)tcbuffer+src_size) )
			return UTF8toWCS(tcbuffer, src_size, destbuffer, dest_size);
		else
		{
#	if !USE_ICONV
			size_t size = std::mbstowcs(NULL,tcbuffer,src_size);

			if( destbuffer == NULL )
				return size;
			else if( size < dest_size )
				return -1;

			size = std::mbstowcs(destbuffer,tcbuffer,src_size);
			return size;
#else
			TEXT_ENCODING dstEncoding = sizeof(wchar) == 2 ? TE_UTF16 : TE_UTF32;
			return CodeConverter::CodeConvert(TE_MBCS, dstEncoding, (char*)tcbuffer, src_size, (char*)destbuffer, dest_size*sizeof(wchar) ) / sizeof(wchar);
#endif
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	size_t	CodeConverter::CodeConvert(TEXT_ENCODING from, TEXT_ENCODING to, const char* source, size_t sourceBytes, char* dest, size_t destBytes)
	{
#if USE_ICONV
		if( from >= TE_START && from < TE_COUNT && to >= TE_START && to < TE_COUNT && from != to )
		{
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4640)
#endif
			static const char* CHAR_SETS[] = 
			{
				CodeConverter::GetCodePage().c_str(),				//mbcs - use locale dependent conversion
				"UTF-8",
				"UTF-16LE",
				"UTF-16BE",
				"UTF-32LE",
				"UTF-32BE",
			};

			iconv_t cd = ::iconv_open(CHAR_SETS[to], CHAR_SETS[from]);
			if( cd != 0 )
			{
				const char* in = source;
				char* out = dest;
				const char **pin = &in;
				char **pout = &out;
				size_t leftBytes = destBytes;

				size_t ret = ::iconv(cd, pin, &sourceBytes, pout, &leftBytes);
				::iconv_close(cd);

				if( ret == size_t(-1) )
				{
					assert(false);
					return ret;
				}
				size_t size = destBytes - leftBytes;
				assert( out - dest ==  (ptrdiff_t)size);
				//std::memset(out+1, 0, std::min(sizeof(int), leftBytes-1) );
				return size;
			}
			else
			{
				assert(false);
				return size_t(-1);
			}
		}
		else
		{
			assert(false);
			return size_t(-1);
		}
#else
		//TODO: deprecate legacy MBCS.
		ConversionResult result = sourceIllegal;
		char* olddest = dest;
		if (to == TE_UTF8)
		{
			switch (from)
			{
			case TE_UTF16:
				result = ::ConvertUTF16toUTF8((const UTF16**)&source, (const UTF16*)(source + sourceBytes), (UTF8**)&dest, (UTF8*)(dest + destBytes), lenientConversion);
				break;
			case TE_UTF32:
				result = ::ConvertUTF32toUTF8((const UTF32**)&source, (const UTF32*)(source + sourceBytes), (UTF8**)&dest, (UTF8*)(dest + destBytes), lenientConversion);
				break;
			default:
				break;
			}
		}
		else if (from == TE_UTF8)
		{
			switch (to)
			{
			case TE_UTF16:
				result = ::ConvertUTF8toUTF16((const UTF8**)&source, (const UTF8*)(source + sourceBytes), (UTF16**)&dest, (UTF16*)(dest + destBytes), lenientConversion);
				break;
			case TE_UTF32:
				result = ::ConvertUTF8toUTF32((const UTF8**)&source, (const UTF8*)(source + sourceBytes), (UTF32**)&dest, (UTF32*)(dest + destBytes), lenientConversion);
				break;
			default:
				break;
			}
		}
		else if (from == TE_UTF16 && to == TE_UTF32)
			result = ::ConvertUTF16toUTF32((const UTF16**)&source, (const UTF16*)(source + sourceBytes), (UTF32**)&dest, (UTF32*)(dest + destBytes), lenientConversion);
		else if (from == TE_UTF32 && to == TE_UTF16)
			result = ::ConvertUTF32toUTF16((const UTF32**)&source, (const UTF32*)(source + sourceBytes), (UTF16**)&dest, (UTF16*)(dest + destBytes), lenientConversion);
		else
			assert(false && "not implemeneted!" );

		assert(result == conversionOK);
		assert(dest >= olddest);
		return (size_t)(dest - olddest);
#endif
	}


	/************************************************************************/
	/* Base64 encoding                                                                     */
	/************************************************************************/
	//if outString == NULL, return the string length (count in characters)
	//if outString != NULL, return converted character count, or -1 if error occurs
	size_t	CodeConverter::Base64Encode(const void* data, size_t src_bytes, tchar* outString, size_t len)
	{
		InternalStringStream inStream;
		inStream.write( (const char*)data, (std::streamsize)src_bytes );

		InternalStringStream outStream;
		if( ::b64_encode(inStream, outStream) != 0 )
			return size_t(-1);

		InternalString buffString = outStream.str();
		const size_t count = buffString.size();
		const char* buffer = buffString.c_str();
		if( outString == NULL )
			return count;
		else if( count > len )
			return size_t(-1);
		else
		{
#if BLADE_UNICODE
			//CodeConverter::OtherStringToTString<char>(buffer, count, outString, len);
			//because base64 uses only ANSI code, (no locale related)
			//we can transform it directly
			for(size_t i = 0; i < count; ++i)
				outString[i] = (tchar)buffer[i];
			outString[count] = tchar();
#else
			Char_Traits<tchar>::copy(outString, buffer, count);
#endif
			return  count;
		}
	}

	//data == NULL, return actual bytes needed for data,\n
	//if data != NULL, return the actual bytes written into data, or -1 if decode failed 
	//(when decode fails usually not enough space to store all decoded data)
	 size_t	CodeConverter::Base64Decode(const tchar* inString, size_t len, void* data, size_t dest_bytes)
	{
		size_t text_bytes = 0;
#if BLADE_UNICODE
		TempBuffer buffer;
		buffer.reserve( len+1 );

		char* text = (char*)buffer.getData();
		text_bytes = len;

		//size_t count = CodeConverter::TStringToOtherString<char>(inString, len, (char*)buffer.getData(), buffer.getCapacity() );
		//assert( count < buffer.getCapacity() );
		for(size_t i = 0; i < len; ++i)
			text[i] = (char)inString[i];
		text[len] = char();
#else
		const char* text = inString;
		text_bytes = len;
#endif

		InternalStringStream inStream;
		inStream.write( (const char*)text, (std::streamsize)text_bytes );

		InternalStringStream outStream;
		if( ::b64_decode(inStream, outStream) != 0 )
			return size_t(-1);

		//TODO: use stream pos instead of size_t
		size_t pos = (size_t)outStream.tellp();

		if( data == NULL )
			return pos;
		else if( pos > dest_bytes )
			return size_t(-1);
		else
		{
			outStream.read( (char*)data, (std::streamsize)pos);
			return pos;
		}
	}

	
}//namespace Blade