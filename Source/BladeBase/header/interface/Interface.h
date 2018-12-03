/********************************************************************
	created:	2015/11/17
	filename: 	Interface.h
	author:		Crazii
	
	purpose:	interfaces for all plugin registered class/objects
				use this interface to avoid direct linkage of plugin 's exported class
				so that you don't need  to explicit link the plugin.
				be careful that any object should be created after plugin installed, (you cannot create if object not registered)
				and deleted before plugin un-installed( after DLL/so unloaded, executable code segments and any v-tables will becomes invalid).
*********************************************************************/
#ifndef __Blade_Interface_h__
#define __Blade_Interface_h__
#include <Factory.h>
#include <typeinfo>

namespace Blade
{
	/** @brief  */
	class Interface
	{
	public:
		virtual ~Interface()	{}
	};

	extern template class BLADE_BASE_API Factory<Interface>;

	typedef std::type_info const &InterfaceName;

	template<typename T>
	struct InterfaceID
	{
		/** @brief  */
		static inline InterfaceName getName()
		{
			//note: DO NOT use typeid(T).name() becasue typeid(T).name() is implementation-defined, 
			//C++ standard doesn't garuantee typeid(T).name() differs among different types.
			return typeid(T);
		}
	};
	
}//namespace Blade


#define NameRegisterInterface(_type, _name) NameRegisterFactory(_type, Interface, _name)
#define RegisterInterface(_type, _namedtype) NameRegisterInterface(_type, BTString(#_namedtype))

//create a registered common interface
#if BLADE_DEBUG
#define BLADE_INTERFACE_NAME_CREATE(_type, _name) dynamic_cast<_type*>(BLADE_FACTORY_CREATE(Blade::Interface, _name))
#else
#define BLADE_INTERFACE_NAME_CREATE(_type, _name) static_cast<_type*>(BLADE_FACTORY_CREATE(Blade::Interface, _name))
#endif

#define BLADE_INTERFACE_CREATE(_type) BLADE_INTERFACE_NAME_CREATE(_type, BTString(#_type))

#define CHECK_RETURN_INTERFACE(_interface_type, _name, _object) \
	if( &InterfaceID<_interface_type>::getName() == &_name || InterfaceID<_interface_type>::getName() == _name ) \
		return static_cast<Interface*>(_object)

#endif // __Blade_Interface_h__