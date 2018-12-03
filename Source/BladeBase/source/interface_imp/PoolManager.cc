/********************************************************************
	created:	2009/02/09
	filename: 	PoolManager.cc
	author:		Crazii
	
	purpose:	pool manager implementation
*********************************************************************/
#include <BladePCH.h>
#include "PoolManager.h"

namespace Blade
{
	template class Factory<IPool>;
	template class Factory<IPoolManager>;

	//////////////////////////////////////////////////////////////////////////
	//ctor & dector
	PoolManager::PoolManager()
	{
		mLock.lock();
		if( mPoolList.size() == 0)
		{
			mPoolList.reserve(64);
			mPoolList.push_back( Memory::getStaticPool() );
			mPoolList.push_back( Memory::getTemporaryPool() );
		}
		mLock.unlock();
	}

	//////////////////////////////////////////////////////////////////////////
	PoolManager::~PoolManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	IPool* PoolManager::getPool(const TString& name) const
	{
		PoolMap::const_iterator iter = mPoolMap.find(name);

		//if we cannot find the IPool,then return NULL
		if( iter == mPoolMap.end() )
		{
			return NULL;
		}
		else
		{
			return iter->second;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	IPool*	PoolManager::getPool(index_t id) const
	{
		if( id > mPoolList.size() )
			return NULL;
		return mPoolList[id];
	}

	//////////////////////////////////////////////////////////////////////////
	index_t PoolManager::registerPool(const TString& name,IPool* pool)
	{
		SPOOL& spool = mPoolMap[name];

		//name conflict
		assert( spool == NULL );
		mLock.lock();

		spool = pool;
		mPoolList.push_back(pool);

		mLock.unlock();
		return mPoolList.size() - 1;
	}

}//namespace Blade