/********************************************************************
	created:	2011/08/13
	filename: 	Vector3i.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_Vector3i_h__
#define __Blade_Vector3i_h__
#include <BladeBase.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	struct Vector3i
	{
		union
		{
			struct
			{
				int x,y,z;
			};
			int	mData[3];
		};


		Vector3i()		{}

		Vector3i(int _x,int _y,int _z)
			:x(_x)
			,y(_y)
			,z(_z)		{}

		Vector3i(const Vector3i& src)
			:x(src.x)
			,y(src.y)
			,z(src.z)	{}


		bool	operator==(const Vector3i& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
		}

		bool	operator!=(const Vector3i& rhs) const
		{
			return x != rhs.x || y != rhs.y || z != rhs.z;
		}

		bool	operator>=(const Vector3i& rhs) const
		{
			return x >= rhs.x && y >= rhs.y && z >= rhs.z;
		}

		bool	operator<=(const Vector3i& rhs) const
		{
			return x <= rhs.x && y <= rhs.y && z <= rhs.z;
		}

		Vector3i&		operator+=(const Vector3i& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		Vector3i		operator+(const Vector3i& rhs) const
		{
			return Vector3i(x+rhs.x,y+rhs.y,z+rhs.z);
		}

		Vector3i&		operator-=(const Vector3i& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		Vector3i		operator-(const Vector3i& rhs) const
		{
			return Vector3i(x-rhs.x,y-rhs.y,z-rhs.z);
		}


		Vector3i&		operator*=(int rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			return *this;
		}

		Vector3i		operator*(int rhs) const
		{
			return Vector3i(x*rhs,y*rhs,z*rhs);
		}

		Vector3i&		operator/=(int rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
			return *this;
		}

		Vector3i		operator/(int rhs) const
		{
			return Vector3i(x/rhs,y/rhs,z/rhs);
		}

		BLADE_BASE_API static const struct Vector3i	ZERO;
		BLADE_BASE_API static const struct Vector3i	UNIT_X;
		BLADE_BASE_API static const struct Vector3i	UNIT_Y;
		BLADE_BASE_API static const struct Vector3i	UNIT_Z;
		BLADE_BASE_API static const struct Vector3i	UINT_ALL;
		BLADE_BASE_API static const struct Vector3i	NEGATIVE_UNIT_X;
		BLADE_BASE_API static const struct Vector3i	NEGATIVE_UNIT_Y;
		BLADE_BASE_API static const struct Vector3i	NEGATIVE_UNIT_Z;
		BLADE_BASE_API static const struct Vector3i	NEGATIVE_UNIT_ALL;
	};


	typedef Vector3i	POINT3I;

}//namespace Blade



#endif // __Blade_Vector3i_h__