/********************************************************************
	created:	2011/04/05
	filename: 	BatchCombiner.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <LoopContainers.h>
#include <interface/IPlatformManager.h>

#include <interface/IRenderQueue.h>
#include <interface/IRenderSchemeManager.h>
#include <Technique.h>
#include "TerrainBatchCombiner.h"
#include "../TerrainBlock.h"


namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//perform a per tile sorting in group, optimize for shared texture bounding
	//////////////////////////////////////////////////////////////////////////
	struct RenderOperationCache
	{
		RenderOperation* renderables;
		size_t count;
	};
	struct RenderableCacheGroup
	{
		Vector<RenderOperationCache> list;
		size_t total;
	};
	bool EmptyBatchCombiner::processQueue(IRenderQueue* queue)
	{
		if (mBuffers.size() == 0)
			return false;

		if (!this->EmptyBatchCombiner::needCombine(queue))
			return false;

		Vector<RenderableCacheGroup> caches;

		{
			RenderableCacheGroup g;
			g.total = 0;
			RenderOperationCache c;
			c.renderables = NULL;
			c.count = 0;
			g.list.resize(mBuffers.size(), c);
			caches.resize(8, g);
		}

		//update LOD index only
		for( size_t i = 0; i < mBuffers.size(); ++i )
		{
			IRenderQueue* createdBuffer = mBuffers[i];
			const size_t groupCount = createdBuffer->getGroupCount();

			if(caches.size() < groupCount )
				caches.resize(groupCount*2);

			for(size_t j = 0; j < groupCount; ++j )
			{
				IRenderGroup* group = createdBuffer->getRenderGroup(j);
				RenderOperation* rops = NULL;
				size_t size = group->getROPArray(rops);
				if( size == 0 || rops == NULL)
					continue;

				for( size_t n = 0; n < size; ++n)
					static_cast<TerrainBlock*>(rops[n].renderable)->updateIndexBuffer();

				caches[j].total += size;
				RenderOperationCache c = { rops , size };
				caches[j].list[i] = c;
			}
		}

		for (size_t i = 0; i < caches.size(); ++i)
		{
			const RenderableCacheGroup& g = caches[i];
			if(g.total == 0)
				continue;

			IRenderGroup* group = queue->getRenderGroup(i);
			assert(group != NULL);
			group->reserve(g.total);
			for (size_t j = 0; j < g.list.size(); ++j)
				group->addRenderables(g.list[j].renderables, g.list[j].count);
		}

		return true;
	}


	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	FullBatchCombiner::FullBatchCombiner(bool combineForAllPass/* = true*/)
	{
		mIndexType = IIndexBuffer::IT_32BIT;
		mCombineForAll = combineForAllPass;
		this->FullBatchCombiner::releaseIndexBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	FullBatchCombiner::~FullBatchCombiner()
	{
		this->FullBatchCombiner::releaseIndexBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	bool		FullBatchCombiner::processQueue(IRenderQueue* queue)
	{
		if (mBuffers.size() == 0)
			return false;
		if (!this->FullBatchCombiner::needCombine(queue))
			return false;

		size_t batchCount = 0;
		size_t IndexOffset = 0;
		size_t totalIndexCount = 0;

		size_t maxGroup = 0;
		for( size_t i = 0; i < mBuffers.size(); ++i )
			maxGroup = std::max<size_t>(mBuffers[i]->getGroupCount(), maxGroup);

		TerrainConfigManager& tcm = TerrainConfigManager::getSingleton();
		//calc total bytes
		{
			for (size_t i = 0; i < mBuffers.size(); ++i)
			{
				IRenderQueue* createdBuffer = mBuffers[i];
				const size_t groupCount = createdBuffer->getGroupCount();
				assert(groupCount == queue->getGroupCount());

				for (size_t j = 0; j < groupCount; ++j)
				{
					IRenderGroup* group = createdBuffer->getRenderGroup(j);

					RenderOperation* blocklist = NULL;
					size_t count = group->getROPArray(blocklist);
					if (count == 0)
						continue;
					for (size_t n = 0; n < count; ++n)
					{
						TerrainBlock* block = static_cast<TerrainBlock*>(blocklist[n].renderable);
						block->updateIndexBuffer();
						const GraphicsGeometry& geom = block->TerrainBlock::getGeometry();
						totalIndexCount += geom.mIndexCount;
					}
				}
			}
		}
		if (totalIndexCount == 0)
			return true;

		IRenderQueue::EUsage usage = queue->getUsage();
		index_t index = queue->getIndex();

		const HIBUFFER& indexBuffer = this->setupIndexBuffer(totalIndexCount, usage, index);
		if (indexBuffer == NULL)
			return false;

		size_t blockVertexCount = tcm.getTerrainBlockVertexCount();
		size_t indexSize = IndexBufferHelper::calcIndexSize(mIndexType);
		size_t totalBytes = totalIndexCount*indexSize;
		
		//note: lock with size bytes so minimize the upload data
		char* cache = (char*)indexBuffer->lock(0, totalBytes, IGraphicsBuffer::GBLF_DISCARDWRITE);
		{
			//BLADE_LW_PROFILING_FUNCTION();
			for( size_t i = 0; i < mBuffers.size(); ++i )
			{
				IRenderQueue* createdBuffer = mBuffers[i];
				const size_t groupCount = createdBuffer->getGroupCount();
				assert(groupCount == queue->getGroupCount() );

				for(size_t j = 0; j < groupCount; ++j )
				{
					IRenderGroup* group = createdBuffer->getRenderGroup(j);

					RenderOperation* blocklist = NULL;
					size_t count = group->getROPArray(blocklist);
					if( count == 0 )
						continue;

					//blocks within the same group has the same parent
					//so set any one's

					ITerrainTile* tile = static_cast<TerrainBlock*>(blocklist[0].renderable)->getParentTile();						
					size_t indexCount = 0;
					assert( batchCount < mCombinedGeom[usage].capacity() );
					size_t IndexStart = IndexOffset;
					HMATERIALINSTANCE material = HMATERIALINSTANCE::EMPTY;

					AABB mergedAAB(BT_NULL);
					for( size_t n = 0; n < count; ++n)
					{
						TerrainBlock* block = static_cast<TerrainBlock*>(blocklist[n].renderable);
						const GraphicsGeometry& geom = block->getGeometry();
						indexCount += geom.mIndexCount;
						mergedAAB.merge(block->getWorldAABB());

						if (material == NULL)	//use any block's material with this group
							material = block->getMaterialInstance();

						IIndexBuffer* srcIndexBuffer = const_cast<IIndexBuffer*>(geom.mIndexBuffer);
						index_t blockIndex = block->getBlockIndex();

						const char* src = (char*)srcIndexBuffer->lock(IGraphicsBuffer::GBLF_READONLY);
						const uint32 srcIndexOffset = uint32(blockIndex*blockVertexCount);	//value offset
						char* dest = cache + IndexOffset*indexSize;

						IPlatformManager::prefetch<PM_WRITE>(dest);
						IPlatformManager::prefetch<PM_READ>(src);

						IndexBufferHelper::copyIndices(dest, mIndexType, src, srcIndexBuffer->getIndexType(), srcIndexOffset, geom.mIndexCount);

						IndexOffset += geom.mIndexCount;
						dest += geom.mIndexCount * indexSize;
						srcIndexBuffer->unlock();
					}
					group->clear();

					if( tile != NULL )
					{
						//TerrainBatchRenderable* cr = BLADE_NEW TerrainBatchRenderable(static_cast<TerrainTile*>(tile), tile->getTileInfo().mTileMaterialInstance);
						//use block material which contains right active texture states for material LOD
						TerrainBatchRenderable* cr = BLADE_NEW TerrainBatchRenderable(static_cast<TerrainTile*>(tile), material);
						cr->setupGeometry(indexBuffer, indexCount, IndexStart);
						cr->setWorldBounding(mergedAAB);

						mCombinedGeom[usage].push_back(cr);
						queue->getRenderGroup(j)->addRenderable(cr);
					}

					++batchCount;
				}//for each group

			}//for each buffers
		}
		indexBuffer->unlock();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	const HIBUFFER&		FullBatchCombiner::setupIndexBuffer(size_t count, IRenderQueue::EUsage usage, index_t renderBufferIndex)
	{
		assert(usage >= IRenderQueue::RQU_START && usage < IRenderQueue::RQU_COUNT);

		IndexBufferList& buffers = mIndexBuffer[usage];
		SizeList& sizes = mTotalCount[usage];

		size_t newSize = std::min((index_t)IRenderQueue::MAX_INDEX, renderBufferIndex + renderBufferIndex / 2 + 1);
		if (buffers.size() <= renderBufferIndex)
			buffers.resize(newSize);
		if (sizes.size() <= renderBufferIndex)
			sizes.resize(newSize);

		if (buffers[renderBufferIndex] == NULL || buffers[renderBufferIndex]->getIndexCount() < count)
		{
			if (count == 0)
				return HIBUFFER::EMPTY;

			sizes[renderBufferIndex] = count;
			//here we use  an approximate maximum index count:
			//calculate the index count as the whole tile in LOD level 0
			//size_t indexCount = TerrainConfigManager::getSingleton().getMaxBlockIndexCount()*TerrainConfigManager::getSingleton().getBlocksPerTile()*mTotalCount;

			//add new method to calculate index count precisely, in order to save memory
			//3264K on optimized triangle index for 1x1 tile --> 537K
			//29376K on optimized triangle index for 3x3 tile --> 806K

			//size_t indexCount = TerrainConfigManager::getSingleton().calculateTotalIndexCount();
			//if (indexCount != 0)
			{
				mIndexType = IndexBufferHelper::calcIndexType(ITerrainConfigManager::getSingleton().getTileVertexCount());
				buffers[renderBufferIndex] = IGraphicsResourceManager::getSingleton().createIndexBuffer(NULL, mIndexType, count, IGraphicsBuffer::GBU_DYNAMIC_WRITE);
			}

			size_t materialLODs = TerrainConfigManager::getSingleton().getTileMaterial()->getActiveTechnique()->getLODSetting().size();
			materialLODs = materialLODs < 1 ? 1 : materialLODs;
			count *= materialLODs;
			mCombinedGeom[usage].reserve(count);
		}
		//else
		//	assert(sizes[renderBufferIndex] >= TerrainConfigManager::getSingleton().getCurrentTileCount());

		return buffers[renderBufferIndex];
	}

}//namespace Blade
