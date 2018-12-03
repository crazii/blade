/********************************************************************
	created:	2010/04/20
	filename: 	Vector4.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Vector4_h__
#define __Blade_Vector4_h__
#include <BladePlatform.h>
#include <math/Vector3.h>

namespace Blade
{
	class Matrix44;

	class Vector4
	{
	public:
		inline Vector4()
		{

		}

		inline Vector4(scalar x,scalar y,scalar z,scalar w)
		{
			mData[0] = x;
			mData[1] = y;
			mData[2] = z;
			mData[3] = w;
		}

#if BLADE_ENABLE_SIMD
		BLADE_ALWAYS_INLINE Vector4(SIMD::vfp32x4 v)
		{
			*this = v;
		}

		BLADE_ALWAYS_INLINE operator SIMD::vfp32x4() const
		{
			return SIMD::VLOAD4(mData);
		}

		BLADE_ALWAYS_INLINE Vector4& operator=(SIMD::vfp32x4 v)
		{
			SIMD::VSTORE4(v, mData);
			return *this;
		}
#endif

		explicit inline Vector4(const scalar* pdata)
		{
			*this = *reinterpret_cast<const Vector4*>(pdata);
		}

		inline Vector4(const Vector4& v)
		{
			*this = v;
		}

		explicit inline Vector4(const Vector3& src, scalar w)
		{
			this->set(src, w);
		}

		/** @brief  */
		inline scalar	operator[](size_t sub) const
		{
			assert( sub < 4 );
			if( sub < 4 )
				return mData[sub];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range."));
		}

		/** @brief  */
		inline scalar&	operator[](size_t sub)
		{
			assert( sub < 4 );
			if( sub < 4 )
				return mData[sub];
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("index out of range."));
		}


		/** @brief  */
		inline const scalar	X() const
		{
			return mData[0];
		}

		/** @brief  */
		inline scalar&	X()
		{
			return mData[0];
		}


		/** @brief  */
		inline const scalar	Y() const
		{
			return mData[1];
		}

		/** @brief  */
		inline scalar&	Y()
		{
			return mData[1];
		}

		/** @brief  */
		inline const scalar	Z() const
		{
			return mData[2];
		}

		/** @brief  */
		inline scalar&	Z()
		{
			return mData[2];
		}

		/** @brief  */
		inline const scalar	W() const
		{
			return mData[3];
		}

		/** @brief  */
		inline scalar&	W()
		{
			return mData[3];
		}

		/** @brief  */
		inline const scalar*	getData() const
		{
			return mData;
		}

		/** @brief  */
		inline scalar*			getData()
		{
			return mData;
		}

		BLADE_ALWAYS_INLINE void			set(const Vector3& rhs, scalar _w)
		{
#if BLADE_ENABLE_SIMD
			SIMD::vfp32x4 v = (SIMD::vfp32x4)rhs;
			v = SIMD::VSETW(v, &_w);
			*this = v;
#else
			mData[0] = rhs[0];
			mData[1] = rhs[1];
			mData[2] = rhs[2];
			mData[3] = _w;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator=(const Vector4& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = (SIMD::vfp32x4)rhs;
#else
			std::memcpy(mData, rhs.mData, sizeof(mData));
#endif
			return *this;
		}


		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator=(const Vector3& rhs)
		{
#if BLADE_ENABLE_SIMD
			this->set(rhs, 1.0f);
#else
			std::memcpy(mData, rhs.mData, sizeof(rhs) );
			mData[3] = 1;
#endif
			return *this;
		}


		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator==(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VEQUAL4(*this, rhs);
#else
			return mData[0] == rhs.mData[0] && mData[1] == rhs.mData[1] && mData[2] == rhs.mData[2] && mData[3] == rhs.mData[3];
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator!=(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return !SIMD::VEQUAL4(*this, rhs);
#else
			return mData[0] != rhs.mData[0] || mData[1] != rhs.mData[1] || mData[2] != rhs.mData[2] || mData[3] != rhs.mData[3];
#endif	
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator<(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLESS4(*this, rhs);
#else
			return mData[0] < rhs.mData[0] && mData[1] < rhs.mData[1] && mData[2] < rhs.mData[2] && mData[3] < rhs.mData[3];
#endif
			
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator>(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VGREATER4(*this, rhs);
#else
			return mData[0] > rhs.mData[0] && mData[1] > rhs.mData[1] && mData[2] > rhs.mData[2] && mData[3] > rhs.mData[3];
#endif
			
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator<=(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLESSEQUAL4(*this, rhs);
#else
			return mData[0] <= rhs.mData[0] && mData[1] <= rhs.mData[1] && mData[2] <= rhs.mData[2] && mData[3] <= rhs.mData[3];
#endif
			
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator>=(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VGREATEREQUAL4(*this, rhs);
#else
			return mData[0] >= rhs.mData[0] && mData[1] >= rhs.mData[1] && mData[2] >= rhs.mData[2] && mData[3] >= rhs.mData[3];
#endif
			
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator+(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD(*this, rhs);
#else
			return Vector4(mData[0]+rhs.mData[0],mData[1]+rhs.mData[1],mData[2] + rhs.mData[2],mData[3] + rhs.mData[3]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator-(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB(*this, rhs);
#else
			return Vector4(mData[0]-rhs.mData[0],mData[1]-rhs.mData[1],mData[2] - rhs.mData[2],mData[3] - rhs.mData[3]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator*(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL(*this, rhs);
#else
			return Vector4(mData[0]*rhs.mData[0],mData[1]*rhs.mData[1],mData[2] * rhs.mData[2],mData[3] * rhs.mData[3]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator/(const Vector4& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV(*this, rhs);
#else
			return Vector4(mData[0]/rhs.mData[0],mData[1]/rhs.mData[1],mData[2] / rhs.mData[2],mData[3] / rhs.mData[3]);
#endif
			
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator+(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD(*this, rhs);
#else
			return Vector4(mData[0]+rhs,mData[1]+rhs,mData[2]+rhs,mData[3]+rhs);
#endif	
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator-(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB(*this, rhs);
#else
			return Vector4(mData[0]-rhs,mData[1]-rhs,mData[2]-rhs,mData[3]-rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator*(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL(*this, rhs);
#else
			return Vector4(mData[0]*rhs,mData[1]*rhs,mData[2]*rhs,mData[3]*rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator/(const scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			return Vector4(mData[0]*inv_rhs,mData[1]*inv_rhs,mData[2]*inv_rhs,mData[3]*inv_rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4			operator-() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VNEG(*this);
#else
			return Vector4(-mData[0],-mData[1],-mData[2],-mData[3]);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator+=(const Vector4& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD(*this, rhs);
#else
			mData[0] += rhs.mData[0];
			mData[1] += rhs.mData[1];
			mData[2] += rhs.mData[2];
			mData[3] += rhs.mData[3];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator-=(const Vector4& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB(*this, rhs);
#else
			mData[0] -= rhs.mData[0];
			mData[1] -= rhs.mData[1];
			mData[2] -= rhs.mData[2];
			mData[3] -= rhs.mData[3];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator*=(const Vector4& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VMUL(*this, rhs);
#else
			mData[0] *= rhs.mData[0];
			mData[1] *= rhs.mData[1];
			mData[2] *= rhs.mData[2];
			mData[3] *= rhs.mData[3];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator/=(const Vector4& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV(*this, rhs);
#else
			mData[0] /= rhs.mData[0];
			mData[1] /= rhs.mData[1];
			mData[2] /= rhs.mData[2];
			mData[3] /= rhs.mData[3];
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator+=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD(*this, rhs);
#else
			mData[0] += rhs;
			mData[1] += rhs;
			mData[2] += rhs;
			mData[3] += rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator-=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB(*this, rhs);
#else
			mData[0] -= rhs;
			mData[1] -= rhs;
			mData[2] -= rhs;
			mData[3] -= rhs;
#endif
			return *this;

		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator*=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VMUL(*this, rhs);
#else
			mData[0] *= rhs;
			mData[1] *= rhs;
			mData[2] *= rhs;
			mData[3] *= rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector4&		operator/=(const scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			mData[0] *= inv_rhs;
			mData[1] *= inv_rhs;
			mData[2] *= inv_rhs;
			mData[3] *= inv_rhs;
#endif
			return *this;
		}

		/** @brief  */
		inline operator			const Vector3&() const
		{
			return reinterpret_cast<const Vector3&>(*this);
		}

		/** @brief  */
		inline operator			Vector3&()
		{
			return reinterpret_cast<Vector3&>(*this);
		}

		/** @brief  */
		typedef struct U8x4N
		{
			typedef uint8 ELEMENT_TYPE;
			uint8 x, y, z, w;
		}PACKED;

		/** @brief pack vectors with range [-1,1] */
		BLADE_ALWAYS_INLINE static PACKED pack(const Vector4& v)
		{
			assert(v >= Vector4::NEGATIVE_UNIT_ALL && v <= Vector4::UNIT_ALL);
			Vector4 n01v = v * 0.5f + 0.5f;
			PACKED p = { PACKED::ELEMENT_TYPE(n01v.x*255.0f), PACKED::ELEMENT_TYPE(n01v.y*255.0f), PACKED::ELEMENT_TYPE(n01v.z*255.0f), PACKED::ELEMENT_TYPE(n01v.w*255.0f) };
			return p;
		}

		/** @brief unpack vector to range [-1,1] */
		BLADE_ALWAYS_INLINE static Vector4 unpack(PACKED p)
		{
			Vector4 v;
			v.x = p.x / 255.0f;
			v.y = p.y / 255.0f;
			v.z = p.z / 255.0f;
			v.w = p.w / 255.0f;
			return v * 2.0f - 1.0f;
		}

		/** @brief pack vectors with range [0,1] */
		BLADE_ALWAYS_INLINE static PACKED pack01(const Vector4& v)
		{
			assert(v >= Vector4::ZERO && v <= Vector4::UNIT_ALL);
			PACKED p = { PACKED::ELEMENT_TYPE(v.x*255.0f), PACKED::ELEMENT_TYPE(v.y*255.0f), PACKED::ELEMENT_TYPE(v.z*255.0f), PACKED::ELEMENT_TYPE(v.w*255.0f) };
			return p;
		}

		/** @brief unpack vector to range [0,1] */
		BLADE_ALWAYS_INLINE static Vector4 unpack01(PACKED p)
		{
			Vector4 v;
			v.x = p.x;
			v.y = p.y;
			v.z = p.z;
			v.w = p.w;
			v *= 1.0f / 255.0f;
			return v;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE PACKED::ELEMENT_TYPE packComponent01(scalar c)
		{
			assert(c >= 0 && c <= 1);
			return PACKED::ELEMENT_TYPE(c*255.0f);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar unpackComponent01(PACKED::ELEMENT_TYPE c)
		{
			scalar uc = c;
			return PACKED::ELEMENT_TYPE(uc * (1.0f / 255.0f));
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE PACKED::ELEMENT_TYPE packComponent(scalar c)
		{
			assert(c >= -1 && c <= 1);
			c = c * 0.5f + 0.5f;
			return PACKED::ELEMENT_TYPE(c*255.0f);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar unpackComponent(PACKED::ELEMENT_TYPE c)
		{
			scalar uc = c;
			uc *= (1.0f / 255.0f);
			uc = uc * 2.0f - 1.0f;
			return uc;
		}

		/** @brief  */
		BLADE_BASE_API Vector4		operator*(const Matrix44& rhs) const;

		/** @brief  */
		BLADE_BASE_API Vector4&		operator*=(const Matrix44& rhs);

		BLADE_BASE_API static const Vector4	ZERO;
		BLADE_BASE_API static const Vector4	UNIT_X;
		BLADE_BASE_API static const Vector4	UNIT_Y;
		BLADE_BASE_API static const Vector4	UNIT_Z;
		BLADE_BASE_API static const Vector4	UNIT_XYZ;
		BLADE_BASE_API static const Vector4	UNIT_ALL;
		BLADE_BASE_API static const Vector4	NEGATIVE_UNIT_X;
		BLADE_BASE_API static const Vector4	NEGATIVE_UNIT_Y;
		BLADE_BASE_API static const Vector4	NEGATIVE_UNIT_Z;
		BLADE_BASE_API static const Vector4	NEGATIVE_UNIT_ALL;

		public:
			union
			{
				struct  
				{
					scalar x,y,z,w;
				};
				scalar	mData[4];
			};
	};

	typedef Vector4 POINT4;
	
}//namespace Blade


#endif //__Blade_Vector4_h__