/********************************************************************
	created:	2011/08/13
	filename: 	Box2i.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Box2i_h__
#define __Blade_Box2i_h__
#include <math/BladeMath.h>
#include <math/Vector2i.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	class Box2i
	{
	public:
		static BLADE_BASE_API Box2i EMPTY;
	public:
		inline Box2i()
			:mType(BT_NULL)
			,mMin(Vector2i::ZERO)
			,mMax(Vector2i::ZERO)		{}

		inline Box2i(const Vector2i& pmin,const Vector2i pmax)
			:mType(BT_FINITE)
			,mMin(pmin)
			,mMax(pmax)					{}

		inline Box2i(BOX_TYPE type,const Vector2i& pmin = Vector2i::ZERO,const Vector2i pmax = Vector2i::ZERO)
			:mType(type)
			,mMin(pmin)
			,mMax(pmax)					{}

		inline Box2i(int left, int top, int right, int bottom)
			:mType(BT_FINITE)
			,mMin(left, top)
			,mMax(right, bottom)		{}


		inline Box2i(const Box2i& src)
			:mType(src.mType)
			,mMin(src.mMin)
			,mMax(src.mMax)				{}

		/** @brief  */
		inline bool		operator==(const Box2i& rhs) const
		{
			if (mType == rhs.mType)
			{
				if (mType == BT_NULL || mType == BT_INFINITE)
					return true;
				else
					return mMin == rhs.mMin && mMax == rhs.mMax;
			}
			else
				return false;
		}

		/** @brief  */
		inline bool		operator!=(const Box2i& rhs) const
		{
			return !((*this) == rhs);
		}

		/** @brief  */
		inline const Vector2i&			getMinPoint() const
		{
			return mMin;
		}

		/** @brief  */
		inline Vector2i&				getMinPoint()
		{
			return mMin;
		}

		/** @brief  */
		inline const Vector2i&			getMaxPoint() const
		{
			return mMax;
		}

		/** @brief  */
		inline Vector2i&				getMaxPoint()
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
		Vector2i						getCenter() const
		{
			assert(mType == BT_FINITE);
			return Vector2i((mMax.x + mMin.x) / 2, (mMax.y + mMin.y) / 2);
		}

		/** @brief  */
		Vector2i						getHalfSize() const
		{
			assert(mType == BT_FINITE);
			return Vector2i((mMax.x - mMin.x) / 2, (mMax.y - mMin.y) / 2);
		}

		/** @brief  */
		inline void						set(const Vector2i& pmin,const Vector2i& pmax)
		{
			assert( pmin <= pmax );
			mType = BT_FINITE;
			mMin = pmin;
			mMax = pmax;
		}

		/** @brief  */
		inline int			getLeft() const
		{
			return mMin.x;
		}

		/** @brief  */
		inline int			getTop() const
		{
			return mMin.y;
		}

		/** @brief  */
		inline int			getRight() const
		{
			return mMax.x;
		}

		/** @brief  */
		inline int			getBottom() const
		{
			return mMax.y;
		}

		/** @brief  */
		inline int			getWidth() const
		{
			return this->getRight() - this->getLeft();
		}

		/** @brief  */
		inline int			getHeight() const
		{
			return this->getBottom() - this->getTop();
		}


	protected:
		BOX_TYPE	mType;
		Vector2i	mMin;
		Vector2i	mMax;
	};

}//namespace Blade



#endif // __Blade_Box2i_h__