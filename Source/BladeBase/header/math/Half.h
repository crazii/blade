/********************************************************************
	created:	2013/04/22
	filename: 	Half.h
	author:		Crazii
	
	purpose:	IEEE 754-2008 16 bit floating point format
*********************************************************************/
#ifndef __Blade_Half_h__
#define __Blade_Half_h__
#include <BladeBase.h>

namespace Blade
{
	//reference:	http://en.wikipedia.org/wiki/Half_precision
	//				http://en.wikipedia.org/wiki/Single_precision
	class Half
	{
	public:
		union UF32U32
		{
			fp32	f;
			uint32	u;			
		};

		static const unsigned short SIGNBIT_MASK16 = 0x8000U;	//1bit

		static const unsigned short FRACTION_MASK16 = 0x3FFU;	//10bit
		static const unsigned short FRACTION_BITS16 = 10;
		
		static const unsigned short EXPONENT_MASK16 = 0x7C00U;	//5bit
		static const unsigned short EXPONENT_BIAS16 = 15;			//singed 2^5 : 2^4

		static const unsigned int SIGNBIT_MASK32 = 0x80000000U;	//1bit

		static const unsigned int FRACTION_MASK32 = 0x7FFFFFU;	//23bit
		static const unsigned short FRACTION_BITS32 = 23;		//10bit
		static const unsigned int EXPONENT_MASK32 = 0x7F800000U;	//8bit
		static const unsigned int EXPONENT_BIAS32 = 127;

	public:
		inline Half() :mHalf(0)	{}
		Half(fp32 f)	{this->setFloat(f);}
		Half(double d)	{this->setFloat( (fp32)d );}

		/** @brief  */
		fp32	getFloat() const
		{
			return _toFloat[mHalf].f;
		}

		/** @brief  */
		bool	isNaN() const
		{
			return *this == QUIET_NAN || *this == SIGNALING_NAN;
		}

		/** @brief  */
		bool	isFinite() const
		{
			return *this == POSITIVE_INF || *this == NEGATIVE_INF;
		}

		/** @brief  */
		bool	isZero () const
		{
			return (mHalf & 0x7fff) == 0;
		}

		Half	getAbs() const
		{
			Half h;
			h.mHalf = (uint16)( mHalf & (~0x8000u) );
			return h;			
		}

		/** @brief  */
		Half	operator-() const
		{
			Half h;
			h.mHalf = (uint16)( mHalf ^ 0x8000u );
			return h;
		}

		/** @brief  */
		Half&	operator=(const Half rhs)
		{
			mHalf = rhs.mHalf;
			return *this;
		}
		/** @brief  */
		Half&	operator=(const fp32 f)
		{
			this->setFloat(f);
			return *this;
		}

		/** @brief  */
		Half&	operator=(const double d)
		{
			this->setFloat( (fp32)d );
			return *this;
		}

		Half&	operator+=(const Half rhs)
		{
			fp32 v = this->getFloat() + rhs.getFloat();
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator+=(const fp32 f)
		{
			fp32 v = this->getFloat() + f;
			this->setFloat(v);
			return *this;
		}
		/** @brief  */
		Half&	operator+=(const double d)
		{
			fp32 v = this->getFloat() + (fp32)d;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator-=(const Half rhs)
		{
			fp32 v = this->getFloat() - rhs.getFloat();
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator-=(const fp32 f)
		{
			fp32 v = this->getFloat() - f;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator-=(const double d)
		{
			fp32 v = this->getFloat() - (fp32)d;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator*=(const Half rhs)
		{
			fp32 v = this->getFloat() * rhs.getFloat();
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator*=(const fp32 f)
		{
			fp32 v = this->getFloat() * f;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator*=(const double d)
		{
			fp32 v = this->getFloat() * (fp32)d;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator/=(const Half rhs)
		{
			fp32 v = this->getFloat() / rhs.getFloat();
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator/=(const fp32 f)
		{
			fp32 v = this->getFloat() / f;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half&	operator/=(const double d)
		{
			fp32 v = this->getFloat() / (fp32)d;
			this->setFloat(v);
			return *this;
		}

		/** @brief  */
		Half	operator+(const Half rhs) const
		{
			return Half(this->getFloat() + rhs.getFloat());
		}

		/** @brief  */
		Half	operator+(const fp32 f) const
		{
			return Half(this->getFloat() + f);
		}

		/** @brief  */
		Half	operator+(const double d) const
		{
			return Half((double)this->getFloat() + d);
		}

		/** @brief  */
		Half	operator-(const Half rhs) const
		{
			return Half(this->getFloat() - rhs.getFloat());
		}

		/** @brief  */
		Half	operator-(const fp32 f) const
		{
			return Half(this->getFloat() - f);
		}

		/** @brief  */
		Half	operator-(const double d) const
		{
			return Half((double)this->getFloat() - (fp32)d);
		}

		/** @brief  */
		Half	operator*(const Half rhs) const
		{
			return Half(this->getFloat() * rhs.getFloat());
		}

		/** @brief  */
		Half	operator*(const fp32 f) const
		{
			return Half(this->getFloat() * f);
		}

		/** @brief  */
		Half	operator*(const double d) const
		{
			return Half((double)this->getFloat() * d);
		}

		/** @brief  */
		Half	operator/(const Half rhs) const
		{
			return Half(this->getFloat() / rhs.getFloat());
		}

		/** @brief  */
		Half	operator/(const fp32 f) const
		{
			return Half(this->getFloat() / f);
		}

		/** @brief  */
		Half	operator/(const double d) const
		{
			return Half((double)this->getFloat() / d);
		}

		/** @brief  */
		bool	operator<(const Half rhs) const
		{
			return this->getFloat() < rhs.getFloat();
		}

		/** @brief  */
		bool	operator>(const Half rhs) const
		{
			return this->getFloat() > rhs.getFloat();
		}

		/** @brief  */
		bool	operator<(const fp32 rhs) const
		{
			return this->getFloat() < rhs;
		}

		/** @brief  */
		bool	operator>(const fp32 rhs) const
		{
			return this->getFloat() > rhs;
		}

		/** @brief  */
		bool	operator<=(const Half rhs) const
		{
			return this->getFloat() <= rhs.getFloat();
		}

		/** @brief  */
		bool	operator>=(const Half rhs) const
		{
			return this->getFloat() >= rhs.getFloat();
		}

		/** @brief  */
		bool	operator<=(const fp32 rhs) const
		{
			return this->getFloat() <= rhs;
		}

		/** @brief  */
		bool	operator>=(const fp32 rhs) const
		{
			return this->getFloat() >= rhs;
		}

		/** @brief  */
		friend bool	operator<(const fp32 lhs, const Half rhs);
		friend bool	operator>(const fp32 lhs, const Half rhs);
		friend bool	operator<=(const fp32 lhs, const Half rhs);
		friend bool	operator>=(const fp32 lhs, const Half rhs);

		/** @brief  */
		bool	operator==(const Half rhs) const
		{
			return mHalf == rhs.mHalf;
		}

		/** @brief  */
		bool	operator==(const fp32 f) const
		{
			return mHalf == Half(f).mHalf;
		}

		/** @brief  */
		bool	operator==(const double d) const
		{
			return mHalf == Half(d).mHalf;
		}

		/** @brief  */
		bool	operator!=(const Half rhs) const
		{
			return mHalf != rhs.mHalf;
		}

		/** @brief  */
		bool	operator!=(const fp32 f) const
		{
			return mHalf != Half(f).mHalf;
		}

		/** @brief  */
		bool	operator!=(const double d) const
		{
			return mHalf != Half(d).mHalf;
		}

		/** @brief  */
		bool	equal(Half rhs, const fp32 tolerance) const
		{
			return this->equal( rhs.getFloat(), tolerance );
		}

		/** @brief  */
		bool	equal(fp32 rhs, const fp32 tolerance) const
		{
			fp32 lhs = this->getFloat();
			return lhs - rhs <= tolerance && lhs - rhs >= -tolerance;
		}

		/** @brief  */
		bool	equal(double d, const fp32 tolerance) const
		{
			return this->equal( (fp32)d, tolerance);
		}

	public:

		/** @brief  */
		static BLADE_BASE_API const Half		POSITIVE_INF;	//+infinite
		static BLADE_BASE_API const Half		NEGATIVE_INF;	//-infinite
		static BLADE_BASE_API const Half		QUIET_NAN;
		static BLADE_BASE_API const Half		SIGNALING_NAN;

	protected:

		/** @brief  */
		inline void setFloat(fp32 f)
		{
			UF32U32 ufu;
			ufu.f = f;

			if (f == 0)
			{
				// Common special case - zero.
				// Preserve the zero's sign bit.
				mHalf = (ufu.u >> 16);
			}
			else
			{
				//
				// We extract the combined sign and exponent, e, from our
				// floating-point number, f.  Then we convert e to the sign
				// and exponent of the half number via a table lookup.
				//
				// For the most common case, where a normalized half is produced,
				// the table lookup returns a non-zero value; in this case, all
				// we have to do is round f's significand to 10 bits and combine
				// the result with e.
				//
				// For all other cases (overflow, zeroes, denormalized numbers
				// resulting from underflow, infinities and NANs), the table
				// lookup returns zero, and we call a longer, non-inline function
				// to do the fp32-to-half conversion.
				//

				int e = (int)((ufu.u >> 23) & 0x000001ff);

				e = _eLut[e];

				if( e )
				{
					// Simple case - round the significand, m, to 10
					// bits and combine it with the sign and exponent.
					uint32 m = ufu.u & 0x007fffff;
					mHalf = uint16( e + ((m + 0x00000fff + ((m >> 13) & 1)) >> 13) );
				}
				else
				{
					// Difficult case - call a function.
					mHalf = F32toF16(ufu.u);
				}
			}
		}

		/** @brief complicated case to convert float32 to float16 */
		BLADE_BASE_API static uint16 F32toF16(uint32 f32);

		BLADE_BASE_API static UF32U32	_toFloat[1 << 16];
		BLADE_BASE_API static uint16	_eLut[1 << 9];

		uint16 mHalf;
	};
	typedef Half			fp16,	*pfp16;


	inline Half operator+(const fp32 lhs, const Half rhs)
	{
		return rhs + lhs;
	}

	inline Half operator+(const double lhs, const Half rhs)
	{
		return rhs + lhs;
	}

	inline Half operator-(const fp32 lhs, const Half rhs)
	{
		return Half(lhs - rhs.getFloat());
	}

	inline Half operator-(const double lhs, const Half rhs)
	{
		return Half(lhs - rhs.getFloat());
	}

	inline Half operator*(const fp32 lhs, const Half rhs)
	{
		return rhs * lhs;
	}

	inline Half operator*(const double lhs, const Half rhs)
	{
		return rhs * lhs;
	}

	inline Half operator/(const fp32 lhs, const Half rhs)
	{
		return rhs / lhs;
	}

	inline Half operator/(const double lhs, const Half rhs)
	{
		return rhs / lhs;
	}

	inline bool	operator==(const fp32 lhs, const Half rhs)
	{
		return rhs == lhs;
	}

	inline bool	operator==(const double lhs, const Half rhs)
	{
		return rhs == lhs;
	}

	inline bool	operator!=(const fp32 lhs, const Half rhs)
	{
		return rhs != lhs;
	}

	inline bool	operator!=(const double lhs, const Half rhs)
	{
		return rhs == lhs;
	}



	inline bool	operator<(const fp32 lhs, const Half rhs)
	{
		return rhs > lhs;
	}

	inline bool	operator>(const fp32 lhs, const Half rhs)
	{
		return rhs < lhs;
	}

	inline bool	operator<=(const fp32 lhs, const Half rhs)
	{
		return rhs >= lhs;
	}

	inline bool	operator>=(const fp32 lhs, const Half rhs)
	{
		return rhs <= lhs;
	}

}//namespace Blade

#endif//__Blade_Half_h__