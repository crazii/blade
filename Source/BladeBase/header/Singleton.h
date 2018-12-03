/********************************************************************
	created:	2009/02/07
	filename: 	Singleton.h
	author:		Crazii
	
	purpose:	the Singleton pattern: lazy initialization mode
*********************************************************************/
#ifndef __Blade_Singleton_h__
#define __Blade_Singleton_h__
#include "BladeTypes.h" //NULL
#include "Lock.h"
#include "memory/BladeMemory.h"

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4640) //construction of local static object is not thread safe
#endif

namespace Blade
{

	template< typename T >
	class Singleton : public StaticAllocatable
	{
	private:
		Singleton(const Singleton&);
		Singleton& operator=(const Singleton&);
		//static void* operator new(size_t);
		//static void* operator new[](size_t);

	public:

		/** @brief public reference access */
		static inline T& getSingleton()
		{
			///lazy initialization
			return *Singleton<T>::operateSingleton();
		}

		/** @brief public pointer access */
		static inline T* getSingletonPtr()
		{
			return Singleton<T>::operateSingleton();
		}

	protected:
		inline Singleton()
		{
			///note: init subclass pointer in base ctor, this is NOT thread safe,
			///but blade now doesn't support new/BLADE_NEW, so the ctor only will happen in operateSingleton(), 
			///which is guarded by spin lock and is thread safe.
			assert( msSingleton == NULL );
			msSingleton = static_cast<T*>(this);
		}

		inline ~Singleton()
		{
			//this only will be set on C++ terminating routine
			msSingleton = NULL;
		}

		/** @brief call to create instance */
		inline static T* operateSingleton()
		{
#if BLADE_DEBUG
			static bool isCreated = false;
#endif
			///spin lock is fast enough, NO NEED for double check
			///actually double check on msSingleton may be incorrect: when msSingleton != NULL, may be the singleton object is not wholely initialized, 
			///only with the base Singleton() initialized.
			ScopedLock _lock(msLock);

			if( msSingleton == NULL )
			{
#if BLADE_DEBUG
				///avoid repeat destroying & creating 2nd time.
				///this usually happens when singleton construction order need explicit control
				assert(isCreated == false);
#endif
				///local static make sure it constructed right on first call
				///won't construct before (memory pool) & destruct after (pool)
				///Effective C++ code 47:
				///C++03 6.7 Declaration statement:4
				static T msInstance;
				assert(msSingleton == &msInstance);
#if BLADE_DEBUG
				isCreated = true;
#endif
			}
			return msSingleton;
		}

	private:
		static T*			msSingleton;
		static StaticLock	msLock;	
	};

	template<typename T>
	T* Singleton<T>::msSingleton = NULL;

	template<typename T>
	StaticLock Singleton<T>::msLock;


	//////////////////////////////////////////////////////////////////////////
	//singleton static init helper
	template <typename T>
	class SingletonInitializer
	{
	public:
		SingletonInitializer() { Singleton<T>::getSingleton(); }
	};

}//namespace Blade

#if BLADE_COMPILER == BLADE_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif // __Blade_Singleton_h__