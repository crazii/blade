/********************************************************************
	created:	2015/11/18
	filename: 	InterfaceSingleton.inc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_InterfaceSingleton_inc__
#define __Blade_InterfaceSingleton_inc__
#include <utility/String.h>
#include <interface/Interface.h>

namespace Blade
{
	namespace FactoryUtil
	{
		//initial count
		static const size_t SINGLETON_INTERFACE_COUNT = 1024;

		struct SingletonInfo
		{
			const std::type_info*	key;
			Interface*			current;
			TString				currentType;
			TString				defaultType;

			inline SingletonInfo()	{}
			inline SingletonInfo(const std::type_info* info) :key(info)	{}

			/** @brief for qsort */
			inline static int compare(const void* l, const void* r)
			{
				const std::type_info& lhs = *(((SingletonInfo*)l)->key);
				const std::type_info& rhs = *(((SingletonInfo*)r)->key);
				if( lhs.before(rhs) )
					return -1;
				else if( rhs.before(lhs) )
					return 1;
				else
					return 0;
			}

			inline bool operator<(const SingletonInfo& rhs) const
			{
				return compare(this, &rhs) < 0;
			}
		};

		typedef StaticVector<SingletonInfo> SingletonList;

		extern StaticLock msInterfaceSingletonLock;
		extern SingletonList msInterfaceSingletonList;
	}

	
}//namespace Blade


#endif // __Blade_InterfaceSingleton_inc__