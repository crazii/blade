#include <QtPluginPCH.h>
#include "QtPlugin.h"
#include <interface/IConfigManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IEventManager.h>
#include <interface/IEnvironmentManager.h>
#include <interface/public/ui/UIEvents.h>
#include <QtMainWindow.h>
#include <QtMenuManager.h>
#include <QtIconManager.h>
#include <QtSplash.h>
#include <QtConfigDialog.h>
#include <QtMediaLibrary.h>

#include <widgets/QtToolPanel.h>
#include <widgets/QtPropertyPanel.h>
#include <widgets/QtMediaView.h>
#include <widgets/QtLogWindow.h>
#include <widgets/QtTrackView.h>

#include <uiwindows/QtViewportWindow.h>
#include <uiwindows/QtTableWindow.h>

//////////////////////////////////////////////////////////////////////////
using namespace Blade;
static const TString PLUGIN_NAME = BTString("BladeQtUIPlugin");
static QtPlugin PLUGIN;

extern "C"
{
	BLADE_PLUGIN_API size_t BladePluginCount(void)
	{
		return 1;
	}

	BLADE_PLUGIN_API IPlugin* BladeGetPlugin(size_t)
	{
		return &PLUGIN;
	}
}

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	QtPlugin::QtPlugin()
		:PluginBase(PLUGIN_NAME)
		,mDefDialog(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	QtPlugin::~QtPlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				QtPlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("ConfigDialog"));
		supportList.push_back(BTString("UI"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtPlugin::getDependency(TStringParam& dependencyList) const
	{
		dependencyList.push_back(BTString("QtDevices"));	//note: it depend on special device other than common ones
		dependencyList.push_back(BTString("WindowDevice"));
		dependencyList.push_back(BTString("KeyboardDevice"));
		dependencyList.push_back(BTString("MouseDevice"));
		dependencyList.push_back(BTString("ImageManager"));
		dependencyList.push_back(BTString("UIService"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtPlugin::install()
	{
		NameRegisterFactory(QtMainWindow, IUIMainWindow, QtMainWindow::QT_MAIN_WINDOW_TYPE);
		RegisterSingleton(QtIconManager, IIconManager);
		RegisterSingleton(QtMenuManager, IMenuManager);
		RegisterSingleton(QtSplash, IStartupOutput);
		RegisterSingleton(QtMediaLibrary, IUIMediaLibrary);

		NameRegisterFactory(QtToolPanel, IUIWidget, QtToolPanel::WIDGET_TYPE);
		NameRegisterFactory(QtPropertyPanel, IUIWidget, QtPropertyPanel::WIDGET_TYPE);
		NameRegisterFactory(QtMediaView, IUIWidget, QtMediaView::WIDGET_TYPE);
		NameRegisterFactory(QtLogWindow, IUIWidget, QtLogWindow::WIDGET_TYPE);
		NameRegisterFactory(QtTrackView, IUIWidget, QtTrackView::WIDGET_TYPE);

		NameRegisterFactory(QtViewportWindow, IUIWindow, IViewportWindow::VIEWPORT_WINDOW_TYPE);
		NameRegisterFactory(QtTableWindow, IUIWindow, ITableWindow::TABLE_WINDOW_TYPE);

		IEventManager::getSingleton().registerEvent(EntitySlectedEvent::NAME);
		IEventManager::getSingleton().registerEvent(OptionHighLightEvent::NAME);
		IEventManager::getSingleton().registerEvent(ConfigurableSlectedEvent::NAME);
		IEventManager::getSingleton().registerEvent(MediaViewEvent::NAME);
		IEventManager::getSingleton().registerEvent(ViewportCameraConfigEvent::NAME);

		mDefDialog = BLADE_NEW QtConfigDialog();
		mDefDialog->setMode(QtConfigDialog::TOPLEVEL);
		IConfigManager::getSingleton().setConfigDialog(mDefDialog);
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtPlugin::uninstall()
	{
		BLADE_DELETE mDefDialog;

		IEventManager::getSingleton().unRegisterEvent(EntitySlectedEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(OptionHighLightEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(ConfigurableSlectedEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(MediaViewEvent::NAME);
		IEventManager::getSingleton().unRegisterEvent(ViewportCameraConfigEvent::NAME);
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtPlugin::initialize()
	{
		if( IConfigManager::getSingleton().getEntry(BTString("ShowDialog"), CONFIG_DIALOG_SECTION) == TString::EMPTY )
		{
			IConfigManager::getSingleton().addEntry(BTString("ShowDialog"),BTString("TRUE"), CONFIG_DIALOG_SECTION);
		}

		bool skipDependency = IEnvironmentManager::getSingleton().getVariable( ConstDef::EnvString::WORKING_MODE) == BTString("tool");

		const TString& showDialog = IConfigManager::getSingleton().getEntry( BTString("ShowDialog"), CONFIG_DIALOG_SECTION) ;
		if( !skipDependency && TStringHelper::toBool( showDialog ) )
		{
			IConfigManager::getSingleton().showConfigDialog();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtPlugin::shutdown()
	{
		QtMenuManager::getSingleton().shutdown();
	}

	//////////////////////////////////////////////////////////////////////////
	EKeyCode QtPlugin::getKeyCode(int key, Qt::KeyboardModifiers modifier)
	{
		typedef EKeyCode (*KEYMAP_FUNC)(int, Qt::KeyboardModifiers);
		static KEYMAP_FUNC keymapFunc = NULL;

		if(keymapFunc == NULL)
		{
			IPlugin* qtdevicePlugin = IPlatformManager::getSingleton().getPlugin(BTString("BladeQtDevicePlugin"));
			assert(qtdevicePlugin != NULL);
			keymapFunc = (KEYMAP_FUNC)qtdevicePlugin->getAddOnObject(BTString("QT_KEYMAPFUNC"));
			assert(keymapFunc != NULL);
		}
		
		return keymapFunc(key, modifier);
	}

}//namespace Blade