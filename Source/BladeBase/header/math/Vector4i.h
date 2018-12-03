/********************************************************************
	created:	2017/12/11
	filename: 	Vector4i.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Vector4i_h__
#define __Blade_Vector4i_h__
#include <BladeBase.h>

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	struct Vector4i
	{
		union
		{
			struct
			{
				int x, y, z, w;
			};
			int	mData[4];
		};


		Vector4i() {}

		Vector4i(int _x, int _y, int _z, int _w)
			:x(_x)
			,y(_y)
			,z(_z)
			,w(_w){}

		Vector4i(const Vector4i& src)
			:x(src.x)
			,y(src.y)
			,z(src.z)
			,w(src.w){}


		bool	operator==(const Vector4i& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
		}

		bool	operator!=(const Vector4i& rhs) const
		{
			return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
		}

		bool	operator>=(const Vector4i& rhs) const
		{
			return x >= rhs.x && y >= rhs.y && z >= rhs.z && w >= rhs.w;
		}

		bool	operator<=(const Vector4i& rhs) const
		{
			return x <= rhs.x && y <= rhs.y && z <= rhs.z && w <= rhs.w;
		}

		Vector4i&		operator+=(const Vector4i& rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
			return *this;
		}

		Vector4i		operator+(const Vector4i& rhs) const
		{
			return Vector4i(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
		}

		Vector4i&		operator-=(const Vector4i& rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
			return *this;
		}

		Vector4i		operator-(const Vector4i& rhs) const
		{
			return Vector4i(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
		}


		Vector4i&		operator*=(int rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;
			return *this;
		}

		Vector4i		operator*(int rhs) const
		{
			return Vector4i(x*rhs, y*rhs, z*rhs, w*rhs);
		}

		Vector4i&		operator/=(int rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
			w /= rhs;
			return *this;
		}

		Vector4i		operator/(int rhs) const
		{
			return Vector4i(x / rhs, y / rhs, z / rhs, w /rhs);
		}

		BLADE_BASE_API static const struct Vector4i	ZERO;
		BLADE_BASE_API static const struct Vector4i	UNIT_X;
		BLADE_BASE_API static const struct Vector4i	UNIT_Y;
		BLADE_BASE_API static const struct Vector4i	UNIT_Z;
		BLADE_BASE_API static const struct Vector4i	UINT_ALL;
		BLADE_BASE_API static const struct Vector4i	NEGATIVE_UNIT_X;
		BLADE_BASE_API static const struct Vector4i	NEGATIVE_UNIT_Y;
		BLADE_BASE_API static const struct Vector4i	NEGATIVE_UNIT_Z;
		BLADE_BASE_API static const struct Vector4i	NEGATIVE_UNIT_ALL;
	};


	typedef Vector4i	POINT4I;

}//namespace Blade


#endif // __Blade_Vector4i_h__