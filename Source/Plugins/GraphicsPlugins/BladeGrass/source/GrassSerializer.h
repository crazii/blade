/********************************************************************
	created:	2017/11/27
	filename: 	GrassSerializer.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GrassSerializer_h__
#define __Blade_GrassSerializer_h__
#include <CascadeSerializer.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include "GrassResource.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GrassSharedSerializer : public ISerializer, public TempAllocatable
	{
	public:
		/**
		@describe this method will be called in current task or background loading task,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		and IO related operations need to be put here.
		@param
		@return
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/**
		@describe process resource.like preLoadResource, this will be called in main synchronous state.\n
		i.e.TextureResource need to be loaded into video card.
		the difference between this and loadResource() is:
		loadResource mainly perform IO related process and maybe time consuming.
		this method mostly process on memory data, especially memory data that need be processed synchronously, and it need to be fast.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource);

		/**
		@describe
		@param
		@return
		*/
		virtual bool	saveResource(const IResource* resource, const HSTREAM& stream);

		/**
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* resource, ParamList& params);
	};


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	class GrassSerializer : public CascadeSerializer, public TempAllocatable
	{
	public:
		GrassSerializer();
		~GrassSerializer();

		/**
		@describe this method will be called in current task or background loading task,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		and IO related operations need to be put here.
		@param
		@return
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/**
		@describe process resource.like preLoadResource, this will be called in main synchronous state.\n
		i.e.TextureResource need to be loaded into video card.
		the difference between this and loadResource() is:
		loadResource mainly perform IO related process and maybe time consuming.
		this method mostly process on memory data, especially memory data that need be processed synchronously, and it need to be fast.
		@param
		@return
		*/
		virtual void	postProcessResource(IResource* resource);

		/**
		@describe
		@param
		@return
		*/
		virtual bool	saveResource(const IResource* resource, const HSTREAM& stream);

		/**
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* resource, ParamList& params);

	protected:

		/** @brief adjust mesh vertex streams for grass rendering. meshes are adjusted in place. */
		void adjustGrassMesh(index_t targetLayer, const HRESOURCE& mesh);

		/** @brief  */
		void buildInstanceBuffer(const GrassResource* res, uint32 x, uint32 z, uint32 size);

		SubResourceGroup*			mGrassMeshes;

		GrassResource::InstanceData	mSoftInstanceData;
		Box3						mBounding;
	};
	
}//namespace Blade
#endif//__Blade_GrassSerializer_h__