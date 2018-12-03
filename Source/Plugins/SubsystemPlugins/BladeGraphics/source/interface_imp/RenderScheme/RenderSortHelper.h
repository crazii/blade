/********************************************************************
	created:	2015/12/18
	filename: 	RenderSortHelper.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderSortHelper_h__
#define __Blade_RenderSortHelper_h__
#include <interface/public/IRenderable.h>
#include <interface/public/IRenderSorter.h>

namespace Blade
{

	//////////////////////////////////////////////////////////////////////////
	class RenderSortHelper
	{
	public:
		/** @brief  */
		static bool setSorter(IRenderSorter* sorter)
		{
			if(sorter == NULL || msSorter != NULL)
			{
				assert(false);
				return false;
			}

			//since sort helper is accessed only by one thread (rendering thread)
			//we don't need the lock here actually.
			msLock.lock();
			msSorter = sorter;
			return true;
		}

		/** @brief  */
		static void clearSorter()
		{
			msSorter = NULL;
			msLock.unlock();
		}

		/** @brief  */
		static int compare(const void* lhs, const void* rhs)
		{
			const RenderOperation& l = *(const RenderOperation*)lhs;
			const RenderOperation& r = *(const RenderOperation*)rhs;
			return msSorter->compare(l, r);
		}
	public:
		static IRenderSorter*	msSorter;
		static StaticLock		msLock;
	};//class RenderSortHelper
	
}//namespace Blade

#endif // __Blade_RenderSortHelper_h__
