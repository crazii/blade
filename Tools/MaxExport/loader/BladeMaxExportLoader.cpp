//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/
#include <BladePlatform.h>
#pragma warning(disable:4191 4365 4471 4266 4263 4264 4640 4458)
#include "BladeMaxExportLoader.h"
#include <string>


/************************************************************************/
/*                                                                      */
/************************************************************************/
extern "C"
{
	typedef int (__cdecl *EXP_ENTRY)(const TCHAR *name,ExpInterface *ei,Interface* i, BOOL suppressPrompts, DWORD options);
	typedef const TCHAR* (__cdecl *FILE_EXT_FUNC)();
};

namespace std
{
#ifdef UNICODE
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}//namespace std

//////////////////////////////////////////////////////////////////////////
HMODULE openExporter()
{
	BOOL ret;
	TCHAR path[MAX_PATH+1];
	ret = ::GetModuleFileName(hLoaderInstance, path, MAX_PATH);
	assert(ret);
	std::tstring pathStr = path;
	pathStr = pathStr.substr(0, pathStr.rfind(TEXT("\\")) );
#if defined(DEBUG) || (_DEBUG)
	pathStr += TEXT("\\Blade_Debug");
#else
	pathStr += TEXT("\\Blade_Release");
#endif
	pathStr += TEXT("\\BladeMaxExport.dll");

	HMODULE hModule = (HMODULE)::LoadLibraryEx( pathStr.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if( hModule == NULL )
	{
		int n = GetLastError();
		n = 1;
	}
	assert( hModule != NULL );
	return hModule;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

#define MaxExport_CLASS_ID	Class_ID(0x721a4a68, 0x98f9f764)

class MaxExportClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 	{ return new BladeMaxExportLoader(); }
	virtual const TCHAR *	ClassName() 			{ return TEXT("BladeMaxExport"); }
	virtual SClass_ID SuperClassID() 				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return MaxExport_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return TEXT("Blade Max Plugin"); }

	virtual const TCHAR* InternalName() 			{ return _T("BladeMaxExport"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hLoaderInstance; }				// returns owning module handle
};


ClassDesc2* GetBladeMaxExportDesc()
{ 
	static MaxExportClassDesc MaxExportDesc;
	return &MaxExportDesc; 
}

//--- BladeMaxExportLoader -------------------------------------------------------
BladeMaxExportLoader::BladeMaxExportLoader()
{
}

BladeMaxExportLoader::~BladeMaxExportLoader() 
{
}

int BladeMaxExportLoader::ExtCount()
{
	//Returns the number of file name extensions supported by the plug-in
	return 1;
}

const TCHAR *BladeMaxExportLoader::Ext(int n)
{		
	//Return the 'i-th' file name extension (i.e. \"3DS\").
	if( n == 0 )
	{
		//load real exporter DLL
		HMODULE hModule = openExporter();

		FILE_EXT_FUNC entry = (FILE_EXT_FUNC)::GetProcAddress(hModule, "BladeMaxExportGetFileExtension");
		assert(entry != NULL);
		//copy result before free library, because result may be static const buffer built into dll image
		static TCHAR EXT[256];
		const TCHAR* ret = (entry != NULL) ? entry() : TEXT("");
		::memcpy(EXT, ret, (_tcslen(ret)+1)*sizeof(TCHAR) );
		::FreeLibrary(hModule);
		return EXT;
	}
	else
		return TEXT("");
}

const TCHAR *BladeMaxExportLoader::LongDesc()
{
	//Return long ASCII description (i.e. \"Targa 2.0 Image File\")
	return TEXT("Model File for Blade Game Engine.");
}
	
const TCHAR *BladeMaxExportLoader::ShortDesc() 
{			
	//Return short ASCII description (i.e. \"Targa\")
	return TEXT("Blade Model");
}

const TCHAR *BladeMaxExportLoader::AuthorName()
{			
	//Return ASCII Author name
	return TEXT("Crazii");
}

const TCHAR *BladeMaxExportLoader::CopyrightMessage() 
{	
	//Return ASCII Copyright message
	return TEXT("Copyright(C) 2013");
}

const TCHAR *BladeMaxExportLoader::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *BladeMaxExportLoader::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int BladeMaxExportLoader::Version()
{				
	//Return Version number * 100 (i.e. v3.01 = 301)
	return 002;
}

void BladeMaxExportLoader::ShowAbout(HWND /*hWnd*/)
{			
	// Optional
}

BOOL BladeMaxExportLoader::SupportsOptions(int /*ext*/, DWORD /*options*/)
{
	//Decide which options to support.  Simply return true for each option supported by each Extension the exporter supports.
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
int	BladeMaxExportLoader::DoExport(const TCHAR *name,ExpInterface *ei,Interface* i, BOOL suppressPrompts, DWORD options)
{
	//load real exporter DLL
	HMODULE hModule = openExporter();

	EXP_ENTRY entry = (EXP_ENTRY)::GetProcAddress(hModule, "BladeMaxExportEntry");
	assert(entry != NULL);
	int result = (entry != NULL) && entry(name, ei, i, suppressPrompts, options);

	__try
	{
		::FreeLibrary(hModule);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return result;
}






/************************************************************************/
/*                                                                      */
/************************************************************************/
#define MaxAnimationExport_CLASS_ID	Class_ID(0x66c25f5d, 0x77790dcb)

class MaxAnimationExportClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 	{ return new BladeMaxAnimationExportLoader(); }
	virtual const TCHAR *	ClassName() 			{ return TEXT("BladeAnimationMaxExport"); }
	virtual SClass_ID SuperClassID() 				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return MaxAnimationExport_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return TEXT("Blade Max Plugin"); }

	virtual const TCHAR* InternalName() 			{ return _T("BladeAnimationMaxExport"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hLoaderInstance; }						// returns owning module handle
};


ClassDesc2* GetBladeMaxAnimationExportDesc()
{ 
	static MaxAnimationExportClassDesc MaxExportDesc;
	return &MaxExportDesc; 
}

//--- BladeMaxAnimationExportLoader -------------------------------------------------------
BladeMaxAnimationExportLoader::BladeMaxAnimationExportLoader()
{
}

BladeMaxAnimationExportLoader::~BladeMaxAnimationExportLoader() 
{
}

int BladeMaxAnimationExportLoader::ExtCount()
{
	//Returns the number of file name extensions supported by the plug-in
	return 1;
}

const TCHAR *BladeMaxAnimationExportLoader::Ext(int n)
{		
	//Return the 'i-th' file name extension (i.e. \"3DS\").
	if( n == 0 )
	{
		//load real exporter DLL
		HMODULE hModule = openExporter();

		FILE_EXT_FUNC entry = (FILE_EXT_FUNC)::GetProcAddress(hModule, "BladeMaxAnimationExportGetFileExtension");
		assert(entry != NULL);
		//copy result before free library, because result may be static const buffer built into dll image
		static TCHAR EXT[256];
		const TCHAR* ret = (entry != NULL) ? entry() : TEXT("");
		::memcpy(EXT, ret, (_tcslen(ret)+1)*sizeof(TCHAR) );
		::FreeLibrary(hModule);
		return EXT;
	}
	else
		return TEXT("");
}

const TCHAR *BladeMaxAnimationExportLoader::LongDesc()
{
	//Return long ASCII description (i.e. \"Targa 2.0 Image File\")
	return TEXT("Animation File for Blade Game Engine.");
}

const TCHAR *BladeMaxAnimationExportLoader::ShortDesc() 
{			
	//Return short ASCII description (i.e. \"Targa\")
	return TEXT("Blade Animation");
}

const TCHAR *BladeMaxAnimationExportLoader::AuthorName()
{			
	//Return ASCII Author name
	return TEXT("Crazii");
}

const TCHAR *BladeMaxAnimationExportLoader::CopyrightMessage() 
{	
	//Return ASCII Copyright message
	return TEXT("Copyright(C) 2013");
}

const TCHAR *BladeMaxAnimationExportLoader::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *BladeMaxAnimationExportLoader::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int BladeMaxAnimationExportLoader::Version()
{				
	//Return Version number * 100 (i.e. v3.01 = 301)
	return 002;
}

void BladeMaxAnimationExportLoader::ShowAbout(HWND /*hWnd*/)
{			
	// Optional
}

BOOL BladeMaxAnimationExportLoader::SupportsOptions(int /*ext*/, DWORD /*options*/)
{
	//Decide which options to support.  Simply return true for each option supported by each Extension the exporter supports.
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
int	BladeMaxAnimationExportLoader::DoExport(const TCHAR *name,ExpInterface *ei,Interface* i, BOOL suppressPrompts, DWORD options)
{
	//load real exporter DLL
	HMODULE hModule = openExporter();

	EXP_ENTRY entry = (EXP_ENTRY)::GetProcAddress(hModule, "BladeMaxExportEntry");
	assert(entry != NULL);
	int result = (entry != NULL) && entry(name, ei, i, suppressPrompts, options);

	__try
	{
		::FreeLibrary(hModule);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return result;
}