/********************************************************************
	created:	2015/12/15
	filename: 	RenderSorter.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/public/IRenderable.h>
#include <interface/IRenderQueue.h>
#include "RenderSorter.h"

namespace Blade
{
	template class Factory<IRenderable>;

	//////////////////////////////////////////////////////////////////////////
	int	Front2BackRenderSorter::compare(const RenderOperation& lhs, const RenderOperation& rhs)
	{
		scalar diff = lhs.viewDistance - rhs.viewDistance;
		if(diff < 0)
			return -1;
		else if(diff > 0)
			return 1;
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	int	Back2FrontRenderSorter::compare(const RenderOperation& lhs, const RenderOperation& rhs)
	{
		scalar diff = lhs.viewDistance - rhs.viewDistance;
		if(diff < 0)
			return 1;
		else if(diff > 0)
			return -1;
		else
			return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	int GeometrySorter::compare(const RenderOperation& lhs, const RenderOperation& rhs)
	{
		const void* l = lhs.geometry;
		const void* r = rhs.geometry;
		//don't care about the > or <, just group rendereables if they have the same geometry (==)
		return l < r ? -1 : (l > r ? 1 : 0);
	}

	//////////////////////////////////////////////////////////////////////////
	int	TextureRenderSorter::compare(const RenderOperation& lhs, const RenderOperation& rhs)
	{
		//sort by the first texture
		void* l = (void*)lhs.texture;
		void* r = (void*)rhs.texture;
		return l < r ? -1 : (l > r ? 1 : 0);
	}

	//////////////////////////////////////////////////////////////////////////
	int	CombinedRenderSorter::compare(const RenderOperation& lhs, const RenderOperation& rhs)
	{
		for(size_t i = 0; i < mSorters.size(); ++i)
		{
			int ret = mSorters[i]->compare(lhs, rhs);
			if( ret != 0 )
				return ret;
			else
				continue;
		}
		return 0;
	}
	
}//namespace Blade