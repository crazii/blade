/********************************************************************
	created:	2013/10/30
	filename: 	BladeLangBuild.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePlatform.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <BladeFramework.h>
#include <LangBuildApplication.h>
#include <ExceptionEx.h>

using namespace Blade;

int main(int argc, char* argv[])
{
	//init blade
	Blade::initializeFoundation();
	Blade::initializeFramework();
	Blade::initializeAppFramework();

	//
	LangBuildConfig config(BTString("Blade Shader Compiler"), BTString("Compiler Tool for Blade Engine."), Version(0,2,'B') );
	//parse the arguments
	bool needRun = ICmdParser::getSingleton().initialize(config, argc, argv);

	if( needRun )
	{
		LangBuildApplication app;

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