/********************************************************************
	created:	2012/12/23
	filename: 	StagePool.h
	author:		Crazii
	purpose:	the stage pool provide stage-life-time memory pool,
	currently this pool can only be only one instance, and if there's more stages,
	then the stages will share this stage pool.
*********************************************************************/
#ifndef __Blade_StagePool_h__
#define __Blade_StagePool_h__
#include <memory/PoolAllocator.h>
#include <BladeFramework.h>

namespace Blade
{

	class BLADE_FRAMEWORK_API StagePool
	{
	private:
		static	SPOOL		msPool;
		static RefCount		msCount;

	protected:

		///note: following two functions will be called automatically by framework.

		/** @brief init the pool, this will be called each time a new stage is created */
		static void initialize(const TString& type = BTString("Resource") );

		/** @brief destroy the pool, this will be called when one stage destroyed */
		static void shutdown();

	public:

		/** @brief  */
		inline static IPool*	getPool()
		{
			return msPool;
		}

		typedef PoolBinding<msPool>	Binding;
		typedef Binding::PooledAllocatable Allocatable;

		template<typename T>
		class Vector : public Binding::PooledVector<T>		{};

		template<typename T>
		class Queue : public Binding::PooledQueue<T>		{};

		template<typename T>
		class List : public Binding::PooledList<T>			{};

		template<typename T,typename C = std::less<T> >
		class Set : public Binding::PooledSet<T>			{};

		template<typename K,typename V,typename C = std::less<K> >
		class Map : public Binding::PooledMap<K,V,C>		{};
	};//class StagePool


#define BLADE_STAGE_ALLOC(_bytes)	BLADE_POOL_ALLOCATE( StagePool::getPool(), _bytes )
#define BLADE_STAGE_ALLOCT(_TYPE,_count) (_TYPE*)BLADE_STAGE_ALLOC( sizeof(_TYPE)*_count )
#define BLADE_STAGE_FREE(_ptr) BLADE_POOL_DEALLOCATE( StagePool::getPool(), _ptr )
	
}//namespace Blade


#endif //  __Blade_StagePool_h__