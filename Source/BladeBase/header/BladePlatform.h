/********************************************************************
	created:	2009/02/07
	filename: 	BladePlatform.h
	author:		Crazii
	
	purpose:	the platform dependent definitions
*********************************************************************/
#ifndef __Blade_BladePlatform_h__
#define __Blade_BladePlatform_h__


/************************************************************************/
/* processor unit                                                                     */
/************************************************************************/
#define BLADE_PROCESSOR_X86		(0)
#define BLADE_PROCESSOR_IA64	(1)
#define BLADE_PROCESSOR_PPC		(2)
#define BLADE_PROCESSOR_ARM		(3)
#define BLADE_PROCESSOR_MIPS	(4)

#if defined ARM || defined __arm__ || defined _ARM || defined(__ARN_ARCH_7S__) || defined(__ARN_ARCH_7A__)\
		|| defined(__ARM_ARCH_ISA_A64) || defined(__aarch64__)
#	define BLADE_PROCESSOR BLADE_PROCESSOR_ARM

#elif defined __ia64 || defined _M_IA64 || defined __ia64__ 
#	define BLADE_PROCESSOR BLADE_PROCESSOR_IA64

#elif defined __X86__ || defined __i386__ || defined i386 || defined _M_IX86 || defined __386__ || defined __x86_64__ || defined _M_X64
#	define BLADE_PROCESSOR BLADE_PROCESSOR_X86

#elif defined __PPC__ || defined __POWERPC__  || defined powerpc || defined _POWER || defined __ppc__ || defined __powerpc__ || defined _M_PPC
#  define BLADE_PROCESSOR BLADE_PROCESSOR_PPC

#elif defined mips || defined _mips || defined __mips || defined __mips__ || defined __MIPS__ || defined _MIPS_ARCH || defined MIPSEL || defined _MIPSEL || defined __MIPSEL__\
	|| defined __mips64  || defined __mips64__
#define BLADE_PROCESSOR BLADE_PROCESSOR_MIPS

#else
#	error "processor not supported yet."
#endif

/************************************************************************/
/* this is the system specific,windows and linux, etc                                                                 */
/************************************************************************/

#define BLADE_PLATFORM_WINDOWS	(0)
#define BLADE_PLATFORM_WINPHONE	(1)

#define BLADE_PLATFORM_LINUX	(2)
#define BLADE_PLATFORM_ANDROID	(3)
#define BLADE_PLATFORM_IOS		(4)
#define BLADE_PLATFORM_MAC		(5)

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)

#	define BLADE_PLATFORM BLADE_PLATFORM_WINDOWS

#elif defined(WP8) && defined(_WP8)

#	define BLADE_PLATFORM BLADE_PLATFORM_WINPHONE

#elif defined(ANDROID) || defined(__ANDROID__)//since android will also define __linux__, we need check android first

#define BLADE_PLATFORM BLADE_PLATFORM_ANDROID

#elif defined(__linux__)

#	define BLADE_PLATFORM BLADE_PLATFORM_LINUX

#else
#	error "current system not support yet."
#endif

///Careful on using those class system definition. different system may have different APIs.
///i.e. Unix POSIX version difference, windows Desktop/Mobile API differences.

#if defined(__unix__)
#	define BLADE_IS_UNIX_CLASS_SYSTEM 1
#else
#	define BLADE_IS_UNIX_CLASS_SYSTEM 0
#endif

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WIN32_WCE) || defined(WINRT) || defined(_WINRT) || defined(WP8) || defined(_WP8)
#	define BLADE_IS_WINDOWS_CLASS_SYSTEM 1
#else
#	define BLADE_IS_WINDOWS_CLASS_SYSTEM 0
#endif

/************************************************************************/
/* Architecture (32/64 bit)                                                                     */
/************************************************************************/

#define BLADE_ARCHITECTURE_32BIT (0)
#define BLADE_ARCHITECTURE_64BIT (1)

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_IA64) || defined(_M_AMD64) || defined(__powerpc64__) || defined(__alpha__) || defined(__ia64__) || defined(__s390__) || defined(__s390x__)\
		|| defined(__ARM_ARCH_ISA_A64) || defined(__aarch64__) || defined __mips64  || defined __mips64__
#   define BLADE_ARCHITECTURE BLADE_ARCHITECTURE_64BIT
#	define BLADE_MEMORY_ALIGNMENT 16
#else
#   define BLADE_ARCHITECTURE BLADE_ARCHITECTURE_32BIT
#	define BLADE_MEMORY_ALIGNMENT 8
#endif

/************************************************************************/
/* this is the compiler specific, MSVC CLANG, and GCC currently supported */
/************************************************************************/
#define BLADE_COMPILER_MSVC (0)
#define BLADE_COMPILER_GNUC (1)
#define BLADE_COMPILER_CLANG (2)


#if defined(_MSC_VER)
#	define BLADE_COMPILER BLADE_COMPILER_MSVC
#elif defined(__clang__)
///note: Clang supports GNU Extension, and it defines (__GNUC__), 
///so we need detect Clang first, or it will be mistaken as GCC
#	define BLADE_COMPILER BLADE_COMPILER_CLANG
#	define BLADE_COMPILER_GNU_EXTENSION 1

#if __cplusplus
#	if !__has_feature(cxx_static_assert)
#		error static_assert is not supported by current Clang.
#	endif
#endif

#elif defined(__GNUC__)
#	define BLADE_COMPILER BLADE_COMPILER_GNUC
#	define BLADE_COMPILER_GNU_EXTENSION 1
#	if(__GNUC__  < 4 || __GNUC__ == 4 && __GNUC_MINOR__ < 3 )
#		error minimal version of gcc 4.3 is needed.
#	endif
#else
#	error Compiler not supported yet.
#endif

#ifndef BLADE_COMPILER_GNU_EXTENSION
#	define BLADE_COMPILER_GNU_EXTENSION 0
#endif

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	define BLADE_ALIGNED(n) __declspec(align(n))
#	define BLADE_ALIGNOF(_t) __alignof(_t)
#	define BLADE_CDECL    __cdecl
#	define BLADE_STDCALL  __stdcall
#	define BLADE_FASTCALL __fastcall
#	define BLADE_FUNCTION	__FUNCTION__
#	define BLADE_ALWAYS_INLINE __forceinline
#	if _MSC_VER >= 1800
#		define BLADE_VCALL __vectorcall
#	else
#		define BLADE_VCALL 
#endif
#elif BLADE_COMPILER_GNU_EXTENSION
#	define BLADE_ALIGNED(n)   __attribute__((aligned(n)))
#	define BLADE_ALIGNOF(_t) __alignof__(_t)
#	define BLADE_CDECL __attribute__((cdecl))
#	define BLADE_STDCALL __attribute__((stdcall))
#	define BLADE_FASTCALL __attribute__((fastcall))
#	define BLADE_FUNCTION	__PRETTY_FUNCTION__
#	define BLADE_ALWAYS_INLINE __attribute__((always_inline))
#	define BLADE_VCALL __attribute__((fastcall))
#else
#	error alignment & calling conventions are not defined properly.
#	define BLADE_ALIGNED(n) 
#	define BLADE_ALIGNOF(_t) BLADE_MEMORY_ALIGNMENT
#	define BLADE_CDECL    
#	define BLADE_STDCALL  
#	define BLADE_FASTCALL 
#	define BLADE_FUNCTION	
#	define BLADE_ALWAYS_INLINE inline
#endif

#if defined(_DEBUG) || defined(DEBUG)
#	define BLADE_DEBUG 1
#else
#	define BLADE_DEBUG 0
#endif

#define BLADE_ENDIAN_LITTLE		0
#define BLADE_ENDIAN_BIG		1

//endian macros
//HOWTO: ARM processor can have dynamic endian with endian map, how to check?

#if BLADE_PROCESSOR == BLADE_PROCESSOR_X86 || \
	BLADE_PROCESSOR == BLADE_PROCESSOR_ARM

#define BLADE_ENDIAN BLADE_ENDIAN_LITTLE

#else

#define BLADE_ENDIAN BLADE_ENDIAN_BIG

#endif

//make unicode macro having a value
#if defined(UNICODE) || defined(_UNICODE)
#	undef UNICODE
#	define UNICODE 1
#	define BLADE_UNICODE 1
#else
#	define BLADE_UNICODE 0
#endif


//warnings
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(disable:4619)		//there is no warning number N

#pragma warning(disable:4061)		//enumerator 'e' in switch of num 'E' is not explicitly handled by a case label (switch case)
#pragma warning(disable:4127)		//conditional expression is constant
#pragma warning(disable:4201)		//nonstandard extension: nameless struct/union
#pragma warning(disable:4231)		//nonstandard extension: extern before template explicit instantiation (C++11's standard)
#pragma warning(disable:4324)		//structure was padded due to alignment specifier
#pragma warning(disable:4371)		//layout of class may have changed from a previous version of the compiler due to better packing of member 'pointer to member'
#pragma warning(disable:4350)		//behavior change T& to const T& , A non-const reference may only be bound to an lvalue
#pragma warning(disable:4514)		//unreferenced inline function has been removed
#pragma warning(disable:4571)		//catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable:4623)		//default constructor could not be generated because a base class default constructor is inaccessible
#pragma warning(disable:4625)		//copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable:4626)		//assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning(disable:4668)		//'MACRO' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning(disable:4710)		//inline function not inlined
#pragma warning(disable:4711)		//function 'F' selected for automatic inline expansion
#pragma warning(disable:4738)		//storing 32-bit float result in memory, possible loss of performance
#pragma warning(disable:4820)		//N bytes padding added after data member '' (padding)
#pragma warning(disable:4910)		//__declspec(dllexport) & extern are incompatible (extern is used for convenience of other compilers
#pragma warning(disable:4987)		//nonstandard extension used: 'throw (...)'

#if (_MSC_VER >= 1800)
#pragma warning(disable:5026)		//move constructor was implicitly defined as deleted because a base class move constructor is inaccessible or deleted
#pragma warning(disable:5027)		//move assignment operator was implicitly defined as deleted because a base class move assignment operator is inaccessible or deleted
#pragma warning(disable:5029)		//nonstandard extension used: alignment attributes in C++ apply to variables, data members and tag types only
#pragma warning(disable:4464)		//relative include path contains '..'
#pragma warning(disable:4643)		//Forward declaring 'exception' in namespace std is not permitted by the C++ Standard.
#pragma warning(disable:5045)		//Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable:5039)		//'qsort': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
#pragma warning(disable:4774)		//'sprintf_s' : format string expected in argument 3 is not a string literal 
#pragma warning(disable:5039)		//pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
#endif


//class X needs to have dll-interface to be used by clients of class Y
//disabling this warning is risky but convenient.
//note: it's OK for most scenarios, i.e. Pimpl which should be private and dll export can be ignored;
//or some inline class with only parts of members exported instead of whole class (i.e. Allocatable).
//it's Okay for most templates too, except std containers.
//but Blade NEVER use and should never use containers in PUBLIC HEADERS, exporting (instantiated) std templates should be avoid too:
//this rule is to avoid different object memory layout(i.e. object size) across different CRT versions.
#pragma warning(disable:4251 4275)

#elif BLADE_COMPILER == BLADE_COMPILER_CLANG

#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-function"

#elif BLADE_COMPILER_GNU_EXTENSION
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif



//Use: #pragma message BLADE_WARN("message")
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	define BLADE_STRINGISE_LN_IMPL(x) #x
#	define BLADE_STRINGISE_LN(x) BLADE_STRINGISE_LN_IMPL(x)
#   define FILE_LINE_LINK __FILE__ "(" BLADE_STRINGISE_LN(__LINE__) ") : "
#   define BLADE_WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#elif BLADE_COMPILER == BLADE_COMPILER_GNUC	//gcc only.clang to be tested
#   define BLADE_WARN(exp) ("WARNING: " exp)
#endif


#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#	define BLADE_IMPORT __declspec(dllimport)
#	define BLADE_EXPORT __declspec(dllexport)
#	define BLADE_VCLASS_EXPORT 
#elif BLADE_COMPILER_GNU_EXTENSION	//gcc only.clang to be tested
#	define BLADE_IMPORT __attribute__ ((visibility("default")))
#	define BLADE_EXPORT __attribute__ ((visibility("default")))
#	define BLADE_VCLASS_EXPORT __attribute__ ((visibility("default")))	//fix vtable linkage
#else
#	error not implemented.
#endif

#endif // __Blade_BladePlatform_h__
