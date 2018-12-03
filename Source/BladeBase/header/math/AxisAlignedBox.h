/********************************************************************
	created:	2010/04/21
	filename: 	AxisAlignedBox.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_AABB_h__
#define __Blade_AABB_h__
#include <math/BladeMath.h>
#include <math/Vector3.h>
#include <limits>

namespace Blade
{
	class Plane;
	class Sphere;
	class Matrix44;

	class Box3
	{
	public:
		/*
           +y
           |
           3-----2
          /|    /|
		 / |   / | -z
        5-----4  |/
        |  0--|--1
        | /   | /
        |/    |/
        6-----7   ---> +x
		*/
		typedef enum EBox3Corner
		{
			BC_BACK_LEFT_BOTTOM = 0,
			BC_BACK_RIGHT_BOTTOM,
			BC_BACK_RIGHT_TOP,
			BC_BACK_LEFT_TOP,
			BC_FRONT_RIGHT_TOP,
			BC_FRONT_LEFT_TOP,
			BC_FRONT_LEFT_BOTTOM,
			BC_FRONT_RIGHT_BOTTOM,

			BC_COUNT,
		}CORNER;

	public:
		BLADE_ALWAYS_INLINE Box3()
			:mType(BT_NULL)
		{
			mMinPoint = Vector3::ZERO;
			mMaxPoint = Vector3::ZERO;
		}

		BLADE_ALWAYS_INLINE Box3(const Box3& src)
			:mType(src.mType)
		{
			mMinPoint = src.mMinPoint;
			mMaxPoint = src.mMaxPoint;
		}

		BLADE_ALWAYS_INLINE Box3(const Vector3& pmin,const Vector3& pmax)
			:mType(BT_FINITE)
		{
			assert(pmin <= pmax);
			mMinPoint = pmin;
			mMaxPoint = pmax;
		}

		BLADE_ALWAYS_INLINE Box3(scalar minx,scalar miny,scalar minz,scalar maxx,scalar maxy,scalar maxz)
			:mType(BT_FINITE)
		{
			mMinPoint.x = minx;
			mMinPoint.y = miny;
			mMinPoint.z = minz;

			mMaxPoint.x = maxx;
			mMaxPoint.y = maxy;
			mMaxPoint.z = maxz;
			assert(mMinPoint <= mMaxPoint);
		}

		BLADE_ALWAYS_INLINE Box3(BOX_TYPE type,const Vector3& pmin = Vector3::ZERO,const Vector3& pmax = Vector3::ZERO)
			:mType(type)
		{
			assert(mType != BT_FINITE || pmin <= pmax);
			mMinPoint = pmin;
			mMaxPoint = pmax;
		}

		BLADE_ALWAYS_INLINE ~Box3()
		{

		}

		BLADE_ALWAYS_INLINE Box3&	operator=(const Box3& rhs)
		{
			mType		= rhs.mType;
			if( mType == BT_FINITE )
			{
				mMinPoint	= rhs.mMinPoint;
				mMaxPoint	= rhs.mMaxPoint;
			}
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE const Vector3&			getMinPoint() const
		{
			return mMinPoint;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE const Vector3&			getMaxPoint() const
		{
			return mMaxPoint;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMinPoint(const Vector3& pmin)
		{
			mMinPoint = pmin;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMinPoint(scalar minx,scalar miny,scalar minz)
		{
			mMinPoint = Vector3(minx,miny,minz);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMinX(scalar minx)
		{
			mMinPoint.x = minx;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMinY(scalar miny)
		{
			mMinPoint.y = miny;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMinZ(scalar minz)
		{
			mMinPoint.z = minz;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMaxPoint(const Vector3& pmax)
		{
			mMaxPoint = pmax;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMaxPoint(scalar maxx,scalar maxy,scalar maxz)
		{
			mMaxPoint = Vector3(maxx,maxy,maxz);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMaxX(scalar maxx)
		{
			mMaxPoint.x = maxx;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMaxY(scalar maxy)
		{
			mMaxPoint.y = maxy;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		setMaxZ(scalar maxz)
		{
			mMaxPoint.z = maxz;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void		set(const Vector3& pmin,const Vector3& pmax)
		{
			assert( pmin <= pmax );
			mMinPoint	= pmin;
			mMaxPoint	= pmax;
			mType		= BT_FINITE;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	void		set(scalar minx,scalar miny,scalar minz,scalar maxx,scalar maxy,scalar maxz)
		{
			mMinPoint	= Vector3(minx,miny,minz);
			mMaxPoint	= Vector3(maxx,maxy,maxz);
			assert( mMinPoint <= mMaxPoint );
			mType		= BT_FINITE;
		}

		/** @brief  */
		inline void						set(BOX_TYPE type)
		{
			mType = type;
		}

		/** @brief  */
		inline void						offset(const Vector3& _offset)
		{
			mMaxPoint += _offset;
			mMinPoint += _offset;
		}

		/** @brief  */
		inline bool						isFinite() const
		{
			return mType == BT_FINITE;
		}

		/** @brief  */
		inline bool						isNull() const
		{
			return mType == BT_NULL;
		}

		/** @brief  */
		inline void						setNull()
		{
			mType = BT_NULL;
		}

		/** @brief  */
		inline bool						isInfinite() const
		{
			return mType == BT_INFINITE;
		}

		/** @brief  */
		inline void						setInfinite()
		{
			mType = BT_INFINITE;
		}

		/** @brief  */
		inline bool						operator==(const Box3& rhs) const
		{
			return (mType == rhs.mType) && (mType != BT_FINITE || (this->mMinPoint == rhs.mMinPoint && this->mMaxPoint == rhs.mMaxPoint));
		}

		/** @brief  */
		inline bool						operator!=(const Box3& rhs) const
		{
			return !( (*this)==rhs );
		}

		/** @brief  */
		inline POS_VOL						intersect(const Vector3& point) const
		{
			if( mType == BT_NULL )
				return PV_OUTSIDE;
			else if( mType == BT_INFINITE )
				return PV_INSIDE;
			else
				return point >= mMinPoint && point <= mMaxPoint ? PV_INSIDE : PV_OUTSIDE;
		}

		/** @brief  */
		inline bool						contains(const POINT3& point) const
		{
			return this->intersect(point) == PV_INSIDE;
		}

		inline scalar					getSquaredDistance(const POINT3& point) const
		{
			Vector3 d = Vector3::getMaxVector(point - mMaxPoint, mMinPoint - point);
			return d.getSquaredLength();
		}

		inline scalar					getDistance(const POINT3& point) const
		{
			Vector3 d = Vector3::getMaxVector(point - mMaxPoint, mMinPoint - point);
			return d.getLength();
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool		contains(const Box3& box) const
		{
			if( mType == BT_NULL || box.mType == BT_INFINITE)
				return false;
			else if( mType == BT_INFINITE || box.mType == BT_NULL)
				return true;
			else
				return box.mMinPoint >= mMinPoint && box.mMaxPoint <= mMaxPoint;
		}

		/** @brief PV_OUTSIDE if box out side this. PV_INSIDE if box inside this. PV_INTERSECTED if intersected. */
		/** @brief return result can be used as boolean value: POS != 0(PV_OUTSIDE) usually means intersected */
		inline POS_VOL			intersect(const Box3& box) const
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
				Vector3 itsMin = Vector3::getMaxVector(this->getMinPoint(), box.getMinPoint());
				Vector3 itsMax = Vector3::getMinVector(this->getMaxPoint(), box.getMaxPoint());
				if (itsMin <= itsMax)
					return PV_INTERSECTED;
				else
					return PV_OUTSIDE;
			}
		}

		/** @brief  */
		inline scalar			getVolume() const
		{
			if (mType == BT_FINITE)
			{
				Vector3 diagonal = mMaxPoint - mMinPoint;
				return diagonal.x * diagonal.y * diagonal.z;
			}
			else if (mType == BT_NULL)
				return 0.0f;
			else
				return std::numeric_limits<scalar>::infinity();
		}

		/** @brief  */
		inline Vector3			getSize() const
		{
			if (mType == BT_FINITE)
			{
				Vector3 diagonal = mMaxPoint - mMinPoint;
				return diagonal;
			}
			else if (mType == BT_NULL)
				return Vector3::ZERO;
			else
				return Vector3::UNIT_ALL*std::numeric_limits<scalar>::infinity();
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3			getHalfSize() const
		{
#if 0
			if (mType == BT_FINITE)
			{
				return (mMaxPoint - mMinPoint)*0.5f;
			}
			else if (mType == BT_NULL)
				return Vector3::ZERO;
			else
				return Vector3::UNIT_ALL*std::numeric_limits<scalar>::infinity();
#else
			assert(mType == BT_FINITE);
			return (mMaxPoint - mMinPoint)*0.5f;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE POINT3			getCenter() const
		{
			assert(mType == BT_FINITE);
			return (mMaxPoint + mMinPoint)*0.5f;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void			merge(const Box3& aab)
		{
			BOX_TYPE type = mType;
			if ((aab.mType == BT_NULL) || (type == BT_INFINITE))
				return;
			mType = aab.mType;

			if (aab.mType == BT_FINITE && type == BT_FINITE)
			{
				mMinPoint = Vector3::getMinVector(mMinPoint, aab.mMinPoint);
				mMaxPoint = Vector3::getMaxVector(mMaxPoint, aab.mMaxPoint);
			}
			else if (type == BT_NULL && aab.mType != BT_INFINITE)
			{
				mMinPoint = aab.mMinPoint;
				mMaxPoint = aab.mMaxPoint;
			}
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void	merge(const POINT3& point)
		{
			if (mType == BT_FINITE)
			{
				mMinPoint.setMinComponents(point);
				mMaxPoint.setMaxComponents(point);
			}
			else if (mType == BT_NULL)
				this->set(point, point);
			else
				return;
		}

		/** @brief get enclosing aab of the specified sphere */
		BLADE_BASE_API Box3(const Sphere& sphere);

		/** @brief  */
		BLADE_BASE_API Box3&			operator*=(const Matrix44& mtransform);

		/** @brief  */
		BLADE_BASE_API Box3				operator*(const Matrix44& mtransform) const;

		/** @brief  */
		BLADE_BASE_API void				scale(const Vector3& vscale);

		/** @brief  */
		BLADE_BASE_API bool				intersect(const Plane& plane) const;

		/** @brief  */
		BLADE_BASE_API POS_VOL			intersect(const Sphere& sphere) const;

		/** @brief triangle intersection  */
		BLADE_BASE_API bool				intersect(const POINT3& p0, const POINT3& p1, const POINT3& p2) const;

		/** @brief  */
		BLADE_BASE_API Box3				getIntersection(const Box3& box) const;

		/** @brief  */
		BLADE_BASE_API bool				getAllCorners(POINT3* corners) const;

		/** @brief  */
		BLADE_BASE_API POINT3			getCorner(CORNER corner_index) const;

		//add class keyword to avoid weird symbol conflicts
		BLADE_BASE_API static const class Box3		EMPTY;
		BLADE_BASE_API static const class Box3		INFINITE;
		BLADE_BASE_API static const class Box3		UNIT;
		BLADE_BASE_API static const CORNER			LINES[12][2];

	protected:
		Vector3		mMinPoint;
		Vector3		mMaxPoint;
		BOX_TYPE	mType;
	};//class Box3

	typedef Box3 AABB;
	
}//namespace Blade


#endif //__Blade_AABB_h__