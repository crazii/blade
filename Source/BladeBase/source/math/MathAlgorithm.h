/********************************************************************
	created:	2010/04/21
	filename: 	MathAlgorithm.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_MathAlgorithm_h__
#define __Blade_MathAlgorithm_h__
#include <math/Vector3.h>
#include <math/Vector2.h>

namespace Blade
{
	namespace Math
	{
		/**
		@describe 2d point-triangle intersection
		@param
		@return
		*/
		inline bool isPointInTriangle(const POINT2& p0, const POINT2& p1, const POINT2& p2, const POINT2& p)
		{
			Vector2 AB = p1 - p0;
			Vector2 AP = p - p0;
			Vector2 BC = p2 - p1;
			Vector2 BP = p - p1;

			scalar cross0 = AB.crossProduct(AP);
			scalar cross1 = BC.crossProduct(BP);
			int	side0 = Math::iSign(cross0);
			int side1 = Math::iSign(cross1);
			bool colinear0 = Math::Equal<scalar>(cross0, 0, Math::HIGH_EPSILON);
			bool colinear1 = Math::Equal<scalar>(cross1, 0, Math::HIGH_EPSILON);
			if (!colinear0 && !colinear1 && side0 != side1)
				return false;

			Vector2 CA = p0 - p2;
			Vector2 CP = p - p2;
			scalar cross2 = CA.crossProduct(CP);
			int side2 = Math::iSign(cross2);
			bool colinear2 = Math::Equal<scalar>(cross2, 0, Math::HIGH_EPSILON);

			if ((!colinear0 && !colinear2 && side0 != side2) ||
				(!colinear1 && !colinear2 && side1 != side2))
				return false;

			return true;
		}

		/**
		@describe 2d point-triangle intersection. triangle & point should be co-planar.
		@param [in] normal: triangle normal
		@return
		*/
		inline bool isPointInTriangle(const Vector3& normal, const POINT3& p0, const POINT3& p1, const POINT3& p2, const POINT3& p)
		{
			// http://www.devmaster.net/wiki/Ray-triangle_intersection
			//project the triangle into dominant axis-aligned plane
			size_t i = 1, j = 2;
			scalar n0 = std::fabs(normal.x);
			scalar n1 = std::fabs(normal.y);
			scalar n2 = std::fabs(normal.z);
			if (n1 > n2)
			{
				if (n1 > n0)
					i = 0;
			}
			else
			{
				if (n2 > n0)
					j = 0;
			}

			Vector2 projectedTriangle[3];
			projectedTriangle[0].x = p0.mData[i];
			projectedTriangle[0].y = p0.mData[j];
			projectedTriangle[1].x = p1.mData[i];
			projectedTriangle[1].y = p1.mData[j];
			projectedTriangle[2].x = p2.mData[i];
			projectedTriangle[2].y = p2.mData[j];

			Vector2 projectedP;
			projectedP.x = p.mData[i];
			projectedP.y = p.mData[j];

			return isPointInTriangle(projectedTriangle[0], projectedTriangle[1], projectedTriangle[2], projectedP);
		}

		
	}//namespace Math

}//namespace Blade



#endif //__Blade_MathAlgorithm_h__