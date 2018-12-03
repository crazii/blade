/********************************************************************
	created:	2016/07/31
	filename: 	QtDevicePlugin.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <QtDevicePlugin.h>
#include <utility/StringList.h>
#include <utility/StringHelper.h>
#include <interface/IConfigManager.h>
#include <interface/IPlatformManager.h>
#include <interface/IEventManager.h>
#include <interface/IEnvironmentManager.h>

#include <QtWidgets/QStyleFactory>
#include <QtWindowDevice.h>
#include <QtKeyboardDevice.h>
#include <QtMouseDevice.h>
#include <QtMessagePump.h>
#include <QtKeyMap.h>

namespace Blade
{
	static const TString PLUGIN_NAME = BTString("BladeQtDevicePlugin");
	BLADE_DEF_PLUGIN(QtDevicePlugin);

	char* QtApplication::msArgv = "";
	int QtApplication::msArgc = 0;

	//////////////////////////////////////////////////////////////////////////
	QtDevicePlugin::QtDevicePlugin()
		:PluginBase(PLUGIN_NAME)
		,mQtApp(NULL)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	QtDevicePlugin::~QtDevicePlugin()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IPlugin::AddOnObject			QtDevicePlugin::getAddOnObject(const TString& name) const
	{
		if (name == BTString("QT_KEYMAPFUNC"))
		{
			return &QtMapKey;
		}
		return PluginBase::getAddOnObject(name);
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtDevicePlugin::getSupportList(TStringParam& supportList) const
	{
		supportList.push_back(BTString("WindowDevice"));
		supportList.push_back(BTString("KeyboardDevice"));
		supportList.push_back(BTString("MouseDevice"));
		supportList.push_back(BTString("QtDevices"));	//add special signature for Qt devices (Qt UI need QtDevices, not any common window/keyboard/mouse)
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtDevicePlugin::getDependency(TStringParam& dependencyList) const
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				QtDevicePlugin::install()
	{
		NameRegisterFactory(QtWindowDevice,IDevice, QtWindowDevice::QT_DEVICE_TYPE);
		NameRegisterFactory(QtWindowDevice,IWindowDevice, QtWindowDevice::QT_DEVICE_TYPE);
		NameRegisterSingleton(QtMessagePump,IWindowMessagePump, QtWindowDevice::QT_DEVICE_TYPE);

		NameRegisterFactory(QtKeyboardDevice,IDevice, QtKeyboardDevice::QT_DEVICE_TYPE);
		NameRegisterFactory(QtKeyboardDevice,IKeyboardDevice, QtKeyboardDevice::QT_DEVICE_TYPE);

		NameRegisterFactory(QtMouseDevice,IDevice, QtMouseDevice::QT_DEVICE_TYPE);
		NameRegisterFactory(QtMouseDevice,IMouseDevice, QtMouseDevice::QT_DEVICE_TYPE);

		TString pluginPath = IPlatformManager::getSingleton().getPluginPath();
		if (!TStringHelper::isAbsolutePath(pluginPath))
			pluginPath = TStringHelper::standardizePath( IPlatformManager::getSingleton().getProcesFilePath() + BTString("/") + pluginPath );
		QApplication::addLibraryPath(QString((QChar*)pluginPath.c_str(), (int)pluginPath.size()));
		QStringList s = QApplication::libraryPaths();

		//note: QApplication is needed even for an UI DLL
		//we don't need to call app.exec() because we use the message pump of WindowSystem, or QtMessagePump
		mQtApp = BLADE_NEW QtApplication();

		////QFile File("DarkMonokai.qss");
		//QFile File("dracula.css");
		//File.open(QFile::ReadOnly);
		//QString styleSheet = QLatin1String(File.readAll());
		//mQtApp->setStyleSheet(styleSheet);

		mQtApp->setStyle(QStyleFactory::create("Fusion"));

		QPalette palette;
		palette.setColor(QPalette::Window, QColor(53, 53, 53));
		palette.setColor(QPalette::WindowText, Qt::white);
		palette.setColor(QPalette::Base, QColor(15, 15, 15));
		palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
		palette.setColor(QPalette::ToolTipBase, QColor(42, 130, 218));
		palette.setColor(QPalette::ToolTipText, Qt::white);
		palette.setColor(QPalette::Text, Qt::white);
		palette.setColor(QPalette::Button, QColor(53, 53, 53));
		palette.setColor(QPalette::ButtonText, Qt::white);
		palette.setColor(QPalette::BrightText, Qt::red);
		palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);

		//palette.setColor(QPalette::Highlight, QColor(142, 45, 197).lighter());
		//palette.setColor(QPalette::HighlightedText, Qt::black);
		mQtApp->setPalette(palette);
		mQtApp->setAttribute(Qt::AA_DontUseNativeDialogs, true);
		
		mQtApp->setApplicationName("Blade Qt Device Plugin");
		mQtApp->setOrganizationName("Crazii");
		mQtApp->setOrganizationDomain("");
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtDevicePlugin::uninstall()
	{
		BLADE_DELETE mQtApp;
	}

	//////////////////////////////////////////////////////////////////////////
	void				QtDevicePlugin::initialize()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	void				QtDevicePlugin::shutdown()
	{

	}
	
}//namespace Blade