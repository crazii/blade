/********************************************************************
	created:	2016/07/31
	filename: 	Vector3i.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Vector3i.h>

namespace Blade
{
	const struct Vector3i	Vector3i::ZERO = Vector3i(0,0,0);
	const struct Vector3i	Vector3i::UNIT_X = Vector3i(1,0,0);
	const struct Vector3i	Vector3i::UNIT_Y = Vector3i(0,1,0);
	const struct Vector3i	Vector3i::UNIT_Z = Vector3i(0,0,1);
	const struct Vector3i	Vector3i::UINT_ALL = Vector3i(1,1,1);
	const struct Vector3i	Vector3i::NEGATIVE_UNIT_X = Vector3i(-1,0,0);
	const struct Vector3i	Vector3i::NEGATIVE_UNIT_Y = Vector3i(0,-1,0);
	const struct Vector3i	Vector3i::NEGATIVE_UNIT_Z = Vector3i(0,0,-1);
	const struct Vector3i	Vector3i::NEGATIVE_UNIT_ALL = Vector3i(-1,-1,-1);	
}//namespace Blade