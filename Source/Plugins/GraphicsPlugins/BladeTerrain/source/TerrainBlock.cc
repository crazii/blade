/********************************************************************
	created:	2010/05/13
	filename: 	TerrainBlock.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <interface/IGraphicsSystem.h>
#include <interface/IRenderQueue.h>
#include "TerrainBufferManager.h"
#include "TerrainBlock.h"
#include "BlockQuery.h"


namespace Blade
{
	static const uint BLOCK_UPDATE_LOD = 0x01;
	static const uint BLOCK_UPDATE_INDICES = 0x02;

	//////////////////////////////////////////////////////////////////////////
	TerrainBlock::TerrainBlock(index_t x,index_t z, ITerrainTile* parent)
		:mParent(parent)
		,mFixedLOD(INVALID_FIXED_LOD)
		,mLODLevel(0)
		,mLODDiffIndex(LODDI_NONE)
		,mMaterialLOD(0)
	{
		mUpdateFlags |= CUF_DEFAULT_VISIBLE | CUF_VISIBLE_UPDATE;
		mSpaceFlags = CSF_CONTENT | CSF_SHADOWCASTER;

		const TERRAIN_INFO& GTInfo = TerrainConfigManager::getSingleton().getTerrainInfo();
		mBlockIndex.mX = (uint16)x;
		mBlockIndex.mZ = (uint16)z;
		mBlockIndex.mIndex = (uint32)( mBlockIndex.mX + mBlockIndex.mZ*GTInfo.mBlocksPerTileSide);

		const TILE_INFO& tileInfo = mParent->getTileInfo();

		GraphicsGeometry& geometry = tileInfo.mRenderGeomBuffer[ this->getBlockIndex() ];
		geometry.reset();
		//setup render geometry
		geometry.useIndexBuffer(true);
		geometry.mIndexBuffer = NULL;
		geometry.mIndexCount = 0;
		geometry.mIndexStart = 0;
		geometry.mVertexSource = tileInfo.mVertexSource;
		geometry.mVertexDecl = TerrainConfigManager::getRenderType().getVertexDeclaration();
		geometry.mVertexCount = (uint32)TerrainConfigManager::getSingleton().getTerrainBlockVertexCount();
		geometry.mVertexStart = 0;
		geometry.mPrimitiveType = GraphicsGeometry::GPT_TRIANGLE_LIST;

#if BLADE_DEBUG
		mQueued = 0;
#endif

		if( IGraphicsSystem::getSingleton().getCurrentProfile() > BTString("2_0") )
			tileInfo.mScene->getMaterialLODUpdater()->addForLODUpdate(this);
	}

	TerrainBlock::~TerrainBlock()
	{
		if(mParent->getTileInfo().mScene != NULL)
			mParent->getTileInfo().mScene->getMaterialLODUpdater()->removeFromLODUpdate(this);
	}

	/************************************************************************/
	/* SpaceContent overrides                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					TerrainBlock::updateRender(IRenderQueue* queue)
	{
		const TILE_INFO& tileInfo = mParent->getTileInfo();
		assert(tileInfo.mMaterialLODCount == 0 || mMaterialLOD < tileInfo.mMaterialLODCount);

		if(tileInfo.mLODMaterials != NULL && mMaterialLOD < tileInfo.mMaterialLODCount)
		{
			TerrainConfigManager& tcm = static_cast<TerrainConfigManager&>(*mParent->getConfigManager());
			ITerrainBatchCombiner* combiner = tcm.getBatchCombiner();
			IRenderQueue* redirect = mParent->getTileInfo().mOutputBuffer;
			if (redirect != NULL && combiner->needCombine(queue) )
			{
#if BLADE_DEBUG
				//assert(mQueued == 0);
				mQueued = 1;
#endif
				redirect->addRenderable(this);
			}
			else
			{
				this->updateIndexBuffer();
				queue->addRenderable(this);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainBlock::visibleUpdateImpl(const Vector3& cameraPos)
	{
		uint8& updateMask = mParent->getTileInfo().mUpdatedMask[this->getBlockIndex()];
		if (updateMask & BLOCK_UPDATE_LOD)	//multiple camera update optimize.
			return;
		updateMask |= BLOCK_UPDATE_LOD;

#define ADAPTIVE_FIXED_LOD 0	//TODO: use adaptive fixed LOD will need morph height for adapted level.

#if !ADAPTIVE_FIXED_LOD 
		if (mFixedLOD != INVALID_FIXED_LOD)
			mLODLevel = mFixedLOD;
		else
#endif
		{
			//Vector3 CameraDir = cameraRotation*Vector3::NEGATIVE_UNIT_Z;
			//Vector3	Vec = mPostion - cameraPos;
			//scalar dist = Vec.dotProduct(CameraDir);
			//mLODLevel = TerrainConfigManager::getSingleton().getLODLevelByDistance(dist);

			//get LOD level only at horizontal dimension
			//to avoid LOD level gap, when height different is too large
			Vector3 position = mWorldAABB.getCenter();
			Vector2 pos2(position.x, position.z);
			Vector2 camPos2(cameraPos.x, cameraPos.z);
			scalar SqrDist = pos2.getSquaredDistance(camPos2);
			mLODLevel = static_cast<TerrainConfigManager*>(mParent->getConfigManager())->getLODLevelBySquaredDistance(SqrDist);
		}

#if ADAPTIVE_FIXED_LOD
		if (mFixedLOD != INVALID_FIXED_LOD)
		{
			if (mFixedLOD == TerrainConfigManager::getSingleton().getFlatLODLevel())
				mLODLevel = std::max<uint8>(mFixedLOD, mLODLevel);
			else //
				mLODLevel = std::min<uint8>(mFixedLOD, mLODLevel);
		}
#endif

#if BLADE_DEBUG
		if(mLastLODLevel != mLODLevel)
			mLastLODLevel = mLODLevel;
		mQueued = 0;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	bool					TerrainBlock::queryNearestPoint(SpaceQuery& query, scalar& distance) const
	{
		//transform ray to local space (actually it's tile's local space, not block's local space, because block's all vertices are in tile's space)
		StaticHandle<SpaceQuery> transformed = query.clone(this->getWorldTransform().getInverse());

		const TERRAIN_POSITION_DATA_Y* vposition = mParent->getSoftHeightPositionData();
		const TERRAIN_POSITION_DATA_XZ* hposition = TerrainBufferManager::getSingleton().getSoftHorizontalPositionBuffer();
		const TerrainQueryIndexGroup* group = mParent->getQueryIndexBuffer();
		size_t blockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();

		BlockQuery blockQuery;
		blockQuery.initialize(vposition + mVertexStart, hposition + mVertexStart, group, mBlockIndex.mX*blockSize, mBlockIndex.mZ*blockSize);

		AABB localAABB = this->getWorldAABB();
		localAABB.offset(-mParent->getStartPos());
		blockQuery.setAABB(localAABB);

		BlockVolumeQuery q(this->getWorldTransform()[3], *transformed, mLODLevel, (LOD_DI)mLODDiffIndex);
		q.mDistance = distance;
		bool ret = blockQuery.raycastPoint(q);
		if (ret)
		{
			//skip scale since not scale for terrain blocks
			distance = q.mDistance;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					TerrainBlock::intersectTriangles(SpaceQuery& query, POS_VOL contentPos/* = PV_INTERSECTED*/) const
	{
		bool result = false;

		//transform the volume from world space to local space
		StaticHandle<SpaceQuery> transformed = query.clone( this->getWorldTransform().getInverse() );

		const TERRAIN_POSITION_DATA_Y* vposition =  mParent->getSoftHeightPositionData();
		const TERRAIN_POSITION_DATA_XZ* hposition = TerrainBufferManager::getSingleton().getSoftHorizontalPositionBuffer();
		const TerrainQueryIndexGroup* group = mParent->getQueryIndexBuffer();
		size_t blockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();

		BlockQuery blockQuery;
		blockQuery.initialize(vposition+mVertexStart, hposition+mVertexStart, group, mBlockIndex.mX*blockSize, mBlockIndex.mZ*blockSize);

		AABB localAABB = this->getWorldAABB();
		localAABB.offset(-mParent->getStartPos());
		blockQuery.setAABB(localAABB);

		BlockVolumeQuery q(this->getWorldTransform()[3], *transformed, mLODLevel, (LOD_DI)mLODDiffIndex);
		result = blockQuery.intersect(q, contentPos);
		return result;
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void					TerrainBlock::setVertexOffsetCount(index_t startVertex, size_t count)
	{
		mVertexStart = startVertex;
		mVertexCount = (uint16)count;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainBlock::updateIndexBuffer(bool force/* = false*/)
	{
		uint8& updateMask = mParent->getTileInfo().mUpdatedMask[this->getBlockIndex()];
		if ((updateMask & BLOCK_UPDATE_INDICES) && !force)	//multiple camera update optimize
			return;
		updateMask |= BLOCK_UPDATE_INDICES;

		TerrainConfigManager& tcm = static_cast<TerrainConfigManager&>(*mParent->getConfigManager());
		ILODComparator* LODCmp = tcm.getIndexGenerator();
		LODDiff diff;
		FixedLODDiff fixedDiff;

		TerrainBlock* up = mParent->getTerrainBlock((int)this->getBlockX(), (int)this->getBlockZ()-1);
		if (up != NULL && (fixedDiff.t=(int8)LODCmp->compareLOD(up->getCurrentLODLevel(), mLODLevel)) > 0 && !up->isFixedLOD())
			diff.setUpDiff();

		TerrainBlock* left = mParent->getTerrainBlock((int)this->getBlockX()-1, (int)this->getBlockZ());
		if (left != NULL && (fixedDiff.l=(int8)LODCmp->compareLOD(left->getCurrentLODLevel(), mLODLevel)) > 0 && !left->isFixedLOD())
			diff.setLeftDiff();

		TerrainBlock* down = mParent->getTerrainBlock((int)this->getBlockX(), (int)this->getBlockZ()+1);
		if (down != NULL && (fixedDiff.b=(int8)LODCmp->compareLOD(down->getCurrentLODLevel(), mLODLevel)) > 0 && !down->isFixedLOD() )
			diff.setDownDiff();

		TerrainBlock* right = mParent->getTerrainBlock((int)this->getBlockX()+1, (int)this->getBlockZ());
		if (right != NULL && (fixedDiff.r=(int8)LODCmp->compareLOD(right->getCurrentLODLevel(), mLODLevel)) > 0 && !right->isFixedLOD())
			diff.setRightDiff();

		GraphicsGeometry& geometry = mParent->getTileInfo().mRenderGeomBuffer[this->getBlockIndex()];

		if (mFixedLOD != INVALID_FIXED_LOD && fixedDiff.isValid() && mLODLevel == tcm.getFlatLODLevel())
		{
			//adaption only work for one side(flat or cliff), disable flat LOD adaption
			int8 specialDiff = (int8)LODCmp->compareLOD(tcm.getCliffLODLevel(), mLODLevel);
			if (std::abs(specialDiff) > 1)
			{
				assert(mLODLevel >= 1u);
				if (fixedDiff.l == specialDiff && left != NULL && left->isFixedLOD())
					fixedDiff.l = 0;
				if (fixedDiff.t == specialDiff && up != NULL && up->isFixedLOD() )
					fixedDiff.t = 0;
				if (fixedDiff.r == specialDiff &&  right != NULL && right->isFixedLOD())
					fixedDiff.r = 0;
				if (fixedDiff.b == specialDiff && down != NULL && down->isFixedLOD())
					fixedDiff.b = 0;
			}
		}
		if (mFixedLOD != INVALID_FIXED_LOD && mLODLevel == mFixedLOD && fixedDiff.isValid())
			geometry.mIndexBuffer = mParent->getFixedIndexBuffer(mLODLevel, fixedDiff, this->getBlockIndex(), geometry.mIndexStart, geometry.mIndexCount);
		else
		{
			mLODDiffIndex = (uint8)LODDiff::generateDifferenceIndex(diff);
#if BLADE_DEBUG
			if (mLastLODDiffIndex != mLODDiffIndex)
				mLastLODDiffIndex = mLODDiffIndex;
#endif
			geometry.mIndexBuffer = mParent->getIndexBuffer(mLODLevel, (LOD_DI)mLODDiffIndex, this->getBlockIndex(), geometry.mIndexStart, geometry.mIndexCount);
		}

		geometry.mIndexCount = (uint32)geometry.mIndexBuffer->getIndexCount();
		geometry.mVertexStart = (uint32)mVertexStart;
		geometry.mVertexCount = (uint32)mVertexCount;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainBlock::updateMaterial()
	{
		const TILE_INFO& tileInfo = mParent->getTileInfo();
		assert(tileInfo.mMaterialLODCount > 0);
		if (mMaterialLOD >= tileInfo.mMaterialLODCount)
			mMaterialLOD = tileInfo.mMaterialLODCount - 1u;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainBlock::cacheOut()
	{
		assert(this->getSpace() == NULL);
		//mParent->getTileInfo().mScene->getMaterialLODUpdater()->removeFromLODUpdate(this);
		this->setElement(NULL);
	}
	
}//namespace Blade