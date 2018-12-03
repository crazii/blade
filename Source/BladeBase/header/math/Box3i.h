/********************************************************************
	created:	2010/04/21
	filename: 	Box3i.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Box3i_h__
#define __Blade_Box3i_h__
#include <math/Vector3i.h>

namespace Blade
{
	class Box3i
	{
	public:
		static BLADE_BASE_API Box3i EMPTY;
	public:
		Box3i(Vector3i& pmin,Vector3i& pmax)
		{
			mMinPoint = pmin;
			mMaxPoint = pmax;
		}

		Box3i(int left,int top,int right,int bottom,int back = 0,int front = 1)
		{
			assert( left<= right && bottom >= top && front >= back );
			mMinPoint.x = left;
			mMinPoint.y = top;
			mMinPoint.z = back;

			mMaxPoint.x = right;
			mMaxPoint.y = bottom;
			mMaxPoint.z = front;
		}

		/** @brief  */
		bool	operator==(const Box3i& rhs) const
		{
			return ::memcmp(this, &rhs, sizeof(*this) ) == 0;
		}

		/** @brief  */
		bool	contains(const Box3i& rhs) const
		{
			return mMinPoint <= rhs.mMinPoint && mMaxPoint >= rhs.mMaxPoint;
		}

		/** @brief  */
		int		getLeft() const		{return mMinPoint.x;}

		/** @brief  */
		void	setLeft(int left)	{mMinPoint.x = left;}

		/** @brief  */
		int		getRight() const	{return mMaxPoint.x;}

		/** @brief  */
		void	setRight(int right)	{mMaxPoint.x = right;}

		/** @brief  */
		int		getTop() const		{return mMinPoint.y;}

		/** @brief  */
		void	setTop(int top)		{mMinPoint.y = top;}

		/** @brief  */
		int		getBottom() const	{return mMaxPoint.y;}

		/** @brief  */
		void	setBottom(int bottom){mMaxPoint.y = bottom;}

		/** @brief  */
		int		getBack() const		{return mMinPoint.z;}

		/** @brief  */
		void	setBack(int back)	{mMinPoint.z = back;}

		/** @brief  */
		int		getFront() const	{return mMaxPoint.z;}

		/** @brief  */
		void	setFront(int front)	{mMaxPoint.z = front;}


		/** @brief  */
		size_t	getWidth() const
		{
			int iw = mMaxPoint.x - mMinPoint.x;
			assert( iw >= 0 );
			return size_t(iw);
		}

		/** @brief  */
		size_t	getHeight() const
		{
			int ih = mMaxPoint.y - mMinPoint.y;
			assert( ih >= 0 );
			return size_t(ih);
		}

		/** @brief  */
		size_t	getDepth() const
		{
			int id = mMaxPoint.z - mMinPoint.z;
			assert( id >= 0 );
			return size_t(id);
		}

		Vector3i mMaxPoint;
		Vector3i mMinPoint;
	};
	
}//namespace Blade



#endif //__Blade_Box3i_h__