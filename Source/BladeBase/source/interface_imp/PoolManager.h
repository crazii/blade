/********************************************************************
	created:	2009/02/09
	filename: 	PoolManager.h
	author:		Crazii
	
	purpose:	memory pool manager,which hold some global pools.
	not:		the pool manager is not used by the framework any more now, 
				but the user/client can still register their pools into this manager if needed.
*********************************************************************/
#ifndef __Blade_PoolManager_h__
#define __Blade_PoolManager_h__
#include <BladeBase.h>
#include <interface/IPoolManager.h>
#include <interface/IPool.h>
#include <memory/PoolAllocator.h>
#include <utility/String.h>
#include <utility/BladeContainer.h>
#include <Singleton.h>
#include <Factory.h>
#include <Handle.h>

namespace Blade
{
	class PoolManager : public IPoolManager, public Singleton<PoolManager>
	{
	public:
		using Singleton<PoolManager>::getSingleton;
		using Singleton<PoolManager>::getSingletonPtr;

	public:
		PoolManager();
		~PoolManager();

		/*
		@describe register a pool handle
		@param [in] name the name of the pool to register
		@param [in] h_pool the pool handle to register
		@return the valid id of the pool
		@note DO NOT register a pool in multi-thread concurrent mode, \n
		@you should register pools serially
		*/
		index_t	registerPool(const TString& name,IPool* pool);

		/*
		@describe get a pool by name
		@param [in] name the name of the pool that you registered with
		@return pointer to pool interface
		@retval valid pool interface if found a matching name
		@retval NULL if the name not found
		@remark you should register the pool first,then you can get the pool, \n
		* by using the registered name
		*/
		IPool*	getPool(const TString& name) const;

		/*
		@describe get a pool by id
		@param [in] id the id of the pool
		@return the pointer to the pool
		@retval valid pool interface if found a matching id
		@retval NULL if the id not found
		@remark the id is the return value of ::registerPool()
		*/
		IPool*	getPool(index_t id) const;

	protected:

		typedef TStringMap<SPOOL>	PoolMap;
		typedef Vector<IPool*>		PoolList;
		
		PoolMap		mPoolMap;
		PoolList	mPoolList;

		Lock		mLock;
		Lock		mDoubleCheckLock;
	};//class PoolManager

}//namespace Blade

#endif // __Blade_PoolManager_h__