/********************************************************************
	created:	2010/06/12
	filename: 	AppMain.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>

/************************************************************************/
/* platform headers                                                                     */
/************************************************************************/
#if BLADE_IS_WINDOWS_CLASS_SYSTEM
#undef TEXT
#include <Windows.h>
#include <tchar.h>
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#define BLADE_USE_PLATFORM_DATA_EXPLICIT
#include <platform/PlatformData.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <android/native_activity.h>
#include <android/log.h>

#include <threading/Thread.h>
#include <interface/public/window/IWindowService.h>
#endif

/************************************************************************/
/* common headers                                                                     */
/************************************************************************/
#include <BladeFramework.h>
#include <interface/IPlatformManager.h>
#include <interface/ILogManager.h>
#include <interface/IFramework.h>
#include <interface/IResourceManager.h>
#include <DebugLogRedirector.h>
#include <Application.h>
#include <ICmdParser.h>
#include <ClientConfig.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/IGraphicsService.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/public/logic/ILogicService.h>
#include <interface/public/window/IWindowService.h>

#include <interface/public/logic/ILogicScene.h>
#include <interface/public/logic/ILogicService.h>
#include <interface/public/logic/ILogicWorldManager.h>
#include <interface/public/logic/ILogic.h>
#include <interface/public/graphics/IImageManager.h>
#include <GameLibrary.h>
#include <App.h>

/** @brief platform specific startup routine */
static void Blade_PlatformStartup();

using namespace Blade;

#define APP_NAME	( BTString("Blade Game") )
#define STAGE_NAME  ( BTString("Default Stage") )
#define APP_DESC	( BTString("Powered by Blade Engine.") )
#define APP_VERSION	( Version(0, 2, 'C') )

static ClientConfig config(APP_NAME, APP_DESC, APP_VERSION);

/************************************************************************/
/* platform entry                                                                     */
/************************************************************************/
#if BLADE_IS_WINDOWS_CLASS_SYSTEM

int APIENTRY _tWinMain(HINSTANCE /*hInstance*/,
					   HINSTANCE /*hPrevInstance*/,
					   LPTSTR    /*lpCmdLine*/,
					   int       /*nCmdShow*/)
{
	int argc = __argc;
	tchar** argv = __targv;

#else //BLADE_PLATFORM

//reference plug-ins (static libs mode only)
//orders are not important, any order is OK.
//using -Wl,--whole-archive may work, but it may have re-defined symbols
BLADE_USE_PLUGIN(BladeDevicePlugin);
BLADE_USE_PLUGIN(TerrainPlugin);
BLADE_USE_PLUGIN(GraphicsSystemPlugin);
BLADE_USE_PLUGIN(WindowSystemPlugin);
BLADE_USE_PLUGIN(UISystemPlugin);
BLADE_USE_PLUGIN(GeometryPlugin);
BLADE_USE_PLUGIN(BladeGamePlugin);
BLADE_USE_PLUGIN(ImageManagerFIPlugin);
BLADE_USE_PLUGIN(AtmospherePlugin);
BLADE_USE_PLUGIN(ModelPlugin);
BLADE_USE_PLUGIN(BladePostFXPlugin);
BLADE_USE_PLUGIN(BladeGrassPlugin);

int main(int argc, char* argv[])
{

#endif //BLADE_PLATFORM

	//init blade
	Blade::initializeFoundation();
	Blade::initializeFramework();
	Blade::initializeAppFramework();
	Blade::initializeGameLibrary();
	//Blade::Memory::enableLeakDump(false);
	Blade_PlatformStartup();
	//init random
	std::srand((uint)std::time(NULL));

	ILogManager::getSingleton().redirectLogs(HLOGREDIRECTOR(BLADE_NEW  DebugLogRedirector()));
	//IPlatformManager::getSingleton().showMessageBox(TEXT("Hello"), TEXT("title"), IPlatformManager::MBT_OK);	//android message test

	//parse the arguments
	bool needRun = ICmdParser::getSingleton().initialize(config, argc, argv);

	if( needRun )
	{
		TString mode = config.getMode();

		if( mode == TString::EMPTY )
			mode = AppFactory::getSingleton().getRegisteredClass(0);
		Application::interchange( mode );
		Application::getSingleton().setPluginConfigFile( BTString("plugins.cfg") );

		try
		{
			if (Application::getSingleton().initialize(config))
			{
				IEventManager::getSingleton().addEventHandler(RenderDeviceReadyEvent::NAME, EventDelegate(&Blade::onRenderDeviceReady, EventDelegate::P_HIGH) );

				//default mode:
				if( config.getMode() == TString::EMPTY )
				{
					Blade::initGameFSM();

					IWindow* window = NULL;

					//auto window? (or Android system with built in window)
					if( IWindowService::getSingleton().getNumWindows() > 0 )
						window = IWindowService::getSingleton().getWindow(0);
					else
						window = IWindowService::getSingleton().createWindow(STAGE_NAME);

					assert( (void*)window->getNativeHandle() != NULL );

					window->setCaption(STAGE_NAME);
					window->setVisibility(true);
					
					//create default stage(scenes) & world
					//game states can create more game stages & worlds

#ifdef MANUAL_SCENE
					//this piece of code demonstrates creating scenes manually for stage,
					//it's more detailed level of control, allowing to create scenes optionally (usually there's no need to do this).
					//@see IStage::DESC::mAutoSceneBinding

					IStage* stage = IFramework::getSingleton().createStage(STAGE_NAME, window, false, false);
					DEF_WORLD_INFO.world = ILogicWorldManager::getSingleton().createWorld(STAGE_NAME);
					DEF_WORLD_INFO.winfo = WORLD_INFO(stage);

					DEF_WORLD_INFO.winfo.graphicsScene = IGraphicsService::getSingleton().createGraphicsScene(BTString("Default Graphics"));
					DEF_WORLD_INFO.winfo.geomScene = IGeometryService::getSingleton().createGeometryScene(BTString("Default Geometry"));
					DEF_WORLD_INFO.winfo.logicScene = ILogicService::getSingleton().createLogicScene(BTString("Default Game Logic"));
					stage->addScene(DEF_WORLD_INFO.info.graphicsScene);
					stage->addScene(DEF_WORLD_INFO.info.geomScene);
					stage->addScene(DEF_WORLD_INFO.info.logicScene);
#else
					IStage* stage = IFramework::getSingleton().createStage(STAGE_NAME, window, false, true);
					DEF_WORLD_INFO.world = ILogicWorldManager::getSingleton().createWorld(STAGE_NAME);
					DEF_WORLD_INFO.winfo = WORLD_INFO(stage);
#endif
					ILogicScene* scene = DEF_WORLD_INFO.winfo.logicScene;
					assert( scene->getLogicWorld() == NULL );
					//bind app logic
					scene->addLogic( BTString("GameLogic") );

					//run
					Application::getSingleton().run(true);

					//clean up
					IFramework::getSingleton().destroyStage(STAGE_NAME);
					ILogicWorldManager::getSingleton().destroyWorld(STAGE_NAME);
				}
				else
					Application::getSingleton().run(true);
			}

			Application::getSingleton().close();
		}

		catch(Exception& e)
		{
			ILog::Error << e.getFullDescription() << ILog::endLog;
			IPlatformManager::getSingleton().showMessageBox(e.getFullDescription(),e.getTypeName() );
			return false;
		}

		catch(std::exception& e)
		{
			ILog::Error << e.what() << ILog::endLog;
			IPlatformManager::getSingleton().showMessageBox(e.what());
			return false;
		}
	}

#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
	::exit(0);
#endif
	return 0;
}

#if BLADE_IS_WINDOWS_CLASS_SYSTEM

#if defined(SHIPPING) || 1
//laptop with NV Optimus enabled
//in editor mode, Optimus doesn't work on Present with hDestWindowOverride.

//now work around this problem by using render window for view port
//@see Viewport.h: USE_RENDER_WINDOW.
//note 0: render window will allocate back buffer for window - a little waste; Presenting using window override doesn't
//note 1: render windows for view ports may differ in size, 
//		editor mode: making buffer sharing across multiple editor files/windows impossible. @see RenderScheme.h: getBuffers & msUseShareBuffer
//update: without using RenderWindow, Optimus works but doesn't work with post effects enabled.
//update2: now Optimus works. dunno why, try to figure it out sometime.
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

//////////////////////////////////////////////////////////////////////////
void pure_virtual_handler()
{
	//just add this handler so that a break point can be set here to view call stack
	::MessageBox(::GetActiveWindow(), TEXT("pure virtual function call"), TEXT("pure virtual"), MB_OK|MB_ICONEXCLAMATION);
};

//////////////////////////////////////////////////////////////////////////
void Blade_PlatformStartup()
{
	::_set_purecall_handler(pure_virtual_handler);
	config.setDataPath( BTString("../Data.bpk") );

	//for ease of direct start/debug
	const TString& exec = IPlatformManager::getSingleton().getProcesFilePath();
	::SetCurrentDirectory(exec.c_str());
	IPlatformManager::getSingleton().setCurrentWorkingPath(exec);
}

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

//////////////////////////////////////////////////////////////////////////
static size_t	extractAsset(AAssetManager* assetManager, const char* subFolder)
{
	assert( assetManager != NULL );
	assert( subFolder != NULL );
	AAssetDir* assetDir = ::AAssetManager_openDir(assetManager, subFolder );
	if(assetDir == NULL )
	{
		assert(false);
		return 0;
	}

	const char* filename = NULL;
	size_t count = 0;
	while ( (filename = ::AAssetDir_getNextFileName(assetDir) ) != NULL)
	{
		StringConcat path(subFolder);
		path += "/";
		path += filename;
		AAsset* asset = ::AAssetManager_open(assetManager, String(path).c_str(), AASSET_MODE_STREAMING);
		char buf[BUFSIZ];
		int nb_read = 0;
		FILE* out = ::fopen(filename, "w");
		if( out != NULL )
		{
			while ( (nb_read = ::AAsset_read(asset, buf, BUFSIZ)) > 0 )
				::fwrite(buf, nb_read, 1, out);
			::fclose(out);
		}
		::AAsset_close(asset);
		++count;
	}
	::AAssetDir_close(assetDir);
	return count;
}

//////////////////////////////////////////////////////////////////////////
void Blade_PlatformStartup()
{
	Thread::setThreadName(Thread::getCurrentID(), BTString("BladeMain"));
	//android Log cat already has time & level
	ILog::Error.enablePrefix(false);
	ILog::Warning.enablePrefix(false);
	ILog::Information.enablePrefix(false);
	ILog::DebugOutput.enablePrefix(false);

	ANativeActivity* activity = PlatformData::getSingleton().getPlatformAppData();
	const TString& packageName = PlatformData::getSingleton().getPackageName();
	const TString& libPath = PlatformData::getSingleton().getLibraryPath();
	int versionCode = PlatformData::getSingleton().getVersionCode();

	//change cwd
#if BLADE_DEBUG || 1
	const char* cwd = activity->externalDataPath;
#else
	const char* cwd = activity->internalDataPath;
#endif
	//ensure path exist
	const char* dirs[] = {activity->externalDataPath, activity->internalDataPath};
	for(size_t i = 0; i < sizeof(dirs)/sizeof(dirs[0]); ++i)
	{
		String dir(dirs[i]);
		for(size_t j = 1; j < dir.size(); ++j)
		{
			if( dir[j] == '/' )
			{
				dir[j] = '\0';
				::mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
				dir[j] = '/';
			}
		}
	}

	int ret = ::chdir( cwd );
	assert(ret == 0);
	BLADE_UNREFERENCED(ret);

	//copy config from APK to cwd & set new config path
	size_t count = ::extractAsset(activity->assetManager, "Config");
	if( count != 0 )
	{
		TempBuffer buffer;
		config.setConfigPath( UTF82TSTRING(buffer, cwd) );
	}
	else
	{
		assert(false);
		return;
	}

	//OBB file path: data path
	TString obbFile = BTString("/sdcard/main.") + TStringHelper::fromInt(versionCode) + BTString(".") + packageName + BTString(".obb");
	struct stat st;
	//always try "/sdcard" first, if it fails, use activity standard path.
	if( blade_stat(obbFile.c_str(), &st) != 0 )
	{
		obbFile = StringConverter::UTF8StringToTString(activity->obbPath);
		obbFile += BTString("/main.") + TStringHelper::fromInt(versionCode) + BTString(".") + packageName + BTString(".obb");
	}

	config.setDataPath( obbFile );
	//set libPath as plugin path
	config.setPluginPath( libPath );
}


//////////////////////////////////////////////////////////////////////////
//real entry point for .so
__attribute__ ((visibility("default")))
	void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
	android_native_activity_oncreate(activity, savedState, savedStateSize, &main);
}

#else

#error not implemented.

#endif
