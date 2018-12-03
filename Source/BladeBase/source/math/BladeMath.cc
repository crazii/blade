/********************************************************************
	created:	2010/04/25
	filename: 	BladeMath.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/BladeMath.h>
#include "3rdparty/PoissonGenerator.h"

namespace Blade
{
	namespace Math
	{
		const scalar PI = scalar(3.141592653589793235462643383279);
		const scalar QUARTER_PI = scalar(3.141592653589793235462643383279*0.25);
		const scalar HALF_PI = scalar(3.141592653589793235462643383279*0.5);
		const scalar TWICE_PI = scalar(3.141592653589793235462643383279*2.0);

		const scalar LOW_EPSILON = scalar(1e-3);	//low precision 1e-3
		const scalar HIGH_EPSILON = scalar(1e-6);	//high precision 1e-6
		const scalar PRESITION_TOLERANCE = LOW_EPSILON;

		/*
		@describe
		@param
		@return
		*/
		size_t CalcMaxMipCount(size_t size)
		{
			size_t mipLevel = (size_t)( std::log( (scalar)size ) / std::log(2.0f) )+1;	//minimal 1x1
			return mipLevel;
		}

		/*
		@describe
		@param
		@return
		*/
		size_t	CalcMaxMipCount(size_t width, size_t height)
		{
			size_t size = std::max(width,height);
			return CalcMaxMipCount(size);
		}

		/**
		  @describe 
		  @param
		  @return
		*/
		bool PoissonDistribution(scalar* outPoints, size_t count, scalar scale/* = 1.0f*/, scalar originX/* = 0*/, scalar originY/* = 0*/, bool circle/* = true*/)
		{
			if (outPoints == NULL || count == 0)
				return false;
			PoissonGenerator::DefaultPRNG prng;

			Vector2 origin = Vector2(originX, originY);
			Vector2* outPoints2 = reinterpret_cast<Vector2*>(outPoints);
			if(circle)
				PoissonGenerator::GeneratePoissonPoints2<true>(outPoints2, count, prng, origin, scale);
			else
				PoissonGenerator::GeneratePoissonPoints2<false>(outPoints2, count, prng, origin, scale);
			return true;
		}

	}//namespace Math
	
}//namespace Blade