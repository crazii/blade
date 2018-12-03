/********************************************************************
	created:	2014/05/01
	filename: 	SkeletonSerializer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_SkeletonSerializer_h__
#define __Blade_SkeletonSerializer_h__
#include <interface/public/ISerializer.h>

namespace Blade
{
	class SkeletonSerializer : public ISerializer, public TempAllocatable
	{
	public:
		SkeletonSerializer();
		~SkeletonSerializer();

		/*
		@describe this method will be called in current task or background loading task,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		and IO related operations need to be put here.
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe process resource.like preLoadResource, this will be called in main synchronous state.\n
		i.e.TextureResource need to be loaded into video card.
		the difference between this and loadResource() is:
		loadResource mainly perform IO related process and maybe time consuming.
		this method mostly process on memory data, especially memory data that need be processed synchronously, and it need to be fast.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource);

		/*
		@describe 
		@param 
		@return 
		*/
		virtual bool	saveResource(const IResource* resource, const HSTREAM& stream);

		/*
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* resource, ParamList& params);


		/*
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
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.

		this is the "reload" version of postProcessResource()
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource);

	protected:

		/** @brief  */
		bool	loadSkeleton(IResource* resource, const HSTREAM& stream, const ParamList& params);

	};//class SkeletonSerializer
	
}//namespace Blade


#endif // __Blade_SkeletonSerializer_h__
