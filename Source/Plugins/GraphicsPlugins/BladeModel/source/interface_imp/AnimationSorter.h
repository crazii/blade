/********************************************************************
	created:	2015/12/15
	filename: 	AnimationSorter.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_AnimationSorter_h__
#define __Blade_AnimationSorter_h__
#include <interface/public/IRenderSorter.h>
#include <interface/IRenderQueue.h>
#include <Math/DualQuaternion.h>
#include "SubMesh.h"

namespace Blade
{

	class AnimationSorter : public IRenderSorter, public Allocatable
	{
	public:
		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs)
		{
			const SubMesh* lmesh = static_cast<const SubMesh*>(lhs.renderable);
			const SubMesh* rmesh = static_cast<const SubMesh*>(rhs.renderable);
			bool lbone = lmesh->isUsingGlobalBone();
			bool rbone = rmesh->isUsingGlobalBone();
			if(lbone & rbone)
			{
				//group mesh by parent to share bone palette
				const Model* lparent = lmesh->getParent();
				const Model* rparent = rmesh->getParent();
				//cannot use lparent-rparent because ptrdiff_t may exceed int limit
				if( lparent < rparent )
					return -1;
				else if( lparent > rparent )
					return 1;
				else
					return 0;
			}
			else if( !lbone && !rbone )
				return 0;
			else
				return (int)lbone - (int)rbone;
		}
	};
	
}//namespace Blade

#endif // __Blade_AnimationSorter_h__