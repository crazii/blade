/********************************************************************
	created:	2010/04/20
	filename: 	Vector2.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Vector2.h>

namespace Blade
{

	const Vector2	Vector2::ZERO = Vector2(0.0f,0.0f);
	const Vector2	Vector2::UNIT_X = Vector2(1.0f,0.0f);
	const Vector2	Vector2::UNIT_Y = Vector2(0.0f,1.0f);
	const Vector2	Vector2::UNIT_ALL = Vector2(1.0f,1.0f);
	const Vector2	Vector2::NEGATIVE_UNIT_X = Vector2(-1.0f,0.0f);
	const Vector2	Vector2::NEGATIVE_UNIT_Y = Vector2(0.0f,-1.0f);
	const Vector2	Vector2::NEGATIVE_UNIT_ALL = Vector2(-1.0f,-1.0f);

}//namespace Blade