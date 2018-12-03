/********************************************************************
	created:	2014/06/14
	filename: 	SkeletonResourceState.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_SkeletonResourceState_h__
#define __Blade_SkeletonResourceState_h__
#include <ResourceState.h>
#include "SkeletonResource.h"

namespace Blade
{
	class SkeletonResourceState : public SubResourceState, public Allocatable
	{
	public:
		SkeletonResourceState()		{}
		~SkeletonResourceState()	{}

		/** @brief  */
		inline SkeletonResource* getSkeletonResource() const
		{
			if( mResource == NULL )
				return NULL;
			return this->getTypedResource<SkeletonResource>();
		}

		using SubResourceState::getResource;
	};
	
}//namespace Blade


#endif // __Blade_SkeletonResourceState_h__