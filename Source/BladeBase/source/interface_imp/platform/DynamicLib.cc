/********************************************************************
	created:	2010/04/02
	filename: 	DynamicLib.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#	define NOMINMAX
#	undef TEXT
#	include <Windows.h>
#endif
#include "DynamicLib.h"

//////////////////////////////////////////////////////////////////////////
#if BLADE_IS_WINDOWS_CLASS_SYSTEM

#	define Blade_DlSymbol( _handle, _symbol )	GetProcAddress( (HMODULE)_handle, _symbol )
#	define Blade_DlClose( _handle )			FreeLibrary( (HMODULE)_handle )
#	define Blade_DlCloseOK( _close_ret )		( (_close_ret) != 0 )

static inline HMODULE bladeLoadLibrary(const Blade::TString& libPath, bool persitent)
{
	HMODULE ret = ::LoadLibraryEx(libPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (ret == NULL)
		ret = ::LoadLibraryEx(libPath.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);	//seems that different Windows SDK/runtime have different behaviors (when older VS installed, code above will work), try app path also.
	if( persitent && ret != NULL )
		::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCTSTR)(ret), &(HMODULE&)ret);
	return ret;
}

#elif BLADE_IS_UNIX_CLASS_SYSTEM

#	include <dlfcn.h>

#	define Blade_DlSymbol( _handle, _symbol )	dlsym( _handle, _symbol )
#	define Blade_DlClose( _handle )			dlclose( _handle )
#	define Blade_DlCloseOK( _close_ret )		( (_close_ret) == 0 )

static void*	bladeLoadLibrary(const Blade::TString& libPath, bool persitent)
{
	int flag = RTLD_GLOBAL/*RTLD_LOCAL*/;

	//crap, ANDROID doesn't support RTLD_NODELETE.
#	if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#		define	RTLD_NODELETE 0
#	endif

	if( persitent )
		flag |= RTLD_NODELETE;
	Blade::TempBuffer buffer;
	return ::dlopen( Blade::StringConverter::TStringToUTF8String(buffer, libPath), flag );
}


#else

#	error "not implemented."

#endif

#	define Blade_DlLoad( _file, _persistent )	bladeLoadLibrary( _file, _persistent)

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	DynamicLib::DynamicLib()
		:mLibHandle(NULL)
		,mPersistent(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DynamicLib::DynamicLib(const TString& name)
		:mName(name)
		,mLibHandle(NULL)
		,mPersistent(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	DynamicLib::~DynamicLib()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	const TString& DynamicLib::getName() const
	{
		return mName;
	}


	//////////////////////////////////////////////////////////////////////////
	bool			DynamicLib::load(const TString& name, bool except/* = true*/, bool persistent/* = false*/)
	{
		if( mLibHandle == NULL )
		{
			mPersistent = persistent;
			mName = name;

#if BLADE_IS_WINDOWS_CLASS_SYSTEM
			TString platformPath = TStringHelper::getReplace(name, BTString("/"), BTString("\\") );

			tchar buffer[MAX_PATH+1];
			::GetFullPathName(platformPath.c_str(),MAX_PATH, buffer, NULL);
			platformPath = buffer;

			mLibHandle = ::Blade_DlLoad(platformPath, mPersistent);

			if( mLibHandle == NULL )
			{
				DWORD errorCode = ::GetLastError();
				LPSTR lpBuffer;

				::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					errorCode,
					LANG_NEUTRAL,
					(LPTSTR)&lpBuffer,
					0,
					NULL);

				::OutputDebugString( (LPCTSTR)lpBuffer );
				::LocalFree(lpBuffer);
			}

#elif BLADE_PLATFORM == BLADE_PLATFORM_LINUX || BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

			mLibHandle = ::Blade_DlLoad(name, mPersistent);

#else
#	error not implemented.
#endif

			if(mLibHandle == NULL && except)
				BLADE_EXCEPT(EXC_UNKNOWN, BTString("unable to load library: \"") + name + BTString("\"") );
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void			DynamicLib::unload()
	{
		if( mLibHandle != NULL )
		{
			if( mPersistent )
				::Blade_DlClose( mLibHandle);
			else
			{
				//while( Blade_DlCloseOK( ::Blade_DlClose(mLibHandle) ) );
				::Blade_DlClose( mLibHandle);
			}
			mLibHandle = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void*			DynamicLib::getExport(const char* symbol)
	{
		if( mLibHandle != NULL )
		{
			return Blade_DlSymbol( mLibHandle,symbol );
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DynamicLib::operator<(const DynamicLib& rhs) const
	{
		return mName < rhs.getName();
	}

	//////////////////////////////////////////////////////////////////////////
	bool			DynamicLib::operator==(const DynamicLib& rhs) const
	{
		return mName == rhs.mName;
	}

	
}//namespace Blade