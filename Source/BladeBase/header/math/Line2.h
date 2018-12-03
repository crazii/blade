/********************************************************************
	created:	2010/05/09
	filename: 	Line2.h
	author:		Crazii
	purpose:	a 2D line ,like 3D plane,represented as normal as offset
*********************************************************************/
#ifndef __Blade_Line2_h__
#define __Blade_Line2_h__
#include <math/Vector2.h>
#include <math/BladeMath.h>

namespace Blade
{
	class Box2;

	class Line2
	{
	public:
		inline Line2()
			:mNormal(Vector2::ZERO)
			,mD(scalar(0))
									{}

		inline Line2(const Vector2& normal,scalar d)
			:mNormal(normal)
			,mD(d)
									{}

		inline Line2(const Line2& src)
			:mNormal(src.mNormal)
			,mD(src.mD)
									{}

		inline Line2(scalar nx,scalar ny,scalar d)
			:mNormal(nx,ny)
			,mD(d)					{}

		inline Line2(const Vector2& p0,const Vector2& p1)
		{
			this->set(p0, p1);
		}

		inline Line2(const Vector2& normal,scalar linepointx,scalar linepointy)
		{
			this->set(normal,linepointx,linepointy);
		}

		/** @brief  */
		inline void					set(const Vector2& normal,scalar d)
		{
			mNormal = normal;
			mD = d;
		}

		/** @brief  */
		inline void					set(const Vector2& p0,const Vector2& p1)
		{
			Vector2 dir = p1 - p0;
			mNormal = dir.getPerpendicularVector();
			mNormal.normalize();
			mD = -mNormal.dotProduct(p0);
		}

		/** @brief  */
		inline void					set(const Vector2& normal,scalar linePx,scalar linePy)
		{
			mNormal = normal.getNormalizedVector();
			mD = -mNormal.dotProduct(Vector2(linePx,linePy));
		}

		/** @brief  */
		inline bool		operator==(const Line2& rhs) const
		{
			return mNormal == rhs.mNormal && mD == rhs.mD;
		}

		/** @brief  */
		inline bool		operator!=(const Line2& rhs) const
		{
			return mD != rhs.mD || mNormal != rhs.mNormal;
		}

		/** @brief  */
		inline const Vector2&		getNormal() const
		{
			return mNormal;
		}

		/** @brief  */
		inline Vector2&				getNormal()
		{
			return mNormal;
		}

		/** @brief offset from origin */
		inline scalar				getLineDistance() const
		{
			return -mD;
		}

		/** @brief  */
		inline scalar				getDistance(const Vector2& point) const
		{
			return mNormal.dotProduct(point) + mD;
		}

		/** @brief  */
		inline POS_SIDE				getSide(const Vector2& point,scalar tolerance = scalar(0)) const
		{
			scalar dist = this->getDistance(point);

			if( dist > tolerance )
				return PS_POSITIVE;
			else if( dist < -tolerance )
				return PS_NEGATIVE;
			else
				return PS_ON;
		}

		/** @brief  */
		inline  POS_SIDE		getSide(const Vector2& center,const Vector2& halfSize,scalar tolerance = scalar(0)) const
		{
			scalar dist = this->getDistance(center);
			// get the maximal allows absolute distance for center-plane the distance
			scalar maxAbsDist = ::fabs(mNormal.x*halfSize.x) + ::fabs(mNormal.y*halfSize.y);

			if (dist < -maxAbsDist-tolerance )
				return PS_NEGATIVE;
			if (dist > maxAbsDist + tolerance )
				return PS_POSITIVE;
			return PS_BOTH;
		}

		/** @brief  */
		BLADE_BASE_API POS_SIDE		getSide(const Box2& box2,scalar tolerance = scalar(0)) const;

	public:
		Vector2	mNormal;
		scalar	mD;
	};//class Line2
	
}//namespace Blade


#endif //__Blade_Line2_h__