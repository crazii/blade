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
// DESCRIPTION: Contains the Dll Entry stuff
// AUTHOR: 
//***************************************************************************/
#include <BladePlatform.h>
#pragma warning(disable:4191 4365 4471 4266 4263 4264 4458)
#include "3dsmaxsdk_preinclude.h"
#include "Max.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "BladeMaxExportLoader.h"

extern ClassDesc2* GetBladeMaxExportDesc();
extern ClassDesc2* GetBladeMaxAnimationExportDesc();

//////////////////////////////////////////////////////////////////////////

HINSTANCE hLoaderInstance;
int controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID /*lpvReserved*/)
{
	if( fdwReason == DLL_PROCESS_ATTACH )
	{
		// Hang on to this DLL's instance handle.
		hLoaderInstance = hinstDLL;
		DisableThreadLibraryCalls(hLoaderInstance);
	}
	return(TRUE);
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
	return TEXT("Blade Engine Exporter for 3DS Max.");
}

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	return 2;
}

// This function returns the number of plug-in classes this DLL
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetBladeMaxExportDesc();
		case 1: return GetBladeMaxAnimationExportDesc();
		default: return 0;
	}
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// This function is called once, right after your plugin has been loaded by 3ds Max. 
// Perform one-time plugin initialization in this method.
// Return TRUE if you deem your plugin successfully loaded, or FALSE otherwise. If 
// the function returns FALSE, the system will NOT load the plugin, it will then call FreeLibrary
// on your DLL, and send you a message.
__declspec( dllexport ) int LibInitialize(void)
{
	return TRUE; // TODO: Perform initialization here.
}

// This function is called once, just before the plugin is unloaded. 
// Perform one-time plugin un-initialization in this method."
// The system doesn't pay attention to a return value.
__declspec( dllexport ) int LibShutdown(void)
{
	return TRUE;// TODO: Perform un-initialization here.	
}

TCHAR *GetString(int /*id*/)
{
	static TCHAR buf[256];

	if(hLoaderInstance)
		return TEXT("BladeMaxExport");
	return NULL;
}

