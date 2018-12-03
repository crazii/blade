/********************************************************************
	created:	2010/04/02
	filename: 	UIPlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePlatform.h>

#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
#include "stdafx.h"
#include <ConfigDialog/ConfigDialog.h>
#include <ConfigDialog/ConfigDialogProxy.h>
#include <ui/MainWindow.h>
#include <ui/MenuManager.h>
#include <ui/IconManager.h>
#include <ui/SplashOutput.h>

#include "ui/uiwindows/ViewportWindow.h"
#include "ui/uiwindows/TableWindow.h"
static Blade::ConfigDialogProxy DialogProxy;
#endif

#include <BladeUIPlugin.h>
#include <UIPlugin.h>
#include <interface/IConfigManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IEventManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/ui/UIEvents.h>
#include "ui/UIMediaLibrary.h"
#include "ui/widgets/TabedReBar.h"
#include "ui/widgets/PropertyWindow.h"
#include "ui/widgets/MediaView.h"
#include "ui/widgets/TrackView.h"
#include "ui/widgets/InfoWindow.h"
#include "ui/widgets/LogWindow.h"

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladeUIPlugin");
	BLADE_DEF_PLUGIN(BladeUIPlugin);

	//////////////////////////////////////////////////////////////////////////
	BladeUIPlugin::BladeUIPlugin()
		:PluginBase(PLUGIN_NAME)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	BladeUIPlugin::~BladeUIPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IPlugin::AddOnObject			BladeUIPlugin::getAddOnObject(const TString& name) const
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		if (name == BTString("HINSTANCE"))
		{
			AFX_MANAGE_STATE(::AfxGetStaticModuleState());
			HINSTANCE hInst = ::AfxGetInstanceHandle();
			return hInst;
		}
#endif
		return PluginBase::getAddOnObject(name);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeUIPlugin::getSupportList(TStringParam& supportList) const
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		supportList.push_back(BTString("ConfigDialog"));
		supportList.push_back(BTString("UI"));
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeUIPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("ImageManager"));
		dependencyList.push_back(BTString("WindowService"));
		dependencyList.push_back(BTString("UIService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeUIPlugin::install()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		NameRegisterWindowFactory(MainWindow, IUIMainWindow, MainWindow::MAIN_WINDOW_TYPE);
		NameRegisterWindowFactory(TabedReBar, IUIWidget, TabedReBar::WIDGET_TYPE);
		NameRegisterWindowFactory(PropertyWindow, IUIWidget, PropertyWindow::WIDGET_TYPE);
		NameRegisterWindowFactory(MediaView, IUIWidget, MediaView::WIDGET_TYPE);
		NameRegisterWindowFactory(LogWindow, IUIWidget, LogWindow::WIDGET_TYPE);
		NameRegisterWindowFactory(TrackView, IUIWidget, TrackView::WIDGET_TYPE);
		NameRegisterWindowFactory(InfoWindow, IUIWidget, InfoWindow::WIDGET_TYPE);

		if( Factory<IStartupOutput>::getSingleton().getNumRegisteredClasses() == 0 )
			RegisterSingleton(SplashOutput, IStartupOutput);
		else
			NameRegisterSingleton(SplashOutput, IStartupOutput, BTString("Win32MFCStartupOutput"));

		if (Factory<IUIMediaLibrary>::getSingleton().getNumRegisteredClasses() == 0)
			RegisterSingleton(UIMediaLibraryImpl, IUIMediaLibrary);
		else
			NameRegisterSingleton(UIMediaLibraryImpl, IUIMediaLibrary, BTString("Win32MFCMediaLibrary"));

		//avoid console window focus blink for tools
		if( IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) != BTString("tool") )
		{
			AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
			CUISplashWindow* splash = BLADE_NEW CUISplashWindow(NULL);
			splash->Create(CUISplashWindow::IDD, NULL);
			splash->ShowWindow(SW_HIDE);
			SplashOutput::getSingleton().initialize(splash);
			ThemeManager::getSingleton().initialize();
		}
		else
		{
			DialogProxy.hideStartupOption();
			DialogProxy.hideDialgBanner();
		}

		if( Factory<IMenuManager>::getSingleton().getNumRegisteredClasses() == 0 )
			RegisterSingleton(MenuManager,IMenuManager);
		else
			NameRegisterSingleton(MenuManager,IMenuManager,BTString("Win32MFCMenuManager"));

		if( Factory<IIconManager>::getSingleton().getNumRegisteredClasses() == 0 )
			RegisterSingleton(IconManager,IIconManager);
		else
			NameRegisterSingleton(IconManager,IIconManager,BTString("Win32MFCIconManager"));
		IConfigManager::getSingleton().setConfigDialog(&DialogProxy);

		//may be used for other UI module
		NameRegisterWindowFactory(ViewportWindow, IUIWindow, IViewportWindow::VIEWPORT_WINDOW_TYPE);
		NameRegisterWindowFactory(TableWindow, IUIWindow, ITableWindow::TABLE_WINDOW_TYPE);
#endif
		IEventManager::getSingleton().registerEvent( EntitySlectedEvent::NAME );
		IEventManager::getSingleton().registerEvent( OptionHighLightEvent::NAME );
		IEventManager::getSingleton().registerEvent( ConfigurableSlectedEvent::NAME );
		IEventManager::getSingleton().registerEvent(MediaViewEvent::NAME);
		IEventManager::getSingleton().registerEvent(ViewportCameraConfigEvent::NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeUIPlugin::uninstall()
	{
		IEventManager::getSingleton().unRegisterEvent( EntitySlectedEvent::NAME );
		IEventManager::getSingleton().unRegisterEvent( OptionHighLightEvent::NAME );
		IEventManager::getSingleton().unRegisterEvent( ConfigurableSlectedEvent::NAME );
		IEventManager::getSingleton().unRegisterEvent(MediaViewEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(ViewportCameraConfigEvent::NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeUIPlugin::initialize()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS

		if( IConfigManager::getSingleton().getEntry(BTString("ShowDialog"), ConifgDialogSection ) == TString::EMPTY )
		{
			IConfigManager::getSingleton().addEntry(BTString("ShowDialog"),BTString("TRUE"),ConifgDialogSection );
		}

		bool skipDependency = IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) == BTString("tool");

		const TString& showDialog = IConfigManager::getSingleton().getEntry( BTString("ShowDialog"),ConifgDialogSection ) ;
		if( !skipDependency && TStringHelper::toBool( showDialog ) )
		{
			IConfigManager::getSingleton().showConfigDialog();
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void				BladeUIPlugin::shutdown()
	{
#if BLADE_PLATFORM == BLADE_PLATFORM_WINDOWS
		IconManager::getSingleton().shutdown();
#endif
	}

	
}//namespace Blade