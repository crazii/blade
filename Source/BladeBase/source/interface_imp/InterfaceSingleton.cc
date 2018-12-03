/********************************************************************
	created:	2015/11/18
	filename: 	InterfaceSingleton.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/InterfaceSingleton.h>
#include "InterfaceSingletonImpl.h"

namespace Blade
{
	namespace FactoryUtil
	{
		//////////////////////////////////////////////////////////////////////////
		Interface*	interface_getSingleton(const std::type_info& type)
		{
			ScopedLock lock(msInterfaceSingletonLock);
			SingletonInfo info(&type);
			SingletonList::iterator i = std::lower_bound(msInterfaceSingletonList.begin(), msInterfaceSingletonList.end(), info);
			if( i != msInterfaceSingletonList.end() && !(info < *i))
			{
				SingletonInfo& target = *i;
				if( target.current != NULL )
					return target.current;
				else if( !target.currentType.empty() )
					target.current = BLADE_FACTORY_CREATE(Interface, target.currentType);

				if(target.current != NULL)
					return target.current;
			}
			BLADE_EXCEPT(EXC_NEXIST, BTString("singleton not registered or no default implementation."));
		}

		//////////////////////////////////////////////////////////////////////////
		Interface*	interface_switchSingleton(const std::type_info& type, const TString& newImp)
		{
			ScopedLock lock(msInterfaceSingletonLock);
			SingletonInfo info(&type);
			SingletonList::iterator i = std::lower_bound(msInterfaceSingletonList.begin(), msInterfaceSingletonList.end(), info);
			if( i != msInterfaceSingletonList.end() && !(info < *i))
			{
				SingletonInfo& target = *i;
				if( target.currentType != newImp )
				{
					target.current = BLADE_FACTORY_CREATE(Interface, target.currentType);
					target.currentType = newImp;
				}
				else
					assert(target.current != NULL);
				return target.current;
			}
			else
			{
				info.currentType = newImp;
				info.defaultType = newImp;
				info.current = NULL;
				msInterfaceSingletonList.push_back(info);
				//qsort on TString is safe for now.
				std::qsort(&msInterfaceSingletonList[0], msInterfaceSingletonList.size(), sizeof(SingletonInfo), &SingletonInfo::compare);
				return NULL;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		void			interface_resetSingleton(const std::type_info& type)
		{
			ScopedLock lock(msInterfaceSingletonLock);
			SingletonInfo info(&type);
			SingletonList::iterator i = std::lower_bound(msInterfaceSingletonList.begin(), msInterfaceSingletonList.end(), info);
			if( i != msInterfaceSingletonList.end() && !(info < *i))
			{
				SingletonInfo& target = *i;
				target.current = BLADE_FACTORY_CREATE(Interface, target.defaultType);
				target.currentType = target.defaultType;
				return;
			}
			BLADE_EXCEPT(EXC_NEXIST, BTString("singleton not registered."));
		}

		//////////////////////////////////////////////////////////////////////////
		TString		interface_combineClassName(const char* clsName, const TString& registerName)
		{
			return StringConverter::StringToTString(clsName) + BTString("/") + registerName;
		}
	}
	
}//namespace Blade
