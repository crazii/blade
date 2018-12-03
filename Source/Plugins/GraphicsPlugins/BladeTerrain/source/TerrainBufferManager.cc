/********************************************************************
	created:	2010/05/14
	filename: 	TerrainBufferManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ExceptionEx.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include "TerrainBufferManager.h"

namespace Blade
{
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TerrainBufferManager::TerrainBufferManager()
		:mIndexGroup(NULL)
		,mQueryIndexGroup(NULL)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainBufferManager::~TerrainBufferManager()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	const TerrainIndexGroup*		TerrainBufferManager::addIndexGroup()
	{
		if( mIndexGroup == NULL )
			mIndexGroup = TerrainConfigManager::getSingleton().getIndexGenerator()->createTileIndexBuffer();
		mIndexGroup->mCount.increment();
		return mIndexGroup;
	}

	//////////////////////////////////////////////////////////////////////////
	const TerrainQueryIndexGroup*	TerrainBufferManager::addQueryIndexGroup()
	{
		if( mQueryIndexGroup == NULL )
			mQueryIndexGroup = TerrainConfigManager::getSingleton().getIndexGenerator()->createBlockQueryIndexBuffer();
		mQueryIndexGroup->mCount.increment();
		return mQueryIndexGroup;
	}

	//////////////////////////////////////////////////////////////////////////
	const TerrainFixedIndexGroup*	TerrainBufferManager::addFixedIndexGroup()
	{
		int diff = (int)TerrainConfigManager::getSingleton().getFlatLODLevel() - (int)TerrainConfigManager::getSingleton().getCliffLODLevel();
		if (diff <= 0)
			return NULL;

		if (mFixedIndexGroup == NULL)
			mFixedIndexGroup = TerrainConfigManager::getSingleton().getIndexGenerator()->createFixedIndexBuffer();
		mFixedIndexGroup->mCount.increment();
		return mFixedIndexGroup;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainBufferManager::releaseIndexGroup(const TerrainIndexGroup* group)
	{
		if( group == NULL || mIndexGroup == NULL || group != mIndexGroup )
			return;

		if( mIndexGroup->mCount.decrement() == 0 )
		{
			BLADE_DELETE mIndexGroup;
			mIndexGroup = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainBufferManager::releaseQueryIndexGroup(const TerrainQueryIndexGroup* group)
	{
		if( group == NULL || mQueryIndexGroup == NULL || group != mQueryIndexGroup )
		{
			assert(false);
			return;
		}

		if( mQueryIndexGroup->mCount.decrement() == 0 )
		{
			BLADE_DELETE mQueryIndexGroup;
			mQueryIndexGroup = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainBufferManager::releaseFixedIndexGroup(const TerrainFixedIndexGroup* group)
	{
		if (group == NULL || mFixedIndexGroup == NULL || group != mFixedIndexGroup)
			return;

		if (mFixedIndexGroup->mCount.decrement() == 0)
		{
			BLADE_DELETE mFixedIndexGroup;
			mFixedIndexGroup = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainBufferManager::createVertexBuffer(
		IGraphicsResourceManager& manager, HVBUFFER& position, HVBUFFER& normal,
		size_t TileSize, TileAABBList& localAABB,
		const int16* heightBuffer, const uint8* normalBuffer) const
	{
		//BLADE_LW_PROFILING_FUNCTION( POSITION_Y_INIT );
		const size_t TileVertexCount = TerrainConfigManager::getSingleton().getTileVertexCount();
		const size_t blockPerSide = TerrainConfigManager::getSingleton().getBlocksPerTileSide();
		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();

		if (heightBuffer != NULL && localAABB.size() != blockPerSide)
		{
			localAABB.clear();
			localAABB.resize(blockPerSide, TileAABBList::value_type(blockPerSide));
		}

		TERRAIN_POSITION_DATA_Y* TileVertices = heightBuffer == NULL ? NULL : BLADE_TMP_ALLOCT(TERRAIN_POSITION_DATA_Y, TileVertexCount);
		TERRAIN_NORMAL_DATA* TmpNormal = normalBuffer == NULL ? NULL : BLADE_TMP_ALLOCT(TERRAIN_NORMAL_DATA, TileVertexCount);
		TERRAIN_NORMAL_DATA* TileNormals = TmpNormal;

		size_t	VertexCount = 0;
		const int16*	HeightData = heightBuffer;
		const uint8*	NormalData = normalBuffer;
		size_t	BlockX	= BlockSize/2;
		size_t	BlockZ	= BlockSize/2;
		const scalar	HalfSizeX = scalar(BlockSize/2);
		const scalar	HalfSizeZ = scalar(BlockSize/2);
		const scalar	HalfSizeXInv = -HalfSizeX;
		const scalar	HalfSizeZInv = -HalfSizeZ;
		const size_t	SourceSize = TileSize + 1;
		const size_t	VertexBlockSize = BlockSize + 1;

		TERRAIN_POSITION_DATA_Y* BlockVertices = TileVertices;
		IIndexGenerator* generator = TerrainConfigManager::getSingleton().getIndexGenerator();

		for( size_t z = 0; z < blockPerSide; ++z)
		{
			for( size_t x = 0; x < blockPerSide; ++x )
			{
				const int16* BlockHeightData = HeightData;
				const uint8* BlockNormalData = NormalData;

				scalar	minBlockHeight = 0;
				scalar	maxBlockHeight = 0;

				TERRAIN_POSITION_DATA_Y* currentBlockVertices = BlockVertices;
				BLADE_UNREFERENCED(currentBlockVertices);

				for( size_t pz = 0; pz < VertexBlockSize; ++pz)
				{
					for( size_t px = 0; px < VertexBlockSize; ++px )
					{
						if (TileVertices != NULL)
						{
							TERRAIN_POSITION_DATA_Y& positionData = *BlockVertices;
							positionData.setHeight(BlockHeightData[px]);
							scalar fRealHeight = (scalar)positionData.getHeight();
							if (minBlockHeight > fRealHeight)
								minBlockHeight = fRealHeight;
							else if (maxBlockHeight < fRealHeight)
								maxBlockHeight = fRealHeight;
						}

						if( TileNormals != NULL )
						{
							TERRAIN_NORMAL_DATA& normalData = *TileNormals;
							{
								normalData.x = BlockNormalData[px*3+0];
								normalData.y = BlockNormalData[px*3+1];
								normalData.z = BlockNormalData[px*3+2];
								normalData.w = 0;
							}
							++TileNormals;
						}
						++VertexCount;
						++BlockVertices;
					}

					//move block data to next row
					BlockHeightData += SourceSize;
					BlockNormalData += SourceSize*3;
				}

#if BLADE_TERRAIN_ENABLE_UVSCALE
				if (TileVertices != NULL)
				{
					//note: use max scale per row/col to avoid LOD uv overlap(stretching artifact) & scale bump
					for (size_t px = 0; px < VertexBlockSize; ++px)
						currentBlockVertices[0 * VertexBlockSize + px].setVTiling(0);

					for (size_t pz = 1; pz < VertexBlockSize; ++pz)
					{
						scalar vScale = 1;
						//use per row max vScale
						for (size_t px = 0; px < VertexBlockSize; ++px)
						{
							const TERRAIN_POSITION_DATA_Y& positionData = currentBlockVertices[pz*VertexBlockSize + px];
							const TERRAIN_POSITION_DATA_Y& up = currentBlockVertices[(pz-1)*VertexBlockSize + px];
							//dy/dx (dx=1)
							vScale = std::max(vScale, std::abs(std::ceil(positionData.getHeight() - up.getHeight())));
						}

						for (size_t px = 0; px < VertexBlockSize; ++px)
						{
							TERRAIN_POSITION_DATA_Y& positionData = currentBlockVertices[pz*VertexBlockSize + px];
							const TERRAIN_POSITION_DATA_Y& up = currentBlockVertices[(pz - 1)*VertexBlockSize + px];
							////fix tiling seam between blocks
							//if (px == 0 && x > 0 && vScale == 1)
							//{
							//	const TERRAIN_POSITION_DATA_Y* leftNeighbor = TileVertices + (z*blockPerSide + x - 1)*(VertexBlockSize*VertexBlockSize);
							//	const TERRAIN_POSITION_DATA_Y& left = leftNeighbor[pz*VertexBlockSize + VertexBlockSize - 1];
							//	const TERRAIN_POSITION_DATA_Y& leftup = leftNeighbor[(pz-1)*VertexBlockSize + VertexBlockSize - 1];
							//	positionData.setVTiling(up.getVTiling() + std::abs(left.getVTiling() - leftup.getVTiling()));
							//}
							//else
								positionData.setVTiling(up.getVTiling() + vScale);
						}
					}

					for (size_t pz = 0; pz < VertexBlockSize; ++pz)
						currentBlockVertices[pz * VertexBlockSize + 0].setUTiling(0);

					for (size_t px = 1; px < VertexBlockSize; ++px)
					{
						scalar uScale = 1;
						//use per col max uScale
						for (size_t pz = 0; pz < VertexBlockSize; ++pz)
						{
							const TERRAIN_POSITION_DATA_Y& positionData = currentBlockVertices[pz*VertexBlockSize + px];
							const TERRAIN_POSITION_DATA_Y& left = currentBlockVertices[pz*VertexBlockSize + px - 1];
							//dy/dz (dz=1)
							uScale = std::max(uScale, std::abs(std::ceil(positionData.getHeight() - left.getHeight())));
						}
						for (size_t pz = 0; pz < VertexBlockSize; ++pz)
						{
							TERRAIN_POSITION_DATA_Y& positionData = currentBlockVertices[pz*VertexBlockSize + px];				
							const TERRAIN_POSITION_DATA_Y& left = currentBlockVertices[pz*VertexBlockSize + px - 1];
							////fix tiling seam between blocks
							//if (pz == 0 && z > 0 && uScale == 1)
							//{
							//	const TERRAIN_POSITION_DATA_Y* upNeighbor = TileVertices + ((z-1)*blockPerSide + x)*(VertexBlockSize*VertexBlockSize);
							//	const TERRAIN_POSITION_DATA_Y& up = upNeighbor[(VertexBlockSize - 1)*VertexBlockSize + px];
							//	const TERRAIN_POSITION_DATA_Y& upleft = upNeighbor[(VertexBlockSize - 1)*VertexBlockSize + px - 1];
							//	positionData.setUTiling(left.getUTiling() + std::abs(up.getUTiling() - upleft.getUTiling()));
							//}
							//else
								positionData.setUTiling(left.getUTiling() + uScale);
						}
					}
				}
#endif
				if (TileVertices != NULL)
				{
					localAABB[z][x].aab.set(HalfSizeXInv, minBlockHeight, HalfSizeZInv, HalfSizeX, maxBlockHeight, HalfSizeZ);
					index_t fixedLOD = INVALID_INDEX;
					ITerrainTile::getFixedLOD(localAABB[z][x].aab, fixedLOD);
					localAABB[z][x].fixedLOD = fixedLOD;
				}

				//move data to next row
				HeightData	+= BlockSize;
				NormalData  += BlockSize*3;

				//move to next col of block
				BlockX += BlockSize;

			}//for x in col of blocks

			//move data to next row (already increased one row(count-1) while on row iteration)
			HeightData	+= 1 + (BlockSize-1)*SourceSize;
			NormalData += (1 + (BlockSize-1)*SourceSize)*3;

			//move to next row of block
			BlockZ += BlockSize;
			BlockX = BlockSize/2;

		}//for z in row of blocks

		//total vertices count
		assert( VertexCount == (TileSize+blockPerSide)*(TileSize+blockPerSide) );
		assert(VertexCount == (blockPerSide*blockPerSide)*(VertexBlockSize*VertexBlockSize));

		//calc low LOD heights
		if (TileVertices != NULL)
		{
			for (size_t z = 0; z < blockPerSide; ++z)
			{
				for (size_t x = 0; x < blockPerSide; ++x)
				{
					TERRAIN_POSITION_DATA_Y* currentBlockVertices = TileVertices + (z*blockPerSide + x)*(VertexBlockSize*VertexBlockSize);

					//use fixed low LOD to prevent morphing
					for (size_t pz = 0; pz < VertexBlockSize; ++pz)
					{
						for (size_t px = 0; px < VertexBlockSize; ++px)
						{
							TERRAIN_POSITION_DATA_Y& positionData = currentBlockVertices[pz*VertexBlockSize + px];
							//tile edges or fixed LOD block inner vertices or fixed LOD block edges with fixed LOD neighbors
							if ( (z == 0 && pz == 0) || (z == blockPerSide-1 && pz == VertexBlockSize-1) || (x == 0 && px == 0) || (x == blockPerSide-1 && px == VertexBlockSize-1) ||
								(localAABB[z][x].fixedLOD != INVALID_INDEX
								&& (pz > 0 || localAABB[z - 1][x].fixedLOD != INVALID_INDEX)
								&& (pz < VertexBlockSize - 1 || localAABB[z + 1][x].fixedLOD != INVALID_INDEX)
								&& (px > 0 || localAABB[z][x - 1].fixedLOD != INVALID_INDEX)
								&& (px < VertexBlockSize - 1 || localAABB[z][x + 1].fixedLOD != INVALID_INDEX) )
								)
							{
								int16 height = positionData.getHeight16();
								if (height >= 0)
									height = int16((height & 0xFFF8) | (int16)localAABB[z][x].fixedLOD);
								else
									height = int16((height & 0xFFF8) | ((-(int16)localAABB[z][x].fixedLOD) & 0x7));
								positionData.setHeightLowLOD(height);
							}
							else
							{
								size_t LODLevel = 0;
								int16 LowLODheight = generator->getLowLODHeight(heightBuffer, LODLevel, x, z, px, pz);
								assert(LODLevel <= TerrainConfigManager::getSingleton().getMaxLODLevel());
								assert(LODLevel <= 7);
								if (LowLODheight >= 0)
									LowLODheight = int16((LowLODheight & 0xFFF8) | (int16)LODLevel);
								else
									LowLODheight = int16((LowLODheight & 0xFFF8) | ((-(int16)LODLevel) & 0x7));
								positionData.setHeightLowLOD(LowLODheight);
							}
						}
					}
				}
			}
		}

		IGraphicsBuffer::USAGE usage = ITerrainConfigManager::getSingleton().isEditingModeEnabled() ? IGraphicsBuffer::GBU_DYNAMIC : (IGraphicsBuffer::GBU_CPU_READ | IGraphicsBuffer::GBAF_READ_WRITE);
		if (normalBuffer == NULL)
		{
			assert(normal == NULL);
			assert(!TerrainConfigManager::getSingleton().isUseVertexNormal());
		}
		else if (normal == NULL || normal->getVertexCount() != TileVertexCount)
		{
			assert(TerrainConfigManager::getSingleton().isUseVertexNormal());
			normal = manager.createVertexBuffer(TmpNormal, sizeof(TERRAIN_NORMAL_DATA), TileVertexCount, usage);
		}

		if (heightBuffer != NULL)
		{
			if (position == NULL || position->getVertexCount() != TileVertexCount)
				position = manager.createVertexBuffer(TileVertices, sizeof(TERRAIN_POSITION_DATA_Y), TileVertexCount, usage);
			else
			{
				void* data = position->lock(IGraphicsBuffer::GBLF_WRITEONLY);
				std::memcpy(data, TileVertices, sizeof(TERRAIN_POSITION_DATA_Y)*TileVertexCount);
				position->unlock();
			}
		}

		BLADE_TMP_FREE(TileVertices);
		BLADE_TMP_FREE(TmpNormal);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	const HVBUFFER&		TerrainBufferManager::getHorizontalPositionBuffer()
	{
		if( mPositionXZBuffer != NULL )
			return mPositionXZBuffer;

		//BLADE_LW_PROFILING_FUNCTION( POSITION_XZ_INIT );
		size_t TileSize = TerrainConfigManager::getSingleton().getTerrainTileSize();
		const size_t TileVertexCount = TerrainConfigManager::getSingleton().getTileVertexCount();
		const size_t blockPerSide = TerrainConfigManager::getSingleton().getBlocksPerTileSide();
		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();

		TERRAIN_POSITION_DATA_XZ* TileVertices = BLADE_TMP_ALLOCT(TERRAIN_POSITION_DATA_XZ, TileVertexCount);

		size_t	VertexCount = 0;
		size_t	BlockX	= BlockSize/2;
		size_t	BlockZ	= BlockSize/2;
		const size_t	VertexBlockSize = BlockSize + 1;

		TERRAIN_POSITION_DATA_XZ* BlockVertices = TileVertices;//+BlockStart;

		for( size_t z = 0; z < blockPerSide; ++z)
		{
			for( size_t x = 0; x < blockPerSide; ++x )
			{
				for( size_t pz = 0; pz < VertexBlockSize; ++pz)
				{
					for( size_t px = 0; px < VertexBlockSize; ++px )
					{
						TERRAIN_POSITION_DATA_XZ& position = *BlockVertices;
						position.x = (uint8)x;
						position.z = (uint8)z;
						position.lx = (uint8)px;
						position.lz = (uint8)pz;
						++VertexCount;
						++BlockVertices;
					}
				}

				//move to next col of block
				BlockX += BlockSize;

			}//for x in col of blocks

			//move to next row of block
			BlockZ += BlockSize;
			BlockX = BlockSize/2;

		}//for z in row of blocks

		//total vertices count
		assert( VertexCount == (TileSize+blockPerSide)*(TileSize+blockPerSide) );
		BLADE_UNREFERENCED(TileSize);

		IGraphicsBuffer::USAGE usage = ITerrainConfigManager::getSingleton().isEditingModeEnabled() ? IGraphicsBuffer::GBU_DYNAMIC : IGraphicsBuffer::GBU_CPU_READ;
		mPositionXZBuffer = IGraphicsResourceManager::getSingleton().createVertexBuffer(TileVertices, sizeof(TERRAIN_POSITION_DATA_XZ), TileVertexCount, usage);
		BLADE_TMP_FREE(TileVertices);
		return mPositionXZBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainBufferManager::releaseHorizontalPositionBuffer()
	{
		if( mPositionXZBuffer == NULL )
			return;
		assert(mPositionXZBuffer.refcount() == 1);
		mPositionXZBuffer.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	const TERRAIN_POSITION_DATA_XZ*	TerrainBufferManager::getSoftHorizontalPositionBuffer()
	{
		assert(mPositionXZBuffer != NULL);
		//GBU_CPU_READ buffer can be accessed anytime
		//GBU_DYNAMIC has a copy CPU data that can be accessed anytime
		const void* src = mPositionXZBuffer->lock(IGraphicsBuffer::GBLF_READONLY);
		mPositionXZBuffer->unlock();
		return (const TERRAIN_POSITION_DATA_XZ*)src;
	}
	
}//namespace Blade