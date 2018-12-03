/********************************************************************
	created:	2010/05/23
	filename: 	ResourceHandle.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <interface/IResource.h>
#include "ResourceManager.h"

namespace Blade
{
	//ResourceHandle::~ResourceHandle()
	//{
	//	if( mPtr != NULL )
	//	{
	//		lock();

	//		//count:this hadnle takes 1, the resource manager takes 1
	//		if( *mRefcount == 2 )
	//		{
	//			//set refcount to 1
	//			this->decRef();
	//			//unlock();

	//			//BLADE_DELETE the resource
	//			ResourceManager::getSingleton().removeResource( mPtr->getName() );

	//			//lock();
	//			mPtr = NULL;
	//			mRefcount = NULL;
	//			unlock();
	//		}
	//		else
	//			unlock();

	//		
	//	}
	//}
	
}//namespace Blade