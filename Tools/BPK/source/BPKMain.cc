/********************************************************************
	created:	2014/02/15
	filename: 	BPKMain.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <tchar.h>
#include <Windows.h>
#endif
#include <BladeFramework.h>
#include <BPKApplication.h>
#include <BladeException.h>

using namespace Blade;

int main(int argc, char* argv[])
{
	//init blade
	Blade::initializeFoundation();
	Blade::initializeFramework();
	Blade::initializeAppFramework();

	//
	BPKAppConfig config(BTString("Blade BPK package tool"), BTString("Package Tool for Blade Engine."), Version(0,2,'C') );

	//parse the arguments
	bool needRun = ICmdParser::getSingleton().initialize(config, argc, argv);
	if( needRun )
	{
		BPKApplication app;

		try
		{
			if( app.initialize(config) )
			{
				app.run(true);
			}
			app.close();
		}

		catch(Exception& e)
		{
	#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
			::MessageBox(NULL, e.getFullDescription(), TEXT("exception"), MB_ICONEXCLAMATION);
	#endif
		}

		catch(...)
		{
	#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
			::MessageBox(NULL,TEXT("exception!"), TEXT("Error"), MB_ICONEXCLAMATION);
	#endif
		}
	}
	return 0;
}

