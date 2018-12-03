/********************************************************************
	created:	2012/03/24
	filename: 	MaterialLOD.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialLOD_h__
#define __Blade_MaterialLOD_h__
#include <BladeGraphics.h>
#include <utility/FixedArray.h>

namespace Blade
{
	typedef struct SMaterialLOD
	{
		typedef uint8 LOD_INDEX;			//material LOD: level 0 is the best(highest) level
		static const LOD_INDEX MAX_LOD = 8;
		static const LOD_INDEX INVALID_LOD = (LOD_INDEX)-1;

		scalar		mSquaredDistanceStart;	//DistanceStart*DistanceStart
		scalar		mSquaredDistanceEnd;	//DistanceEnd*DistanceEnd
		uint8		mTargetGroupIndex;
		LOD_INDEX	mLODIndex;
	}MATERIALLOD;

	extern template class BLADE_GRAPHICS_API FixedVector<MATERIALLOD,MATERIALLOD::MAX_LOD>;
	typedef FixedVector<MATERIALLOD,MATERIALLOD::MAX_LOD> LODSetting;

}//namespace Blade


#endif //__Blade_MaterialLOD_h__