/********************************************************************
	created:	2010/04/02
	filename: 	InterfaceSingleton.h
	author:		Crazii
	
	purpose:	singleton pattern for pure abstract class

	log:		2/17/2012 - remove extern factory dependency and embed it into this class
				add external factory dependency because now class InterfaceSingleton<T> may be not exported

				although InterfaceSingleton(and its subclass) can be exported, it's highly recommend that
				DO NOT export DLL for InterfaceSingleton, so that you can possibly use it without linking to the dependent shared library.

				getInterface() v.s. getSingleton()
				getSingleton() need direct linkage of the dependent shared library.
				if the dependent library is not linked, getInterface() is preferred.

				getInterface uses unique class Factory<Interface> exported by BladeBase library,
				getSingleton uses Factory<T> which needed be exported by the shared library which actually implements it.

				theoretically getSingleton() can be replaced by getInterface() to reduce complexity & dependency,
				i.e. without dependency of Factory<T> and fewer functions;
				But the internal implementation of interface_xxxSingleton functions are not fast enough compared to using getSingleton() style.
				so for now both approaches are used. This will change when better approaches found.

				use getSingleton() if the interface is frequently used and possbily higher performance is required.
				or cache the reference/pointer return by getInterface(), if the interface is ensured to have unique but no other implementations

				06/07/2016 TODO: add local cache for interface singleton:
				local cache gives very fast access, but break consistency among DLL/so, each instance will have different cache.
*********************************************************************/
#ifndef __Blade_InterfaceSingleton_h__
#define __Blade_InterfaceSingleton_h__
#include "Interface.h"
#include <typeinfo>

namespace Blade
{
	namespace FactoryUtil
	{
		/** @brief create singleton for specified type. if singleton not created yet, use the registered default type creator */
		BLADE_BASE_API Interface*	interface_getSingleton(const std::type_info& type);

		/** @brief switch current singleton implementation for specified type */
		/** @note: the first call of type switching will be set to default implementation  */
		BLADE_BASE_API Interface*	interface_switchSingleton(const std::type_info& type, const TString& newImp);

		/** @brief reset singleton to initial state for specified type */
		BLADE_BASE_API void			interface_resetSingleton(const std::type_info& type);

		/** @brief combine C++ class name with registered name */
		BLADE_BASE_API TString		interface_combineClassName(const char* clsName, const TString& registerName);

		
		/** @brief add class prefix to resolve name conflicts for Factory<Interface> */
		template<typename T>
		inline const TString interface_convertClassName(const TString& name)
		{
			return interface_combineClassName(typeid(T).name(), name);
		}
	}


	template<typename T>
	class InterfaceSingleton : public Interface
	{
	public:
		~InterfaceSingleton()
		{
			if (Factory<T>::testInterfaceSingleton())
				InterfaceSingleton<T>::resetSingleton();
		}

		/** @brief get the singleton pointer */
		inline static T*	getSingletonPtr()
		{
			//note: this class may be NOT exported, but Factory<T> should be exported, so we use Factory<T> as an helper
			return Factory<T>::getInterfaceSingleton();
		}

		/** @brief get the singleton reference */
		inline static T&	getSingleton()
		{
			return *getSingletonPtr();
		}

		/** @brief clear the current singleton interface to default implementation */
		inline static void	resetSingleton()
		{
			//note: this class may be NOT exported, but Factory<T> should be exported, so we use Factory<T> as an helper
			Factory<T>::resetInterfaceSingleton();
		}

		/** @brief change default singleton interface to another implementation */
		inline static T*	interchange(const TString& className)
		{
			//note: this class may be NOT exported, but Factory<T> should be exported, so we use Factory<T> as an helper
			return Factory<T>::switchInterfaceSingleton(className);
		}

		/** @brief get another implementation without override the current one */
		/** @note this will NOT override the current impelmentation, later calls of getSingleton() will get the orinal implementation */
		inline static T*	getOtherSingletonPtr(const TString& className)
		{
			return BLADE_FACTORY_CREATE(T, className);
		}

		/** @brief  */
		inline static T&	getOtherSingleton(const TString& className)
		{
			return *getOtherSingletonPtr(className);
		}


		/** @brief get pointer from common interface */
		inline static T* getInterfacePtr()
		{
#if BLADE_DEBUG
			return dynamic_cast<T*>(FactoryUtil::interface_getSingleton(typeid(T)));
#else
			return static_cast<T*>(FactoryUtil::interface_getSingleton(typeid(T)));
#endif
		}

		/** @brief  */
		inline static T& getInterface()
		{
			return *getInterfacePtr();
		}

		/** @brief  */
		inline static void resetInterface()
		{
			FactoryUtil::interface_resetSingleton(typeid(T));
		}

		/** @brief  */
		inline static T* switchInterface(const TString& name)
		{
			TString interfaceName = FactoryUtil::interface_convertClassName<T>(name);
#if BLADE_DEBUG
			return dynamic_cast<T*>(FactoryUtil::interface_switchSingleton(typeid(T), interfaceName));
#else
			return static_cast<T*>(FactoryUtil::interface_switchSingleton(typeid(T), interfaceName));
#endif
		}
	};


	namespace FactoryUtil
	{
		template<typename T, typename CT>
		class SingletonGetter : public FactoryUtil::Creator<T>
		{
		private:
			virtual T*	create(const char* file,int line) const
			{
				BLADE_UNREFERENCED(file);
				BLADE_UNREFERENCED(line);
				//No debug info
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
				return CT::Singleton<CT>::getSingletonPtr();
#else
				return CT::template Singleton<CT>::getSingletonPtr();
#endif
			}

			virtual T* create() const
			{
				//here force the interface singleton 's implementation to be a real singleton class
				//remove the ambiguous name
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
				return CT::Singleton<CT>::getSingletonPtr();
#else
				return CT::template Singleton<CT>::getSingletonPtr();
#endif
			}

			virtual void destroy(T*) const
			{}
		};


		template<typename T, typename CT>
		class InterfaceSingletonGetter : public FactoryUtil::Creator<Interface>
		{
		private:
			//SFINAE
			static inline InterfaceSingleton<T>* staticCast(InterfaceSingleton<T>* p)	{return p;}
			static inline InterfaceSingleton<T>* staticCast(...)						{return NULL;}

			virtual Interface*	create(const char* file,int line) const
			{
				BLADE_UNREFERENCED(file);BLADE_UNREFERENCED(line);
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
				return staticCast( CT::Singleton<CT>::getSingletonPtr() );
#else
				return staticCast( CT::template Singleton<CT>::getSingletonPtr() );
#endif
			}

			virtual Interface* create() const
			{
				//here force the interface singleton 's implementation to be a real singleton class
				//remove the ambiguous name
#if BLADE_COMPILER == BLADE_COMPILER_MSVC
				return staticCast( CT::Singleton<CT>::getSingletonPtr() );
#else
				return staticCast( CT::template Singleton<CT>::getSingletonPtr() );
#endif
			}

			virtual void destroy(Interface*) const
			{}

		public:
			static inline bool isValid() {return staticCast( (T*)(uintptr_t)0xFF00 ) != NULL;}
		};


		/** @brief interface singleton register proxy */
		template<typename T, typename CT>
		class InterfaceSingletonRegisterProxy
		{
		public:
			InterfaceSingletonRegisterProxy(const TString& name, bool bDefault)
			{
				Factory<T>::getSingleton().registerClass(name, &msTypeCreator);
				if( bDefault )
					Factory<T>::getSingleton().registerClass(BTString("Default"), &msTypeCreator);

				if( msInterfaceCreator.isValid() )
				{
					TString interfaceName = interface_convertClassName<T>(name);
					Factory<Interface>::getSingleton().registerClass(interfaceName, &msInterfaceCreator);
					if( bDefault )
						interface_switchSingleton(typeid(T), interfaceName);
				}
			}
		protected:
			static SingletonGetter<T, CT>	msTypeCreator;
			static InterfaceSingletonGetter<T, CT>	msInterfaceCreator;
		};

		template< typename T, typename CT>
		SingletonGetter<T, CT> InterfaceSingletonRegisterProxy<T, CT>::msTypeCreator;

		template< typename T, typename CT>
		InterfaceSingletonGetter<T, CT> InterfaceSingletonRegisterProxy<T, CT>::msInterfaceCreator;


		template<typename T>
		inline void unregisterInterfaceSingleton(const TString& name, bool bDefault)
		{
			Factory<T>::getSingleton().unregisterClass(name);
			if( bDefault )
				Factory<T>::getSingleton().unregisterClass(BTString("Default"));

			TString interfaceName = interface_convertClassName<T>(name);
			Factory<Interface>::getSingleton().unregisterClass(interfaceName);
			InterfaceSingleton<T>::resetSingleton();//clear interface pointer
		};

	}//namespace FactoryUtil

	///register Singleton<T> to factory, note it's optional that the class inherited from InterfaceSingleton
#define NameRegisterSingletonWithDefault(_type, _basetype, _name, _default) do{volatile const Blade::FactoryUtil::InterfaceSingletonRegisterProxy<_basetype, _type> _reg(_name, _default);}while(false)
#define NameRegisterSingleton(_type, _basetype, _name) NameRegisterSingletonWithDefault(_type, _basetype, _name, false)
#define RegisterSingleton(_type, _basetype) NameRegisterSingletonWithDefault(_type, _basetype, BTString(#_type), true)

	//unregister singletons. usually you don't need unregister them, conventionally, most singletons will not be accessed after DLL/so unloading.
	//but sometimes users still access them by mistake, that will crash the app. 
	//by unregister them at shutting down/cleaning up time, and then later access will not directly crash the app, but throw an exception.
#define NameUnregisterSingletonWithDefault(_basetype, _name, _default) Blade::FactoryUtil::unregisterInterfaceSingleton<_basetype>(_name, _default)
#define NameUnregisterSingleton(_name, _basetype) NameUnregisterSingletonWithDefault(_basetype, _name, false)
#define UnregisterSingleton(_type, _basetype) NameUnregisterSingletonWithDefault(_basetype, BTString(#_type), true)
	
}//namespace Blade


#endif //__Blade_InterfaceSingleton_h__