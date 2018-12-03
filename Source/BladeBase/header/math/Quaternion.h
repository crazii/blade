/********************************************************************
	created:	2010/04/25
	filename: 	Quaternion.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_Quaternion_h__
#define __Blade_Quaternion_h__
#include <math/BladeMath.h>
#include <math/Vector3.h>

namespace Blade
{
	class Matrix33;

	class Quaternion;
	/** @brief quaternion rotation - this is not a multiplication */
	inline static Vector3	operator*(const Vector3& v, const Quaternion& q);

	class Quaternion
	{
	public:
		inline Quaternion(){}

		inline Quaternion(scalar _x, scalar _y, scalar _z, scalar _w)
			:x(_x)
			,y(_y)
			,z(_z)
			,w(_w)			{}

#if BLADE_ENABLE_SIMD
		BLADE_ALWAYS_INLINE Quaternion(SIMD::vfp32x4 v)
		{
			*this = v;
		}

		BLADE_ALWAYS_INLINE operator SIMD::vfp32x4() const
		{
#	if BLADE_ENABLE_SIMD
			return SIMD::VLOAD4(mData);
#	else
			return reinterpret_cast<const SIMD::vfp32x4&>(*this);
#	endif
		}

		BLADE_ALWAYS_INLINE Quaternion& operator=(SIMD::vfp32x4 v)
		{
#	if BLADE_ENABLE_SIMD
			SIMD::VSTORE4(v, mData);
#	else
			*this = reinterpret_cast<const Quaternion&>(v);
#	endif
			return *this;
		}
#endif

		/** @brief construction from pure data array */
		BLADE_ALWAYS_INLINE  Quaternion(const scalar* data)
		{
			this->set(data);
		}

		/** @brief copy constructor */
		BLADE_ALWAYS_INLINE  Quaternion(const Quaternion& src)
		{
			*this = src;
		}

		/** @brief construction from a rotation matrix */
		BLADE_ALWAYS_INLINE  Quaternion(const Matrix33& roationMatrix)
		{
			this->set(roationMatrix);
		}

		/** @brief construction from an axis and a angle */
		BLADE_ALWAYS_INLINE  Quaternion(const Vector3& axis,scalar fAngleRadians)
		{
			this->set(axis,fAngleRadians);
		}

		/** @brief construction from 3 orthonormal axis */
		BLADE_ALWAYS_INLINE  Quaternion(const Vector3& axis_x,const Vector3& axis_y,const Vector3& axis_z)
		{
			this->set(axis_x,axis_y,axis_z);
		}

		/** @brief construction from yaw pitch roll,in radians */
		BLADE_ALWAYS_INLINE  Quaternion(scalar yaw,scalar pitch,scalar roll)
		{
			this->set(yaw,pitch,roll);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  Quaternion(const Vector3* axis)
		{
			this->set(axis);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE	Quaternion& operator=(const Quaternion& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = (SIMD::vfp32x4)rhs;
#else
			std::memcpy(mData, rhs.mData, sizeof(mData));
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator==(const Quaternion& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VEQUAL4(*this, rhs);
#else
			return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool			operator!=(const Quaternion& rhs) const
		{
			return !((*this) == rhs);
		}

		/** @brief  */
		inline scalar		operator[](size_t index) const
		{
			assert( index < 4 );
			return mData[index];
		}

		/** @brief  */
		inline scalar&		operator[](size_t index)
		{
			assert( index < 4 );
			return mData[index];
		}

		/** @brief  */
		inline const scalar*getData() const
		{
			return mData;
		}

		/** @brief  */
		inline scalar*		getData()
		{
			return mData;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	operator-() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VNEG(*this);
#else
			return Quaternion(-x,-y,-z,-w);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	operator+(const Quaternion& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VADD(*this, rhs);
#else
			return Quaternion(x+rhs.x,y+rhs.y,z+rhs.z,w+rhs.w);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	operator-(const Quaternion& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VSUB(*this, rhs);
#else
			return Quaternion(x-rhs.x,y-rhs.y,z-rhs.z,w-rhs.w);
#endif
		}

		/** @brief  */
		/** @note:
			Standard quaternion multiplication:
				q1*q2 = (w1*w2-v1*v2, w1*v2 + w2*v1 + cross(v1,v2))

			transform order: q2, q1
		*/
		BLADE_ALWAYS_INLINE Quaternion	multiply(const Quaternion& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::QMUL(*this, rhs);
#else
			return Quaternion(
				w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
				w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
				w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
				w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
				);
#endif
		}

		/**
		@describe rotation concatenation
		@param
		@return
		@remarks: this is a reverse order of quaternion multiplication, be careful with it.
		if you need a "quaternion multiplication" you should use Quaternion::multiply()
		if you are doing a rotation concatenation, use this function.
		
		rotation order of q1, q2 are represented as q1*q2
		*/
		BLADE_ALWAYS_INLINE  Quaternion	operator*(const Quaternion& rhs) const
		{
			return rhs.multiply(*this);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	operator*(scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VMUL(*this, rhs);
#else
			return Quaternion(x*rhs, y*rhs, z*rhs, w*rhs);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	operator/(scalar rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDIV(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			return Quaternion(x*inv_rhs, y*inv_rhs, z*inv_rhs, w*inv_rhs);
#endif
		}


		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion&	operator+=(const Quaternion& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VADD(*this, rhs);
#else
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			w += rhs.w;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion&	operator-=(const Quaternion& rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VSUB(*this, rhs);
#else
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			w -= rhs.w;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion&	operator*=(const Quaternion& rhs)
		{
			*this = (*this) * rhs;
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion&	operator*=(scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VMUL(*this, rhs);
#else
			x *= rhs;
			y *= rhs;
			z *= rhs;
			w *= rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion&	operator/=(scalar rhs)
		{
#if BLADE_ENABLE_SIMD
			*this = SIMD::VDIV(*this, rhs);
#else
			fp32 inv_rhs = 1 / rhs;
			x *= inv_rhs;
			y *= inv_rhs;
			z *= inv_rhs;
			w *= inv_rhs;
#endif
			return *this;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE bool	equal(const Quaternion& rhs,scalar angle_radian_tolerance) const
		{
			const scalar fCos = this->dotProduct(rhs);
			scalar angle = std::acos( Math::Clamp(fCos, -1.f, 1.f) );

			return (std::fabs(angle) <= angle_radian_tolerance )
				|| Math::Equal(angle, Math::PI, angle_radian_tolerance);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void	set(const scalar* xyzw)
		{
			*this = *reinterpret_cast<const Quaternion*>(xyzw);
		}
		
		/************************************************************************/
		/* quaternion functions */
		/************************************************************************/
		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	dotProduct(const Quaternion& rhs) const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VDOT4(*this, rhs);
#else
			return w*rhs.w + x*rhs.x + y*rhs.y + z*rhs.z;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	getLength() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::VLENGTH4(*this);
#else
			return std::sqrt( w*w + x*x + y*y + z*z );
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  scalar	getSquaredLength() const
		{
			return this->dotProduct(*this);
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	normalize()
		{
#if BLADE_ENABLE_SIMD
			SIMD::vfp32x4 t = *this;
			fp32 ret = SIMD::VNORMALIZE4(t);
			*this = t;
			return ret;
#else
			scalar fNorm = std::sqrt( w*w + x*x + y*y + z*z );
			*this *= scalar(1.0)/fNorm;
			return fNorm;
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	getConjugate() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::QCONJ(*this);
#else
			return Quaternion(-x,-y,-z,w);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Quaternion	getLn() const
		{
#if BLADE_ENABLE_SIMD
			return SIMD::QLN(*this);
#else
			scalar A = std::acos(Math::Clamp(w, -1.0f, 1.0f));
			return Quaternion(A,A,A,0);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE void	toAxes(Vector3& axisX, Vector3& axisY, Vector3& axisZ) const
		{
#if BLADE_ENABLE_SIMD
			//this may be better for SIMD
			axisX = Vector3::UNIT_X * (*this);
			axisY = Vector3::UNIT_Y * (*this);
			axisZ = Vector3::UNIT_Z * (*this);
#else
			scalar fTx  = 2.0f*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwx = fTx*w;
			scalar fTwy = fTy*w;
			scalar fTwz = fTz*w;
			scalar fTxx = fTx*x;
			scalar fTxy = fTy*x;
			scalar fTxz = fTz*x;
			scalar fTyy = fTy*y;
			scalar fTyz = fTz*y;
			scalar fTzz = fTz*z;

			axisX[0] = 1.0f-(fTyy+fTzz);
			axisX[1] = fTxy+fTwz;
			axisX[2] = fTxz-fTwy;

			axisY[0] = fTxy-fTwz;
			axisY[1] = 1.0f-(fTxx+fTzz);
			axisY[2] = fTyz+fTwx;

			axisZ[0] = fTxz+fTwy;
			axisZ[1] = fTyz-fTwx;
			axisZ[2] = 1.0f-(fTxx+fTyy);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getXAxis() const
		{
#if BLADE_ENABLE_SIMD
			return Vector3::UNIT_X * (*this);
#else
			//scalar fTx  = 2.0*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwy = fTy*w;
			scalar fTwz = fTz*w;
			scalar fTxy = fTy*x;
			scalar fTxz = fTz*x;
			scalar fTyy = fTy*y;
			scalar fTzz = fTz*z;

			return Vector3(1.0f-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getYAxis() const
		{
#if BLADE_ENABLE_SIMD
			return Vector3::UNIT_Y * (*this);
#else
			scalar fTx  = 2.0f*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwx = fTx*w;
			scalar fTwz = fTz*w;
			scalar fTxx = fTx*x;
			scalar fTxy = fTy*x;
			scalar fTyz = fTz*y;
			scalar fTzz = fTz*z;

			return Vector3(fTxy-fTwz, 1.0f-(fTxx+fTzz), fTyz+fTwx);
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE Vector3	getZAxis() const
		{
#if BLADE_ENABLE_SIMD
			return Vector3::UNIT_Z * (*this);
#else
			scalar fTx  = 2.0f*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwx = fTx*w;
			scalar fTwy = fTy*w;
			scalar fTxx = fTx*x;
			scalar fTxz = fTz*x;
			scalar fTyy = fTy*y;
			scalar fTyz = fTz*y;

			return Vector3(fTxz+fTwy, fTyz-fTwx, 1.0f-(fTxx+fTyy));
#endif
		}


		/** @brief get yaw pitch roll in radians */
		/** @note: yaw/pitch/row are related to rotation orders, different order have different results.
		Here we use yaw(Y) pitch(X), roll(Z) order.
		*/
		BLADE_ALWAYS_INLINE scalar	getYaw() const
		{
#if BLADE_ENABLE_SIMD
			Vector3 axisZ = this->getZAxis();
			return std::atan2(axisZ.x, axisZ.z);
#else
			scalar fTx  = 2.0f*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwy = fTy*w;
			scalar fTxx = fTx*x;
			scalar fTxz = fTz*x;
			scalar fTyy = fTy*y;
			return std::atan2(fTxz+fTwy, 1.0f-(fTxx+fTyy));
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	getPitch() const
		{
#if BLADE_ENABLE_SIMD
			//roll won't change z axis
			Vector3 axisZ = this->getZAxis();
			return std::asin(Math::Clamp(-axisZ.y, -1.0f, 1.0f));//[-1,1] or asin returns NaN
#else
			return std::asin(Math::Clamp(2*(w*x - y*z), -1.0f, 1.0f));//[-1,1] or asin returns NaN
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE scalar	getRoll() const
		{
			//http://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf : Appendix A(5) YXZ 
#if BLADE_ENABLE_SIMD
			//atan(m12 / m22)
			Vector3 axisX = this->getXAxis();
			Vector3 axisY = this->getYAxis();
			return std::atan2(axisX.y, axisY.y);
#else
			scalar fTx  = 2.0f*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwz = fTz*w;
			scalar fTxy = fTy*x;
			scalar fTxx = fTx*x;
			scalar fTzz = fTz*z;
			//atan(m12 / m22)
			return std::atan2(fTxy+fTwz, 1.0f-(fTxx+fTzz));
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  void	getYawPitchRoll(scalar& yaw, scalar& pitch, scalar& roll) const
		{
#if BLADE_ENABLE_SIMD
			Vector3 axisZ = this->getZAxis();
			Vector3 axisX = this->getXAxis();
			Vector3 axisY = this->getYAxis();
			yaw = std::atan2(axisZ.x, axisZ.z);
			pitch = std::asin(Math::Clamp(-axisZ.y, -1.0f, 1.0f));
			roll = std::atan2(axisX.y, axisY.y);
#else
			scalar fTx  = 2.0f*x;
			scalar fTy  = 2.0f*y;
			scalar fTz  = 2.0f*z;
			scalar fTwy = fTy*w;
			scalar fTxx = fTx*x;
			scalar fTxz = fTz*x;
			scalar fTyy = fTy*y;
			scalar fTwz = fTz*w;
			scalar fTxy = fTy*x;
			scalar fTzz = fTz*z;
			yaw = std::atan2(fTxz+fTwy, 1.0f-(fTxx+fTyy));
			pitch = std::asin( Math::Clamp(2*(w*x - y*z), -1.0f, 1.0f) );	//[-1,1] or asin returns NaN
			roll = std::atan2(fTxy+fTwz, 1.0f-(fTxx+fTzz));
#endif
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  Quaternion	getNormalizedQuaternion() const
		{
			Quaternion ret = *this;
			ret.normalize();
			return ret;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  Quaternion	getInversedQuaternion() const
		{
			Quaternion ret = *this;
			ret.inverse();
			return ret;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  Quaternion	getSlerp(const Quaternion& rhs, scalar t, bool shortPath = false) const
		{
			Quaternion ret = *this;
			ret.slerpWith(rhs, t, shortPath);
			return ret;
		}

		/** @brief  */
		inline Quaternion	getNlerp(const Quaternion& rhs, scalar t, bool shortPath = false) const
		{
			Quaternion ret = *this;
			ret.nlerpWith(rhs, t, shortPath);
			return ret;
		}

		/** @brief  */
		BLADE_ALWAYS_INLINE  Quaternion	getSquad(const Quaternion& qA,const Quaternion& qB,const Quaternion& qC,scalar t, bool shortPath = false)
		{
			Quaternion ret = *this;
			ret.squadWith(qA, qB, qC, t, shortPath);
			return ret;
		}

		/** @brief  */
		static BLADE_ALWAYS_INLINE  Quaternion	slerp(const Quaternion& q0, const Quaternion& q1, scalar t, bool shortPath = false)
		{
			Quaternion ret = q0;
			ret.slerpWith(q1, t, shortPath);
			return ret;
		}

		/** @brief  */
		static BLADE_ALWAYS_INLINE  Quaternion	nlerp(const Quaternion& q0, const Quaternion& q1, scalar t, bool shortPath = false)
		{
			Quaternion ret = q0;
			ret.nlerpWith(q1, t, shortPath);
			return ret;
		}

		/** @brief  */
		static BLADE_ALWAYS_INLINE  Quaternion	squad(const Quaternion& q,const Quaternion& qA,const Quaternion& qB,const Quaternion& qC,scalar t, bool shortPath = false)
		{
			Quaternion ret = q;
			ret.squadWith(qA, qB, qC, t, shortPath);
			return ret;
		}

		/** @brief  */
		BLADE_BASE_API void	set(const Matrix33& rotationMatrix);

		/** @brief  */
		BLADE_BASE_API void	set(const Vector3& axis,scalar fAngleRadians);

		/** @brief  */
		BLADE_BASE_API void	set(const Vector3& axis_x,const Vector3& axis_y,const Vector3& axis_z);

		/** @brief  */
		BLADE_BASE_API void	set(const Vector3* axes);

		/** @brief  */
		BLADE_BASE_API void	set(scalar fYawRadians,scalar fPitchRadians,scalar fRollRadians);

		/** @brief  */
		BLADE_BASE_API void	toAxisAngle(Vector3& axis, scalar& fAngleRadians) const;

		/** @brief  */
		BLADE_BASE_API void	toRotationMatrix(Matrix33& rotationMatrix) const;

		/** @brief  */
		BLADE_BASE_API Quaternion	getLog() const;

		/** @brief return original squared length */
		BLADE_BASE_API scalar		inverse();

		/** @brief  */
		BLADE_BASE_API Quaternion	getExp() const;

		/** @brief spherical linear interpolation */
		BLADE_BASE_API bool	slerpWith(const Quaternion& rhs, scalar t, bool shortPath = false, scalar* length = NULL);

		/** @brief normalized linear interpolation */
		BLADE_BASE_API bool	nlerpWith(const Quaternion& rhs, scalar t, bool shortPath = false, scalar* length = NULL);

		/** @brief  */
		BLADE_BASE_API Quaternion& squadWith(const Quaternion& qA,const Quaternion& qB,const Quaternion& qC,scalar t, bool shortPath = false);

		/** @brief return false if dir0 & dir1 already in the same direction, and out set to IDENTITY */
		BLADE_BASE_API static bool			generateRotation(Quaternion& out,const Vector3& dir0,const Vector3& dir1);

		/** @brief  */
		BLADE_ALWAYS_INLINE static Quaternion	generateRotation(const Vector3& dir0, const Vector3& dir1)
		{
			Quaternion ret;
			Quaternion::generateRotation(ret, dir0, dir1);
			return ret;
		}

		BLADE_BASE_API static void			generateLookAtRotation(Quaternion& out, const Vector3& lookat, const Vector3& upVec = Vector3::UNIT_Y);

		BLADE_ALWAYS_INLINE static Quaternion	generateLookAtRotation(const Vector3& dir0, const Vector3& dir1)
		{
			Quaternion ret;
			Quaternion::generateLookAtRotation(ret, dir0, dir1);
			return ret;
		}

		//add class keyword to avoid weird symbol conflicts
		BLADE_BASE_API static const class Quaternion IDENTITY;
		BLADE_BASE_API static const class Quaternion ZERO;

		union
		{
			struct
			{
				scalar	x,y,z,w;
			};
			scalar	mData[4];
		};
	};//class Quaternion


	//commutative
	BLADE_ALWAYS_INLINE static Quaternion	operator*(scalar lhs, const Quaternion& rhs)
	{
		return rhs*lhs;
	}

	BLADE_ALWAYS_INLINE static Vector3& operator*=(Vector3& v, const Quaternion& q)
	{
		return (v = v * q);
	}

	//////////////////////////////////////////////////////////////////////////
	BLADE_ALWAYS_INLINE static Vector3	operator*(const Vector3& v, const Quaternion& q)
	{
		const Vector3& qvec = reinterpret_cast<const Vector3&>(q);
		Vector3 t = qvec.crossProduct(v) * 2;
		return v + t*q.w + qvec.crossProduct(t);
	}
	
}//namespace Blade


#endif //__Blade_Quaternion_h__