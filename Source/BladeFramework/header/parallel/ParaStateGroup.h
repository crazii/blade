/********************************************************************
	created:	2012/11/24
	filename: 	ParaStateGroup.h
	author:		Crazii
	
	purpose:	parallel state group for one entity
	note:		different between ParaStateSet & ParaStateGroup:
				ParaStateSet collect all states for one element,
				ParaStateGroup collect related state from different elements of one entity
*********************************************************************/
#ifndef __Blade_ParaStateGroup_h__
#define __Blade_ParaStateGroup_h__
#include <BladeFramework.h>
#include <utility/TList.h>
#include "ParallelPriority.h"

namespace Blade
{
	class IParaState;

	class ParaStateGroup : public Allocatable
	{
	public:
		ParaStateGroup() :mSourcePriority(PP_LOWEST) { mGroup.reserve(4); }
		~ParaStateGroup() {}

		/** @brief  */
		BLADE_FRAMEWORK_API bool		add(IParaState* state, bool checkSync);

		/** @brief  */
		BLADE_FRAMEWORK_API bool		remove(IParaState* state);

		/** @brief  */
		BLADE_FRAMEWORK_API bool		notify(IParaState* source);

		/** @brief  call IParaState::update directly, skip enqueue */
		BLADE_FRAMEWORK_API bool		synchronize();

		/** @brief  */
		inline void		resetSourcePriority()
		{
			mSourcePriority = PP_LOWEST;
		}
	protected:
		int32	mSourcePriority;
		Lock	mSyncLock;

	private:
		typedef TList<IParaState*> ParaStateGroupImpl;
		ParaStateGroupImpl	mGroup;
	};//class ParaStateGroup

}//namespace Blade

#endif//__Blade_ParaStateGroup_h__