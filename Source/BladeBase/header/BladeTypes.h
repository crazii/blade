/********************************************************************
	created:	2009/02/07
	filename: 	Types.h
	author:		Crazii
	
	purpose:	basic types definitions
*********************************************************************/
#ifndef __Blade_Types_h__
#define __Blade_Types_h__
#include <BladePlatform.h>

#define BLADE_UNREFERENCED(var)	((void)var)

//note: include std C/C++ headers only, no platform headers
//note: no C++ STL headers
#include <cstdlib>
#include <cstdio>
#include <cwchar>
#include <cfloat>
#include <cctype>
#include <cwctype>
#include <cstring>
#include <cassert>
#include <climits>
#include <ctime>
#include <cstddef>	//size_t
#include <cerrno>
#include <typeinfo>

#include <new>
#include <utility>

#include <BladeCppDef.h>

#ifndef TEXT
#	if BLADE_UNICODE
#		define BLADE_TEXT(str) L##str
#		define TEXT(str)	BLADE_TEXT(str)
#	else
#		define TEXT(str) str
#	endif
#endif

namespace Blade
{
	///basic type definitions
	///for commonly use,just use the 'int' type.
	///when you want explicit specify the size,using the following types,
	///this is useful when declare members in a struct especially for IO.

	///8bit int
	typedef signed char			int8,	*pint8;
	typedef unsigned char		uint8,	*puint8;
	typedef	unsigned char		uchar,	*puchar;

	///16bit int
	typedef short				int16,	*pint16;
	typedef unsigned short		uint16,	*puint16;
	typedef	unsigned short		ushort,	*pushort;

	///uint ulong
	typedef	unsigned int		uint,	*puint;
	typedef	unsigned long		ulong,	*pulong;

	///32bit int
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
	typedef __int32				int32,	*pint32;
	typedef unsigned __int32	uint32,	*puint32;
#else
	///using standard type int32_t
	typedef int32_t				int32,	*pint32;
	typedef uint32_t			uint32,	*puint32;
#endif

	///64bit int
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
	typedef __int64				int64,	*pint64;
	typedef unsigned __int64	uint64,	*puint64;
#else
	typedef long long			int64,	*pint64;
	typedef unsigned long long 	uint64,	*puint64;
#endif

	///16 bit floating point is included in Half.h
	///which is not included here, use it on your demand

	///32,64bit floating point
	typedef float			fp32,	*pfp32;
	typedef double			fp64,	*pfp64;

	///wchar_t
	typedef wchar_t			wchar,	*pwchar;

#if BLADE_UNICODE
	///tchar
	typedef wchar			tchar,	*ptchar;
#else
	typedef char			tchar,	*ptchar;
#endif

	//index
	typedef size_t			index_t;
	typedef ptrdiff_t		indexdiff_t;


	static const index_t INVALID_INDEX = (index_t(-1));


	///pointer to member size
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
	static const size_t MAX_POINTER2MEMBER_SIZE = 16;
#elif BLADE_COMPILER == BLADE_COMPILER_GNUC || BLADE_COMPILER == BLADE_COMPILER_CLANG
	static const size_t MAX_POINTER2MEMBER_SIZE = sizeof(void*)*2;
#else
	static const size_t MAX_POINTER2MEMBER_SIZE = 20;
#endif



#ifndef BLADE_DOUBLE_PRESITION
#	define BLADE_DOUBLE_PRESITION 0
#else
#	undef BLADE_DOUBLE_PRESITION
#	define BLADE_DOUBLE_PRESITION 1
#endif

#if BLADE_DOUBLE_PRESITION
	typedef	fp64			scalar;
#else
	typedef	fp32			scalar;
#endif


	typedef enum ETextEncoding
	{
		TE_START,

		TE_MBCS = TE_START,
		TE_UTF8,

		TE_UTF16LE,
		TE_UTF16BE,

		TE_UTF32LE,
		TE_UTF32BE,

		TE_COUNT,
#if BLADE_ENDIAN == BLADE_ENDIAN_LITTLE
		TE_UTF16	= TE_UTF16LE,
		TE_UTF32	= TE_UTF32LE,
#else
		TE_UTF16	= TE_UTF16BE,
		TE_UTF32	= TE_UTF32BE,
#endif
		TE_DEFAULT = TE_UTF8,
		TE_UNKNOWN = 0xFFFFFFFF,
	}TEXT_ENCODING;


	///four character code utility
	union FourCC
	{
		uint8	seq[4];
		uint32	fourcc;

		inline FourCC() :fourcc(0)				{}
		inline FourCC(uint32 val): fourcc(val)	{}
		inline FourCC(uint8* s)					{seq[0]=s[0];seq[1]=s[1];seq[2]=s[2];seq[3]=s[3];}
		inline operator uint32() const			{return fourcc;}
		inline bool operator==(uint32 rhs) const{return fourcc == rhs;}
		inline bool operator!=(uint32 rhs) const{return fourcc != rhs;}
		inline bool operator==(FourCC rhs) const{return fourcc == rhs.fourcc;}
		inline bool operator!=(FourCC rhs) const{return fourcc != rhs.fourcc;}
	};


	//basic types "stealing" form boost
	/** @brief  */
	class NonConstructable
	{
	private:
		NonConstructable();
	};

	//note: mark dector & copy ctor as delete so that type still is_trivially_copyable

	/** @brief  */
	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
#if BLADE_STDCPP >= BLADE_STDCPP11
		NonCopyable(const NonCopyable&) = delete;
#else
		NonCopyable(const NonCopyable&);
#endif
	};

	/** @brief  */
	class NonAssignable
	{
	protected:
		NonAssignable() {}
	private:
#if BLADE_STDCPP >= BLADE_STDCPP11
		NonAssignable& operator=(const NonAssignable&) = delete;
#else
		NonAssignable& operator=(const NonAssignable&);
#endif
	};

	/** @brief  */
	class NonAllocatable
	{
	protected:
		NonAllocatable() {}
#if BLADE_STDCPP >= BLADE_STDCPP11
		~NonAllocatable() = default;
#else
		~NonAllocatable() {}
#endif
		inline void* operator new(size_t) throw() { return NULL; }
		inline void* operator new[](size_t) throw() {return NULL; }
		inline void operator delete(void*) {}
		inline void operator delete[](void*) {}
	};

///make FCC in bit order MSB => LSB
#define BLADE_FCC_BITS(m3,m2,m1,m0) ( (((m3)&0xFF)<<24) | (((m2)&0xFF)<<16) | (((m1)&0xFF)<<8) | ((m0)&0xFF) )

///make FCC in byte order [0],[1],[2],[3], actually useful in serialization
#if BLADE_ENDIAN == BLADE_ENDIAN_BIG
#define BLADE_FCC(m0, m1, m2, m3) BLADE_FCC_BITS(m0,m1,m2,m3)
#else
#define BLADE_FCC(m0, m1, m2, m3) BLADE_FCC_BITS(m3,m2,m1,m0)
#endif


	///basic helper functions
	template<typename T, std::size_t N>
	constexpr std::size_t countOf(T const (&)[N]) noexcept
	{
		return N;
	}

}//namespace Blade


#endif // __Blade_Types_h__