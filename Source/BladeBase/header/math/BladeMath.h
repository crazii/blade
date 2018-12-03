/********************************************************************
	created:	2010/05/09
	filename: 	BladeMath.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_BladeMath_h__
#define __Blade_BladeMath_h__
#include <BladeBase.h>
#include <cmath>

namespace Blade
{
	//position vs. plane side
	typedef enum EPositionSide
	{
		PS_NEGATIVE	= -1,
		PS_ON		= 0,	//right in plane
		PS_POSITIVE	= 1,

		PS_BOTH		= 2,	//for infinite volume vs. plane
		PS_NONE		= 3,
	}POS_SIDE;

	//position vs. volume
	typedef enum EPositionVolume
	{
		PV_OUTSIDE = 0,	//not intersected
		PV_INSIDE,		//fully inside volume
		PV_INTERSECTED,	//partial overlap
	}POS_VOL;

	typedef enum EBoxType
	{
		BT_FINITE,
		BT_INFINITE,
		BT_NULL,
	}BOX_TYPE;

	namespace Math
	{
		extern const scalar BLADE_BASE_API PI;
		extern const scalar BLADE_BASE_API QUARTER_PI;
		extern const scalar BLADE_BASE_API HALF_PI;
		extern const scalar BLADE_BASE_API TWICE_PI;

		///extra EPSILONs
		extern const scalar BLADE_BASE_API LOW_EPSILON;		//low precision 1e-3
		extern const scalar BLADE_BASE_API HIGH_EPSILON;	//high precision 1e-6
		extern const scalar BLADE_BASE_API PRESITION_TOLERANCE;

		/************************************************************************/
		/* common functions                                                                     */
		/************************************************************************/
		/**
		@describe 
		@param 
		@return 
		*/
		inline scalar Degree2Radian(scalar degree)
		{
			return degree*PI/180;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		inline scalar Radian2Degree(scalar radian)
		{
			return radian*180/PI;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		template<typename T>
		static inline T Min(const T& a,const T& b){return a<b?a:b;}

		template<typename T>
		static inline T Max(const T& a,const T& b){return a>b?a:b;}

		template<typename T,T _a,T _b>
		struct SCTMinMax
		{
			enum
			{
				MIN = _a<_b?_a:_b,
				MAX = _a>_b?_a:_b,
			};
		};


		/**
		@describe 
		@param 
		@return 
		*/
		template<typename T>
		static inline T		Sign(const T& tV)	{ return tV>0?T(1):(tV<0?T(-1):T() ); }

		/**
		@describe 
		@param 
		@return 
		*/
		template<typename T>
		static inline int	iSign(const T& tV)	{ return tV>0?1:(tV<0?-1:0);}

		/**
		@describe 
		@param 
		@return 
		*/
		template<typename T>
		static inline T		Abs(const T& tV)		{ return tV>=0?tV:-tV; }
		template<>
		inline float Abs<float>(const float& tV)	{ return std::fabs(tV); }
		template<>
		inline double Abs<double>(const double& tV)	{ return std::fabs(tV); }

		/**
		@describe 
		@param 
		@return 
		*/
		template<typename T>
		inline bool			Equal(const T& lhs,const T&rhs,const T& tolerance)
		{
			return Math::Abs(lhs-rhs) <= tolerance;
		}

		inline scalar		Cotan(scalar radian)
		{
			return std::tan( HALF_PI-radian);
		}

		template<typename T>
		inline T&			Clamp(T&out, const T& tmin, const T& tmax)
		{
			if( out < tmin )
				out = tmin;
			else if( out > tmax )
				out = tmax;
			return out;
		}


		template<typename T>
		inline const T&		Clamp(const T&v, const T& tmin, const T& tmax)
		{
			if( v < tmin )
				return tmin;
			else if( v > tmax )
				return tmax;
			else
				return v;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		template<typename T>
		static inline int			Compare(const T& lhs,const T&rhs,const T& tolerance)
		{
			const T result = lhs - rhs;

			if( result < -tolerance )
				return -1;
			else if( result > tolerance )
				return 1;
			else
				return 0;
		}

		/**
		@describe random number ranges [min, max]
		@param
		@return
		*/
		template<typename T>
		static inline T				Random(const T min, const T max)
		{
			return (T)(min + std::rand() % (max - min));
		}
		template<>
		inline fp32			Random(const fp32 min, const fp32 max)
		{
			static const fp32 REC_RAND_MAX = 1.0f / (fp32)RAND_MAX;
			return min + (fp32)std::rand() * REC_RAND_MAX * (max - min);
		}
		template<>
		inline fp64			Random(const fp64 min, const fp64 max)
		{
			static const fp64 REC_RAND_MAX = 1.0 / (fp64)RAND_MAX;
			return min + (fp64)std::rand() * REC_RAND_MAX * (max - min);
		}

		/**
		@describe
		@param
		@return
		*/
		static inline bool		IsPowerOf2(uint val)
		{
			return (val & (val-1)) == 0;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static inline uint		GetHigherLog2(uint val)
		{
			uint r = 0;
			while (val >>= 1)
			{
				r++;
			}
			if( IsPowerOf2(val) )
				return r;
			return ++r;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static inline uint		GetLowerLog2(uint val)
		{
			uint r = 0;
			while (val >>= 1)
			{
				r++;
			}
			return r;
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static inline uint		GetHigherPowerOf2(uint val)
		{
			if( IsPowerOf2(val) )
				return val;

			int r = 0;
			while (val >>= 1)
			{
				r++;
			}
			return 1u<<(r+1);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		static inline uint		GetLowerPowerOf2(uint val)
		{
			if( IsPowerOf2(val) )
				return val;

			int r = 0;
			while (val >>= 1)
			{
				r++;
			}
			return 1u<<r;
		}

		/**
		@describe 
		@param
		@return
		*/
		static inline scalar GaussianDistribution(scalar x, scalar offset, scalar sigma)
		{
			scalar nom = std::exp(-(x - offset) * (x - offset)  / (2 * sigma*sigma));
			scalar denom = sigma * std::sqrt(2 * Math::PI);
			return nom / denom;
		}

		/**
		  @describe 2D Poisson distribution on unit disk [(0,0),(1,1)], centered at (0.5,0.5)
		  @param [in][out] outPoints: output points in pairs of floats (x,y)
		  @param [in] count: count of points. total floats is count*2
		  @param [in] circle: true to use circle disk. false to use box
		  @param [in] scale: scale the original unit distribution
		  @param [in] originX, originY: offset/origin of the disk, default is (0,0). i.e. if you want the disk centered at (0,0), use (-0.5,-0.5) as origin\
		  @return
		*/
		bool BLADE_BASE_API PoissonDistribution(scalar* outPoints, size_t count, scalar scale = 1.0f, scalar originX = 0, scalar originY = 0, bool circle = true);

		/**
		@describe
		@param
		@return
		*/
		size_t BLADE_BASE_API CalcMaxMipCount(size_t size);

		/**
		@describe
		@param
		@return
		*/
		size_t	BLADE_BASE_API CalcMaxMipCount(size_t width,size_t height);

	}//namespace Math

	class Box3;
	typedef Box3 AABB;
	
}//namespace Blade


#endif //__Blade_BladeMath_h__