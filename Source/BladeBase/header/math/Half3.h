/********************************************************************
	created:	2015/11/15
	filename: 	Half3.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Half3_h__
#define __Blade_Half3_h__
#include "Half.h"
#include "Vector3.h"

namespace Blade
{
	class /*BLADE_BASE_API*/ Half3
	{
	public:
		Half3() {}
		Half3(Half _x, Half _y, Half _z) :x(_x),y(_y),z(_z)	{}
		Half3(scalar _x, scalar _y, scalar _z) :x(_x),y(_y),z(_z)	{}
		explicit Half3(scalar* data) :x(data[0]),y(data[0]),z(data[2]) {}

		/** @brief  */
		Half3(const Vector3& src) : x(src.x), y(src.y), z(src.z)	{}

		/** @brief  */
		Half3& operator=(const Vector3& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			return *this;
		}

		/** @brief  */
		operator Vector3() const
		{
			return Vector3(x.getFloat(), y.getFloat(), z.getFloat() );
		}

		/** @brief  */
		Half3	operator-() const
		{
			return Half3(-x, -y, -z);
		}

		/** @brief  */
		Half3&	operator+=(const Half3 rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		/** @brief  */
		Half3&	operator+=(const Half f)
		{
			x += f;
			y += f;
			z += f;
			return *this;
		}

		/** @brief  */
		Half3&	operator+=(const fp32 f)
		{
			x += f;
			y += f;
			z += f;
			return *this;
		}

		/** @brief  */
		Half3&	operator+=(const double d)
		{
			x += d;
			y += d;
			z += d;
			return *this;
		}

		/** @brief  */
		Half3&	operator-=(const Half3 rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		/** @brief  */
		Half3&	operator-=(const Half f)
		{
			x -= f;
			y -= f;
			z -= f;
			return *this;
		}

		/** @brief  */
		Half3&	operator-=(const fp32 f)
		{
			x -= f;
			y -= f;
			z -= f;
			return *this;
		}

		/** @brief  */
		Half3&	operator-=(const double d)
		{
			x -= d;
			y -= d;
			z -= d;
			return *this;
		}

		/** @brief  */
		Half3&	operator*=(const Half3 rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;
			return *this;
		}

		/** @brief  */
		Half3&	operator*=(const Half rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			return *this;
		}

		/** @brief  */
		Half3&	operator*=(const fp32 f)
		{
			x *= f;
			y *= f;
			z *= f;
			return *this;
		}

		/** @brief  */
		Half3&	operator*=(const double d)
		{
			x *= d;
			y *= d;
			z *= d;
			return *this;
		}

		/** @brief  */
		Half3&	operator/=(const Half3 rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;
			return *this;
		}

		/** @brief  */
		Half3&	operator/=(const Half rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;
			return *this;
		}

		/** @brief  */
		Half3&	operator/=(const fp32 f)
		{
			x /= f;
			y /= f;
			z /= f;
			return *this;
		}

		/** @brief  */
		Half3&	operator/=(const double d)
		{
			x /= d;
			y /= d;
			z /= d;
			return *this;
		}

		/** @brief  */
		Half3	operator+(const Half3 rhs) const
		{
			return Half3(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		/** @brief  */
		Half3	operator+(const Half rhs) const
		{
			return Half3(x + rhs, y + rhs, z + rhs);
		}

		/** @brief  */
		Half3	operator+(const fp32 f) const
		{
			return Half3(x + f, y + f, z + f);
		}

		/** @brief  */
		Half3	operator+(const double d) const
		{
			return Half3(x + d, y + d, z + d);
		}

		/** @brief  */
		Half3	operator-(const Half3 rhs) const
		{
			return Half3(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		/** @brief  */
		Half3	operator-(const Half rhs) const
		{
			return Half3(x - rhs, y - rhs, z - rhs);
		}

		/** @brief  */
		Half3	operator-(const fp32 f) const
		{
			return Half3(x - f, y - f, z - f);
		}

		/** @brief  */
		Half3	operator-(const double d) const
		{
			return Half3(x - d, y - d, z - d);
		}

		/** @brief  */
		Half3	operator*(const Half3 rhs) const
		{
			return Half3(x * rhs.x, y * rhs.y, z * rhs.z);
		}

		/** @brief  */
		Half3	operator*(const Half rhs) const
		{
			return Half3(x * rhs, y * rhs, z * rhs);
		}

		/** @brief  */
		Half3	operator*(const fp32 f) const
		{
			return Half3(x * f, y * f, z * f);
		}

		/** @brief  */
		Half3	operator*(const double d) const
		{
			return Half3(x * d, y * d, z * d);
		}

		/** @brief  */
		Half3	operator/(const Half3 rhs) const
		{
			return Half3(x / rhs.x, y / rhs.y, z / rhs.z);
		}

		/** @brief  */
		Half3	operator/(const Half rhs) const
		{
			return Half3(x / rhs, y / rhs, z / rhs);
		}

		/** @brief  */
		Half3	operator/(const fp32 f) const
		{
			return Half3(x / f, y / f, z / f);
		}

		/** @brief  */
		Half3	operator/(const double d) const
		{
			return Half3(x / d, y / d, z / d);
		}

	public:
		Half x;
		Half y;
		Half z;

		static const BLADE_BASE_API class Half3	ZERO;
		static const BLADE_BASE_API class Half3	UNIT_X;
		static const BLADE_BASE_API class Half3	UNIT_Y;
		static const BLADE_BASE_API class Half3	UNIT_Z;
		static const BLADE_BASE_API class Half3	UNIT_ALL;
		static const BLADE_BASE_API class Half3	NEGATIVE_UNIT_X;
		static const BLADE_BASE_API class Half3	NEGATIVE_UNIT_Y;
		static const BLADE_BASE_API class Half3	NEGATIVE_UNIT_Z;
		static const BLADE_BASE_API class Half3	NEGATIVE_UNIT_ALL;
	};

	
}//namespace Blade


#endif // __Blade_Half3_h__