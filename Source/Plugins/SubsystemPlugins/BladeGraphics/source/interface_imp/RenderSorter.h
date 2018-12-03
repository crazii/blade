/********************************************************************
	created:	2015/12/15
	filename: 	RenderSorter.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_RenderSorter_h__
#define __Blade_RenderSorter_h__
#include <interface/public/IRenderSorter.h>
#include <utility/BladeContainer.h>

namespace Blade
{

	class Front2BackRenderSorter : public IRenderSorter, public Allocatable
	{
	public:
		virtual ~Front2BackRenderSorter() {}

		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs);
	};

	class Back2FrontRenderSorter : public IRenderSorter, public Allocatable
	{
	public:
		virtual ~Back2FrontRenderSorter() {}

		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs);
	};

	class GeometrySorter : public IRenderSorter, public Allocatable
	{
	public:
		virtual ~GeometrySorter() {}
		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs);
	};

	class TextureRenderSorter : public IRenderSorter, public Allocatable
	{
	public:
		virtual ~TextureRenderSorter() {}

		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs);
	};

	class CombinedRenderSorter : public IRenderSorter, public Allocatable
	{
	public:
		virtual ~CombinedRenderSorter()
		{
			for(size_t i = 0; i < mSorters.size(); ++i)
				BLADE_DELETE mSorters[i];
		}

		/** @brief  */
		virtual int	compare(const RenderOperation& lhs, const RenderOperation& rhs);

		/** @brief  */
		inline void reserve(size_t size)		  {mSorters.reserve(size);}

		/** @brief  */
		inline void push_back(IRenderSorter* sorter) {mSorters.push_back(sorter);}

		/** @brief  */
		inline size_t size() const					{return mSorters.size();}

		typedef Vector<IRenderSorter*> SorterList;
		SorterList mSorters;
	};

	
}//namespace Blade

#endif // __Blade_RenderSorter_h__