/********************************************************************
	created:	2010/05/13
	filename: 	Vector2i.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Vector2i_h__
#define __Blade_Vector2i_h__
#include <BladeBase.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	struct Vector2i
	{
		int x;
		int y;

		Vector2i()		{}

		Vector2i(int _x,int _y) 
			:x(_x)
			,y(_y)		{}

		Vector2i(const Vector2i& src)
			:x(src.x)
			,y(src.y)	{}


		bool	operator==(const Vector2i& rhs) const
		{
			return x == rhs.x && y == rhs.y;
		}

		bool	operator!=(const Vector2i& rhs) const
		{
			return x != rhs.x || y != rhs.y;
		}

		bool	operator>=(const Vector2i& rhs) const
		{
			return x >= rhs.x && y >= rhs.y;
		}

		bool	operator<=(const Vector2i& rhs) const
		{
			return x <= rhs.x && y <= rhs.y;
		}

		Vector2i&		operator+=(const Vector2i& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		Vector2i		operator+(const Vector2i& rhs) const
		{
			return Vector2i(x+rhs.x,y+rhs.y);
		}

		Vector2i&		operator-=(const Vector2i& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

		Vector2i		operator-(const Vector2i& rhs) const
		{
			return Vector2i(x-rhs.x,y-rhs.y);
		}

		Vector2i		operator*(int rhs) const
		{
			return Vector2i(x*rhs,y*rhs);
		}

		Vector2i&		operator*=(int rhs)
		{
			x *= rhs;
			y *= rhs;
			return *this;
		}

		Vector2i		operator/(int rhs) const
		{
			return Vector2i(x/rhs,y/rhs);
		}

		Vector2i&		operator/=(int rhs)
		{
			x /= rhs;
			y /= rhs;
			return *this;
		}

		//add class keyword to avoid weird symbol conflicts
		static BLADE_BASE_API const struct Vector2i	ZERO;
		static BLADE_BASE_API const struct Vector2i	UNIT_X;
		static BLADE_BASE_API const struct Vector2i	UNIT_Y;
		static BLADE_BASE_API const struct Vector2i	UINT_ALL;
		static BLADE_BASE_API const struct Vector2i	NEGATIVE_UNIT_X;
		static BLADE_BASE_API const struct Vector2i	NEGATIVE_UNIT_Y;
		static BLADE_BASE_API const struct Vector2i	NEGATIVE_UNIT_ALL;
	};

	typedef Vector2i	POINT2I;
	typedef Vector2i	SIZE2I;
	
}//namespace Blade


#endif //__Blade_Vector2i_h__