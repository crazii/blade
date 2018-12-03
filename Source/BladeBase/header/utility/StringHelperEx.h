/********************************************************************
	created:	2013/12/26
	filename: 	StringHelperEx.h
	author:		Crazii
	
	purpose:	string conversion to temporary buffer, used for frequently conversion to temporary strings
*********************************************************************/
#ifndef __Blade_StringHelperEx_h__
#define __Blade_StringHelperEx_h__
#include "String.h"
#include "Buffer.h"

namespace Blade
{

	namespace StringConverter
	{
		BLADE_BASE_API const wchar*		StringToWString(TempBuffer& buffer, const char* mbcString, size_t len = 0, size_t* count = NULL);
		inline const wchar*				StringToWString(TempBuffer& buffer, const String& mbString, size_t* count = NULL)
		{
			return StringToWString(buffer, mbString.c_str(), mbString.size(), count);
		}

		BLADE_BASE_API const char*		WStringToString(TempBuffer& buffer, const wchar* wcString, size_t len = 0, size_t* count = NULL);
		inline const char*				WStringToString(TempBuffer& buffer, const WString& wString, size_t* count = NULL)
		{
			return WStringToString(buffer, wString.c_str(), wString.size(), count);
		}

		inline const char*		TStringToString(TempBuffer& buffer, const tchar* tcString,size_t len = 0, size_t* count = NULL)
		{
#if BLADE_UNICODE
			return WStringToString(buffer, tcString,len, count);
#else
			BLADE_UNREFERENCED(len);
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return tcString;
#endif
		}
		inline const char*				TStringToString(TempBuffer& buffer, const TString& tString, size_t* count = NULL)
		{
#if BLADE_UNICODE
			return WStringToString(buffer, tString.c_str(), tString.size(), count);
#else
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return tString.c_str();
#endif
		}

		inline const wchar*		TStringToWString(TempBuffer& buffer, const tchar* tcString, size_t len = 0, size_t* count = NULL)
		{
#if BLADE_UNICODE
			BLADE_UNREFERENCED(len);
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return tcString;
#else
			return StringToWString(buffer, tcString, len, count);
#endif
		}
		inline const wchar*		TStringToWString(TempBuffer& buffer, const TString& tString, size_t* count = NULL)
		{
#if BLADE_UNICODE
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return tString.c_str();
#else
			return StringToWString(buffer, tString.c_str(), tString.size(), count);
#endif
		}

		inline const tchar*		StringToTString(TempBuffer& buffer, const char* mbcString, size_t len = 0, size_t* count = NULL)
		{
#if BLADE_UNICODE
			return StringToWString(buffer, mbcString,len, count);
#else
			BLADE_UNREFERENCED(len);
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return mbcString;
#endif
		}
		inline const tchar*		StringToTString(TempBuffer& buffer, const String& mbString, size_t* count = NULL)
		{
#if BLADE_UNICODE
			return StringToWString(buffer, mbString.c_str(), mbString.size(), count);
#else
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return mbString.c_str();
#endif
		}

		inline const tchar*		WStringToTString(TempBuffer& buffer, const wchar* wcString,size_t len = 0, size_t* count = NULL)
		{
#if BLADE_UNICODE
			BLADE_UNREFERENCED(len);
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return wcString;
#else
			return WStringToString(buffer, wcString,len, count);
#endif
		}
		inline const tchar*		WStringToTString(TempBuffer& buffer, const WString& wString, size_t* count = NULL)
		{
#if BLADE_UNICODE
			BLADE_UNREFERENCED(buffer);
			BLADE_UNREFERENCED(count);
			return wString.c_str();
#else
			return WStringToString(buffer, wString.c_str(),wString.size(), count);
#endif
		}

		BLADE_BASE_API	const char*		StringToUTF8String(TempBuffer& buffer, const char* mbcString, size_t len = 0, size_t* count = NULL);
		inline const char*				StringToUTF8String(TempBuffer& buffer, const String& mbString, size_t* count = NULL)
		{
			return StringToUTF8String(buffer, mbString.c_str(), mbString.size(), count);
		}

		BLADE_BASE_API	const char*		WStringToUTF8String(TempBuffer& buffer, const wchar* wcString, size_t len = 0, size_t* count = NULL);
		inline const char*				WStringToUTF8String(TempBuffer& buffer, const WString& wString, size_t* count = NULL)
		{
			return WStringToUTF8String(buffer, wString.c_str(), wString.size(), count);
		}

		inline BLADE_BASE_API const char* TStringToUTF8String(TempBuffer& buffer, const tchar* tcString, size_t len = 0, size_t* count = NULL)
		{
#if BLADE_UNICODE
			return StringConverter::WStringToUTF8String(buffer, tcString, len, count);
#else
			return StringConverter::StringToUTF8String(buffer, tcString, len, count);
#endif
		}

		inline const char*				TStringToUTF8String(TempBuffer& buffer, const TString& tString, size_t* count = NULL)
		{
			return TStringToUTF8String(buffer, tString.c_str(), tString.size(), count);
		}

		BLADE_BASE_API	const char*		UTF8StringToString(TempBuffer& buffer, const char* utf8String, size_t bytes = 0, size_t* count = NULL);
		inline const char*				UTF8StringToString(TempBuffer& buffer, const String& utf8String, size_t* count = NULL)
		{
			return StringToUTF8String(buffer, utf8String.c_str(), utf8String.size(), count);
		}

		BLADE_BASE_API	const wchar*		UTF8StringToWString(TempBuffer& buffer, const char* utf8String, size_t bytes = 0, size_t* count = NULL);
		inline const wchar*				UTF8StringToWString(TempBuffer& buffer, const String& utf8String, size_t* count = NULL)
		{
			return UTF8StringToWString(buffer, utf8String.c_str(), utf8String.size(), count);
		}

		inline BLADE_BASE_API const tchar* UTF8StringToTString(TempBuffer& buffer, const char* utf8String, size_t bytes = 0, size_t* count = NULL)
		{
#if BLADE_UNICODE
			return StringConverter::UTF8StringToWString(buffer, utf8String, bytes, count);
#else
			return StringConverter::UTF8StringToString(buffer, utf8String, bytes, count);
#endif
		}

		inline const tchar*				UTF8StringToTString(TempBuffer& buffer, const String& utf8String, size_t* count = NULL)
		{
			return UTF8StringToTString(buffer, utf8String.c_str(), utf8String.size(), count);
		}
	}//namespace StringConverter


	
}//namespace Blade


#endif // __Blade_StringHelperEx_h__
