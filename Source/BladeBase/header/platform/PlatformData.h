/********************************************************************
	created:	2015/08/04
	filename: 	PlatformData.h
	author:		Crazii
	
	purpose:	Data Storing platform specific info.
				You need to initialize platform data at program start up, i.e. in main() function.
				then Later platform dependent modules may access these data.

	IMPORTANT:	DO NOT include it in public headers! KEEP IT PRIVATE, although this header has pubic access.
				It's used & shared by PLATFORM DEPENDENT MODULES ONLY.			
*********************************************************************/
#ifndef __Blade_PlatformData_h__
#define __Blade_PlatformData_h__

#ifndef BLADE_USE_PLATFORM_DATA_EXPLICIT
#error Do not use it unless you really need to!
#endif

#include <BladeTypes.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	ifndef _WINDOWS_
#		define NOMINMAX
#		undef TEXT
#		include <Windows.h>
#	endif
#	include <io.h>
#	include <tchar.h>
#	include <direct.h>
#	include <sys/stat.h>
#	include <errno.h>
#	define PATH_MAX			MAX_PATH
#	define blade_stat_t		_stat
#	define blade_stat		_tstat
#	define blade_remove		_tremove
#	define blade_rmdir		_trmdir
#	define blade_mkdir		_tmkdir
#	define blade_getcwd		_tgetcwd
#elif BLADE_IS_UNIX_CLASS_SYSTEM
#	if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#		include <android/native_activity.h>
#		include <jni.h>
#		include <platform/android/android_native_app_glue.h>
#	endif
#	include <dirent.h>
#	include <sys/ioctl.h>
#	include <sys/stat.h>
#	include <errno.h>
#	include <unistd.h>
#	include <utility/StringHelper.h>
#	include <utility/StringHelperEx.h>

///macro compatible to Windows
#	define S_IREAD		(S_IRUSR|S_IRGRP)
#	define S_IEXEC		(S_IXUSR|S_IXGRP)
#	define S_IWRITE		(S_IWUSR|S_IXGRP)

namespace Blade
{

#define TSTRING2UTF8(_buffer, _tstr) (StringConverter::TStringToUTF8String(_buffer, _tstr) )
#define UTF82TSTRING(_utf8str, _len) (StringConverter::UTF8StringToTString(_utf8str, _len) )

	static inline int blade_remove(const tchar* name)
	{
		TempBuffer buffer;
		return remove( TSTRING2UTF8(buffer, name) );
	}

	static inline int blade_rmdir(const tchar* name)
	{
		TempBuffer buffer;
		return ::rmdir( TSTRING2UTF8(buffer, name) );
	}

	static inline int blade_mkdir(const tchar* name)
	{
		TempBuffer buffer;
		return ::mkdir( TSTRING2UTF8(buffer, name), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	}

#	define blade_stat_t		stat
	static inline int blade_stat(const tchar* name, struct stat* st)
	{
		TempBuffer buffer;
		return stat( TSTRING2UTF8(buffer, name), st);
	}

	static inline tchar* blade_getcwd(tchar* dest, size_t len)
	{
		char buffer[PATH_MAX];
		::getcwd(buffer, PATH_MAX);
		TString str = UTF82TSTRING(buffer, 0);
		size_t strLen = str.length();
		len = strLen > len ? len : strLen;
		Char_Traits<tchar>::copy(dest, str.c_str(), len + sizeof(tchar) );
		return dest;
	}

}//namespace Blade

#else

#	error un-implemented platform.

#endif

namespace Blade
{
	typedef void*			PLATFORM_USER_DATA;

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
	typedef HINSTANCE		PLATFORM_APP_DATA;
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
	typedef ANativeActivity*PLATFORM_APP_DATA;
#else
#	error un-implemented platform.
#endif

}//namespace Blade

#include <Singleton.h>
#include <utility/String.h>


namespace Blade
{
	typedef struct SPlatformData
	{
		PLATFORM_USER_DATA	userData;
		PLATFORM_APP_DATA	appData;
	}PLATFORM_DATA;

	///important: GCC external linkage
	///use fwd declaration to avoid attribute warning
	class PlatformData;
	extern template class BLADE_BASE_API Singleton<PlatformData>;
	//

	class BLADE_BASE_API PlatformData : public Singleton<PlatformData>
	{
	public:
		PlatformData();
		~PlatformData();

		/**
		@describe init the platform data, it's called on Blade::InitializeFoundattion()
		@param
		@return
		*/
		bool initialize();

		/**
		@describe auto called on termination, in destructor: ~PlatformData()
		@param
		@return
		*/
		bool shutdown();

		/**
		@describe get the platform data set by user
		@param
		@return
		*/
		const PLATFORM_DATA&		getPlatformData() const;

		inline PLATFORM_APP_DATA	getPlatformAppData() const
		{
			return this->getPlatformData().appData;
		}
		inline PLATFORM_USER_DATA	getPlatformUserData() const
		{
			return this->getPlatformData().userData;
		}

		/**
		@describe set the platform user data
		@param
		@return
		*/
		void setPlatformUserData(PLATFORM_USER_DATA appData);

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

		/** @brief this is not quite useful, unless when IWindowMessagePump is not ready */
		void			internalMessagePump() const;

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

		/** @brief get the .so library folder */
		const TString&	getLibraryPath() const	{return mLibPath;}

		/** @brief  */
		const TString&	getPackageName() const	{return mPackageName;}

		/** @brief  */
		int				getVersionCode() const	{return mVersionCode;}

		/** @brief get android_app */
		android_app*	getAndroidApp() const;

		/** @brief this is not quite useful, unless when IWindowMessagePump is not ready */
		void			internalMessagePump() const;

		///JNI helpers

		/** @brief attach JNI to current thread */
		JNIEnv*		attachJNI();

		/** @brief detach JNI to current thread */
		bool		detachJNI();

		/** @brief load JNI class */
		jclass		loadClassJNI(JNIEnv* env, const TString& className);
#endif

	private:
		PLATFORM_DATA	mData;

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		TString		mLibPath;
		TString		mPackageName;
		int			mVersionCode;
		jobject		mClassLoader;
		jmethodID	mLoadMethod;
#endif
	};
	
}//namespace Blade

#endif // __Blade_PlatformData_h__