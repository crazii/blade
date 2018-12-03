/********************************************************************
	created:	2010/04/20
	filename: 	Matrix33.h
	author:		Crazii
	
	purpose:	row major matrix3x3
				when applying transform to a (row) vector,
				use vector*matrix33
*********************************************************************/
#ifndef __Blade_Matrix33_h__
#define __Blade_Matrix33_h__
#include <math/Vector3.h>
#include <math/BladeMath.h>

namespace Blade
{
	class Matrix33
	{
	public:
		inline Matrix33()
		{

		}

		inline Matrix33(scalar mm[3][3])
		{
			(*this)[0].setData(mm[0]);
			(*this)[1].setData(mm[1]);
			(*this)[2].setData(mm[2]);
		}

		inline Matrix33(const Matrix33& m)
		{
			*this = m;
		}

		inline Matrix33(scalar i00,scalar i01,scalar i02,
					scalar i10,scalar i11,scalar i12,
					scalar i20,scalar i21,scalar i22)
		{
			mV[0][0] = i00;
			mV[0][1] = i01;
			mV[0][2] = i02;
			mV[1][0] = i10;
			mV[1][1] = i11;
			mV[1][2] = i12;
			mV[2][0] = i20;
			mV[2][1] = i21;
			mV[2][2] = i22;	
		}

		inline Matrix33(const Vector3& row0,const Vector3& row1,const Vector3& row2)
		{
			(*this)[0] = row0;
			(*this)[1] = row1;
			(*this)[2] = row2;
		}

		/** @brief  */
		inline const Vector3&	operator[](size_t row_index) const
		{
			if( row_index < 3 )
				return reinterpret_cast<const Vector3&>(mV[row_index]);
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("row index out of range."));

		}

		/** @brief  */
		inline Vector3&		operator[](size_t row_index)
		{
			if( row_index < 3 )
				return reinterpret_cast<Vector3&>(mV[row_index]);
			else
				BLADE_EXCEPT(EXC_OUT_OF_RANGE,BTString("row index out of range."));
		}

		/** @brief  */
		inline Matrix33&		operator=(const Matrix33& rhs)
		{
			(*this)[0] = rhs[0];
			(*this)[1] = rhs[1];
			(*this)[2] = rhs[2];
			return *this;
		}

		/** @brief  */
		inline bool			operator==(const Matrix33& rhs) const
		{
			return (*this)[0] == rhs[0] && (*this)[1] == rhs[1] && (*this)[2] == rhs[2];
		}

		/** @brief  */
		inline bool			operator!=(const Matrix33& rhs) const
		{
			return (*this)[0] != rhs[0] || (*this)[1] != rhs[1] || (*this)[2] != rhs[2];
		}

		/** @brief  */
		inline Matrix33		operator*(const Matrix33& rhs) const
		{
			return this->concatenate(rhs);
		}

		/** @brief  */
		inline void			setScale(const Vector3& vscale)
		{
			(*this)[0] *= vscale[0];
			(*this)[1] *= vscale[1];
			(*this)[2] *= vscale[2];
		}

		/** @brief  */
		inline Vector3	getScale() const
		{
			//SRT de-composition
			return Vector3((*this)[0].getLength(), (*this)[1].getLength(), (*this)[2].getLength());
		}

		/** @brief  */
		inline Matrix33		operator-() const
		{
			Matrix33 ret;
			ret[0] = -(*this)[0];
			ret[1] = -(*this)[1];
			ret[2] = -(*this)[2];
			return ret;
		}

		/** @brief  */
		inline void			transpose()
		{
			std::swap( mV[0][1], mV[1][0] );
			std::swap( mV[0][2], mV[2][0] );
			std::swap( mV[1][2], mV[2][1] );
		}

		/** @brief  */
		inline Matrix33		getTranspose() const
		{
			Matrix33 m33 = *this;
			m33.transpose();
			return m33;
		}

		/** @brief  */
		BLADE_BASE_API bool			inverse(scalar tolerance = Math::LOW_EPSILON);

		/** @brief  */
		inline Matrix33				getInverse(scalar tolerance = Math::LOW_EPSILON) const
		{
			Matrix33 m = *this;
			m.inverse(tolerance);
			return m;
		}

		/** @brief  */
		BLADE_BASE_API scalar		getDeterminant() const;

		/** @brief  */
		BLADE_BASE_API Matrix33		concatenate(const Matrix33 &rhs) const;

		//add class keyword to avoid weird symbol conflicts
		BLADE_BASE_API static const class Matrix33	ZERO;
		BLADE_BASE_API static const class Matrix33	IDENTITY;

	protected:
		union
		{
			scalar	mV[3][3];
			scalar	mM[9];
		};
	};//class Matrix33
	
}//namespace Blade



#endif //__Blade_Matrix33_h__