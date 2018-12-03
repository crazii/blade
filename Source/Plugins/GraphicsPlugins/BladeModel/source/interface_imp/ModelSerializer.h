/********************************************************************
	created:	2013/04/03
	filename: 	ModelSerializer.h
	author:		Crazii
	
	purpose:	
*********************************************************************/
#ifndef __Blade_ModelSerializer_h__
#define __Blade_ModelSerializer_h__
#include <math/AxisAlignedBox.h>
#include <utility/BladeContainer.h>
#include <interface/public/ISerializer.h>
#include <interface/public/graphics/IImage.h>

#include <CascadeSerializer.h>
#include <TypeVersionSerializer.h>
#include <interface/IModelResource.h>

namespace Blade
{
	class ModelResource;

	class ModelSerializerBase : public CascadeSerializer, public TempAllocatable
	{
	public:
		virtual ~ModelSerializerBase()	{}
	};

	//default model serializer
	class ModelSerializer : public TypeVersionSerializer<ModelSerializerBase>, public TempAllocatable
	{
	public:
		typedef TypeVersionSerializer<ModelSerializerBase> BaseType;
	public:
		ModelSerializer() :BaseType(ModelConsts::MODEL_SERIALIZER_BINARY, IModelResource::MODEL_LATEST_SERIALIZER_VERSION )
		{}
	};

	//xml model serializer
	class ModelSerializerXML : public TypeVersionSerializer<ModelSerializerBase>, public TempAllocatable
	{
	public:
		typedef TypeVersionSerializer<ModelSerializerBase> BaseType;
	public:
		ModelSerializerXML() :BaseType(ModelConsts::MODEL_SERIALIZER_XML, IModelResource::MODEL_LATEST_SERIALIZER_VERSION )
		{}
	};

	class ModelSerializer_Binary : public ModelSerializerBase
	{
	public:
		ModelSerializer_Binary();
		~ModelSerializer_Binary();
		
		/*
		@describe this method will be called in main thread or background loading thread,\n
		and the serializer should NOT care about in which thread it is executed(better to be thread safe always).
		if the callback param is not NULL, then this is executed on main thread, otherwise it is on other threads.
		@param 
		@return 
		*/
		virtual bool	loadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe process resource.like preLoadResource, this will be called in main thread.\n
		i.e.TextureResource need to be loaded into graphics card.
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
		@param
		@return
		*/
		virtual bool	reloadResource(IResource* resource, const HSTREAM& stream, const ParamList& params);

		/*
		@describe this method will be called in main thread (synchronous thread),
		after the ISerializer::reloadResource called in asynchronous state.
		@param 
		@return 
		*/
		virtual bool	reprocessResource(IResource* resource);

	protected:
		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief load stream into mesh & materials */
		bool	loadModel(const HSTREAM& stream, const ParamList& params);
		/** @brief postload/re-create from cached data */
		bool	createModel(ModelResource* resource);
		/** @brief  */
		bool	createBuffers(ModelResource* resource, IGraphicsResourceManager& manager, bool flipUV);
		/** @brief  */
		bool	mergeBuffer(IModelResource::MESHDATA** meshes, size_t count, IModelResource::MESHDATA& outMergedTarget);

		typedef TempVector<IModelResource::MATERIAL_INFO > MaterialCache;
		typedef TempVector<IModelResource::MESHDATA> MeshCache;
		typedef TempVector<CascadeSerializer::SubResourceGroup*>		TextureList;
		typedef TempVector<uint32> BoneIDCache;
		MaterialCache	mMaterials;
		BoneIDCache		mBoneIDs;
		MeshCache		mMeshes;
		TextureList		mTextures;

		ModelResource::MESHDATA mMergedOpaqueCache;
		ModelResource::MESHDATA mMergedAlphaCache;

		CascadeSerializer::SubResourceGroup* mSkeletonAnimation;
		AABB			mModelAABB;
		scalar			mModelRadius;
		uint32			mNormalType;
		bool			mSoftWareMesh;
		bool			mSharedBuffer;
	};
	
}//namespace Blade

#endif//__Blade_ModelSerializer_h__