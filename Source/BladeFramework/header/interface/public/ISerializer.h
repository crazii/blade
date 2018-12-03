/********************************************************************
	created:	2010/04/10
	filename: 	ISerializer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ISerializer_h__
#define __Blade_ISerializer_h__
#include <BladeFramework.h>
#include <Factory.h>
#include <Handle.h>
#include <interface/public/file/IFileDevice.h>
#include <interface/public/ISerializable.h>

namespace Blade
{
	class IResource;

#define BLADE_SERIALIZE_CHECK(_val) if( !(_val) )	{assert(false); return (_val);}

	/**
	the framework's default main flow of using ISerilaizer is:
	n.step (where)

	0.check the existing resource list (anywhere)
	1.create one serailizer instance & resource instance corresponding to type info (any task, where IResourceManager::loadResource is called)
	2.perform ISerializer::loadResource in another task (loading task)
	3.test until ISerializer::isloaded() is true (loading task)
	4.put the serlializer & resource instance pair into ready queue (loading task)
	5.process the ready queue in main synchronous state: call ISerializer::postPorcessResource() (main sync state)
	6.delete ISerializer instance and put the resource into existing resource list (main sync state)
	7.dispatch the IReosurce::IListener::onReady /onFailed state (main sync state)
	*/
	class BLADE_FRAMEWORK_API ISerializer
	{
	public:
		virtual ~ISerializer()		{}

		/**
		@describe this method will be called in current task or background loading task,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		and IO related operations need to be put here.
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params) = 0;

		/**
		@describe this method is lately added to solve sub resource problem : whether load synchronously or not.
		commonly, you don't need to override this method, it's used by framework.
		@param 
		@return 
		*/
		virtual bool	loadResourceSync(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			return this->loadResource(resource, stream, params);
		}

		/**
		@describe check whether the serializer finished loading the resource (before post-process)
		commonly one serlializer is done loading after loadResource().
		but if some cascade resource contains linkage to other sub resources, then it is not done until the sub resources is loaded
		and this method is right for the cascade resource type.
		@param 
		@return 
		*/
		virtual bool	isLoaded(IResource* resource) { BLADE_UNREFERENCED(resource);return true; }

		/**
		@describe load resource in main synchronous state,if success,return true \n
		and then the resource manager will not load it again in background loading task.
		@param
		@return
		*/
		virtual bool	preLoadResource(IResource* /*resource*/)	{return true;}

		/**
		@describe process resource.like preLoadResource, this will be called in main synchronous state.\n
		i.e.TextureResource need to be loaded into video card.
		the difference between this and loadResource() is:
		loadResource mainly perform IO related process and maybe time consuming.
		this method mostly process on memory data, especially memory data that need be processed synchronously, and it need to be fast.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource) { BLADE_UNREFERENCED(resource); }

		/**
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* resource, const HSTREAM& stream) = 0;

		/**
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* resource, ParamList& params) = 0;


		/**
		@describe this method is called when resource is reloaded,
		the serializer hold responsibility to cache the loaded data for resource,
		then in main thread ISerializer::reprocessResource() is called to fill the existing resource with new data.\n
		this mechanism is used for reloading existing resource for multi-thread,
		the existing resource is updated in main thread(synchronizing state),
		to ensure that the data is changed only when it is not used in another thread.

		like the loadResouce,this method will be called in main thread or background loading thread,\n
		and the serializer should NOT care about in which thread it is executed.

		this is the "reload" version of loadResource()
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			return this->loadResource(resource, stream, params);
		}

		/**
		@describe this method is lately added to solve sub resource problem : whether load synchronously or not.
		commonly, you don't need to override this method, it's used by framework.
		@param 
		@return 
		*/
		virtual bool	reloadResourceSync(IResource* resource, const HSTREAM& stream, const ParamList& params)
		{
			return this->loadResource(resource, stream, params);
		}

		/**
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.

		this is the "reload" version of postProcessResource()
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource)
		{
			this->postProcessResource(resource);
			return true;
		}

	};//class  ISerializer
	
	extern template class BLADE_FRAMEWORK_API Factory<ISerializer>;

	typedef Factory<ISerializer>	SerializerFactory;
	typedef Handle<ISerializer>		HSERIALIZER;

}//namespace Blade


#endif //__Blade_ISerializer_h__