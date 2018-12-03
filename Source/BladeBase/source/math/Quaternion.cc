/********************************************************************
	created:	2010/04/25
	filename: 	Quaternion.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Quaternion.h>
#include <math/Matrix33.h>
#include <math/Vector4.h>

//reference :	http://en.wikipedia.org/wiki/Quaternion
//				http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

namespace Blade
{
	const Quaternion Quaternion::IDENTITY = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	const Quaternion Quaternion::ZERO = Quaternion(0.0f, 0.0f, 0.0f, 0.0f);

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::set(const Matrix33& rotationMatrix)
	{
		//http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
		//code alted for row major matrix

		scalar fTrace = rotationMatrix[0][0]+rotationMatrix[1][1]+rotationMatrix[2][2];
		scalar fRoot;

		if ( fTrace > 0.0 )
		{
			fRoot = std::sqrt(fTrace + 1.0f);
			w = 0.5f*fRoot;
			fRoot = 0.5f/fRoot;
			x = (rotationMatrix[1][2]-rotationMatrix[2][1])*fRoot;
			y = (rotationMatrix[2][0]-rotationMatrix[0][2])*fRoot;
			z = (rotationMatrix[0][1]-rotationMatrix[1][0])*fRoot;
		}
		else
		{
			if ( rotationMatrix[0][0] > rotationMatrix[1][1] && rotationMatrix[0][0] > rotationMatrix[2][2] )
			{
				fRoot = 0.5f / std::sqrt( 1.0f + rotationMatrix[0][0] - rotationMatrix[1][1] - rotationMatrix[2][2]);
				w = (rotationMatrix[1][2] - rotationMatrix[2][1] ) * fRoot;
				x = 0.25f / fRoot;
				y = (rotationMatrix[1][0] + rotationMatrix[0][1] ) * fRoot;
				z = (rotationMatrix[2][0] + rotationMatrix[0][2] ) * fRoot;
			}
			else if (rotationMatrix[1][1] > rotationMatrix[2][2])
			{
				fRoot = 0.5f / std::sqrt( 1.0f + rotationMatrix[1][1] - rotationMatrix[0][0] - rotationMatrix[2][2]);
				w = (rotationMatrix[2][0] - rotationMatrix[0][2] ) * fRoot;
				x = (rotationMatrix[1][0] + rotationMatrix[0][1] ) * fRoot;
				y = 0.25f / fRoot;
				z = (rotationMatrix[2][1] + rotationMatrix[1][2] ) * fRoot;
			}
			else
			{
				fRoot = 0.5f / std::sqrt( 1.0f + rotationMatrix[2][2] - rotationMatrix[0][0] - rotationMatrix[1][1] );
				w = (rotationMatrix[0][1] - rotationMatrix[1][0] ) * fRoot;
				x = (rotationMatrix[2][0] + rotationMatrix[0][2] ) * fRoot;
				y = (rotationMatrix[2][1] + rotationMatrix[1][2] ) * fRoot;
				z = 0.25f / fRoot;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::set(const Vector3& axis,scalar fAngleRadians)
	{
		//cos(theta), v*sin(theta)
		//w,			x,y,z
		scalar fHalfAngle = scalar(0.5)*fAngleRadians;
		scalar fSin = std::sin(fHalfAngle);
		scalar fCos = std::cos(fHalfAngle);
#if BLADE_ENABLE_SIMD
		reinterpret_cast<Vector4&>(*this).set(axis * fSin, fCos);
#else
		x = fSin*axis.x;
		y = fSin*axis.y;
		z = fSin*axis.z;
		w = fCos;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::set(const Vector3& axis_x,const Vector3& axis_y,const Vector3& axis_z)
	{
		this->set( Matrix33(axis_x, axis_y, axis_z) );
	}

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::set(const Vector3* axes)
	{
		this->set( Matrix33(axes[0],axes[1],axes[2]) );
	}

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::set(scalar fYawRadians, scalar fPitchRadians, scalar fRollRadians)
	{
		scalar fHalfYaw = fYawRadians * scalar(0.5);
		scalar fHalfPitch = fPitchRadians * scalar(0.5);
		scalar fHalfRoll = fRollRadians * scalar(0.5);

		scalar cosYaw = std::cos(fHalfYaw);
		scalar sinYaw = std::sin(fHalfYaw);
		scalar cosPitch = std::cos(fHalfPitch);
		scalar sinPitch = std::sin(fHalfPitch);
		scalar cosRoll = std::cos(fHalfRoll);
		scalar sinRoll = std::sin(fHalfRoll);

		//yaw pitch roll axes : y, x, z (y UP)
		//http://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19770024290.pdf : Appendix A(5) YXZ 
		//(note there's a typo in the PDF - theta3 = tan-1(m12/m22) see getRoll() )
		// m12 = cos2sin3, m22 = cos2cos3
		w = sinYaw*sinPitch*sinRoll + cosYaw*cosPitch*cosRoll;
		x = sinYaw*sinRoll*cosPitch + sinPitch*cosYaw*cosRoll;
		y = sinYaw*cosPitch*cosRoll - sinPitch*sinRoll*cosYaw;
		z = -sinYaw*sinPitch*cosRoll + sinRoll*cosYaw*cosPitch;
	}

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::toAxisAngle(Vector3& axis, scalar& fAngleRadians) const
	{
		scalar fSqrLength = this->dotProduct(*this);
		if ( fSqrLength > 0.0 )
		{
			fAngleRadians = scalar(2.0)*std::cos(w);
			scalar fInvLength = scalar(1.0)/std::sqrt(fSqrLength);
			axis *= fInvLength;
		}
		else
		{
			fAngleRadians = 0;
			axis.x = 1.0;
			axis.y = 0.0;
			axis.z = 0.0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	Quaternion::toRotationMatrix(Matrix33& rotationMatrix) const
	{
#if BLADE_SIMD != BLADE_SIMD_NONE
		//this may be better for SIMD
		rotationMatrix[0] = Vector3::UNIT_X * (*this);
		rotationMatrix[1] = Vector3::UNIT_Y * (*this);
		rotationMatrix[2] = Vector3::UNIT_Z * (*this);
#else
		//above is OK too, but the below is optimized
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

		rotationMatrix[0][0] = 1.0f-(fTyy+fTzz);
		rotationMatrix[0][1] = fTxy+fTwz;
		rotationMatrix[0][2] = fTxz-fTwy;

		rotationMatrix[1][0] = fTxy-fTwz;
		rotationMatrix[1][1] = 1.0f-(fTxx+fTzz);
		rotationMatrix[1][2] = fTyz+fTwx;

		rotationMatrix[2][0] = fTxz+fTwy;
		rotationMatrix[2][1] = fTyz-fTwx;
		rotationMatrix[2][2] = 1.0f-(fTxx+fTyy);

		//below is a column major version ( matrix33 * vector3 ), in spite that most arciles found on internet says its a row major matrix version

		//rotationMatrix[0][0] = 1.0f-(fTyy+fTzz);
		//rotationMatrix[0][1] = fTxy-fTwz;
		//rotationMatrix[0][2] = fTxz+fTwy;

		//rotationMatrix[1][0] = fTxy+fTwz;
		//rotationMatrix[1][1] = 1.0f-(fTxx+fTzz);
		//rotationMatrix[1][2] = fTyz-fTwx;

		//rotationMatrix[2][0] = fTxz-fTwy;
		//rotationMatrix[2][1] = fTyz+fTwx;
		//rotationMatrix[2][2] = 1.0f-(fTxx+fTyy);
#endif
	}

	/************************************************************************/
	/* quaternion functions */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	Quaternion	Quaternion::getLog() const
	{
		Quaternion ret;
		ret.w = 0.0;

		if ( std::fabs(w) < scalar(1.0) )
		{
			scalar fAngle ( std::acos(w) );
			scalar fSin = std::sin(fAngle);
			if ( std::fabs(fSin) >= Math::PRESITION_TOLERANCE )
			{
				scalar fCoeff = fAngle/fSin;
#if BLADE_ENABLE_SIMD
				reinterpret_cast<Vector3&>(ret) = reinterpret_cast<const Vector3&>(*this) * fCoeff;
#else
				ret.x = fCoeff*x;
				ret.y = fCoeff*y;
				ret.z = fCoeff*z;
#endif
				return ret;
			}
		}

#if BLADE_ENABLE_SIMD
		reinterpret_cast<Vector3&>(ret) = reinterpret_cast<const Vector3&>(*this);
#else
		ret.x = x;
		ret.y = y;
		ret.z = z;
#endif
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar	Quaternion::inverse()
	{
		scalar fSqauredNorm = this->dotProduct(*this);

		if( Math::Equal(fSqauredNorm, scalar(1.0), Math::HIGH_EPSILON ) )
		{
			*this = this->getConjugate();
		}
		else if ( fSqauredNorm > Math::HIGH_EPSILON )
		{
			scalar fInvNorm = scalar(1.0)/fSqauredNorm;
			*this *= fInvNorm;
			*this = this->getConjugate();
		}
		else
		{
			*this = Quaternion::ZERO;
		}

		return fSqauredNorm;
	}

	//////////////////////////////////////////////////////////////////////////
	Quaternion	Quaternion::getExp() const
	{
		scalar fAngle ( std::sqrt(x*x+y*y+z*z) );
		scalar fSin = std::sin(fAngle);

		Quaternion ret;

		if ( std::fabs(fSin) >= Math::PRESITION_TOLERANCE )
		{
			scalar fCoeff = fSin/(fAngle);
#if BLADE_ENABLE_SIMD
			reinterpret_cast<Vector3&>(ret) = reinterpret_cast<const Vector3&>(*this) * fCoeff;
#else
			ret.x = fCoeff*x;
			ret.y = fCoeff*y;
			ret.z = fCoeff*z;
#endif
		}
		else
		{
#if BLADE_ENABLE_SIMD
			reinterpret_cast<Vector3&>(ret) = reinterpret_cast<const Vector3&>(*this);
#else
			ret.x = x;
			ret.y = y;
			ret.z = z;
#endif
		}

		ret.w = std::cos(fAngle);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Quaternion::slerpWith(const Quaternion& rhs, scalar t, bool shortPath/* = false*/, scalar* length/* = NULL*/)
	{
		
		scalar fCos = this->dotProduct(rhs);
		scalar sign = 1.0f;

		if( shortPath && fCos < 0.0f )
		{
			fCos = -fCos;
			sign = -1.0f;
		}
		else
			shortPath = false;

		if( t + Math::PRESITION_TOLERANCE >= 1.0f )
			*this = rhs;
		else if( t - Math::PRESITION_TOLERANCE <= 0.0f )
		{
		}
		//common situation
		else if (std::fabs(fCos) < 1 - Math::PRESITION_TOLERANCE )
		{
			scalar fSin = std::sqrt(1 - fCos*fCos);
			scalar fAngle = std::atan2(fSin, fCos);
			scalar fInvSin = scalar(1.0) / fSin;
			scalar fCoeff0 = std::sin((scalar(1.0)- t) * fAngle) * fInvSin;
			scalar fCoeff1 = std::sin(t * fAngle) * fInvSin;

			*this *= fCoeff0;
			*this += rhs*(fCoeff1*sign);
		}
		//close or inverse
		else
		{
			*this *= (1.0f - t);
			*this += rhs*(t*sign);
		}

		scalar len = this->normalize();
		if( length != NULL )
			*length = len;

		return shortPath;
	}

	//////////////////////////////////////////////////////////////////////////
	bool	Quaternion::nlerpWith(const Quaternion& rhs, scalar t, bool shortPath/* = false*/, scalar* length/* = NULL*/)
	{
		scalar fCos = this->dotProduct(rhs);
		shortPath = shortPath && (fCos < 0.0f);
		*this *= (1-t);

		if( shortPath )		
			*this -= rhs*t;
		else
			*this += rhs*t;

		scalar len = this->normalize();
		if( length != NULL )
			*length = len;

		return shortPath;
	}

	//////////////////////////////////////////////////////////////////////////
	Quaternion&	Quaternion::squadWith(const Quaternion& qA,const Quaternion& qB,const Quaternion& qC,scalar t, bool shortPath/* = false*/)
	{
		scalar fSlerpT = scalar(2.0)*t*(scalar(1.0)-t);

		this->slerpWith(qC, t, shortPath);
		Quaternion slerpAB = qA.getSlerp(qB,t);

		this->slerpWith(slerpAB, fSlerpT);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		Quaternion::generateRotation(Quaternion& out,const Vector3& dir0,const Vector3& dir1)
	{
		Vector3 ndir0 = dir0.getNormalizedVector();
		Vector3 ndir1 = dir1.getNormalizedVector();

		scalar cos_angle = ndir0.dotProduct(ndir1);

		//same direction
		if( Math::Equal(cos_angle, scalar(1), Math::HIGH_EPSILON ) )
		{
			out = IDENTITY;
			return false;
		}
		//inversed direction
		else if( Math::Equal(cos_angle, scalar(-1), Math::HIGH_EPSILON ) )
		{
			//try to find an axis,because the two dirs are collinear
			Vector3 axis = Vector3::UNIT_X.crossProduct(ndir0);
			if( axis.isZero() )
				axis = Vector3::UNIT_Y.crossProduct(ndir0);
			axis.normalize();
			out.set(axis,Math::PI);
		}
		else
		{
			//Vector3 axis = ndir0.crossProduct(ndir1);
			//out.set(axis,::acos(cos_angle) );

			scalar s = std::sqrt( (1+cos_angle)*2 );
			scalar invs = 1 / s;

			Vector3 c = ndir0.crossProduct(ndir1);
			Vector4 q;
			q.set(c*invs, s*0.5f);
			out.set( q.getData() );
			out.normalize();
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	void		Quaternion::generateLookAtRotation(Quaternion& out, const Vector3& lookat, const Vector3& _upVec/* = Vector3::UNIT_Y*/)
	{
		Vector3	right, up, forward;
		forward = -lookat.getNormalizedVector();
		Vector3 upVec = _upVec;

		if (forward.equal(upVec, Math::LOW_EPSILON))
			upVec = Vector3::NEGATIVE_UNIT_Z;
		else if (forward.equal(-upVec, Math::LOW_EPSILON))
			upVec = Vector3::UNIT_Z;

		right = upVec.crossProduct(forward);
		right.normalize();
		up = forward.crossProduct(right);
		up.normalize();

		out.set(right, up, forward);
	}
	
}//namespace Blade