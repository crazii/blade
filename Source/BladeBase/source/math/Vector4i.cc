/********************************************************************
	created:	2017/12/11
	filename: 	Vector4i.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Vector4i.h>

namespace Blade
{
	const struct Vector4i	Vector4i::ZERO = Vector4i(0, 0, 0, 0);
	const struct Vector4i	Vector4i::UNIT_X = Vector4i(1, 0, 0, 0);
	const struct Vector4i	Vector4i::UNIT_Y = Vector4i(0, 1, 0, 0);
	const struct Vector4i	Vector4i::UNIT_Z = Vector4i(0, 0, 1, 0);
	const struct Vector4i	Vector4i::UINT_ALL = Vector4i(1, 1, 1, 1);
	const struct Vector4i	Vector4i::NEGATIVE_UNIT_X = Vector4i(-1, 0, 0, 0);
	const struct Vector4i	Vector4i::NEGATIVE_UNIT_Y = Vector4i(0, -1, 0, 0);
	const struct Vector4i	Vector4i::NEGATIVE_UNIT_Z = Vector4i(0, 0, -1, 0);
	const struct Vector4i	Vector4i::NEGATIVE_UNIT_ALL = Vector4i(-1, -1, -1, -1);
}//namespace Blade