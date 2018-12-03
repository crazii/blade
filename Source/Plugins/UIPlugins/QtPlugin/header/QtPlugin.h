/********************************************************************
	created:	2016/07/31
	filename: 	QtPlugin.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_QtPlugin_h__
#define __Blade_QtPlugin_h__

#include <PluginBase.h>
#include "qtplugin_global.h"

namespace Blade
{
	class QtConfigDialog;

	class QTPLUGIN_EXPORT QtPlugin : public Blade::PluginBase, public Singleton<QtPlugin>
	{
	public:
		using Singleton<QtPlugin>::operator new;
		using Singleton<QtPlugin>::operator delete;
	public:
		QtPlugin();
		~QtPlugin();

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

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		/** @brief  */
		EKeyCode getKeyCode(int key, Qt::KeyboardModifiers modifier);

	private:
		QtConfigDialog* mDefDialog;
	};
	
}//namespace Blade



#endif // __Blade_QtPlugin_h__