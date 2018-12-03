/********************************************************************
	created:	2009/02/12
	filename: 	FunctorsExt.h
	author:		Crazii
	
	purpose:	STL compatible functors extension,this file is more frequently modified.
*********************************************************************/
#ifndef __Blade_FunctorsExt_h__
#define __Blade_FunctorsExt_h__

#include "Functors.h"
#include "String.h"

namespace Blade
{
	template<size_t tolerance>
	struct FnToleranceLess
	{
		bool operator()(size_t lhs, size_t rhs) const
		{
			return lhs - rhs < tolerance;
		}
	};

	template<const scalar& tolerance>
	struct FnScalarLess
	{
		bool operator()(scalar lhs, scalar rhs) const
		{
			return lhs - rhs < tolerance;
		}
	};

}//namespace Blade

#endif // __Blade_FunctorsExt_h__