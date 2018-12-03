/********************************************************************
	created:	2009/02/13
	filename: 	allocator.cc
	author:		Crazii
	
	purpose:	some allocator need template-lock(not specialized template lock),
				that means,all specialized types of this template share one lock
*********************************************************************/
#include <BladePCH.h>

namespace Blade
{
}//namespace Blade