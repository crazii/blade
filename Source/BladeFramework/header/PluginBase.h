/********************************************************************
	created:	2017/01/14
	filename: 	PluginBase.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladeFramework.h>
#include <interface/public/IPlugin.h>
#include <interface/IConfig.h>

namespace Blade
{
	class BLADE_FRAMEWORK_API PluginBase : public IPlugin
	{
	public:
		/** @brief add-on object for option, getAddOnObject(ADDON_PLUGIN_OPT) returns PLUGIN_OPT (optional) */
		static const TString ADDON_PLUGIN_OPT;
		typedef HCONFIG* PLUGIN_OPT;
	public:
		PluginBase(const TString& name);
		virtual ~PluginBase() {}

		/**
		@describe
		@param
		@return
		*/
		virtual void				setEnable(bool enable) { mEnabled = enable; }

		/**
		@describe
		@param
		@return
		*/
		virtual bool				isEnabled() const { return mEnabled; }

		/**
		@describe get the plugin 's name
		@param none
		@return none
		*/
		virtual const TString&		getName() const { return mName; }

		/**
		@describe get an addOn object
		@param name the name specified to find the object
		@return the object
		@if name not found,return NULL
		*/
		virtual AddOnObject			getAddOnObject(const TString& name) const;

	protected:
		TString	mName;
		mutable HCONFIG mConfig;
		bool	mEnabled;
	};
	
}//namespace Blade