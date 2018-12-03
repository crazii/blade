/********************************************************************
	created:	2017/12/08
	filename: 	StaticModelSerializer.h
	author:		Crazii
	purpose:	pre transformed model resource loader
*********************************************************************/
#ifndef __Blade_StaticModelSerializer_h__
#define __Blade_StaticModelSerializer_h__
#include <CascadeSerializer.h>
#include "StaticModelResource.h"

namespace Blade
{
	class StaticModelSerializer : public CascadeSerializer
	{
	public:
		StaticModelSerializer()
		{
			mModelResource = NULL;
		}

		~StaticModelSerializer()
		{

		}

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
		virtual bool	saveResource(const IResource* /*resource*/, const HSTREAM& /*stream*/)
		{
			return false;
		}

		/**
		@describe
		@param
		@return
		*/
		virtual bool	createResource(IResource* /*resource*/, ParamList& /*params*/)
		{
			return false;
		}

	protected:
		/** @brief CascadeSerializer override */
		virtual void postLoad(IResource* resource);

	protected:
		SubResource*	mModelResource;
		Matrix44		mTransform;
	};
	
}//namespace Blade

#endif//__Blade_StaticModelSerializer_h__