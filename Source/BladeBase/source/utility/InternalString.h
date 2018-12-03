/********************************************************************
	created:	2013/01/26
	filename: 	InternalString.h
	author:		Crazii
	
	purpose:	these are mainly temporary string classes using temporary allocators
				they're used only internally, to reduce the complexity of string usage
				(if there are TString and TmpTString for clients, 
				they may got bored when converting between two types.

				actually the conversion between TString & TmpTString
				can be committed by using full template class
				for StringHelper & StringStream & StringConcat & StringTokenizer,etc.
				but this will cause the compiling slow like hell,
				so we don't do it that way.

*********************************************************************/
#ifndef __Blade_InternalString_h__
#define __Blade_InternalString_h__

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning( disable : 4996 )
#endif

#include <memory/TempAllocator.h>
#include <utility/String.h>
#include <utility/StringHelper.h>
#include <string>
#include <sstream>
#include <fstream>
#include <istream>
#include <ostream>

namespace Blade
{

	typedef FnBasicStringFastLess<char,TempStringAllocator>						FnTmpStringFastLess;
	typedef FnBasicStringFastLess<wchar,TempStringAllocator>					FnTmpWStringFastLess;
	typedef FnBasicStringFastLess<tchar,TempStringAllocator>					FnTmpTStringFastLess;

	typedef BasicStringHelper<char, TempStringAllocator>						TmpStringHelper;
	typedef BasicStringHelper<wchar, TempStringAllocator>						TmpWStringHelper;

	//because we use tmp string for temp objects/operations internally, and reinterpret_cast to string to expose to user
	//we need the two types are exactly match
	//note: returned string ref const_cast by user will f**k up the memory allocator
	static_assert(sizeof(TmpString) == sizeof(String), "memory layout mismatch");
	static_assert(sizeof(TmpWString) == sizeof(WString), "memory layout mismatch");

	typedef std::basic_string<char, std::char_traits<char>, TempAllocator<char> > InternalString;
	typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, TempAllocator<wchar_t> > InternalWString;

	typedef std::basic_stringstream<char, std::char_traits<char>, TempAllocator<char> > InternalStringStream;
	typedef std::basic_stringstream<wchar_t, std::char_traits<wchar_t>, TempAllocator<wchar_t> > InternalWStringStream;

}//namespace Blade

#endif//__Blade_InternalString_h__