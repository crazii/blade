/********************************************************************
	created:	2009/02/07
	filename: 	Factory.h
	author:		Crazii
	
	purpose:	the Factory pattern
*********************************************************************/
#ifndef __Blade_Factory_h__
#define __Blade_Factory_h__
#include <Singleton.h>
#include <utility/String.h>

namespace Blade
{

	/************************************************************************/
	/* abstract Creator                                                                     */
	/************************************************************************/
	namespace FactoryUtil
	{
		class Worker
		{};

		template <typename T>
		class Creator : public Worker
		{
		public:
			virtual ~Creator()	{}

			virtual T*		create(const char* file,int line) const = 0;
			virtual T*		create() const = 0;
			virtual void	destroy(T*) const = 0;
		};//class Creator


		///well,human resource department :D
		///this class 's purpose is to hide the STL container inside Factory, \n
		///which may has diff memory layout with diff C runtime lib has may cause DLL boundary issues
		class BLADE_BASE_API HRD
		{
		public:
			/** @brief  */
			virtual void			recruitWorker(const TString& typeName,const Worker* pWorker) = 0;

			/** @brief  */
			virtual const Worker*	findWorker(const TString& typeName, bool except = true) const = 0;

			/** @brief  */
			virtual bool			removeWorker(const TString& typeName) = 0;

			/** @brief  */
			virtual size_t			getWorkerCount() const = 0;

			/** @brief  */
			virtual const TString&	getWorkerType(index_t index) const = 0;

			///static function
			static HRD&				buildDepartment();

			static void				closeDepartment(HRD& pd);
		};


	}//namespace FactoryUtil


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template< typename T >
	class Factory : public Singleton< Factory<T> >, public NonAssignable, public NonCopyable
	{
	private:
		Factory(const Factory&);
		Factory& operator=(const Factory&);
	public:

		inline Factory()
			:mHRDepartment( FactoryUtil::HRD::buildDepartment() )
		{
		}

		inline ~Factory()
		{
			FactoryUtil::HRD::closeDepartment(mHRDepartment);
		}

		/**
		@describe hacking for GCC: "extern template Factory<T>" will prevent Factory<T> from local instantiation, but its base Single<T> doesn't.
		simply overwrite base function to skip Single<T>'s local instantiation,
		or we have to add and extra declaration "extern template BLADE_XXX_API Singleton<T>;" for each exported factory instances.
		@param 
		@return 
		*/
		static Factory&	getSingleton();
		static Factory*	getSingletonPtr();

		/**
		@describe
		@param
		@return
		*/
		inline void registerClass(const TString& className,const FactoryUtil::Creator<T>* classCreator)
		{
			mHRDepartment.recruitWorker(className,classCreator);
		}

		/**
		@describe
		@param
		@return
		*/
		inline bool isClassRegistered(const TString& className) const
		{
			return mHRDepartment.findWorker( className, false ) != NULL;
		}

		/**
		@describe 
		@param
		@return
		*/
		inline bool unregisterClass(const TString& className)
		{
			return mHRDepartment.removeWorker(className);
		}


		/**
		@describe
		@param
		@return
		*/
		inline T* createInstance(const TString& className) const
		{
			const FactoryUtil::Creator<T>* pWorker = static_cast<const FactoryUtil::Creator<T>*>( mHRDepartment.findWorker( className ) );
			return pWorker->create();
		}

		/**
		@describe
		@param
		@return
		*/
		inline T* createInstance(const TString& className,const char* file,int line) const
		{
			const FactoryUtil::Creator<T>* pWorker = static_cast<const FactoryUtil::Creator<T>*>( mHRDepartment.findWorker( className ) );
			return pWorker->create(file,line);
		}

		/**
		@describe
		@param
		@return
		*/
		inline size_t getNumRegisteredClasses() const
		{
			return mHRDepartment.getWorkerCount();
		}

		/**
		@describe
		@param
		@return
		*/
		inline const TString& getRegisteredClass(index_t n) const
		{
			return mHRDepartment.getWorkerType(n);
		}

	private:
		FactoryUtil::HRD&	mHRDepartment;

		/** @brief helper for InterfaceSingleton */
		static StaticLock	msLock;
		static T*			msInterfaceSingleton;

		/** @brief helper for InterfaceSingleton */
		static T* switchInterfaceSingleton(const TString& className)
		{
			ScopedLock _lock(msLock);

			T *volatile* ppSingleton = &msInterfaceSingleton;
			T* instance = Factory<T>::getSingleton().createInstance( className );
			*ppSingleton = instance;

			return msInterfaceSingleton;
		}
		
		/** @brief helper for InterfaceSingleton */
		static T* getInterfaceSingleton()
		{
			ScopedLock _lock(msLock);

			if(msInterfaceSingleton != NULL || !Factory<T>::getSingleton().isClassRegistered( BTString("Default") ))
				return msInterfaceSingleton;

			T *volatile* ppSingleton = &msInterfaceSingleton;
			T* instance = Factory<T>::getSingleton().createInstance( BTString("Default") );
			*ppSingleton = instance;

			return msInterfaceSingleton;
		}

		/** @brief helper for InterfaceSingleton */
		static bool testInterfaceSingleton()
		{
			ScopedLock _lock(msLock);
			return msInterfaceSingleton != NULL;
		}

		/** @brief helper for InterfaceSingleton */
		static void resetInterfaceSingleton()
		{
			ScopedLock _lock(msLock);
			msInterfaceSingleton = NULL;
		}

		template<typename U> friend class InterfaceSingleton;
	};//template class Factory

	/** @brief  */
	template<typename T>
	StaticLock	Factory<T>::msLock;

	/** @brief  */
	template<typename T>
	T*	Factory<T>::msInterfaceSingleton;

	/** @brief  */
	template<typename T>
	Factory<T>&	Factory<T>::getSingleton()
	{
		return Singleton< Factory<T> >::getSingleton();
	}

	/** @brief  */
	template<typename T>
	Factory<T>*	Factory<T>::getSingletonPtr()
	{
		return Singleton< Factory<T> >::getSingletonPtr();
	}

	/************************************************************************/
	/* concrete Creator for concrete type                                                                     */
	/************************************************************************/
	namespace FactoryUtil
	{
		template<typename T,typename CT >
		class ContreteCreator : public Creator<T>
		{
			virtual T*	create(const char* file, int line) const
			{
#if !BLADE_MEMORY_DEBUG
				BLADE_UNREFERENCED(file);BLADE_UNREFERENCED(line);
#endif
				return static_cast<T*>( BLADE_TRACE_NEW(file,line) CT() );
			}

			virtual T* create() const
			{
				return static_cast<T*>( BLADE_NEW CT() );
			}

			virtual void destroy(T* ptr) const
			{
				BLADE_DELETE ptr;
			}

		};//template class ContreteCreator
		
		template< typename T, typename CreatorT>
		class RegisterProxy
		{
		public:
			RegisterProxy(const TString& name)
			{
				Factory<T>::getSingleton().registerClass( name, &msTypeCreator );
			}
		protected:
			static CreatorT	msTypeCreator;
		};//template class RegisterProxy

		template< typename T,typename CreatorT >
		CreatorT RegisterProxy<T, CreatorT>::msTypeCreator;

	}//namespace FactoryUtil

}//namespace Blade

///It's OK for scoped class i.e. NameRegisterFactoryScope(A::IB, A::B, ... )
#define NameRegisterFactory(_type, _basetype, _name) do{volatile const Blade::FactoryUtil::RegisterProxy<_basetype,Blade::FactoryUtil::ContreteCreator<_basetype,_type> > _reg(_name);}while(false)
#define RegisterFactory(_type,_basetype) NameRegisterFactory(_type, _basetype, BTString(#_type))
#define NameUnregisterFactory(_basetype, _name) Factory<_basetype>::getSingleton().unregisterClass(_name)
#define UnRegisterFactory(_type) NameRegisterFactory(_type, BTString(#_type))

#if BLADE_MEMORY_DEBUG
#	define BLADE_FACTORY_CREATE(_type,_name) Blade::Factory<_type>::getSingleton().createInstance(_name, __FILE__, __LINE__)
#else
#	define BLADE_FACTORY_CREATE(_type,_name) Blade::Factory<_type>::getSingleton().createInstance( _name )
#endif

#endif // __Blade_Factory_h__