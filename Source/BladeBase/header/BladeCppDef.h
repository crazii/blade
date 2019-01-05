/********************************************************************
	created:	2013/02/03
	filename: 	BladeCppDef.h
	author:		Crazii
	purpose:	detect C++ standards that compiler supports
*********************************************************************/
#ifndef __Blade_BladeCppDef_h__
#define __Blade_BladeCppDef_h__
#include <BladePlatform.h>

#define BLADE_STDCPP98 0
#define BLADE_STDCPP11 1
#define BLADE_STDCPP17 2

//c++ std macro
#if (__cplusplus > 199711L)
#define BLADE_STDCPP BLADE_STDCPP11
#else
#define BLADE_STDCPP BLADE_STDCPP98
#endif

#if BLADE_STDCPP == BLADE_STDCPP98
#	if BLADE_COMPILER == BLADE_COMPILER_MSVC && (_MSC_VER >= 1900)
#	elif BLADE_COMPILER == BLADE_COMPILER_MSVC && (_MSC_VER >= 1700)

#		include <xkeycheck.h>
#		include <functional>

#		define constexpr 
#		define noexcept throw()
#	else
#		define constexpr 
#		define noexcept throw()
#	endif
#endif

#if BLADE_STDCPP >= BLADE_STDCPP11
#	if !defined(BLADE_ALIGNOF)
#		define BLADE_ALIGNOF(_t) alignof(_t)
#	endif
#	include <cstdint>
#else
#	if (BLADE_COMPILER == BLADE_COMPILER_MSVC && (_MSC_VER >= 1600))
#		define BLADE_HAS_STATIC_ASSERT
#	elif BLADE_COMPILER == BLADE_COMPILER_CLANG
#		if __has_extension(cxx_static_assert) 
#			define BLADE_HAS_STATIC_ASSERT
#		endif
#	else
#		if (defined(__cpp_static_assert) && __cpp_static_assert >= 201411)
#			define _BLADE_HAS_STATIC_ASSERT
#		endif
#	endif
#	if !defined(BLADE_HAS_STATIC_ASSERT)
#		define static_assert(exp, desc) typedef char assert_type[(exp) ? 1 : -1]
#	endif
#endif

namespace std
{

#if BLADE_STDCPP >= BLADE_STDCPP11 || (defined(_MSC_VER) && _MSC_VER >= 1600)	//VC10(VS2010)
#include <type_traits>
#else
	//is_base_of
	//SFINAE
	template<typename Base, typename Derived>
	struct is_base_of
	{
		static int64	check(Base*) { return int64(); }
		static int8		check(...) { return int8(); }
		enum { value = sizeof(check(static_cast<Derived*>(NULL))) == sizeof(int64) };
	};

	//remove_pointer
	template< typename T > struct remove_pointer { typedef T type; };
	template< typename T > struct remove_pointer<T*> { typedef T type; };
	template< typename T > struct remove_pointer<T* const> { typedef T type; };
	template< typename T > struct remove_pointer<T* volatile> { typedef T type; };
	template< typename T > struct remove_pointer<T* const volatile> { typedef T type; };

	//is_pointer
	template<typename T> struct is_pointer { enum { value = false }; };
	template< typename T > struct is_pointer<T*> { enum { value = true }; };
	template< typename T > struct is_pointer<T* const> { enum { value = true }; };
	template< typename T > struct is_pointer<T* volatile> { enum { value = true }; };
	template< typename T > struct is_pointer<T* const volatile> { enum { value = true }; };

	//remove_reference
	template< typename T > struct remove_reference { typedef T type; };
	template< typename T > struct remove_reference<T&> { typedef T type; };
	template< typename T > struct remove_reference<T& const> { typedef T type; };
	template< typename T > struct remove_reference<T& volatile> { typedef T type; };
	template< typename T > struct remove_reference<T& const volatile> { typedef T type; };

#endif

#if defined(_MSC_VER) && _MSC_VER <= 1700
	//https://social.msdn.microsoft.com/Forums/vstudio/en-US/260e04fc-dd05-4a96-8953-9c6ea1ad62fb/cant-find-stdround-in-cmath?forum=vclanguage
	static inline double round(double f)
	{
		return (double)static_cast<int>(f + 0.5);
	}
	static inline float roundf(float f)
	{
		return (float)static_cast<int>(f + 0.5);
	}
#endif

}//namespace std

#endif //  __Blade_BladeCppDef_h__