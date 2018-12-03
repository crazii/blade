/********************************************************************
	created:	2015/12/18
	filename: 	RenderSortHelper.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "RenderSortHelper.h"

namespace Blade
{

	IRenderSorter*	RenderSortHelper::msSorter = NULL;
	StaticLock		RenderSortHelper::msLock;

}//namespace Blade
