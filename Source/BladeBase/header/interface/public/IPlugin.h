/********************************************************************
	created:	2010/04/01
	filename: 	IPlugin.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IPlugin_h__
#define __Blade_IPlugin_h__
#include <memory/BladeMemory.h>
#include <utility/String.h>
#include <utility/StringList.h>

namespace Blade
{
#if BLADE_PLATFORM == BLADE_PLATFORM_IOS || BLADE_PLATFORM == BLADE_PLATFORM_ANDROID
#define BLADE_FORCE_STATIC_PLUGIN 1
#endif


#if defined(BLADE_STATIC) || defined(BLADE_FORCE_STATIC_PLUGIN)
#	define BLADE_STATIC_PLUGIN 1
#else
#	define BLADE_STATIC_PLUGIN 0
#endif

#if !BLADE_STATIC_PLUGIN

#	define BLADE_MAX_PLUGINS_IN_1_MODULE 4

#	define BLADE_PLUGIN_API BLADE_EXPORT
#	define BLADE_DEF_PLUGINS() \
	static IPlugin* localPluginList[BLADE_MAX_PLUGINS_IN_1_MODULE]; \
	static size_t localPluginCount = 0;\
	inline bool addLocalPlugin(IPlugin* plugin)\
	{\
		assert(localPluginCount<BLADE_MAX_PLUGINS_IN_1_MODULE);\
		localPluginList[localPluginCount++] = plugin;\
		return true;\
	}\
	extern "C"\
	{ \
		BLADE_PLUGIN_API size_t BladePluginCount(void)\
		{\
			return localPluginCount;\
		}\
		BLADE_PLUGIN_API IPlugin* BladeGetPlugin(size_t index)\
		{\
			return localPluginList[index];\
		}\
	}

#	define BLADE_ADD_PLUGIN(_type) static Blade::_type _type##_Object; static volatile bool _type##_Added = addLocalPlugin(&_type##_Object)

#	define BLADE_USE_PLUGIN(_type) 

#else

	//note: add force linkage reference to make compile unit linked in. use extern "C" to use C ABI & avoid namespace scope mangling
#	define BLADE_PLUGIN_API	static
#	define BLADE_DEF_PLUGINS() 
#	define BLADE_ADD_PLUGIN(_type) static Blade::_type _type##_Object; extern "C" const volatile bool BLADE_EXPORT BladePlugin_##_type##_Symbol = &_type##_Object
#	define BLADE_USE_PLUGIN(_type) extern "C" {extern const volatile bool BladePlugin_##_type##_Symbol;} volatile bool BladePlugin_##_type##_Symbol_Referenced = BladePlugin_##_type##_Symbol;

#endif

#define BLADE_DEF_PLUGIN(_type) \
	BLADE_DEF_PLUGINS()\
	BLADE_ADD_PLUGIN(_type)

	class BLADE_BASE_API IPlugin : public StaticAllocatable, public NonCopyable, public NonAssignable
	{
	public:
		IPlugin(const TString& name);	/*note: name is not stored*/
		virtual ~IPlugin()	{}

		/** @brief object that this plugin can supply \n
		this is a object export in plugin level
		i.e. a pointer to an object or a pointer to a function 
		actual type is defined by plugin implementation.
		*/
		typedef void*	AddOnObject;

		/**
		@describe
		@param
		@return
		*/
		virtual void				setEnable(bool enable) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool				isEnabled() const = 0;

		/**
		@describe get the plugin 's name
		@param none
		@return none
		*/
		virtual const TString&		getName() const = 0;

		/**
		@describe get an addOn object
		@param name the name specified to find the object
		@return the object
		@if name not found,return NULL
		*/
		virtual AddOnObject			getAddOnObject(const TString& name) const = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual void				getSupportList(TStringParam& supportList) const = 0;

		/**
		@describe return other plugin that this one depend on
		@param none
		@return dependency list of other plugins's support
		*/
		virtual void				getDependency(TStringParam& dependencyList) const = 0;

		/**
		@describe install the plugin to the framework
		@param none
		@return none
		@remark it's to initialize the plugin itself \n
		and must not use the framework or other plugin
		*/
		virtual void				install() = 0;

		/**
		@describe uninstall the plugin
		@param 
		@return 
		*/
		virtual void				uninstall() = 0;

		/**
		@describe this will be called after subsystems are initialized
		@param 
		@return 
		*/
		virtual void				initialize() = 0;

		/**
		@describe this will be called right before the framework terminate all systems
		@param 
		@return 
		*/
		virtual void				shutdown() = 0;

	};//class IPlugin


	extern "C"
	{
		typedef size_t (*BladePluginCountFn)(void);
		typedef IPlugin* (*BladePluginEntryFn)(size_t);
	};

}//namespace Blade


#endif //__Blade_IPlugin_h__