/********************************************************************
	created:	2010/05/09
	filename: 	Box2.h
	author:		Crazii
	purpose:	2d box (rectangle)
*********************************************************************/
#ifndef __Blade_Box2_h__
#define __Blade_Box2_h__
#include <math/BladeMath.h>
#include <math/Vector2.h>

namespace Blade
{
	class Box2
	{
	public:
		inline Box2()
			:mMin(POINT2::ZERO)
			,mMax(POINT2::ZERO)
			,mType(BT_NULL)		{}

		inline Box2(scalar minx, scalar miny, scalar maxx, scalar maxy)
			:mType(BT_FINITE)
		{
			mMin.x = minx;
			mMin.y = miny;
			mMax.x = maxx;
			mMax.y = maxy;
		}

		inline Box2(const POINT2& pmin,const POINT2 pmax)
			:mMin(pmin)
			,mMax(pmax)
			,mType(BT_FINITE)	{}

		inline Box2(BOX_TYPE type,const POINT2& pmin = POINT2::ZERO,const POINT2 pmax = POINT2::ZERO)
			:mMin(pmin)
			,mMax(pmax)
			,mType(type)	{}


		inline Box2(const Box2& src)
			:mMin(src.mMin)
			,mMax(src.mMax)
			,mType(src.mType)	{}

		/** @brief  */
		inline bool		operator==(const Box2& rhs) const
		{
			if( mType == rhs.mType )
			{
				if( mType == BT_NULL || mType == BT_INFINITE )
					return true;
				else
					return mMin == rhs.mMin && mMax == rhs.mMax;
			}
			return false;
		}

		/** @brief  */
		inline bool		operator!=(const Box2& rhs) const
		{
			return !((*this) == rhs);
		}

		/** @brief  */
		inline const POINT2&		getMinPoint() const
		{
			return mMin;
		}

		/** @brief  */
		inline POINT2&				getMinPoint()
		{
			return mMin;
		}

		/** @brief  */
		inline const POINT2&		getMaxPoint() const
		{
			return mMax;
		}

		/** @brief  */
		inline POINT2&				getMaxPoint()
		{
			return mMax;
		}

		/** @brief  */
		inline bool						isNull() const
		{
			return mType == BT_NULL;
		}

		/** @brief  */
		void							setNull()
		{
			mType = BT_NULL;
		}

		/** @brief  */
		inline bool						isInfinite() const
		{
			return mType == BT_INFINITE;
		}

		/** @brief  */
		Vector2							getCenter() const
		{
			assert( mType == BT_FINITE );
			return Vector2( (mMax.x+mMin.x)*scalar(0.5), (mMax.y+mMin.y)*scalar(0.5) );
		}

		/** @brief  */
		POINT2							getHalfSize() const
		{
			assert( mType == BT_FINITE );
			return POINT2( (mMax.x-mMin.x)*scalar(0.5), (mMax.y-mMin.y)*scalar(0.5) );
		}

		/** @brief  */
		inline void						set(const POINT2& pmin,const POINT2& pmax)
		{
			assert( pmin <= pmax );
			mType = BT_FINITE;
			mMin = pmin;
			mMax = pmax;
		}

		/** @brief  */
		inline bool						intersect(const POINT2& pt) const
		{
			return this->isInfinite() || (!this->isNull() && pt >= mMin && pt <= mMax);
		}

		/** @brief  */
		inline POS_VOL					intersect(const Box2& box) const
		{
			if (this->isNull() || box.isNull())
				return PV_OUTSIDE;
			if (this->isInfinite())
				return PV_INSIDE;
			else if (box.isInfinite())
				return PV_INTERSECTED;

			if (this->getMinPoint() <= box.getMinPoint() && this->getMaxPoint() >= box.getMaxPoint())
				return PV_INSIDE;
			else
			{
				Vector2 itsMin = Vector2::getMaxVector(this->getMinPoint(), box.getMinPoint());
				Vector2 itsMax = Vector2::getMinVector(this->getMaxPoint(), box.getMaxPoint());
				if (itsMin <= itsMax)
					return PV_INTERSECTED;
				else
					return PV_OUTSIDE;
			}
		}

		/** @brief  */
		inline bool						contains(const Box2& box) const
		{
			if (mType == BT_NULL)
				return false;
			else if (mType == BT_INFINITE)
				return true;
			else if (box.mType == BT_NULL)
				return true;
			else if (box.mType == BT_INFINITE)
				return false;
			else
				return box.mMin >= mMin && box.mMax <= mMax;
		}

		BLADE_BASE_API static Box2 EMPTY;
	protected:
		POINT2		mMin;
		POINT2		mMax;
		BOX_TYPE	mType;
	};

}//namespace Blade


#endif //__Blade_Box2_h__