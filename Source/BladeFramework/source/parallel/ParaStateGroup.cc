/********************************************************************
	created:	2012/11/24
	filename: 	ParaStateGroup.cc
	author:		Crazii
	
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IParaState.h>
#include <parallel/ParaStateGroup.h>

namespace Blade
{
	struct FnParaStatePriorityLess
	{
		bool operator()(const IParaState* lhs, const IParaState* rhs) const
		{
			return lhs->getPriority() > rhs->getPriority();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	bool		ParaStateGroup::add(IParaState* state, bool checkSync)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		if( state == NULL || state->getGroup() != NULL )
		{
			assert(false);
			return false;
		}

#if BLADE_DEBUG
		if( mGroup.size() > 0 && (*mGroup.begin())->getIdentifier() != state->getIdentifier() )
		{
			assert(false);
			return false;
		}
#endif
		if (mGroup.find(state) != INVALID_INDEX)
			return false;

		mGroup.push_back(state);
		std::sort(mGroup.begin(), mGroup.end(), FnParaStatePriorityLess());

		state->setGroup(this);

		if (checkSync)
		{
			IParaState* highest = *mGroup.begin();
			//new item has highest priority
			if (state == highest)
				this->synchronize();
			else
				state->update(highest);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ParaStateGroup::remove(IParaState* state)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		index_t index = mGroup.find(state);
		if (state == NULL || index == INVALID_INDEX)
		{
			assert(false);
			return false;
		}
		mGroup.erase(index);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//note: this function will be called during parallel running among all tasks
	//so we need to care about the synchronization
	bool		ParaStateGroup::notify(IParaState* source)
	{
		if( source == NULL)
		{
			assert(false);
			return false;
		}

		int32 sourcePriority = (int32)source->getPriority();

		//sync CAS
		if( !Lock::compareOPAndSwap32( (int32*)&mSourcePriority, (int32)sourcePriority, (int32)sourcePriority, std::less_equal<int32>() ) )
			return false;
		Lock::memoryBarrier();

		//note: the following loop may be executed by multiple tasks in parallel
		bool hasSource = false;
		const void* sourceData = source->getStateData();
		ParaStateGroupImpl::const_iterator i = std::lower_bound(mGroup.begin(), mGroup.end(), source, FnParaStatePriorityLess());
		for(; i != mGroup.end(); ++i)
		{
			//higher priority set by other tasks, early quit to avoid useless notification
			if (mSourcePriority != sourcePriority)
			{
				assert(mSourcePriority > sourcePriority);
				return false;
			}

			IParaState* state = *i;
			if (state == NULL)
			{
				assert(false);
				continue;
			}
			else if( state == source )
				hasSource = true;
			else if( state->getPriority() <= (int16)sourcePriority )
				state->onNotified(sourceData, (int16)sourcePriority);
		}

		if( !hasSource )
			BLADE_EXCEPT(EXC_NEXIST,BTString("error - state is not mine."));		
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool		ParaStateGroup::synchronize()
	{
		if( mGroup.size() <= 1 )
			return false;

		//we need a sync lock for synchronize
		ScopedLock lock(mSyncLock);

		//get the highest priority item
		IParaState* source= *mGroup.begin();

		ParaStateGroupImpl::const_iterator i = mGroup.begin();
		for(++i; i != mGroup.end(); ++i)
		{
			IParaState* state = *i;
			state->update(source);
		}
		source->update();

		return true;
	}
	
}//namespace Blade