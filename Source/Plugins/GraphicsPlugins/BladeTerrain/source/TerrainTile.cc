/********************************************************************
created:	2010/05/04
filename: 	TerrainTile.cc
author:		Crazii
purpose:
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <interface/public/graphics/IGraphicsResourceManager.h>
#include <interface/IEventManager.h>
#include <interface/IResourceManager.h>
#include <ConstDef.h>

#include <interface/IMaterialManager.h>
#include <Element/GraphicsElement.h>
#include <interface/IRenderScheme.h>
#include <Material.h>
#include <Technique.h>
#include <Pass.h>

#include <TerrainUtility.h>
#include "TerrainElement.h"
#include "TerrainTile.h"
#include "TerrainResource.h"
#include "TerrainConfigManager.h"
#include "TerrainBufferManager.h"
#include "TerrainTextureManager.h"

namespace Blade
{

#if COMPACT_TERRAIN_POSITION
	fp32 STerrainPositionElementY::getHeight() const
	{
		//[-1,1]*scale
		//manually normalize height
		const scalar	NormalizedHeight = (scalar)height / TERRAIN_MOD_HEIGHT;	//[-1,1];
		const fp32*		TerrainScale = ITerrainConfigManager::getSingleton().getTerrainScale();	//apply scale in CPU end. GPU is applied by shader constant
		return NormalizedHeight*TerrainScale[1];
	}
#else
	inline fp32 STerrainPositionElementY::getHeight() const
	{
		const fp32*		TerrainScale = ITerrainConfigManager::getSingleton().getTerrainScale();
		return height*TerrainScale[1];
	}
#endif

	//sampler uniforms (match names in shader)
	static const TString BLEND_MAP = BTString("blendMap");
	static const TString LAYER_MAP = BTString("layerMap");
	static const TString ATLAS_MAP = BTString("diffAtlas");
	static const TString NORMAL_MAP = BTString("normalMap");
	static const TString NORMAL_ATLAS_MAP = BTString("normalAtlas");
	
	bool TerrainTile::msPerframeConstantUpdated = false;
	ShaderVariableMap::Handles TerrainTile::msCameraPosHandle;

	//////////////////////////////////////////////////////////////////////////
	TerrainTile::TerrainTile()
		:mTileX(uint16(-1))
		,mTileZ(uint16(-1))
	{
		mSpaceFlags = CSF_ELEMENT;
		mUpdateFlags = CUF_VISIBLE_UPDATE | CUF_DEFAULT_VISIBLE;

		mTileInfo.mConfigManager = ITerrainConfigManager::getSingletonPtr();
		mTileInfo.mScene = NULL;
		mTileInfo.mIndexGroup = NULL;
		mTileInfo.mFixedGroup = NULL;
		mTileInfo.mQueryIndexGroup = NULL;
		mTileInfo.mTileMaterial = IMaterialManager::getSingleton().getMaterial(BTString("Default"));
		mTileInfo.mLODMaterials = NULL;
		mTileInfo.mVertexSource = IVertexSource::create();
		mTileInfo.mOutputBuffer = NULL;

		mTileInfo.mHeightData = NULL;
		mTileInfo.mNormalData = NULL;
		mTileInfo.mBlendData = NULL;
		mTileInfo.mLayerData = NULL;
		mTileInfo.mUpdatedMask = NULL;
		
		mTileInfo.mWorldTransform = Matrix44::IDENTITY;
		mTileInfo.mRenderGeomBuffer = NULL;
		mTileInfo.mMaterialLODQualityLimit = TML_LOW;
		mTileInfo.mMaterialLODCount = 0;
		mTileInfo.mUsed = false;

		mTextureInited = false;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainTile::~TerrainTile()
	{
		this->cleanUp();

		TerrainBufferManager::getSingleton().releaseIndexGroup(mTileInfo.mIndexGroup);
		if (mTileInfo.mQueryIndexGroup != NULL)
		{
			TerrainBufferManager::getSingleton().releaseQueryIndexGroup(mTileInfo.mQueryIndexGroup);
			mTileInfo.mQueryIndexGroup = NULL;
		}
		TerrainBufferManager::getSingleton().releaseFixedIndexGroup(mTileInfo.mFixedGroup);

		//mTileInfo.mTileMaterialInstance->unload();
		mNormalAtlas.clear();
		mAtlas.clear();
		mTileInfo.mVertexSource->clearAllSource();
	}

	/************************************************************************/
	/* ITerrainTile interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	TerrainBlock*	TerrainTile::getTerrainBlock(int x, int y) const
	{
		int blockCount = (int)this->getConfigManager()->getTerrainInfo().mBlocksPerTileSide;

		int16 neighborX = 0;
		int16 neighborY = 0;

		if (x - blockCount >= 0)
		{
			x -= blockCount;
			neighborX = 1;
		}
		else if (x < 0)
		{
			x += blockCount;
			neighborX = -1;
		}

		if (y - blockCount >= 0)
		{
			y -= blockCount;
			neighborY = 1;
		}
		else if (y < 0)
		{
			y += blockCount;
			neighborY = -1;
		}
		//only neighbor tiles can be handled. if x > blockCount * 2, x < -blockCount, then this is not gonna work.
		assert(x >= 0 && x < blockCount);
		assert(y >= 0 && y < blockCount);

		if (neighborX == 0 && neighborY == 0)
			return this->getTerrainBlockUnsafe(x, y);
		else
		{
			TerrainTile* tile = this->getNeighbor(neighborX, neighborY);
			return tile == NULL ? NULL : tile->getTerrainBlockUnsafe(x, y);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	const TERRAIN_POSITION_DATA_Y*	TerrainTile::getSoftHeightPositionData() const
	{
		const HVBUFFER& vbuffer = mTileInfo.mVertexSource->getBuffer(TSI_POSITION_Y);
		const void* ret = vbuffer->lock(IGraphicsBuffer::GBLF_READONLY);
		vbuffer->unlock();
		return (const TERRAIN_POSITION_DATA_Y*)ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::releaseIndexBuffer()
	{
		TerrainBufferManager::getSingleton().releaseIndexGroup(mTileInfo.mIndexGroup);
		mTileInfo.mIndexGroup = NULL;

		TerrainBufferManager::getSingleton().releaseQueryIndexGroup(mTileInfo.mQueryIndexGroup);
		mTileInfo.mQueryIndexGroup = NULL;

		TerrainBufferManager::getSingleton().releaseFixedIndexGroup(mTileInfo.mFixedGroup);
		mTileInfo.mFixedGroup = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::buildIndexBuffer()
	{
		TerrainConfigManager& tcm = TerrainConfigManager::getSingleton();
		TerrainBufferManager& tbm = TerrainBufferManager::getSingleton();

		bool built = false;
		if (mTileInfo.mIndexGroup == NULL)
		{
			assert(tcm.getTerrainInfo().mTileSize != 0);
			mTileInfo.mIndexGroup = tbm.addIndexGroup();
			built = true;
		}

		//check the presence of physics service
		//if the the physics service is available, don't create query data
		if (built && tcm.isQueryDataEnabled())
		{
			if (mTileInfo.mQueryIndexGroup == NULL)
				mTileInfo.mQueryIndexGroup = tbm.addQueryIndexGroup();
		}

		if (mTileInfo.mFixedGroup == NULL)
			mTileInfo.mFixedGroup = tbm.addFixedIndexGroup();

		const size_t blockPerSide = tcm.getBlocksPerTileSide();
		for (size_t z = 0; z < blockPerSide; ++z)
		{
			for (size_t x = 0; x < blockPerSide; ++x)
			{
				TerrainBlock* block = mBlockData[z][x];
				block->updateIndexBuffer(true);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::rebuildBlocks()
	{
		assert(TerrainConfigManager::getSingleton().isEditingModeEnabled());
		size_t blockCount = TerrainConfigManager::getSingleton().getBlocksPerTileSide();
		//assert(blockCount != mBlockData.size());	//maybe LOD change
		BLADE_UNREFERENCED(blockCount);

		TILE_INFO info = mTileInfo;
		const TERRAIN_INFO& gti = TerrainConfigManager::getSingleton().getTerrainInfo();

		//note: old position data will be cleared on setup (const HVBUFFER& is invalid)
		HVBUFFER position;
		HVBUFFER normal;
		TileAABBList TileAABB;
		const uint8* normalBuffer = TerrainConfigManager::getSingleton().isUseVertexNormal() ? info.mNormalData : NULL;
		TerrainBufferManager::getSingleton().createVertexBuffer(IGraphicsResourceManager::getSingleton(),
			position, normal, gti.mTileSize, TileAABB, info.mHeightData, normalBuffer);

		TextureState* ts = mTileInfo.mTileMaterialInstance->getTextureState();
		HTEXTURE hBlend = ts->getTextureStage(BLEND_MAP)->getTexture();
		HTEXTURE hNormal = ts->getTextureStage(NORMAL_MAP)->getTexture();
		HTEXTURE hLayer = ts->getTextureStage(LAYER_MAP)->getTexture();

		//block ABB change after size change, but current quad tree space is fixed in size
		//TODO: complete quad tree
		this->setup(info.mHeightData, info.mBlendData, info.mNormalData, info.mLayerData,
			gti.mTileSize, info.mWorldTransform.getTranslation(), TileAABB,
			position, normal,
			hBlend, hNormal, hLayer,
			mAtlas, mNormalAtlas,
			mTextureInited, true, false);
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::updateMaterialLOD(bool updateMat/* = false*/)
	{
		ETerrainMaterialLOD materialLOD;

		if (ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_HIGH)
			materialLOD = TML_HIGH;	//this call will add normal texture to normal atlas
		else if (ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_MIDDLE)
			materialLOD = TML_MIDDLE;
		else if (TerrainConfigManager::getSingleton().isUseVertexNormal())
			materialLOD = TML_LOW;
		else
			materialLOD = TML_MIDDLE;

		if (!mTextureInited && (materialLOD != TML_LOW))
		{
			this->updateMaterialInstances();
			return;
		}

		mTileInfo.mMaterialLODQualityLimit = (uint8)materialLOD;
		TextureState* ts = mTileInfo.mTileMaterialInstance->getTextureState();

		bool success = false;
		if (materialLOD == TML_LOW)//vertex normal
		{
			//clear normal map & atlas
			if (mNormalAtlas != NULL)
				mNormalAtlas.clear();
			ts->getTextureStage(NORMAL_ATLAS_MAP)->setTexture(HTEXTURE::EMPTY);
			ts->getTextureStage(NORMAL_MAP)->setTexture(HTEXTURE::EMPTY);

			//TML_LOW use vertex normal
			//assert( TerrainConfigManager::getSingleton().isUseVertexNormal() );

			//normal map <=> vertex normal switch 
			//create normal vertex buffer
			if (mTileInfo.mVertexSource->getBuffer(TSI_NORMAL) == NULL)
			{
				HRESOURCE terrainRes = this->getElement()->getBoundResource();
				assert(terrainRes != NULL);
				TerrainResource* res = terrainRes;
				if (res->getNormalBuffer() == NULL)	//internal data maybe cleared on non editing runtime
				{
					ParamList param;
					param[BTString("NORMAL_ONLY")] = true;
					IResourceManager::getSingleton().reloadResourceSync(terrainRes, this->getElement()->getResourcePath(), &param);
				}
				const uint8* normalData = res->getNormalBuffer();
				this->updateNormalBuffer(normalData);
				res->clearup();
			}
			success = true;
		}
		else
		{
			assert(!TerrainConfigManager::getSingleton().isUseVertexNormal());
			//normal map <=> vertex normal switch 
			//release vertex normal buffer 
			mTileInfo.mVertexSource->setSource(TSI_NORMAL, HVBUFFER::EMPTY);
			//create normal map if needed
			if (ts->getTextureStage(NORMAL_MAP)->getTexture() == NULL)
			{
				//read vertex normal and set as normal map, 
				//scale from tileSize+1 to normal map size (usually tileSize)
				size_t tileSize = TerrainConfigManager::getSingleton().getTerrainTileSize();

				//resource should be already loaded and returned immediately
				HRESOURCE terrainRes = this->getElement()->getBoundResource();
				TerrainResource* res = terrainRes;
				if (res->getNormalBuffer() == NULL) //internal data maybe cleared on non editing runtime
				{
					ParamList param;
					param[BTString("NORMAL_ONLY")] = true;
					IResourceManager::getSingleton().reloadResourceSync(terrainRes, this->getElement()->getResourcePath(), &param);
				}
				const uint8* normalData = res->getNormalBuffer();
				HIMAGE img = TerrainTextureManager::getSingleton().createNormalImage(normalData, tileSize + 1);
				res->clearup();

				//create normal map
				HTEXTURE normalMap = TerrainTextureManager::getSingleton().createNormalTexture(img);
				ts->getTextureStage(NORMAL_MAP)->setTexture(normalMap);
			}
		}

		if (materialLOD == TML_MIDDLE)
		{
			if (mNormalAtlas != NULL)
			{
				mNormalAtlas.clear();
				ts->getTextureStage(NORMAL_ATLAS_MAP)->setTexture(HTEXTURE::EMPTY);
				success = true;
			}
			else
				success = false;
		}
		else if (materialLOD == TML_HIGH)
		{
			if (mNormalAtlas == NULL)
			{
				TStringParam list;

				const TStringList& diffuseList = mAtlas->getTextureList();
				for (size_t i = 0; i < diffuseList.size(); ++i)
				{
					const TString& texture = diffuseList[i];
					const TString normalTexture = TerrainUtility::getNormalTextureName(texture);
					list.push_back(normalTexture);
				}
				mNormalAtlas.bind(TerrainTextureManager::getSingleton().buildTerrainAtlas(list, TERRAIN_NORMAL_FORMAT));
				success = true;
			}
			else
			{
				assert(ts->getTextureStage(NORMAL_ATLAS_MAP)->getTexture() != NULL);
				success = false;
			}
			ts->getTextureStage(NORMAL_ATLAS_MAP)->setTexture(mNormalAtlas->getAtlasTexture());
		}

		if (success || updateMat)
		{
			//material/technique changed, LOD setting may changes too
			this->updateMaterialInstances();

			IRenderScene* scene = mTileInfo.mScene;

			for (size_t i = 0; i < mBlockData.size(); ++i)
			{
				BlockRowData& row = mBlockData[i];
				for (size_t j = 0; j < row.size(); ++j)
				{
					TerrainBlock* block = row[j];
					if (updateMat)
						block->updateMaterial();
					scene->getMaterialLODUpdater()->updateLOD(block);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainTile::visibleUpdate(const ICamera* current, const ICamera* main, const ICamera::VISIBILITY /*visibility*/)
	{
		if (current == main && !msPerframeConstantUpdated)
		{
			msPerframeConstantUpdated = true;	//per shader constant don't need multiple tile update

			//const size_t blockPerSide = this->getConfigManager()->getBlocksPerTileSide();
			const POINT3& cameraPos = current->getEyePosition();

			//use fixed camera pos for all passes to avoid morphing divergence. i.e shadow passes
			Vector4 pos4 = Vector4(cameraPos, 1);
			msCameraPosHandle.setData(&pos4, sizeof(pos4));
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::setup(const int16* height, const uint8* blendData, const uint8* normalData, TERRAIN_LAYER* layerData,
		size_t size, const Vector3& startPos, const TileAABBList& localAABBList,
		const HVBUFFER& position, const HVBUFFER& normal,
		const HTEXTURE& blendMap, const HTEXTURE& normalMap, const HTEXTURE& layerMap,
		const HTERRAINATLAS& atlas, const HTERRAINATLAS& normalAtlas,
		bool textureInited, bool rebuildBuffer/* = false*/, bool rebuildTexture/* = false*/)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		assert(position != NULL);
		assert(normal != NULL || !TerrainConfigManager::getSingleton().isUseVertexNormal());
		assert(TerrainConfigManager::getSingleton().isUseVertexNormal() || normal == NULL);

		mTileInfo.mScene = static_cast<GraphicsElement*>(mElement)->getRenderScene();

		if (height == NULL || normalData == NULL || blendData == NULL)
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid terrain data."));

		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		if (size == 0 || size % BlockSize != 0)
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("terrain size must be multiple of") + TStringHelper::fromInt((int)BlockSize));

		const size_t blockPerSide = TerrainConfigManager::getSingleton().getBlocksPerTileSide();

		if (TerrainConfigManager::getSingleton().isEditingModeEnabled())
		{
			mTileInfo.mHeightData = height;
			if (mTileInfo.mLayerData == NULL || layerData != NULL)
				mTileInfo.mLayerData = layerData;
		}
		else
		{
			mTileInfo.mHeightData = NULL;
			mTileInfo.mLayerData = NULL;
		}

		assert(localAABBList.size() == blockPerSide);
		if (mBlockData.size() == 0 || rebuildBuffer)
		{
			if (mBlockData.size() != 0)
				this->cleanUp();
			mBlockData.resize(blockPerSide, BlockRowData(blockPerSide, NULL));

			//clear old index buffer
			if (mTileInfo.mIndexGroup != NULL)
			{
				TerrainBufferManager::getSingleton().releaseIndexGroup(mTileInfo.mIndexGroup);
				mTileInfo.mIndexGroup = NULL;
			}

			if (mTileInfo.mQueryIndexGroup != NULL)
			{
				TerrainBufferManager::getSingleton().releaseQueryIndexGroup(mTileInfo.mQueryIndexGroup);
				mTileInfo.mQueryIndexGroup = NULL;
			}

			if (rebuildBuffer)
			{
				//clear old vertex data buffer if size diff
				mTileInfo.mVertexSource->clearAllSource();
			}
		}

		if (mTileInfo.mUpdatedMask == NULL)
			mTileInfo.mUpdatedMask = BLADE_RES_ALLOCT(uint8, blockPerSide*blockPerSide);
		std::memset(mTileInfo.mUpdatedMask, 0, blockPerSide*blockPerSide);

		if (mTileInfo.mRenderGeomBuffer == NULL)
			mTileInfo.mRenderGeomBuffer = BLADE_RES_ALLOCT(GraphicsGeometry, blockPerSide*blockPerSide);

		//mTerrainAABB.set(BT_NULL);
		mTileInfo.mWorldTransform.setTranslation(startPos);

		//build vertex buffer directly

		//vertex horizontal position
		mTileInfo.mVertexSource->setSource(TSI_POSITION_XZ, TerrainBufferManager::getSingleton().getHorizontalPositionBuffer());
		//vertex vertical position
		mTileInfo.mVertexSource->setSource(TSI_POSITION_Y, position);
		//vertex normal
		mTileInfo.mVertexSource->setSource(TSI_NORMAL, normal);

		{
			//BLADE_LW_PROFILING( TEXTURE_INIT );
			this->initTextureData(blendMap, normalMap, layerMap, atlas, normalAtlas, blendData, normalData, layerData, textureInited, rebuildTexture);
		}
		//setup shader params
		this->setupShaderParameters();

		//create render buffer
		if (mTileInfo.mOutputBuffer != NULL)
			TerrainConfigManager::getSingleton().getBatchCombiner()->releaseCustomRenderBuffer(mTileInfo.mOutputBuffer);
		mTileInfo.mOutputBuffer = TerrainConfigManager::getSingleton().getBatchCombiner()->createCustomRenderBuffer();

		{
			//BLADE_LW_PROFILING( POSITION_INIT );
			const fp32*		TerrainScale = TerrainConfigManager::getSingleton().getTerrainScale();
			size_t			BlockX = BlockSize / 2;
			size_t			BlockZ = BlockSize / 2;
			const size_t	VertexBlockSize = BlockSize + 1;
			const size_t	VertexblockCount = VertexBlockSize*VertexBlockSize;
			size_t			BlockVertexStart = 0;
			AABB	localAABB;

			IVertexDeclaration* decl = TerrainConfigManager::getRenderType().getVertexDeclaration();
			BLADE_UNREFERENCED(decl);
			assert(decl->getVertexSize(TSI_POSITION_Y) == sizeof(TERRAIN_POSITION_DATA_Y));
			assert(decl->getVertexSize(TSI_POSITION_XZ) == sizeof(TERRAIN_POSITION_DATA_XZ));
			const uint32* partitionMasks = TerrainConfigManager::getSingleton().getBlockPartitions();
			index_t cliffLOD = TerrainConfigManager::getSingleton().getCliffLODLevel();

			for (size_t z = 0; z < blockPerSide; ++z)
			{
				for (size_t x = 0; x < blockPerSide; ++x)
				{
					TerrainBlock*& block = mBlockData[z][x];
					if (block == NULL)
					{
						block = BLADE_NEW TerrainBlock(x, z, this);
						block->setVertexOffsetCount(BlockVertexStart, VertexblockCount);
					}
					//else
					//	block->updateMaterial();

					BlockVertexStart += VertexblockCount;

					AABB blockWorldAAB = localAABBList[z][x].aab;
					blockWorldAAB.offset(this->getStartPos() + Vector3((BlockX)*TerrainScale[0], 0.0f, (BlockZ)*TerrainScale[2]));
					block->setWorldAABB(blockWorldAAB);
					block->setFixedLODLevel((uint8)localAABBList[z][x].fixedLOD, (uint8)cliffLOD);
					block->setSpacePartitionMask(partitionMasks[x + z*blockPerSide]);
					block->setElement(mElement);
					//init LOD
					block->visibleUpdateImpl(blockWorldAAB.getCenter());

					localAABB.merge(blockWorldAAB);

					//move to next col of block
					BlockX += BlockSize;

				}//for x in col of blocks

				 //move to next row of block
				BlockZ += BlockSize;
				BlockX = BlockSize / 2;

			}//for z in row of blocks

			localAABB.offset(-this->getStartPos());	//world to local
			this->setLocalAABB(localAABB);
			this->setPosition(this->getStartPos());
		}

		//build index buffer
		{
			//BLADE_LW_PROFILING( INDEX_BUFFER );
			this->buildIndexBuffer();
		}

		//update indices on init LOD
		for (size_t i = 0; i < blockPerSide; ++i)
		{
			for (size_t j = 0; j < blockPerSide; ++j)
				mBlockData[i][j]->updateIndexBuffer();
		}

		std::memset(mTileInfo.mUpdatedMask, 0, blockPerSide*blockPerSide);
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::loadMaterial()
	{
		TerrainType& renderType = TerrainConfigManager::getRenderType();
		mTileInfo.mTileMaterial = renderType.getMaterial();
		if (mTileInfo.mTileMaterial == NULL)
		{
			assert(false);
			BLADE_EXCEPT(EXC_NEXIST, BTString("terrain material is NULL."));
		}

		if (!msCameraPosHandle.isValid())
		{
			for (size_t i = 0; i < mTileInfo.mTileMaterial->getTechniqueCount(); ++i)
				mTileInfo.mTileMaterial->getTechnique(i)->getShaderVariableHandle(msCameraPosHandle, BTString("cameraPos"), SCT_FLOAT4, 1);
		}

		mTileInfo.mTileMaterialInstance.bind(BLADE_NEW MaterialInstance(mTileInfo.mTileMaterial));

		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		bool loaded = mTileInfo.mTileMaterialInstance->loadSync();
		assert(loaded);
		BLADE_UNREFERENCED(loaded);
		this->updateMaterialInstances();
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::updateVertexBuffer(size_t StartX, size_t StartZ, size_t SizeX, size_t SizeZ)
	{
		const int16* data = this->getTileInfo().mHeightData;
		const size_t BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
		//vertex count per block side
		const size_t VertexBlockSize = BlockSize + 1;
		//vertex count per block
		const size_t VertexblockCount = VertexBlockSize*VertexBlockSize;
		index_t cliffLOD = TerrainConfigManager::getSingleton().getCliffLODLevel();
		const size_t TileSize = TerrainConfigManager::getSingleton().getTerrainInfo().mTileSize;
		const size_t blockPerSide = TerrainConfigManager::getSingleton().getTerrainInfo().mBlocksPerTileSide;

		//block index:
		//expand the block size by 1, to update the shared edge
		size_t blockStartX;
		size_t blockStartZ;
		size_t blockEndX;
		size_t blockEndZ;

		if (StartX != 0 && StartX%BlockSize == 0)
			blockStartX = StartX / BlockSize - 1;
		else
			blockStartX = StartX / BlockSize;

		if (StartZ != 0 && StartZ%BlockSize == 0)
			blockStartZ = StartZ / BlockSize - 1;
		else
			blockStartZ = StartZ / BlockSize;

		const size_t EndX = StartX + SizeX;
		if (EndX%BlockSize == 0)
			blockEndX = std::min(EndX / BlockSize + 1, blockPerSide - 1);
		else
			blockEndX = std::min(EndX / BlockSize, blockPerSide - 1);

		const size_t EndZ = StartZ + SizeZ;
		if (EndZ%BlockSize == 0)
			blockEndZ = std::min(EndZ / BlockSize + 1, blockPerSide - 1);
		else
			blockEndZ = std::min(EndZ / BlockSize, blockPerSide - 1);

		const size_t	totalblockCount = (blockEndZ - blockStartZ + 1)*blockPerSide - blockStartX - (blockPerSide - blockEndX - 1);

		const size_t	BlockStart = (blockStartZ/*-1*/)*blockPerSide + blockStartX/*-1*/;
		//size of the height data
		const size_t	SourceSize = TileSize + 1;
		//vertex count per terrain tile
		//const size_t	VertexSize = TileSize + blockPerSide;
		const index_t	CurBlockVertex = BlockStart*VertexblockCount;
		const size_t	TotalVertexSize = totalblockCount*VertexblockCount * sizeof(TERRAIN_POSITION_DATA_Y);
		const size_t	VertexOffset = CurBlockVertex * sizeof(TERRAIN_POSITION_DATA_Y);

		//const int16*	HeightData = data + CurBlockVertex;
		const int16*	HeightData = data + blockStartZ*BlockSize*SourceSize + blockStartX*BlockSize;

		TERRAIN_POSITION_DATA_Y* TileVertices = NULL;
		TileVertices = (TERRAIN_POSITION_DATA_Y*)mTileInfo.mVertexSource->getBuffer(TSI_POSITION_Y)->lock(VertexOffset, TotalVertexSize, IGraphicsBuffer::GBLF_NORMAL);

		typedef TempSet<TerrainBlock*> FixedLODBlocks;
		FixedLODBlocks fixedLODBlocks;
		TERRAIN_POSITION_DATA_Y* BlockVertices = TileVertices;

		for (size_t z = blockStartZ; z <= blockEndZ; ++z)
		{
			size_t xStart = ((z == blockStartZ) ? blockStartX : 0);
			size_t xEnd = ((z == blockEndZ) ? blockEndX : blockPerSide - 1);

			for (size_t x = xStart; x <= xEnd; ++x)
			{
				if (x >= blockStartX && x <= blockEndX)
				{
					TerrainBlock* block = mBlockData[z][x];
					assert(x == block->getBlockX() && z == block->getBlockZ());
					const int16* BlockHeightData = HeightData;

					scalar	minBlockHeight = 0;
					scalar	maxBlockHeight = 0;

					//Only update edge if block is expanded
					bool leftExpanded = (x == StartX / BlockSize - 1);
					bool rightExpanded = (x == EndX / BlockSize + 1);
					bool topExpanded = (z == StartZ / BlockSize - 1);
					bool bottomExpaned = (z == EndZ / BlockSize + 1);

					for (size_t pz = 0; pz < VertexBlockSize; ++pz)
					{
						for (size_t px = 0; px < VertexBlockSize; ++px)
						{
							assert(TotalVertexSize >(uintptr_t)BlockVertices - (uintptr_t)(TileVertices));

							bool leftEdge = px == 0;
							bool rightEdge = px == VertexBlockSize - 1;
							bool topEdge = pz == 0;
							bool bottomEdge = pz == VertexBlockSize - 1;

							if (((!leftExpanded || rightEdge) && (!rightExpanded || leftEdge)
								&& (!topExpanded || bottomEdge) && (!bottomExpaned || topEdge)) )
							{
								TERRAIN_POSITION_DATA_Y& position = *BlockVertices;
								position.setHeight(BlockHeightData[px]);

								scalar fRealHeight = position.getHeight();
								if (minBlockHeight > fRealHeight)
									minBlockHeight = fRealHeight;
								else if (maxBlockHeight < fRealHeight)
									maxBlockHeight = fRealHeight;

								assert(!isnan(minBlockHeight));
								assert(!isnan(maxBlockHeight));
							}

							++BlockVertices;
						}

						//move block data to next row
						BlockHeightData += SourceSize;
					}

					block->updateWorldAABBHeight(mPosition.y + minBlockHeight, mPosition.y + maxBlockHeight);
					size_t LOD = INVALID_INDEX;
					ITerrainTile::getFixedLOD(block->getWorldAABB(), LOD);
					block->setFixedLODLevel((uint8)LOD, (uint8)cliffLOD);

					fixedLODBlocks.insert(block);
					//don't update neighbor tile. TODO?
					if (x > 0)
						fixedLODBlocks.insert(mBlockData[z][x - 1]);
					if (x < blockPerSide - 1)
						fixedLODBlocks.insert(mBlockData[z][x + 1]);
					if (z > 0)
						fixedLODBlocks.insert(mBlockData[z - 1][x]);
					if (z < blockPerSide - 1)
						fixedLODBlocks.insert(mBlockData[z + 1][x]);

					//TODO: world AABB need full rebuild, especially if block AABB is smaller than before
					mWorldAABB.merge(block->getWorldAABB());

					assert(block->getSpace() == this->getSpace());
					//update immediately & strict fit
					if (this->getSpace() != NULL)
					{
						this->getSpace()->removeContent(block);
						this->getSpace()->addContent(block, block->getSpacePartitionMask(), true);
					}
				}
				else
					BlockVertices += VertexblockCount;

				//move data to next col
				HeightData += BlockSize;

			}//for x in col of blocks

			 //move data to next row (already increased one row(count-1) while on row iteration)
			HeightData += 1 + (BlockSize - 1)*SourceSize;

		}//for z in row of blocks

		assert(TotalVertexSize == (uintptr_t)BlockVertices - (uintptr_t)(TileVertices));

		mTileInfo.mVertexSource->getBuffer(TSI_POSITION_Y)->unlock();

		 //update low LOD heights. note that updated blocks will affect neighbors
		TileVertices = (TERRAIN_POSITION_DATA_Y*)mTileInfo.mVertexSource->getBuffer(TSI_POSITION_Y)->lock(IGraphicsBuffer::GBLF_NORMAL);
		IIndexGenerator* generator = TerrainConfigManager::getSingleton().getIndexGenerator();
		for (FixedLODBlocks::iterator i = fixedLODBlocks.begin(); i != fixedLODBlocks.end(); ++i)
		{
			TerrainBlock* block = *i;
			assert(block != NULL && block->getParentTile() == this);
			index_t x = block->getBlockX();
			index_t z = block->getBlockZ();
			TERRAIN_POSITION_DATA_Y* currentBlockVertices = TileVertices + (z*blockPerSide + x)*(VertexBlockSize*VertexBlockSize);
			for (size_t pz = 0; pz < VertexBlockSize; ++pz)
			{
				for (size_t px = 0; px < VertexBlockSize; ++px)
				{
					TERRAIN_POSITION_DATA_Y& positionData = currentBlockVertices[pz*VertexBlockSize + px];
					//tile edges or fixed LOD block inner vertices or fixed LOD block edges with fixed LOD neighbors
					if ((z == 0 && pz == 0) || (z == blockPerSide - 1 && pz == VertexBlockSize - 1) || (x == 0 && px == 0) || (x == blockPerSide - 1 && px == VertexBlockSize - 1) ||
						(block->isFixedLOD() &&
						(pz > 0 || mBlockData[z - 1][x]->isFixedLOD())
						&& (pz < VertexBlockSize - 1 || mBlockData[z + 1][x]->isFixedLOD())
						&& (px > 0 || mBlockData[z][x - 1]->isFixedLOD())
						&& (px < VertexBlockSize - 1 || mBlockData[z][x + 1]->isFixedLOD()))
						)
					{
						int16 height = positionData.getHeight16();
						if (height >= 0)
							height = int16((height & 0xFFF8) | (int16)block->isFixedLOD());
						else
							height = int16((height & 0xFFF8) | ((-(int16)block->isFixedLOD()) & 0x7));
						positionData.setHeightLowLOD(height);
					}
					else
					{
						size_t LODLevel = 0;
						int16 LowLODheight = generator->getLowLODHeight(data, LODLevel, x, z, px, pz);
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
		mTileInfo.mVertexSource->getBuffer(TSI_POSITION_Y)->unlock();
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::updateNormalBuffer(const uint8* normalData)
	{
		size_t	TerrainSize = TerrainConfigManager::getSingleton().getTerrainInfo().mTileSize;

		if (TerrainConfigManager::getSingleton().isUseVertexNormal())
		{
			HVBUFFER hvbNormal = mTileInfo.mVertexSource->getBuffer(TSI_NORMAL);
			if (hvbNormal == NULL)
			{
				IGraphicsResourceManager& hwManager = IGraphicsResourceManager::getSingleton();
				HVBUFFER dummy;
				TileAABBList dummy2;
				TerrainBufferManager::getSingleton().createVertexBuffer(hwManager, dummy, hvbNormal, TerrainSize, dummy2, NULL, normalData);
				mTileInfo.mVertexSource->setSource(TSI_NORMAL, hvbNormal);
				return;
			}

			Color::COLOR* TileNormals = (Color::COLOR*)hvbNormal->lock(IGraphicsBuffer::GBLF_WRITEONLY);

			const size_t	BlockSize = TerrainConfigManager::getSingleton().getTerrainBlockSize();
			const size_t	blockPerSide = TerrainSize / BlockSize;
			const size_t	SourceSize = TerrainSize + 1;
			//const size_t	VertexSize = TerrainSize + blockPerSide;
			const size_t	VertexBlockSize = BlockSize + 1;
			//const size_t	VertexblockCount = VertexBlockSize*VertexBlockSize;

			for (size_t z = 0; z < blockPerSide; ++z)
			{
				for (size_t x = 0; x < blockPerSide; ++x)
				{
					//TerrainBlock*& block = mBlockData[z][x];

					const uint8* BlockNormalData = normalData;
					for (size_t pz = 0; pz < VertexBlockSize; ++pz)
					{
						for (size_t px = 0; px < VertexBlockSize; ++px)
						{
							Color::COLOR& normal = *TileNormals;
							{
								normal.component[0] = BlockNormalData[px * 3 + Color::R];
								normal.component[1] = BlockNormalData[px * 3 + Color::G];
								normal.component[2] = BlockNormalData[px * 3 + Color::B];
							}
							++TileNormals;
						}
						BlockNormalData += SourceSize * 3;
					}
					normalData += BlockSize * 3;

				}//for x in col of blocks

				normalData += (1 + (BlockSize - 1)*SourceSize) * 3;

			}//for z in row of blocks
			hvbNormal->unlock();
		}//isUseVertexNormal
		else
		{
			//normal map
			TextureState* ts = mTileInfo.mTileMaterialInstance->getTextureState();

			const HTEXTURE& normalMap = ts->getTextureStage(NORMAL_MAP)->getTexture();
			TerrainTextureManager::getSingleton().updateNormalTexture(normalMap, normalData, TerrainSize + 1);
		}

	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::updateBlockBlendBuffer(const TerrainBlockBlendData& blockBlendData)
	{
		if (blockBlendData.getCount() == 0)
			return;

		const TString& texture = blockBlendData.getData(0).getTexture();
		assert(texture != TString::EMPTY);
		if (!mTextureInited)
		{
			assert(mAtlas->getSubTextureCount() == 0);
			assert(mNormalAtlas == NULL || mNormalAtlas->getSubTextureCount() == 0);
			mTextureInited = true;

			TStringParam params;
			params.push_back(texture);
			TerrainTextureManager::getSingleton().addTextureToAtlas(mAtlas, params);

			this->updateMaterialLOD();

			//re-init layer buffer
			TextureState* ts = mTileInfo.mTileMaterialInstance->getTextureState();
			const HTEXTURE& hLayerTexture = ts->getTextureStage(LAYER_MAP)->getTexture();
			const size_t TileSize = TerrainConfigManager::getSingleton().getTerrainTileSize();
			std::memset(mTileInfo.mLayerData, 0, TileSize*TileSize * sizeof(TERRAIN_LAYER));
			TerrainTextureManager::getSingleton().updateLayerTexture(hLayerTexture, mTileInfo.mLayerData);
			return;
		}

		LAYER_ID layerID = mAtlas->findTexture(texture);
		if (layerID == INVALID_LAYER_ID)
		{
			TStringParam param;
			param.push_back(texture);
			bool ret = TerrainTextureManager::getSingleton().addTextureToAtlas(mAtlas, param);
			assert(ret);

			if ((ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_HIGH))
			{
				param.clear();
				param.push_back(TerrainUtility::getNormalTextureName(texture));
				ret = TerrainTextureManager::getSingleton().addTextureToAtlas(mNormalAtlas, param);
				assert(ret);
			}
			BLADE_UNREFERENCED(ret);
			layerID = mAtlas->findTexture(texture);
			assert(layerID != INVALID_LAYER_ID);
		}

		TextureState* ts = mTileInfo.mTileMaterialInstance->getTextureState();

		//update layer texture
		const HTEXTURE& hLayerTexture = ts->getTextureStage(LAYER_MAP)->getTexture();
		TerrainTextureManager::getSingleton().updateBlockLayerTexture(hLayerTexture, blockBlendData, mTileInfo.mLayerData);

		//blend texture
		const HTEXTURE& hBlendTexture = ts->getTextureStage(BLEND_MAP)->getTexture();
		TerrainTextureManager::getSingleton().updateBlockBlendTexture(hBlendTexture, blockBlendData, mTileInfo.mBlendData);
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::setTileTexture(index_t index, const TString& texture)
	{
		TerrainTextureManager::getSingleton().setAtlasSubTexture(mAtlas, (LAYER_ID)index, texture);
		if (mNormalAtlas != NULL)
		{
			TString normalTexture = TerrainUtility::getNormalTextureName(texture);
			TerrainTextureManager::getSingleton().setAtlasSubTexture(mNormalAtlas, (LAYER_ID)index, normalTexture);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::cacheOut()
	{
		assert(this->isUsed());
		this->setUsed(false);

		if (!mBlockData.empty())
		{
			const TERRAIN_INFO& gti = TerrainConfigManager::getSingleton().getTerrainInfo();
			for (size_t z = 0; z < gti.mBlocksPerTileSide; ++z)
			{
				for (size_t x = 0; x < gti.mBlocksPerTileSide; ++x)
				{
					TerrainBlock* block = mBlockData[z][x];
					//blocks' space partition mask usually are fixed for all spaces
					block->cacheOut();
				}
			}
		}
		mAtlas.clear();
		mNormalAtlas.clear();
		mTileInfo.mScene = NULL;
		this->setElement(NULL);
		this->setSpacePartitionMask(ISpace::INVALID_PARTITION);
		IEventManager::getSingleton().removeEventHandlers(ConstDef::EVENT_LOOP_DATA_INIT, this);
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::cacheIn()
	{
		assert(!this->isUsed());
		this->setUsed(true);
		assert(mTileInfo.mScene == NULL);
		assert(this->getElement() == NULL);
		assert(this->getSpacePartitionMask() == ISpace::INVALID_PARTITION);

		IEventManager::getSingleton().addEventHandler(ConstDef::EVENT_LOOP_DATA_INIT, EventDelegate(this, &TerrainTile::loopBegin));
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::updateMaterialInstances()
	{
		const LODSetting& LODSettings = *(mTileInfo.mTileMaterialInstance->getLODSetting());
		mMaterials.resize(LODSettings.size());
		for (size_t i = 0; i < mMaterials.size(); ++i)
		{
			mMaterials[i] = mTileInfo.mTileMaterialInstance->clone();
			mMaterials[i]->load();
			assert(mMaterials[i]->isLoaded());	//cloned from an loaded material, should be ready
			mMaterials[i]->setLOD((MATERIALLOD::LOD_INDEX)i);
		}
		assert(mMaterials.size() < sizeof(mTileInfo.mMaterialLODCount) * 255);
		mTileInfo.mMaterialLODCount = (uint8)mMaterials.size();
		mTileInfo.mLODMaterials = mTileInfo.mMaterialLODCount > 0 ? &mMaterials[0] : NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::notifySpaceChange()
	{
		if (mBlockData.size() == 0)
			return;

		const TERRAIN_INFO& gti = TerrainConfigManager::getSingleton().getTerrainInfo();
		uint32* partitionMasks = TerrainConfigManager::getSingleton().getBlockPartitions();
		uint32 prefix = this->getSpacePartitionMask()&ISpace::EXTRA_PARTITION_MASK;

		for (size_t z = 0; z < gti.mBlocksPerTileSide; ++z)
		{
			for (size_t x = 0; x < gti.mBlocksPerTileSide; ++x)
			{
				TerrainBlock* block = mBlockData[z][x];
				if (block->getSpace() != this->getSpace())
				{
					assert(block->getSpace() == NULL || this->getSpace() == NULL);
					if (block->getSpace() != NULL)
						block->getSpace()->removeContent(block);
					if (this->getSpace() != NULL)
					{
						uint32& partitionMask = partitionMasks[x + z * gti.mBlocksPerTileSide];

						this->getSpace()->addContent(block, partitionMask | prefix, true);

						if (partitionMask == ISpace::INVALID_PARTITION)
							partitionMask = block->getSpacePartitionMask()&ISpace::PARTITION_MASK;
						else
							assert(partitionMask == (block->getSpacePartitionMask()&ISpace::PARTITION_MASK));
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::cleanUp()
	{
		BLADE_RES_FREE(mTileInfo.mRenderGeomBuffer);
		mTileInfo.mRenderGeomBuffer = NULL;
		BLADE_RES_FREE(mTileInfo.mUpdatedMask);
		mTileInfo.mUpdatedMask = NULL;

		size_t blockCount = mBlockData.size();
		for (size_t z = 0; z < blockCount; ++z)
		{
			for (size_t x = 0; x < blockCount; ++x)
			{
				TerrainBlock* block = mBlockData[z][x];
				BLADE_DELETE block;
			}
		}
		mBlockData.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::initTextureData(const HTEXTURE& blendMap, const HTEXTURE& normalMap, const HTEXTURE& layerMap,
		const HTERRAINATLAS& atlas, const HTERRAINATLAS& normalAtlas,
		const uint8* blendData, const uint8* normalData, TERRAIN_LAYER* layerData,
		bool textureInited, bool rebuildTexture)
	{
		//create blend map
		TextureState* ts = mTileInfo.mTileMaterialInstance->getTextureState();
		assert(ts != NULL);

		const size_t tileSize = TerrainConfigManager::getSingleton().getTerrainInfo().mTileSize;
		size_t blendSize = TerrainConfigManager::getSingleton().getBlendBufferSize();

		assert(normalMap == NULL || !TerrainConfigManager::getSingleton().isUseVertexNormal());
		assert(normalMap != NULL || TerrainConfigManager::getSingleton().isUseVertexNormal());
		TextureStage* blendStage = ts->addTextureStage(BLEND_MAP);;

		if (mTileInfo.mBlendData != blendData
			|| blendStage->getTexture() == NULL
			|| rebuildTexture)
		{
			if (TerrainConfigManager::getSingleton().isEditingModeEnabled())
				mTileInfo.mBlendData = blendData;
			else
				mTileInfo.mBlendData = NULL;

			//try to use existing texture
			if (blendStage->getTexture() == NULL ||
				blendStage->getTexture()->getWidth() != blendSize ||
				blendStage->getTexture()->getHeight() != blendSize)
			{
				assert(blendMap != blendStage->getTexture());
			}
			blendStage->setTexture(blendMap);
		}

		TextureStage* normalStage = ts->addTextureStage(NORMAL_MAP);

		if (mTileInfo.mNormalData != normalData
			|| normalStage->getTexture() == NULL
			|| rebuildTexture)
		{
			assert(ts != NULL);

			if (TerrainConfigManager::getSingleton().isEditingModeEnabled())
				mTileInfo.mNormalData = normalData;
			else
				mTileInfo.mNormalData = NULL;

			if (!TerrainConfigManager::getSingleton().isUseVertexNormal())
			{

				if (normalStage->getTexture() == NULL ||
					normalStage->getTexture()->getWidth() != tileSize ||
					normalStage->getTexture()->getWidth() != tileSize)
				{
					assert(normalStage->getTexture() != normalMap);
				}

				normalStage->setTexture(normalMap);
			}

		}

		TextureStage* layerStage = ts->addTextureStage(LAYER_MAP);

		if (mTileInfo.mLayerData != layerData
			|| layerStage->getTexture() == NULL
			|| rebuildTexture)
		{
			assert(ts != NULL);

			if (TerrainConfigManager::getSingleton().isEditingModeEnabled())
				mTileInfo.mLayerData = layerData;
			else
				mTileInfo.mLayerData = NULL;

			if (layerStage->getTexture() == NULL ||
				layerStage->getTexture()->getWidth() != tileSize ||
				layerStage->getTexture()->getWidth() != tileSize)
			{
				assert(layerStage->getTexture() != layerMap);
			}
			layerStage->setTexture(layerMap);
		}

		mTextureInited = textureInited;

		//setup atlas
		mAtlas = atlas;
		TextureStage* diffuseStage = ts->addTextureStage(ATLAS_MAP);
		diffuseStage->setTexture(mAtlas->getAtlasTexture());

		TextureStage* detailNormalStage = ts->addTextureStage(NORMAL_ATLAS_MAP);

		//normal atlas
		if (ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_HIGH)
		{
			mNormalAtlas = normalAtlas;
			if (mNormalAtlas != NULL)
			{
				detailNormalStage->setTexture(mNormalAtlas->getAtlasTexture());
				mTileInfo.mMaterialLODQualityLimit = TML_HIGH;
			}
			else //temp limit to low, then change on texture modification update later		
				mTileInfo.mMaterialLODQualityLimit = TML_MIDDLE;
		}
		else if (ITerrainConfigManager::getSingleton().getTextureQuality() == TerrainConsts::TEXTURE_QUALITY_MIDDLE)
		{
			assert(normalAtlas == NULL);
			mTileInfo.mMaterialLODQualityLimit = TML_MIDDLE;
		}
		else
		{
			assert(normalAtlas == NULL);
			mTileInfo.mMaterialLODQualityLimit = TML_LOW;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainTile*	TerrainTile::getNeighbor(int16 offsetX, int16 offsetZ) const
	{
		assert(offsetX != 0 || offsetZ != 0);
		index_t x = (index_t)((int16)mTileX + offsetX);
		index_t z = (index_t)((int16)mTileZ + offsetZ);
		size_t tileCount = this->getConfigManager()->getVisibleTileCount();
		if (x < tileCount && z < tileCount)
			return static_cast<TerrainConfigManager*>(this->getConfigManager())->getTile(x, z);
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainTile::setupShaderParameters()
	{
		//scale factor of terrain vertex shader
		const fp32*		TerrainScale = TerrainConfigManager::getSingleton().getTerrainScale();
		Vector4			scaleFactor(TerrainScale[0], TerrainScale[1], TerrainScale[2], 1);

		scalar UVInfo[4];
		scalar& UVBlendOffset = UVInfo[0];
		scalar& UVTileSize = UVInfo[1];
		scalar& UVBlockSize = UVInfo[2];
		scalar& UVBlockMultiple = UVInfo[3];
		size_t blendMultiple = TerrainConfigManager::getSingleton().getBlendBufferSize() / TerrainConfigManager::getSingleton().getTerrainTileSize();
		UVBlendOffset = (scalar)1 / scalar(blendMultiple) * 0.5f;	//half texel offset for blend map
		UVTileSize = (scalar)TerrainConfigManager::getSingleton().getTerrainTileSize();
		UVBlockSize = (scalar)TerrainConfigManager::getSingleton().getTerrainBlockSize();
		UVBlockMultiple = (scalar)TerrainConfigManager::getSingleton().getTileUVMultiple();

		scalar atlasInfo[4];
		scalar& atlasTextureCount = atlasInfo[0];
		scalar& textureSize = atlasInfo[1];
		scalar& maxMipLevel = atlasInfo[2];
		scalar& invTextureSize = atlasInfo[3];
		atlasTextureCount = (scalar)TerrainConfigManager::getSingleton().getTextureCount();
		textureSize = (scalar)TerrainConfigManager::getSingleton().getTextureSize();
		maxMipLevel = (scalar)mAtlas->getMaxMipLevel();
		invTextureSize = 1.0f / textureSize;

		scalar blendInfo[4] = { 0 };
		blendInfo[0] = (scalar)TerrainConfigManager::getSingleton().getBlendBufferSize();
		blendInfo[1] = scalar(0.5) / blendInfo[0];

		ShaderVariableMap::Handles handle;	//use handle to skip runtime log errors
		for (size_t i = 0; i < mTileInfo.mTileMaterial->getTechniqueCount(); ++i)
		{
			mTileInfo.mTileMaterial->getTechnique(i)->getShaderVariableHandle(handle, BTString("scaleFactor"), SCT_FLOAT4, 1);
			handle.setData(&scaleFactor, sizeof(scaleFactor));

			mTileInfo.mTileMaterial->getTechnique(i)->getShaderVariableHandle(handle, BTString("UVInfo"), SCT_FLOAT4, 1);
			handle.setData(&UVInfo, sizeof(UVInfo));

			mTileInfo.mTileMaterial->getTechnique(i)->getShaderVariableHandle(handle, BTString("atlasInfo"), SCT_FLOAT4, 1);
			handle.setData(&atlasInfo, sizeof(atlasInfo));

			mTileInfo.mTileMaterial->getTechnique(i)->getShaderVariableHandle(handle, BTString("blendInfo"), SCT_FLOAT4, 1);
			handle.setData(&blendInfo, sizeof(blendInfo));
		}
	}

}//namespace Blade