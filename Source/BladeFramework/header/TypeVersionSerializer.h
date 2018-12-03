/********************************************************************
	created:	2013/04/01
	filename: 	TypeVersionSerializer.h
	author:		Crazii
	purpose:	this serializer pattern is used for different serializers or resource formats for the same resource
*********************************************************************/
#ifndef __Blade_TypeVersionSerializer_h__
#define __Blade_TypeVersionSerializer_h__
#include <interface/public/ISerializer.h>
#include <VersionFactory.h>

namespace Blade
{
	namespace TypeVersionSerializerUtil
	{
		BLADE_FRAMEWORK_API bool	readSerializerInfo(const HSTREAM& stream, TString& type, Version& version);
		BLADE_FRAMEWORK_API bool	readSerializerInfo(const HSTREAM& stream, Version& version);

		BLADE_FRAMEWORK_API bool	writeSerializerInfo(const HSTREAM& stream, const TString& type, const Version& version);
		BLADE_FRAMEWORK_API bool	writeSerializerInfo(const HSTREAM& stream, const Version& version);
	}//namespace TypeVersionSerializerUtil

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T>
	class TypeVersionSerializer : public ISerializer
	{
	public:
		typedef TypeVersionFactory<T> SerializerImplFactory;
	public:
		TypeVersionSerializer(const TString& type, Version version)
			:mInitType(type)
			,mInitVersion(version)
		{
			mImpl = BLADE_TYPEVERSION_FACOTRY_CREATE(T, type, version);
		}
		~TypeVersionSerializer()
		{
			BLADE_DELETE mImpl;
		}

		/**
		@describe this method will be called in main thread or background loading thread,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		if the callback param is not NULL, then this is executed on main thread, otherwise it is on other threads.
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			this->createImpl(stream);
			return mImpl->loadResource(resource, stream, params);
		}

		/**
		@describe this method is lately added to solve sub resource problem : whether load synchronously or not.
		commonly, you don't need to override this method, it's used by framework.
		@param 
		@return 
		*/
		virtual bool	loadResourceSync(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			this->createImpl(stream);
			if(mImpl != NULL)
				return mImpl->loadResourceSync(resource, stream, params);
			return false;
		}

		/**
		@describe check whether the serializer finished loading the resource (before post-process)
		commonly one serlializer is done loading after loadResource().
		but if some cascade resource contains linkage to other sub resources, then it is not done until the sub resources is loaded
		and this method is right for the cascade resource type.
		@param
		@return
		*/
		virtual bool	isLoaded(IResource* res)
		{
			assert(mImpl != NULL);
			return mImpl->isLoaded(res);
		}

		/**
		@describe load resource in main thread,if success,return true \n
		and then the resource manager will not load it again in loading thread.
		@param
		@return
		*/
		virtual bool	preLoadResource(IResource* resource)
		{
			return mImpl->preLoadResource(resource);
		}

		/**
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource)
		{
			return mImpl->postProcessResource(resource);
		}


		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* resource, const HSTREAM& stream)
		{
			return TypeVersionSerializerUtil::writeSerializerInfo(stream, mInitType, mInitVersion)
				&& mImpl->saveResource(resource, stream);
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* resource, ParamList& params)
		{
			return mImpl->createResource(resource, params);
		}

		/**
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.

		like the loadResouce,this method will be called in main thread or background loading thread,\n
		and the serializer should NOT care about in which thread it is executed.
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			this->createImpl(stream);
			return mImpl->reloadResource(resource, stream, params);
		}

		/**
		@describe this method is lately added to solve sub resource problem : whether load synchronously or not.
		commonly, you don't need to override this method, it's used by framework.
		@param 
		@return 
		*/
		virtual bool	reloadResourceSync(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			this->createImpl(stream);
			return mImpl->reloadResourceSync(resource, stream, params);
		}

		/**
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource)
		{
			return mImpl->reprocessResource(resource);
		}

	protected:

		/** @brief  */
		void		createImpl(const HSTREAM& stream)
		{
			BLADE_DELETE mImpl;
			mImpl = NULL;
			TString type;
			Version version;
			if( TypeVersionSerializerUtil::readSerializerInfo(stream, type,version) )
				mImpl = BLADE_TYPEVERSION_FACOTRY_CREATE(T, type, version);
			else
				assert(false);
		}

		TString	mInitType;
		Version	mInitVersion;
		T*		mImpl;
	};//class TypeVersionSerializer



	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	template<typename T>
	class VersionSerializer : public ISerializer
	{
	public:
		typedef TypeVersionFactory<T> SerializerImplFactory;
	public:
		VersionSerializer(Version version)
			:mInitVersion(version)
		{
			mImpl = BLADE_VERSION_FACOTRY_CREATE(T, version);
		}
		~VersionSerializer()
		{
			BLADE_DELETE mImpl;
		}

		/**
		@describe this method will be called in main thread or background loading thread,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		if the callback param is not NULL, then this is executed on main thread, otherwise it is on other threads.
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			this->createImpl(stream);
			return mImpl->loadResource(resource, stream, params);
		}

		/**
		@describe load resource in main thread,if success,return true \n
		and then the resource manager will not load it again in loading thread.
		@param
		@return
		*/
		virtual bool	preLoadResource(IResource* resource)
		{
			return mImpl->preLoadResource(resource);
		}

		/**
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource)
		{
			return mImpl->postProcessResource(resource);
		}


		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* resource, const HSTREAM& stream)
		{
			return TypeVersionSerializerUtil::writeSerializerInfo(stream, mInitVersion)
				&& mImpl->saveResource(resource, stream);
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* resource, ParamList& params)
		{
			return mImpl->createResource(resource, params);
		}

		/**
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.

		like the loadResouce,this method will be called in main thread or background loading thread,\n
		and the serializer should NOT care about in which thread it is executed.
		@param
		@return
		*/
		virtual bool	reloadResource(const HSTREAM& stream, const ParamList& params)
		{
			this->createImpl(stream);
			return mImpl->reloadResource(stream, params);
		}

		/**
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource)
		{
			return mImpl->reprocessResource(resource);
		}

	protected:

		/** @brief  */
		void		createImpl(const HSTREAM& stream)
		{
			BLADE_DELETE mImpl;
			mImpl = NULL;
			Version version;
			if( TypeVersionSerializerUtil::readSerializerInfo(stream, version) )
				mImpl = BLADE_VERSION_FACOTRY_CREATE(T, version);
			else
				assert(false);
		}

		Version	mInitVersion;
		T*		mImpl;
	};//class VersionSerializer
	
}//namespace Blade

#endif //  __Blade_TypeVersionSerializer_h__
