/********************************************************************
	created:	2010/05/09
	filename: 	Line2.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Line2.h>
#include <math/Box2.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	POS_SIDE					Line2::getSide(const Box2& box2,scalar tolerance/* = scalar(0)*/) const
	{
		if( box2.isNull() )
			return PS_NONE;
		else if( box2.isInfinite() )
			return PS_BOTH;

		Vector2 center = box2.getCenter();
		Vector2 half = box2.getHalfSize();

		return this->getSide(center,half,tolerance);
	}
	
}//namespace Blade