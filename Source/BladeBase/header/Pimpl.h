/********************************************************************
	created:	2012/02/26
	filename: 	Pimpl.h
	author:		Crazii
	purpose:	
	note:		because Pimpl using template and by concept, it is not (actually should not be) accessible,
				thus there's no need to export the template as DLL interface
				so you can ignore the MSVC C4251 warning.
				and do remember to declare the member variable as private when using Pimpl idiom.
*********************************************************************/
#ifndef __Blade_Pimpl_h__
#define __Blade_Pimpl_h__
#include <RefCount.h>

namespace Blade
{
	namespace PimplTraits
	{
		struct DefaultTraits
		{
			typedef void (*FnDelete)(void*);
			FnDelete   pfnDelete;

			/** @brief  */
			inline void destroy(void* p)	{(*pfnDelete)(p);}
		};

		template <typename T>
		struct DefaultTraitsT : public DefaultTraits
		{
			static void    deleteT(void* p)	{BLADE_DELETE (T*)p;}

			DefaultTraitsT()	{pfnDelete = &deleteT;}
		};

		struct AdvancedTraits
		{
			typedef void* (*FnNew)(void);
			typedef void (*FnDelete)(void*);
			FnNew      pfnNew;
			FnDelete   pfnDelete;

			/** @brief  */
			inline void* create()		{return (*pfnNew)();}
			/** @brief  */
			inline void destroy(void* p){(*pfnDelete)(p);}
		};

		template <typename T>
		struct AdvancedTraitsT : public AdvancedTraits
		{
			/** @brief  */
			static void*   newT()				{return BLADE_NEW T();}
			static void    deleteT(void* p)		{BLADE_DELETE (T*)p;}

			AdvancedTraitsT()
			{
				pfnNew = &newT;
				pfnDelete = &deleteT;
			}
		};
	}

	///C++ Pimpl idiom, please ignore all C4251 warning on Pimpl for MSVC
	template<typename T>
	class Pimpl
	{
	public:
		typedef T value_type;
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef PimplTraits::DefaultTraits traits_type;

		explicit Pimpl(pointer ptr, traits_type traits = PimplTraits::DefaultTraitsT<T>() )
		{
			mPtr = ptr;
			mTraits = traits;
		}

		~Pimpl()
		{
			mTraits.destroy(mPtr);
		}

		/** @brief  */
		bool		operator==(const Pimpl<T>& rhs) const
		{
			return mPtr == rhs.mPtr;
		}

		/** @brief  */
		bool		operator!=(const Pimpl<T>& rhs) const
		{
			return mPtr != rhs.mPtr;
		}

		/** @brief  */
		bool		operator==(const pointer rhs) const
		{
			return mPtr == rhs;
		}

		/** @brief  */
		bool		operator!=(const pointer rhs) const
		{
			return mPtr != rhs;
		}

		/** @brief  */
		pointer operator-> (void)
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		reference operator * (void)
		{
			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		const_pointer operator -> (void) const
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		const_reference operator * (void) const 
		{
			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		pointer	get()
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		const pointer	get() const
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

	private:
		pointer		mPtr;
		traits_type	mTraits;

		/** @brief  */
		Pimpl(const Pimpl& src);
		/** @brief  */
		Pimpl& operator=(const Pimpl& rhs);
	};//class Pimpl
	
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	class LazyPimpl
	{
	public:
		typedef T value_type;
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef PimplTraits::AdvancedTraits traits_type;

		LazyPimpl(traits_type traits = PimplTraits::AdvancedTraitsT<T>() )
		{
			mPtr = NULL;
			mTraits = traits;
		}

		~LazyPimpl()
		{
			this->destruct();
		}

		/** @brief  */
		bool		operator==(const LazyPimpl<T>& rhs) const
		{
			return mPtr == rhs.mPtr;
		}

		/** @brief  */
		bool		operator!=(const LazyPimpl<T>& rhs) const
		{
			return mPtr != rhs.mPtr;
		}

		/** @brief  */
		bool		operator==(const pointer rhs) const
		{
			return mPtr == rhs;
		}

		/** @brief  */
		bool		operator!=(const pointer rhs) const
		{
			return mPtr != rhs;
		}

		/** @brief const access to no-const pointer. not recommended */
		pointer get(void) const
		{
			assert(this->mPtr != NULL);
			return this->mPtr;
		}

		/** @brief  */
		pointer operator-> (void)
		{
			if( this->mPtr == NULL )
				this->construct();

			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		reference operator * (void)
		{
			if( this->mPtr == NULL )
				this->construct();

			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		const_pointer operator -> (void) const
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		const_reference operator * (void) const 
		{
			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		void destruct()
		{
			mTraits.destroy(mPtr);
			mPtr = NULL;
		}
	private:
		/** @brief  */
		void construct()
		{
			assert(mPtr == NULL);
			mPtr = (T*)mTraits.create();
		}


		pointer		mPtr;
		traits_type	mTraits;

		LazyPimpl(const LazyPimpl& src);
		LazyPimpl& operator=(const LazyPimpl& rhs);
	};//class LazyPimpl

	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	class RefPimpl
	{
	public:
		typedef T value_type;
		typedef value_type * pointer;
		typedef const value_type * const_pointer;
		typedef value_type & reference;
		typedef const value_type & const_reference;
		typedef PimplTraits::AdvancedTraits traits_type;

		RefPimpl(traits_type traits = PimplTraits::AdvancedTraitsT<T>() )
		{
			mPtr = NULL;
			mRefCount = NULL;
			mTraits = traits;
		}

		~RefPimpl()
		{
			this->decRef();
		}

		RefPimpl(const RefPimpl<T>& src)
		{
			mPtr = src.mPtr;
			mRefCount = src.mRefCount;
			mTraits = src.mTraits;
			this->incRef();
		}

		RefPimpl& operator=(const RefPimpl<T>& rhs)
		{
			this->decRef();
			mPtr = rhs.mPtr;
			mRefCount = rhs.mRefCount;
			this->incRef();
			return *this;
		}

		/** @brief  */
		bool		operator==(const RefPimpl<T>& rhs) const
		{
			return mPtr == rhs.mPtr;
		}

		/** @brief  */
		bool		operator!=(const RefPimpl<T>& rhs) const
		{
			return mPtr != rhs.mPtr;
		}

		/** @brief  */
		bool		operator==(const pointer rhs) const
		{
			return mPtr == rhs;
		}

		/** @brief  */
		bool		operator!=(const pointer rhs) const
		{
			return mPtr != rhs;
		}

		/** @brief  */
		pointer operator-> (void)
		{
			if( this->mPtr == NULL )
				this->construct();

			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		reference operator * (void)
		{
			if( this->mPtr == NULL )
				this->construct();

			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		const_pointer operator -> (void) const
		{
			assert( this->mPtr != NULL );
			return this->mPtr;
		}

		/** @brief  */
		const_reference operator * (void) const 
		{
			assert( this->mPtr != NULL );
			return *(this->mPtr);
		}

		/** @brief  */
		void incRef()
		{
			if(mRefCount != NULL)
				mRefCount->safeIncrement();
		}
		/** @brief  */
		void decRef()
		{
			if( mRefCount != NULL )
			{
				assert( mPtr != NULL );
				if( mRefCount->safeDecrement() == 0 )
				{
					this->destruct();
				}
			}
			else
				assert(mPtr == NULL);
		}
		/** @brief  */
		void clear()
		{
			this->decRef();
			mRefCount = NULL;
			mPtr = NULL;
		}

		/** @brief  */
		void destruct()
		{
			mTraits.destroy(mPtr);
			mPtr = NULL;
			BLADE_DELETE mRefCount;
			mRefCount = NULL;
		}

	private:
		/** @brief  */
		void construct()
		{
			assert(mPtr == NULL);
			assert(mRefCount == NULL);
			mPtr = (T*)mTraits.create();
			mRefCount = BLADE_NEW RefCount(1);
		}

		pointer		mPtr;
		RefCount*	mRefCount;
		traits_type	mTraits;
	};//class RefPimpl

}//namespace Blade


#endif //__Blade_Pimpl_h__