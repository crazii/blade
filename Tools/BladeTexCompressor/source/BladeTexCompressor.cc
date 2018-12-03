/********************************************************************
	created:	2015/01/14
	filename: 	BladeTexCompressor.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <tchar.h>
#include <Windows.h>
#endif

#include <BladeFramework.h>
#include <ExceptionEx.h>
#include <TexCompressorApp.h>

using namespace Blade;

int main(int argc, char* argv[])
{
	//debug only
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
	//::MessageBox(NULL,TEXT("attach process for debug"), TEXT("Attach"), MB_ICONEXCLAMATION);
#endif

	//init blade
	Blade::initializeFoundation();
	Blade::initializeFramework();
	Blade::initializeAppFramework();

	//
	TexCompressorConfig config(BTString("Blade Texture Compressor"), BTString("Texture Compressor Tool for Blade Engine."), Version(0,2,'B') );
	//parse the arguments
	bool needRun = ICmdParser::getSingleton().initialize(config, argc, argv);

	if( needRun )
	{
		TexCompressorApp app;

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

		return app.hasError() ? -1 : 0;
	}

	return 0;
}

