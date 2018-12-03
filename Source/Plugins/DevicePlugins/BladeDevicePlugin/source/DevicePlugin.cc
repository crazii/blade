/********************************************************************
	created:	2010/04/09
	filename: 	DevicePlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladeTypes.h>
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#	define NOMINMAX
#	undef TEXT
#	include <Windows.h>

#	include <window/windows/Win32WindowDevice.h>
#	include <window/windows/Win32WindowMessagePump.h>
#	include <graphics/windows/D3D9/D3D9RenderDevice.h>
#	include <graphics/windows/D3D9/D3D9GraphicsResourceManager.h>
#	include <input/windows/Win32MouseDevice.h>
#	include <input/windows/Win32KeyMap.h>
#	include <input/windows/Win32KeyboardDevice.h>
#	include <input/windows/DInput8Mouse.h>
#	include <input/windows/DInput8Keyboard.h>

#include "D3DIncludeLoaderEx.h"
#include "D3D9Helper.h"

namespace Blade
{
	static const EKeyCode* getKeyMap(void)
	{
		return KeyMap;
	}
	
}//namespace Blade

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

#include <android/native_activity.h>
#include <window/android/AndroidWindowDevice.h>
#include <window/android/AndroidWindowMessagePump.h>
#include <input/android/AndroidTouchDevice.h>
#include <file/android/ApkFileDevice.h>
#include <graphics/GLES/GLESRenderDevice.h>
#include <graphics/GLES/GLESGraphicsResourceManager.h>
#endif

#include <BladeDevicePlugin.h>
#include <BladeDevice.h>
#include <interface/IPlatformManager.h>
#include "DevicePlugin.h"
#if BLADE_USE_GLES
#include <ConstDef.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/IEventManager.h>
#endif

namespace Blade
{
	static const TString		PLUGIN_NAME = BTString("BladeDevice");
	BLADE_DEF_PLUGIN(BladeDevicePlugin);

	//////////////////////////////////////////////////////////////////////////
	BladeDevicePlugin::BladeDevicePlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BladeDevicePlugin::~BladeDevicePlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IPlugin::AddOnObject	BladeDevicePlugin::getAddOnObject(const TString& name) const
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		if (name == BTString("GET_KEYMAP"))
			return &getKeyMap;
#else
		BLADE_UNREFERENCED(name);
#endif
		return PluginBase::getAddOnObject(name);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeDevicePlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("RenderDevice"));
		supportList.push_back(BTString("WindowDevice"));

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		supportList.push_back(BTString("KeyboardDevice"));
		supportList.push_back(BTString("MouseDevice"));
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
		supportList.push_back(BTString("TouchDevice"));
		supportList.push_back(BTString("FileDevice"));
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	void				BladeDevicePlugin::getDependency(TStringParam& dependencyList) const
	{
		//render device need image system
		dependencyList.push_back( BTString("ImageManager") );
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeDevicePlugin::install()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		NameRegisterFactory(Win32WindowDevice,IDevice, Win32WindowDevice::WIN32_WINDOW_DEVICE_TYPE);
		NameRegisterFactory(Win32WindowDevice,IWindowDevice, Win32WindowDevice::WIN32_WINDOW_DEVICE_TYPE);
		NameRegisterSingleton(Win32MessagePump,IWindowMessagePump, Win32WindowDevice::WIN32_WINDOW_DEVICE_TYPE);

		NameRegisterFactory(D3D9RenderDevice, IDevice,BTString("Win32RenderD3D9"));
		NameRegisterFactory(D3D9RenderDevice,IRenderDevice,BTString("Direct3D9"));

		NameRegisterFactory(Win32KeyboardDevice,IDevice,BTString("Win32DefaultKeyboard"));
		NameRegisterFactory(Win32KeyboardDevice,IKeyboardDevice,BTString("Win32Default"));
		NameRegisterFactory(DirectInputKeyboard,IKeyboardDevice,BTString("DirectInput8"));

		NameRegisterFactory(Win32MouseDevice,IDevice,BTString("Win32DefaultMouse"));
		NameRegisterFactory(Win32MouseDevice,IMouseDevice,BTString("Win32Default"));
		NameRegisterFactory(DirectInputMouse,IMouseDevice,BTString("DirectInput8"));

		RegisterSingleton(D3D9GraphicsResourceManager,ID3D9LostResourceManager);
		NameRegisterSingleton(D3D9GraphicsResourceManager,IGraphicsResourceManager, D3D9GraphicsResourceManager::TYPE);
		RegisterSingleton(D3D9Helper,ID3D9Helper);

#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

		NameRegisterFactory(AndroidWindowDevice, IDevice, AndroidWindowDevice::ANDROID_WINDOW_TYPE);
		NameRegisterFactory(AndroidWindowDevice, IWindowDevice, AndroidWindowDevice::ANDROID_WINDOW_TYPE );
		NameRegisterSingleton(AndroidMessagePump,IWindowMessagePump, AndroidWindowDevice::ANDROID_WINDOW_TYPE);

		NameRegisterFactory(AndroidTouchDevice, ITouchDevice, BTString("AndroidNative"));
		NameRegisterFactory(ApkFileDevice, IFileDevice, ApkFileDevice::APK_FILE_EXT);

#if BLADE_USE_GLES
		NameRegisterFactory(GLESRenderDevice, IRenderDevice, BTString("GLESRenderDevice"));
		NameRegisterSingleton(GLESGraphicsResourceManager, IGraphicsResourceManager, GLESGraphicsResourceManager::TYPE);
#endif

#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeDevicePlugin::uninstall()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	static const TString INCLUDE_TYPE = BTString("PackageIncludeLoader");
	void				BladeDevicePlugin::initialize()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		NameRegisterFactory(D3DIncludeLoader,D3DIncludeLoader,BTString("Default") );
		NameRegisterFactory(D3DIncludeLoaderEx,D3DIncludeLoader,INCLUDE_TYPE);
		D3DIncludeLoader::setIncludeLoaderType( INCLUDE_TYPE );
#elif BLADE_PLATFORM == BLADE_PLATFORM_ANDROID

#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeDevicePlugin::shutdown()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#endif
	}
	
}//namespace Blade