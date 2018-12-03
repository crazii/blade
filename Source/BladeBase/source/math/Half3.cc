/********************************************************************
	created:	2015/11/15
	filename: 	Half3.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <math/Half3.h>

namespace Blade
{

	const Half3	Half3::ZERO = Half3(0, 0, 0);
	const Half3	Half3::UNIT_X = Half3(1, 0, 0);
	const Half3	Half3::UNIT_Y = Half3(0, 1, 0);
	const Half3	Half3::UNIT_Z = Half3(0, 0, 1);
	const Half3	Half3::UNIT_ALL = Half3(1, 1, 1);
	const Half3	Half3::NEGATIVE_UNIT_X = Half3(-1, 0, 0);
	const Half3	Half3::NEGATIVE_UNIT_Y = Half3(0, -1, 0);
	const Half3	Half3::NEGATIVE_UNIT_Z = Half3(0, 0, -1);
	const Half3	Half3::NEGATIVE_UNIT_ALL = Half3(-1, -1, -1);

	
}//namespace Blade