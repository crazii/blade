/********************************************************************
	created:	2013/04/01
	filename: 	VersionFactory.h
	author:		Crazii
	purpose:	for versionized creation of objects
*********************************************************************/
#ifndef __Blade_VersionFactory_h__
#define __Blade_VersionFactory_h__
#include <Factory.h>
#include <utility/Version.h>

namespace Blade
{
	namespace FactoryUtil
	{
		///class registry with type & version info
		class BLADE_BASE_API VTHRD
		{
		public:
			/** @brief  */
			virtual void			recruitWorker(const TString& TypeName, Version version, const Worker* pWorker) = 0;

			/** @brief  */
			virtual const Worker*	findWorker(const TString& TypeName, Version version, bool except = true) const = 0;

			/** @brief type count */
			virtual size_t			getWorkerCount() const = 0;

			/** @brief  */
			virtual const TString&	getWorkerType(index_t typeIndex) const = 0;

			/** @brief version count for same type */
			virtual size_t			getVersionCount(index_t typeIndex) const = 0;

			/** @brief  */
			virtual Version			getVersion(index_t typeIndex, index_t versionIndex) const = 0;

			///static function
			static VTHRD&			buildDepartment();
			static void				closeDepartment(VTHRD& pd);
		};


		///class registry with version info
		class BLADE_BASE_API VHRD
		{
		public:
			/** @brief  */
			virtual void			recruitWorker(Version version, const Worker* pWorker) = 0;

			/** @brief  */
			virtual const Worker*	findWorker(Version version, bool except = true) const = 0;

			/** @brief version count for same type */
			virtual size_t			getVersionCount(index_t typeIndex) const = 0;

			/** @brief  */
			virtual Version			getVersion(index_t versionIndex) const = 0;

			///static function
			static VHRD&			buildDepartment();
			static void				closeDepartment(VHRD& pd);
		};
	}





	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template< typename T >
	class TypeVersionFactory : public Singleton< TypeVersionFactory<T> >, public NonAssignable
	{
	public:

		TypeVersionFactory()
			:mHRDepartment( FactoryUtil::VTHRD::buildDepartment() )
		{
		}

		~TypeVersionFactory()
		{
			FactoryUtil::VTHRD::closeDepartment(mHRDepartment);
		}

		/**
		@describe
		@param
		@return
		*/
		void registerClass(const TString& className, Version ver, const FactoryUtil::Creator<T>* classCreator)
		{
			mHRDepartment.recruitWorker(className, ver, classCreator);
		}

		/**
		@describe
		@param
		@return
		*/
		bool isClassRegistered(const TString& className, Version ver) const
		{
			return mHRDepartment.findWorker( className, ver, false ) != NULL;
		}


		/**
		@describe
		@param
		@return
		*/
		T* createInstance(const TString& className, Version ver) const
		{
			const FactoryUtil::Creator<T>* pWorker = static_cast<const FactoryUtil::Creator<T>*>( mHRDepartment.findWorker( className, ver ) );
			return pWorker->create();
		}

		/**
		@describe
		@param
		@return
		*/
		T* createInstance(const TString& className, Version ver, const char* file,int line) const
		{
			const FactoryUtil::Creator<T>* pWorker = static_cast<const FactoryUtil::Creator<T>*>( mHRDepartment.findWorker( className, ver ) );
			return pWorker->create(file,line);
		}

		/**
		@describe
		@param
		@return
		*/
		size_t			getNumRegisteredClasses() const
		{
			return mHRDepartment.getWorkerCount();
		}

		/**
		@describe
		@param
		@return
		*/
		const TString&	getRegisteredClass(index_t n) const
		{
			return mHRDepartment.getWorkerType(n);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		size_t			getNumVersions(index_t classIndex) const
		{
			return mHRDepartment.getVersionCount(classIndex);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		Version			getVersion(index_t classIndex, index_t versionIndex) const
		{
			return mHRDepartment.getVersion(classIndex, versionIndex);
		}

	private:
		FactoryUtil::VTHRD&	mHRDepartment;
	};//template class Factory



	namespace FactoryUtil
	{
		template< typename T,typename CreatorT >
		class TypeVersionRegisterProxy
		{
		public:
			TypeVersionRegisterProxy(const TString& name, Version version)
			{
				TypeVersionFactory<T>::getSingleton().registerClass( name, version, &msTypeCreator );
			}
		protected:
			static CreatorT	msTypeCreator;
		};//template class VersionRegisterProxy
		template< typename T,typename CreatorT >
		CreatorT TypeVersionRegisterProxy<T, CreatorT>::msTypeCreator;

	}//namespace FactoryUtil









	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template< typename T >
	class VersionFactory : public Singleton< VersionFactory<T> >, public NonAssignable
	{
	public:

		VersionFactory()
			:mHRDepartment( FactoryUtil::VHRD::buildDepartment() )
		{
		}

		~VersionFactory()
		{
			FactoryUtil::VHRD::closeDepartment(mHRDepartment);
		}

		/**
		@describe
		@param
		@return
		*/
		void registerClass(Version ver, const FactoryUtil::Creator<T>* classCreator)
		{
			mHRDepartment.recruitWorker(ver, classCreator);
		}

		/**
		@describe
		@param
		@return
		*/
		bool isClassRegistered(Version ver) const
		{
			return mHRDepartment.findWorker(ver, false ) != NULL;
		}


		/**
		@describe
		@param
		@return
		*/
		T* createInstance(Version ver) const
		{
			const FactoryUtil::Creator<T>* pWorker = static_cast<const FactoryUtil::Creator<T>*>( mHRDepartment.findWorker(ver) );
			return pWorker->create();
		}

		/**
		@describe
		@param
		@return
		*/
		T* createInstance(Version ver, const char* file,int line) const
		{
			const FactoryUtil::Creator<T>* pWorker = static_cast<const FactoryUtil::Creator<T>*>( mHRDepartment.findWorker( ver ) );
			return pWorker->create(file,line);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		size_t			getNumVersions(index_t classIndex) const
		{
			return mHRDepartment.getVersionCount(classIndex);
		}

		/**
		@describe 
		@param 
		@return 
		*/
		Version			getVersion(index_t versionIndex) const
		{
			return mHRDepartment.getVersion(versionIndex);
		}

	private:
		FactoryUtil::VHRD&	mHRDepartment;
	};//template class Factory



	namespace FactoryUtil
	{
		template< typename T,typename CreatorT >
		class VersionRegisterProxy
		{
		public:
			VersionRegisterProxy(Version version)
			{
				TypeVersionFactory<T>::getSingleton().registerClass( version, &msTypeCreator );
			}
		protected:
			static CreatorT	msTypeCreator;
		};//template class VersionRegisterProxy
		template< typename T,typename CreatorT >
		CreatorT VersionRegisterProxy<T, CreatorT>::msTypeCreator;

	}//namespace FactoryUtil
	
}//namespace Blade

#define NameVersionRegisterFactory(_type, _basetype, _name, _version) do{volatile const Blade::FactoryUtil::TypeVersionRegisterProxy<_basetype, Blade::FactoryUtil::ContreteCreator<_basetype,_type> > _reg(_name, _version);}while(false)

#if BLADE_MEMORY_DEBUG
#	define BLADE_TYPEVERSION_FACOTRY_CREATE(_type, _name, _version) TypeVersionFactory<_type>::getSingleton().createInstance(_name, _version, __FILE__,__LINE__)
#else
#	define BLADE_TYPEVERSION_FACOTRY_CREATE(_type, _name, _version) TypeVersionFactory<_type>::getSingleton().createInstance( _name, _version)
#endif

#define VersionRegisterFactory(_type, _basetype, _version) do{const Blade::FactoryUtil::VersionRegisterProxy<_basetype, Blade::FactoryUtil::ContreteCreator<_basetype,_type> > _reg(_version);}while(false)

#if BLADE_MEMORY_DEBUG
#	define BLADE_VERSION_FACOTRY_CREATE(_type, _version) VersionFactory<_type>::getSingleton().createInstance(_version, __FILE__, __LINE__)
#else
#	define BLADE_VERSION_FACOTRY_CREATE(_type, _version) VersionFactory<_type>::getSingleton().createInstance( _version)
#endif



#endif //  __Blade_VersionFactory_h__