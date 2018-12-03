/********************************************************************
	created:	2010/04/30
	filename: 	GraphicsUpdater.cc
	author:		Crazii
	purpose:	
	log:		2012/11/23 change name from GraphicsStateQueue to GraphicsUpdater
*********************************************************************/
#include <BladePCH.h>
#include "GraphicsUpdater.h"
#include <interface/ITimeService.h>
#include <interface/IGraphicsSystem.h>
#include <Technique.h>

namespace Blade
{

#define FPS_LIMIT (60.0)

	//tolerance time: 1 frame time
	scalar GraphicsUpdater::TOLERANCE = scalar(1 / FPS_LIMIT);

	//////////////////////////////////////////////////////////////////////////
	GraphicsUpdater::GraphicsUpdater()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	GraphicsUpdater::~GraphicsUpdater()
	{
	}

	/************************************************************************/
	/* IGraphicsUpdater interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	bool GraphicsUpdater::addForUpdateOnce(IGraphicsUpdatable* updatable)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if (updatable != NULL)
		{
			Mask mask = updatable->getStateMask();
			for (index_t i = 0; i < SS_TOTAL_COUNT; ++i)
			{
				if (mask.checkBitAtIndex(i))
				{
					UpdateOnceList& list = mOnceList[i];
					ScopedLock lock(list.mOnceLock);

					if (list.size() == 0)
						list.reserve(32);
					list.push_back(updatable);
				}
			}		
			return true;
		}
		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsUpdater::addForUpdate(IGraphicsUpdatable* updatable)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if( updatable == NULL )
			return false;

		bool success = true;
#if BLADE_DEBUG
		{
			ScopedLock lock(mRegistryLock);
			success = mRegistry.insert(updatable).second;
			assert(success);
			if (!success)
				return false;
		}
#endif
		Mask mask = updatable->getStateMask();
		for(index_t i = 0; i < SS_TOTAL_COUNT; ++i)
		{
			if( mask.checkBitAtIndex(i) )
			{
				ScopedLock lock(mUpdateList[i].mLock);

				bool once_success = mUpdateList[i].insert(updatable).second;
				success = success && once_success;
			}
		}
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsUpdater::removeFromUpdate(IGraphicsUpdatable* updatable)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if( updatable == NULL )
			return false;

		bool success = true;
#if BLADE_DEBUG
		{
			ScopedLock lock(mRegistryLock);
			success = mRegistry.erase(updatable) == 1;
			assert(success);
		}
#endif
		//iterate all sates, in case mask changes?
		for(index_t i = 0; i < SS_TOTAL_COUNT; ++i)
		{
			ScopedLock lock(mUpdateList[i].mLock);

			bool once_success = mUpdateList[i].erase(updatable) == 1;
			success = success && once_success;
		}
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsUpdater::addForIntervalUpate(IGraphicsUpdatable* updatable, scalar interval)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if( updatable == NULL || isnan(interval) )
			return false;

		IntervalUpdateList* plist = NULL;
		{
			ScopedLock lock(mIntervalLock);
			plist = &mIntervalMap[interval];
		}
		IntervalUpdateList& list = *plist;

		bool succes = true;
		Mask mask = updatable->getStateMask();
		for(index_t i = 0; i < SS_TOTAL_COUNT; ++i)
		{
			if (mask.checkBitAtIndex(i))
			{
				ScopedLock lock(list.mList[i].mLock);
				succes = succes && list.mList[i].insert(updatable).second;
			}
		}
		return succes;
	}

	//////////////////////////////////////////////////////////////////////////
	bool GraphicsUpdater::removeFromIntervalUpdate(IGraphicsUpdatable* updatable, scalar interval)
	{
		//only should be called in async run state from other sub systems
		//if within graphics system thread, any state is allowed
		BLADE_TS_VERITY_GRAPHICS_PUBLIC_ACCESS();

		if( updatable == NULL || isnan(interval) )
			return false;

		IntervalUpdateList* plist = NULL;
		{
			ScopedLock lock(mIntervalLock);
			plist = &mIntervalMap[interval];
		}
		IntervalUpdateList& list = *plist;
		
		bool success = true;
		Mask mask = updatable->getStateMask();

		for(index_t i = 0; i < SS_TOTAL_COUNT; ++i)
		{
			if (mask.checkBitAtIndex(i))
			{
				ScopedLock lock(list.mList[i].mLock);

				bool once_success = list.mList[i].erase(updatable) == 1;
				success = success && once_success;
			}
		}
		return success;
	}

	//////////////////////////////////////////////////////////////////////////
	void		GraphicsUpdater::update(SSTATE state)
	{
		//common update
		{
			UpdateList& list = mUpdateList[state];
			ScopedLock lock(list.mLock);
			std::for_each(list.begin(), list.end(),
				std::bind2nd(std::mem_fun(&IGraphicsUpdatable::stateUpdate), state));
		}

		//once only update
		{
			UpdateOnceList& list = mOnceList[state];
			ScopedLock lock(list.mOnceLock);
			for (UpdateOnceList::iterator i = list.begin(); i != list.end(); ++i)
			{
				IGraphicsUpdatable* updatable = *i;
				updatable->stateUpdate(state);
			}
		}
		if (state == SS_FINAL_STATE)
		{
			for (size_t i = 0; i < mOnceList.size(); ++i)
			{
				ScopedLock lock(mOnceList[i].mOnceLock);
				mOnceList[i].clear();
			}
		}

		//interval update
		const TimeSource& time = ITimeService::getSingleton().getTimeSource();
		for(IntervalMap::iterator i = mIntervalMap.begin(); i != mIntervalMap.end(); ++i)
		{
			scalar interval = i->first;
			IntervalUpdateList& intervalList = i->second;

			if( state == SS_FIRST_STATE )
			{
				scalar loopTime = time.getTimeThisLoop();
				if( loopTime >= 0 )
					intervalList.mTimer += loopTime;
			}

			if(intervalList.mTimer >= interval )
			{
				if( state == SS_FINAL_STATE )
					intervalList.mTimer = std::fmod(intervalList.mTimer, interval);

				UpdateList& list = intervalList.mList[state];
				ScopedLock lock(list.mLock);

				std::for_each(list.begin(), list.end(),
					std::bind2nd( std::mem_fun(&IGraphicsUpdatable::stateUpdate), state) );
			}
		}
	}
	
}//namespace Blade