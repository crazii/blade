/********************************************************************
	created:	2017/11/27
	filename: 	GrassResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_GrassResource_h__
#define __Blade_GrassResource_h__
#include <GraphicsResource.h>
#include <utility/BladeContainer.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IResourceManager.h>
#include <interface/IModelResource.h>
#include <BladeGrass.h>
#include "GrassConfig.h"

namespace Blade
{
	//grass resource for one paging. graphics elements within the same page shares one resource
	class GrassSharedResource : public GraphicsResource, public Allocatable
	{
	public:
		static const TString GRASS_SHARED_RESOURCE_TYPE;
	public:
		GrassSharedResource()
			:GraphicsResource(GRASS_SHARED_RESOURCE_TYPE)
		{
			std::memset(mDensity, 0, sizeof(mDensity));
		}
		~GrassSharedResource()
		{
			for(index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
				BLADE_RES_FREE(mDensity[i]);
		}

		/** @brief  */
		inline const uint32*	getDensity(index_t layer) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			return mDensity[layer]; 
		}

		/** @brief  */
		inline uint32		getSize() const { return mSize; }

		/** @brief  */
		inline uint32		getX() const { return mX; }
		
		/** @brief  */
		inline uint32		getZ() const { return mZ; }

	protected:
		uint32			mSize;		//meter per side total area: mSize*mSize
		uint32			mX;			//world offset
		uint32			mZ;			//world offset
		uint32*			mDensity[GrassConfig::MAX_LAYERS];
		friend class GrassSharedSerializer;
	};

	//per element grass resource. it doesn't have its own data to load (UF_PHONY), but load sub resources through parameters
	//this resource uses part of shared density resource(the real resource) and build instancing data
	//the used part is defined as offset x,z and a range
	class GrassResource : public GraphicsResource, public Allocatable
	{
		typedef List<HRESOURCE> GrassMeshList;

		struct InstancingArea
		{
			HVBUFFER		buffer;		//instancing vertex buffer
			Box3			bounding;
			size_t			count;		//instance count
		};
		typedef TList<InstancingArea> InstancingRow;
		typedef TList<InstancingRow> InstancingAreaList;

		//TODO: procedural generate mesh? but not suitable for all cases like flowers
		struct InstancingLayer
		{
			HRESOURCE		mesh;
			AABB			meshBounding;
			AABB			boundig;	//layer bounding
			HIBUFFER		indices;
			HVERTEXSOURCE	vertices;
			InstancingAreaList	areas;
		};
		typedef FixedArray<InstancingLayer, GrassConfig::MAX_LAYERS> InstanceData;

	public:
		GrassResource()
			:GraphicsResource(GrassConsts::GRASS_RESOURCE_TYPE)
		{
			this->setUserFlag(UF_PHONY);
		}

		~GrassResource()
		{
			this->clear();
		}

		/** @brief  */
		inline const uint32*	getDensity(index_t layer) const
		{
			return mSharedResource != NULL ? static_cast<GrassSharedResource*>(mSharedResource)->getDensity(layer) : NULL;
		}

		/** @brief  */
		inline uint32		getTotalSize() const
		{
			return mSharedResource != NULL ? static_cast<GrassSharedResource*>(mSharedResource)->getSize() : 0;
		}

		/** @brief  */
		inline uint32		getSharedOffsetX() const
		{
			return mSharedResource != NULL ? static_cast<GrassSharedResource*>(mSharedResource)->getX() : 0;
		}

		/** @brief  */
		inline uint32		getSharedOffsetZ() const
		{
			return mSharedResource != NULL ? static_cast<GrassSharedResource*>(mSharedResource)->getZ() : 0;
		}

		/** @brief  */
		inline const Box3&	getLocalBounding() const
		{
			return mBounding;
		}

		/** @brief  */
		inline const HVERTEXSOURCE& getVertexSource(index_t layer) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			return mInstanceData[layer].vertices;
		}

		/** @brief  */
		inline const HIBUFFER& getIndexBuffer(index_t layer) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			return mInstanceData[layer].indices;
		}

		/** @brief  */
		inline const AABB& getLayerLocalBounding(index_t layer) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			return mInstanceData[layer].boundig;
		}

		/** @brief  */
		inline const AABB& getLayerMeshBounding(index_t layer) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			return mInstanceData[layer].meshBounding;
		}

		/** @brief  */
		inline size_t	getAreaCount() const
		{
#if BLADE_DEBUG
			for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
			{
				const InstancingAreaList& areas = mInstanceData[i].areas;
				assert(areas.size() > 0 && areas[0].size() == areas.size());
			}
#endif
			return mInstanceData[0].areas.size();
		}

		/** @brief  */
		inline const HVBUFFER& getAreaInstanceBuffer(index_t layer, index_t lx, index_t lz) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			const InstancingAreaList& areas = mInstanceData[layer].areas;
			assert(lx < areas.size() && lz < areas.size() && lx < areas[lz].size());
			return areas[lz][lx].buffer;
		}

		/** @brief  */
		inline const Box3& getAreaLocalBounding(index_t layer, index_t lx, index_t lz) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			const InstancingAreaList& areas = mInstanceData[layer].areas;
			assert(lx < areas.size() && lz < areas.size() && lx < areas[lz].size());
			return areas[lz][lx].bounding;
		}

		/** @brief  */
		inline size_t getAreaInstanceCount(index_t layer, index_t lx, index_t lz) const
		{
			assert(layer < GrassConfig::MAX_LAYERS);
			const InstancingAreaList& areas = mInstanceData[layer].areas;
			assert(lx < areas.size() && lz < areas.size() && lx < areas[lz].size());
			return areas[lz][lx].count;
		}

	protected:
		/** @brief  */
		inline void setInstanceData(const InstanceData& softData)
		{
			for (index_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
			{
				const InstancingLayer& layer = softData[i];
				mInstanceData[i] = layer;

				if (layer.vertices->getBufferSourceCount() == 0 || layer.indices == NULL)
					continue;
				InstancingAreaList& areas = mInstanceData[i].areas;
				for (index_t j = 0; j < areas.size(); ++j)
				{
					for (index_t k = 0; k < areas[j].size(); ++k)
					{
						HVBUFFER softBuffer = areas[j][k].buffer;
						if (softBuffer != NULL && areas[j][k].count != 0)
						{
							areas[j][k].buffer = GrassConfig::getSingleton().getInstanceBufferPool().get();
							IGraphicsResourceManager::getSingleton().cloneVertexBuffer(areas[j][k].buffer, *softBuffer, IGraphicsBuffer::GBU_STATIC);
						}
					}
				}
			}
		}

		/** @brief  */
		inline void clear()
		{
			//this is an recursion of unload resource.
			//usually clear() happens when the grass resource is unloading
			//unload another resource while unloading is un tested.
			IResourceManager& resMan = IResourceManager::getSingleton();
			resMan.unloadResource(mSharedResource);
			for (size_t i = 0; i < mInstanceData.size(); ++i)
			{
				resMan.unloadResource(mInstanceData[i].mesh);

				InstancingAreaList& areas = mInstanceData[i].areas;
				for (size_t j = 0; j < areas.size(); ++j)
				{
					for (size_t k = 0; k < areas[j].size(); ++k)
						GrassConfig::getSingleton().getInstanceBufferPool().add(areas[j][k].buffer);
				}
			}
		}

		Box3			mBounding;		//overall bounding for all layers
		InstanceData	mInstanceData;
		HRESOURCE		mSharedResource;
		friend class GrassSerializer;
	};
	
}//namespace Blade

#endif//__Blade_GrassResource_h__