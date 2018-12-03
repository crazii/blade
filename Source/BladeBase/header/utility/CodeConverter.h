/********************************************************************
	created:	2009/04/12
	filename: 	CodeConverter.h
	author:		Crazii
	
	purpose:	convert UTF-8 converter, support MBCS and system UNICODE
*********************************************************************/
#ifndef __Blade_CodeConverter_h__
#define __Blade_CodeConverter_h__
#include <String.h>

namespace Blade
{

	namespace CodeConverter
	{
		///get current code page of the system
		BLADE_BASE_API const String& GetCodePage();

		///convert UTF-8 to multi byte code set, failed if return -1 ( 0xFFFFFFFF or 0xFFFFFFFFFFFFFFFF )
		///otherwise return the converted char count in mbcsbuffer
		BLADE_BASE_API size_t UTF8toMBCS(const char* utf8buffer,size_t src_bytes,char* mbcsbuffer,size_t dest_size);

		///convert UTF-8 to wide char set ,failed if return -1
		///otherwise return the converted wchar count in wcsbuffer
		BLADE_BASE_API size_t UTF8toWCS(const char* utf8buffer,size_t src_bytes,wchar* wcsbuffer,size_t dest_size);

		///convert multi byte code set to UTF-8 ,failed if return -1
		///otherwise return the converted bytes in utf8buffer
		BLADE_BASE_API size_t MBCStoUTF8(const char* mbcsbuffer,size_t src_size,char* utf8buffer,size_t dest_bytes);

		///convert wide char set to UTF-8, failed if return -1
		///otherwise return the converted bytes in utf8buffer
		BLADE_BASE_API size_t WCStoUTF8(const wchar* wcsbuffer,size_t src_size,char* utf8buffer,size_t dest_bytes);

		///specify destbuffer to NULL to get the real requested size
		BLADE_BASE_API size_t	StringToTString(const char* srcbuffer,size_t src_size,tchar* destbuffer, size_t dest_bytes);

		BLADE_BASE_API size_t	WStringToTString(const wchar* srcbuffer,size_t src_size,tchar* destbuffer, size_t dest_bytes);

		///specify destbuffer to NULL to get the real requested size
		BLADE_BASE_API size_t	TStringToString(const tchar* srcbuffer,size_t src_size,char* destbuffer,size_t dest_bytes);

		BLADE_BASE_API size_t	TStringToWString(const tchar* srcbuffer,size_t src_size, wchar* destbuffer,size_t dest_bytes);

		///convert between encodings. input size in bytes, return size in bytes
		BLADE_BASE_API size_t	CodeConvert(TEXT_ENCODING from, TEXT_ENCODING to, const char* source, size_t sourceBytes, char* dest, size_t destBytes);


		/************************************************************************/
		/* Base64 encoding                                                                     */
		/************************************************************************/

		///if outString == NULL, return the string length (count in characters)
		///if outString != NULL, return converted character count, or -1 if error occurs
		extern BLADE_BASE_API size_t	Base64Encode(const void* data, size_t src_bytes, tchar* outString, size_t len);

		///data == NULL, return actual bytes needed for data,\n
		///if data != NULL, return the actual bytes written into data, or -1 if decode failed 
		///(when decode fails usually not enough space to store all decoded data)
		extern BLADE_BASE_API size_t	Base64Decode(const tchar* inString, size_t len, void* data, size_t dest_bytes);

		
		/************************************************************************/
		/* UTF conversions                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param [in] src_bytes : size of the utf8buffer in 'bytes'
		@param [in] dest_size : count of destbuffer in 'characters'
		@return converted character in 'bytes'
		*/
		template<typename CharT>
		inline size_t UTF8toString(const char* utf8buffer,size_t src_bytes,CharT* destbuffer,size_t dest_size){ assert(false);return 0; }

		template<>
		inline size_t UTF8toString<char>(const char* utf8buffer,size_t src_bytes, char* destbuffer,size_t dest_size)
		{
			return UTF8toMBCS(utf8buffer, src_bytes, destbuffer, dest_size);
		}

		template<>
		inline size_t UTF8toString<wchar>(const char* utf8buffer,size_t src_bytes, wchar* destbuffer,size_t dest_size)
		{
			return UTF8toWCS(utf8buffer, src_bytes, destbuffer, dest_size);
		}


		/**
		@describe 
		@param [in] src_size: count of srcbuffer in 'characters'
		@param [in] dest_bytes : size of the utf8buffer in 'bytes'
		@return converted (actually used) utf8buffer in 'bytes'
		*/
		template<typename CharT>
		inline size_t	StringtoUTF8(const CharT* srcbuffer,size_t src_size, char* utf8buffer,size_t dest_bytes)	{ assert(false);return 0; }

		template<>
		inline size_t	StringtoUTF8<char>(const char* srcbuffer,size_t src_size, char* utf8buffer,size_t dest_bytes)
		{
			return MBCStoUTF8(srcbuffer, src_size, utf8buffer, dest_bytes);
		}

		template<>
		inline size_t	StringtoUTF8<wchar>(const wchar* srcbuffer,size_t src_size, char* utf8buffer,size_t dest_bytes)
		{
			return WCStoUTF8(srcbuffer, src_size, utf8buffer, dest_bytes);
		}

		/************************************************************************/
		/* Wide character - Multi bytes conversion                                                                     */
		/************************************************************************/

		/**
		@describe 
		@param [in] src_size: count of srcbuffer in 'characters'
		@param [in] dest_size : count of the destbuffer in 'characters'
		@return converted character in 'characters'
		*/
		template<typename CharT>
		inline size_t	OtherStringToTString(const CharT* srcbuffer, size_t src_size,tchar* tcbuffer,size_t dest_size)	{assert(false);return 0;}

		template<>
		inline size_t	OtherStringToTString<char>(const char* srcbuffer, size_t src_size, tchar* tcbuffer,size_t dest_size)
		{
			return StringToTString(srcbuffer, src_size, tcbuffer, dest_size);
		}

		template<>
		inline size_t	OtherStringToTString<wchar>(const wchar* srcbuffer, size_t src_size, tchar* tcbuffer,size_t dest_size)
		{
			return WStringToTString(srcbuffer, src_size, tcbuffer, dest_size);
		}

		/**
		@describe 
		@param [in] src_size: count of srcbuffer in 'characters'
		@param [in] dest_size : count of the destbuffer in 'characters'
		@return converted character in 'characters'
		*/
		template<typename CharT>
		inline size_t	TStringToOtherString(const tchar* tcbuffer,size_t src_size,CharT* destbuffer,size_t dest_size)	{assert(false);return 0;}

		template<>
		inline size_t	TStringToOtherString<char>(const tchar* tcbuffer,size_t src_size, char* destbuffer,size_t dest_size)
		{
			return TStringToString(tcbuffer, src_size, destbuffer, dest_size);
		}

		template<>
		inline size_t	TStringToOtherString<wchar>(const tchar* tcbuffer,size_t src_size, wchar* destbuffer,size_t dest_size)
		{
			return TStringToWString(tcbuffer, src_size, destbuffer, dest_size);
		}

	};//namespace CodeConverter
	
}//namespace Blade

#endif // __Blade_CodeConverter_h__