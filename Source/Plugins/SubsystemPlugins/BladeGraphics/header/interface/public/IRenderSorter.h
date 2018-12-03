/********************************************************************
	created:	2015/12/15
	filename: 	IRenderSorter.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_IRenderSorter_h__
#define __Blade_IRenderSorter_h__
#include <BladeGraphics.h>
#include <Factory.h>

namespace Blade
{
	struct RenderOperation;

	///sorter interface for qsort
	class BLADE_GRAPHICS_API IRenderSorter
	{
	public:
		virtual ~IRenderSorter() {}

		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs) = 0;
	};//class IRenderSorter

	extern template class BLADE_GRAPHICS_API Factory<IRenderSorter>;
	
}//namespace Blade

#endif // __Blade_IRenderSorter_h__
