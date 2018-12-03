/********************************************************************
created:	2017/10/20
filename: 	GrassContent.h
author:		Crazii
purpose:	
*********************************************************************/
#ifndef __Blade_GrassContent_h__
#define __Blade_GrassContent_h__
#include <SpaceContent.h>
#include <interface/public/graphics/IIndexBuffer.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <interface/public/graphics/IVertexSource.h>
#include <interface/ISpace.h>
#include "GrassRenderable.h"
#include "GrassResource.h"
#include "GrassElement.h"

namespace Blade 
{
	class GrassResource;

	///minimal drawing unit, corresponding to the InstancingArea of grass resource
	///each cluster/area has one single instanced draw call per layer
	class GrassCluster : public SpaceContentBase, public Allocatable
	{
	public:
		GrassCluster();
		~GrassCluster();

		/************************************************************************/
		/* SpaceContent interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual	uint32			getAppFlag() const { return GrassConfig::getSingleton().getRenderType().getAppFlag().getMask(); }

		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* queue);

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/** @brief  */
		void			initialize(const Vector3& pos, index_t x, index_t z, const GrassResource* res);

		/** @brief  */
		void			cleanup();

		/** @brief  */
		inline const Matrix44&		getWorldTransform() const
		{
			return mTransform;
		}

		/** @brief  */
		inline bool		hasValidGeometry() const
		{
			for (size_t i = 0; i < GrassConfig::MAX_LAYERS; ++i)
			{
				if (mRenderable[i]->isGeometryValid())
					return true;
			}
			return false;
		}

	protected:
		GrassRenderable* mRenderable[GrassConfig::MAX_LAYERS];
		HVERTEXSOURCE	mSource[GrassConfig::MAX_LAYERS];
		Matrix44		mTransform;
	};

	///minimal streaming unit (for grass element)
	///corresponding to the whole grass resource data (all areas)
	class GrassContent : public SpaceContentBase, public Allocatable
	{
	public:
		GrassContent() { mUpdateFlags = CUF_NONE | CUF_HIDDEN;  mSpaceFlags = CSF_ELEMENT | CSF_CONTENT | CSF_VIRTUAL; }
		virtual ~GrassContent()
		{
			GrassConfig& cfg = GrassConfig::getSingleton();

			size_t count = mClusters.size();
			for (size_t i = 0; i < count; ++i)
			{
				for (size_t j = 0; j < count; ++j)
				{
					cfg.getClusterPool().add(mClusters[i][j]);
				}
			}
		}

		/************************************************************************/
		/* SpaceContent interface                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		virtual	uint32			getAppFlag() const { return GrassConfig::getSingleton().getRenderType().getAppFlag().getMask(); }

		/*
		@describe add renderables to render buffer
		@return
		@note  this function will be called if CUF_*_VISIBLE is set
		*/
		virtual void			updateRender(IRenderQueue* /*buffer*/) {}

		/**
		@describe 
		@param
		@return
		*/
		virtual void			notifySpaceChange()
		{
			size_t count = mClusters.size();
			if (count == 0)
				return;

			GrassElement* elem = static_cast<GrassElement*>(this->getElement());
			index_t startx = elem->getOffsetX();
			index_t startz = elem->getOffsetZ();

			uint32* partitionMasks = GrassConfig::getSingleton().getInstancingPartition();
			size_t partitionSize = GrassConfig::getSingleton().getInstancingPartitionSize();
			uint32 prefix = this->getSpacePartitionMask()&ISpace::EXTRA_PARTITION_MASK;

			for (size_t z = 0; z < count; ++z)
			{
				assert(mClusters[z].size() == count);
				for (size_t x = 0; x < count; ++x)
				{
					GrassCluster* cluster = mClusters[z][x];
					if (cluster->getSpace() != this->getSpace())
					{
						assert(cluster->getSpace() == NULL || this->getSpace() == NULL);

						if (cluster->getSpace() != NULL)
							cluster->getSpace()->removeContent(cluster);
						if (this->getSpace() != NULL)
						{
							size_t cx = startx + x;
							size_t cz = startz + z;
							assert(cx < partitionSize && cz < partitionSize);
							uint32& partitionMask = partitionMasks[cz*partitionSize+cx];

							this->getSpace()->addContent(cluster, partitionMask | prefix, true);

							if (partitionMask == ISpace::INVALID_PARTITION)
								partitionMask = cluster->getSpacePartitionMask()&ISpace::PARTITION_MASK;
							else
								assert(partitionMask == (cluster->getSpacePartitionMask()&ISpace::PARTITION_MASK));
						}
					}
				}
			}
		}

		/** @brief  */
		inline void			initialize(const Vector3& pos, const AABB& localAABB, const GrassResource* res)
		{
			assert(!localAABB.isNull() && !localAABB.isInfinite());
			AABB worldAAB = localAABB;
			worldAAB.offset(pos);
			this->setWorldAABB(worldAAB);
			
			GrassConfig& cfg = GrassConfig::getSingleton();

			size_t count = res->getAreaCount();
			size_t oldCount = mClusters.size();
			if (oldCount > count)
			{
				for (size_t i = count; i < oldCount; ++i)
				{
					for (size_t j = 0; j < oldCount; ++j)
					{
						cfg.getClusterPool().add(mClusters[i][j]);
						mClusters[i][j] = NULL;
					}
				}

				for (size_t i = 0; i < count; ++i)
				{
					for (size_t j = count; j < oldCount; ++j)
					{
						cfg.getClusterPool().add(mClusters[i][j]);
						mClusters[i][j] = NULL;
					}
				}
			}

			size_t areaSize = GrassConfig::getSingleton().getInstancingSize();

			mClusters.resize(count);
			for (size_t i = 0; i < count; ++i)
			{
				mClusters[i].resize(count);
				for (size_t j = 0; j < count; ++j)
				{
					if (mClusters[i][j] == NULL)
						mClusters[i][j] = cfg.getClusterPool().get();

					mClusters[i][j]->setElement(mElement);
					mClusters[i][j]->initialize(pos + Vector3( scalar(j*areaSize), 0, scalar(i*areaSize)), j, i, res);
				}
			}
		}

		/** @brief  */
		inline void			cleanup()
		{
			GrassConfig& cfg = GrassConfig::getSingleton();

			size_t count = mClusters.size();
			for (size_t i = 0; i < count; ++i)
			{
				for (size_t j = 0; j < count; ++j)
				{
					cfg.getClusterPool().add(mClusters[i][j]);
				}
			}
		}

	protected:
		typedef TList<GrassCluster*> ClusterRow;
		typedef TList<ClusterRow> Clusters;
		Clusters mClusters;
	};
	
}//namespace Blade 


#endif // __Blade_GrassContent_h__
