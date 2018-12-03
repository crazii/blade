/********************************************************************
	created:	2013/10/18
	filename: 	GraphicsUtility.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GraphicsUtility_h__
#define __Blade_GraphicsUtility_h__
#include <BladeFramework.h>
#include <interface/public/graphics/GraphicsGeometry.h>
#include <math/Ray.h>

namespace Blade
{

	namespace GraphicsUtility
	{
		/**
		@describe ray intersect a float3/float4/half4 triangle list
		@param [in][out] t ray length limit as input and intersect length as output
		@return 
		*/
		//TODO: triangle strips
		bool BLADE_FRAMEWORK_API rayIntersectTriangles(const Ray& ray, const GraphicsGeometry& geometry, scalar& t, POS_SIDE side = PS_POSITIVE);
		
	}//namespace GraphicsUtility
	
}//namespace Blade

#endif //  __Blade_GraphicsUtility_h__