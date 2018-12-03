/********************************************************************
	created:	2014/08/15
	filename: 	DualQuaternion.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/DualQuaternion.h>

namespace Blade
{
	//note: don't use constants to avoid static object init order issue.
	//const DualQuaternion DualQuaternion::IDENTITY = DualQuaternion( Quaternion::IDENTITY, POINT3::ZERO );
	//const DualQuaternion DualQuaternion::ZERO = DualQuaternion( Quaternion::ZERO, POINT3::ZERO );
	const DualQuaternion DualQuaternion::IDENTITY = DualQuaternion( Quaternion(0,0,0,1), POINT3(0,0,0) );
	const DualQuaternion DualQuaternion::ZERO = DualQuaternion( Quaternion(0,0,0,0), POINT3(0,0,0) );

	//////////////////////////////////////////////////////////////////////////
	void				DualQuaternion::set(const Matrix44& matrix)
	{
		//TODO: extract rotation without scale?
		Matrix33 rotation = static_cast<Matrix33>(matrix);
		this->set(rotation,  reinterpret_cast<const Vector3&>( matrix.getTranslation()) );
	}

	void				DualQuaternion::set(const Matrix33& rotation, const Vector3& translation)
	{
		Quaternion q = static_cast<Quaternion>( rotation );
		q.normalize();
		this->set(q, translation);
	}


	//////////////////////////////////////////////////////////////////////////
	void				DualQuaternion::toMatrix(Matrix44& outMatrix) const
	{
#if 0
		// As the dual quaternions may be the results from a
		// linear blending we have to normalize it :
		this->normalize();
#else
		assert( Math::Equal<scalar>(real.getLength(), 1.0f, Math::LOW_EPSILON) );
#endif
		Matrix33 rotation;
		real.toRotationMatrix(rotation);

		outMatrix = rotation;
		outMatrix[3] = this->getTranslation();
	}

	//////////////////////////////////////////////////////////////////////////
	DualQuaternion&		DualQuaternion::sclerpWith(const DualQuaternion& _rhs, scalar t, bool shortPath/* = false*/)
	{
		// shortest path 
		DualQuaternion rhs = _rhs;

		scalar dot = real.dotProduct(rhs.real);
		if ( shortPath && dot < 0 )
		{
			dot = -dot;
			rhs *= -1.0f;
		}

		if( ::fabs(dot) + Math::LOW_EPSILON >= 1 )
		{
			//same or inversed direction,
			//use nlerp
			*this *= (1-t);
			*this += rhs*t;
			this->normalize();
			return *this;
		}

		assert( Math::Equal<scalar>(real.getLength(), 1.0f, Math::LOW_EPSILON) );
		assert( Math::Equal<scalar>(rhs.real.getLength(), 1.0f, Math::LOW_EPSILON) );

		// ScLERP = qa(qa^-1 qb)^t
		DualQuaternion diff = this->getConjugate().multiply(rhs);

		Vector3 vr = Vector3(diff.real.x, diff.real.y, diff.real.z);
		Vector3 vd = Vector3(diff.dual.x, diff.dual.y, diff.dual.z);
		scalar invr = 1/vr.getLength();

		//screw parameters
		scalar angle = (scalar)::acos( Math::Clamp(diff.real.w,-1.f,1.f) );
		scalar pitch = -diff.dual.w * invr;
		Vector3 direction = vr * invr;
		Vector3 moment = (vd - direction*pitch*diff.real.w*0.5f) * invr;

		//exponential power 
		angle *= t;
		pitch *= t;

		//convert back to dual-quaternion 
		scalar sinAngle = ::sin(angle);
		scalar cosAngle = ::cos(angle);

		moment = sinAngle*moment + (pitch*0.5f*cosAngle)*direction;
		direction *= sinAngle;

		Quaternion _Real = Quaternion( direction.x, direction.y, direction.z, cosAngle ); 
		Quaternion _Dual = Quaternion( moment.x, moment.y, moment.z,  -pitch*0.5f*sinAngle );

		DualQuaternion dq(_Real, _Dual);
		dq.normalize();

		// Complete the multiplication and return the interpolated value 
		*this = this->multiply(dq);
		return *this;
	}

}//namespace Blade
