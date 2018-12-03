/********************************************************************
	created:	2016/07/31
	filename: 	QtDevicePlugin.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtDevicePlugin_h__
#define __Blade_QtDevicePlugin_h__
#include <BladeQtDevice.h>
#include <PluginBase.h>
#include <QtWidgets/QApplication>

namespace Blade
{

	class QtApplication : public QApplication, public StaticAllocatable
	{
	public:
		static char* msArgv;
		static int msArgc;
		QtApplication()
			:QApplication(msArgc, &msArgv)
		{
		}
	};

	class BLADE_QTDEVICE_PLUGIN_API QtDevicePlugin : public Blade::PluginBase
	{
	public:
		QtDevicePlugin();
		~QtDevicePlugin();

		/**
		@describe get an addOn object
		@param name the name specified to find the object
		@return the object
		@if name not found,return NULL
		*/
		virtual AddOnObject			getAddOnObject(const TString& name) const;

		/**
		@describe
		@param
		@return
		*/
		virtual void				getSupportList(TStringParam& supportList) const;

		/**
		@describe return other plugin that this one depend on
		@param none
		@return dependency list of other plugins's support
		*/
		virtual void				getDependency(TStringParam& dependencyList) const;

		/**
		@describe install the plugin to the framework
		@param none
		@return none
		@remark it's to initialize the plugin itself \n
		and must not use the framework or other plugin
		*/
		virtual void				install();

		/**
		@describe uninstall the plugin
		@param 
		@return 
		*/
		virtual void				uninstall();

		/**
		@describe this will be called after subsystems are initialized
		@param 
		@return 
		*/
		virtual void				initialize();

		/**
		@describe this will be called right before the framework terminate all systems
		@param 
		@return 
		*/
		virtual void				shutdown();

	private:
		QtApplication* mQtApp;	//app instance
	};
	
}//namespace Blade



#endif // __Blade_QtDevicePlugin_h__