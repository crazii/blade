/********************************************************************
	created:	2010/08/10
	filename: 	IPoolManager.h
	author:		Crazii
	purpose:	this class is deprecated
*********************************************************************/
#ifndef __Blade_IPoolManager_h__
#define __Blade_IPoolManager_h__
#include <interface/IPool.h>
#include <interface/InterfaceSingleton.h>
#include <utility/String.h>

namespace Blade
{
	extern template class BLADE_BASE_API Factory<IPool>;
	typedef Factory<IPool>	PoolFactory;

	#define MEMORY_ALIGN(bytes,align) ( (bytes+align-1)/align*align )

	class IPoolManager : public InterfaceSingleton<IPoolManager>
	{
	public:
		virtual ~IPoolManager()		{}

		/**
		@describe register a pool handle
		@param [in] name the name of the pool to register
		@param [in] h_pool the pool handle to register
		@return the valid id of the pool
		@note DO NOT register a pool in multi-thread concurrent mode, \n
		@you should register pools serially
		*/
		virtual index_t	registerPool(const TString& name,IPool* pool) = 0;

		/**
		@describe get a pool by name
		@param [in] name the name of the pool that you registered with
		@return pointer to pool interface
		@retval valid pool interface if found a matching name
		@retval NULL if the name not found
		@remark you should register the pool first,then you can get the pool, \n
		* by using the registered name
		*/
		virtual IPool*	getPool(const TString& name) const = 0;

		/**
		@describe get a pool by id
		@param [in] id the id of the pool
		@return the pointer to the pool
		@retval valid pool interface if found a matching id
		@retval NULL if the id not found
		@remark the id is the return value of ::registerPool()
		*/
		virtual IPool*	getPool(index_t id) const = 0;
	};

	extern template class BLADE_BASE_API Factory<IPoolManager>;
	typedef Factory<IPoolManager> PoolManagerFactory;
	
}//namespace Blade


#endif //__Blade_IPoolManager_h__