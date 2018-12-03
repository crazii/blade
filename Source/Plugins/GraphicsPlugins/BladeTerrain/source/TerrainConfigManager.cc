/********************************************************************
	created:	2011/04/13
	filename: 	TerrainConfigManager.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <ConfigSerializer.h>
#include <interface/IGraphicsSystem.h>
#include <databinding/DataSourceWriter.h>
#include <BladeTerrain_blang.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>
#include <interface/public/physics/IPhysicsService.h>

#include <interface/IRenderSchemeManager.h>
#include <interface/IRenderScene.h>
#include <interface/ISpaceCoordinator.h>
#include <Technique.h>
#include <TerrainShaderShared.inl>
#include "TerrainElement.h"
#include "TerrainConfigManager.h"
#include "TerrainBufferManager.h"
#include "TerrainResource.h"

namespace Blade
{
	enum
	{
		DefaultTilingUVMultiple	= 1,
	};

	//LOD distance static configs
	enum
	{
		LOD_SQUARE_DIST_INDEX = 0,
		LOD_DIST_INDEX = 1,
	};

	namespace ConfigString
	{
		static const TString INDEX_TYPE = BXLang(BLANG_MESH_QUALITY);
		static const TString BATCH_COMBINE = BXLang(BLANG_BATCH_COMBINE);
		static const TString LOD_LEVEL = BXLang(BLANG_LOD_LEVEL);
		static const TString BLOCK_SIZE = BXLang(BLANG_BLOCK_SIZE);
		static const TString BLEND_BUFFER_SCALE = BXLang(BLANG_BLENDMAP_SCALE);
		static const TString MAX_HEIGHT = BXLang(BLANG_MAX_HEIGHT);

		static const TString TERRAIN_PATH = BXLang(BLANG_DATA_PATH);
		static const TString RESOURCE_PATH = BXLang(BLANG_RESOURCE_PATH);

		static const TString TEXTURE_QUALITY = BXLang(BLANG_TEXTURE_QUALITY);

		//internally used
		static const TString TEXTURE_SIZE = BXLang("Texture Size");
		static const TString BLEND_NORMAL_SCALE = BXLang("Blend Normal Scale");
	}//namespace ConfigString

	TerrainType	TerrainConfigManager::msTerrainType;

	const TString TerrainConfigManager::GLOBAL_CONFIG_TYPENAME = BXLang(BLANG_GLOBAL_CONFIG);
	const TString TerrainConfigManager::CREATION_CONFIG_TYPENAME = BXLang(BLANG_CREATION_CONFIG);
	//////////////////////////////////////////////////////////////////////////
	TerrainConfigManager::TerrainConfigManager()
	{
		//global config
		{
			//force material LOD on low spec devices
			mTerrainConfig.mTextureQuality = TerrainConsts::TEXTURE_QUALITY_HIGH;

			mTerrainConfig.mBatchCominer = BatchCombinerFactory::getSingleton().getRegisteredClass(0);
			mTerrainConfig.mTraingleIndexFormat = IndexGeneratorFactory::getSingleton().getRegisteredClass(0);

			mTerrainConfig.mMaxLODLevels = 5u;			////log2(BlockSize)
			mTerrainConfig.mBlendNormalScale = 1.0f;
			mTerrainConfig.mTileTextureSize = 512;		//atlas sub size 512 x 512 //2048 x 2048 texture size
			mTerrainConfig.mBlockSize = 32u;
			mTerrainConfig.mLODLevels = (uint8)mTerrainConfig.mMaxLODLevels; //3u;			//log2(BlockSize)
			mTerrainConfig.mFlatLODLevel = mTerrainConfig.mLODLevels;
			mTerrainConfig.mCliffLODLevel = std::min<uint8>(mTerrainConfig.mLODLevels, 1u);

			this->setBatchCombiner(mTerrainConfig.mBatchCominer);
			assert( IndexGeneratorFactory::getSingleton().getNumRegisteredClasses() > 0 );
			mLODComparator = BLADE_FACTORY_CREATE(IIndexGenerator, mTerrainConfig.mTraingleIndexFormat );

			DataSourceWriter<TERRAIN_CFG> writer(GLOBAL_CONFIG_TYPENAME);
			//version 0.1
			writer.beginVersion(Version(0,1,'A'));
			{
				writer << ConfigString::INDEX_TYPE << IndexGeneratorFactory::getSingleton() << &TERRAIN_CFG::mTraingleIndexFormat;
				writer << ConfigString::BATCH_COMBINE << BatchCombinerFactory::getSingleton() << CAF_NORMAL_DEVMODE << &TERRAIN_CFG::mBatchCominer;
				writer << ConfigString::TEXTURE_SIZE << CUIH_NONE << CAF_NONE_CONFIG_DEVMODE << &TERRAIN_CFG::mTileTextureSize;
				writer << ConfigString::BLEND_NORMAL_SCALE << CUIH_NONE << CAF_NONE_CONFIG_DEVMODE << &TERRAIN_CFG::mBlendNormalScale;

				{
					//block size config is hard coded.
					//TODO: better way?
					const TString	VALUE_16 = TStringHelper::fromUInt(16);
					const TString	VALUE_32 = TStringHelper::fromUInt(32);
					const TString	VALUE_64 = TStringHelper::fromUInt(64);

					Variant blockSizes[] = { (uint8)16u, (uint8)32u, (uint8)64u};
					DataSourceWriter<TERRAIN_CFG> LODWriter = 
						( writer << ConfigString::BLOCK_SIZE << blockSizes << CAF_NORMAL_DEVMODE << DS_GROUP<TERRAIN_CFG>(&TERRAIN_CFG::mBlockSize) );

					//redundant LOD-level will be saved/loaded through config interface
					//but maybe this is better because different block size has its own LOD level config

					const uint8 minLOD = 0;
					const uint8 step = 1;
					{
						const uint8 maxLOD = (uint8)4u;
						LODWriter[VALUE_16] << ConfigString::LOD_LEVEL << ConfigAtom::Range(minLOD, maxLOD, step) << &TERRAIN_CFG::mLODLevels;
					}
					{
						const uint8 maxLOD = (uint8)5u;
						LODWriter[VALUE_32] << ConfigString::LOD_LEVEL << ConfigAtom::Range(minLOD, maxLOD, step) << &TERRAIN_CFG::mLODLevels;
					}
					{
						const uint8 maxLOD = (uint8)6u;
						LODWriter[VALUE_64] << ConfigString::LOD_LEVEL << ConfigAtom::Range(minLOD, maxLOD, step) << &TERRAIN_CFG::mLODLevels;
					}

					LODWriter.endVersion();
				}

				{
					TStringParam list;
					list.push_back( TerrainConsts::TEXTURE_QUALITY_LOW );
					list.push_back( TerrainConsts::TEXTURE_QUALITY_MIDDLE );
					list.push_back( TerrainConsts::TEXTURE_QUALITY_HIGH );
					writer << ConfigString::TEXTURE_QUALITY << list << CUIH_RANGE << &TERRAIN_CFG::mTextureQuality;
				}
			}
			writer.endVersion();
		}

		//config for creating new instance
		{
			mTerrainInfo.mTileSize = 0;
			mTerrainInfo.mTileCount = 0;
			mTerrainInfo.mVisibleTileCount = 0;
			mTerrainInfo.mEditingMode = false;
			mTerrainInfo.mEnableMultiLoading = true;
			mTerrainInfo.mEnableQueryData = false;

			//default options
			mTerrainInfo.mBlendBufferScale = 1;
			mTerrainInfo.mBlockVertexCount = (mTerrainConfig.mBlockSize+1u)*(mTerrainConfig.mBlockSize+1u);
			mTerrainInfo.mTileUVMultiple = DefaultTilingUVMultiple;
			//atlas
			mTerrainInfo.mTileTextureCount = 4;		//4x4 atlas
#if TERRAIN_TEXTURE_COMPRESSED
			mTerrainInfo.mTileTextureMinMip = 4;	//uncompressed one block size
#else
			mTerrainInfo.mTileTextureMinMip = 4;	//uncompressed one block size
#endif
			mTerrainInfo.mTerrainScale[0] = 1.0f;	//not used
			mTerrainInfo.mTerrainScale[1] = 512.0f;	//must less than 1024
			mTerrainInfo.mTerrainScale[2] = 1.0f;	//not used

			DataSourceWriter<TERRAIN_INFO> writer( CREATION_CONFIG_TYPENAME );
			writer.beginVersion( Version(0,1,'A') );
			//version 0.1
			{
				//min: 128, max: 1024, step: 128
				writer << ConfigString::BLEND_BUFFER_SCALE << ConfigAtom::Range(1u, 4u, 1u) << &TERRAIN_INFO::mBlendBufferScale;
				writer << ConfigString::MAX_HEIGHT << ConfigAtom::Range(16.0f, 1023.0f, 1.0f, mTerrainInfo.mTerrainScale[1]) << DS_GETTER_SETTER(&TERRAIN_INFO::getMaxHeight, &TERRAIN_INFO::setMaxHeight);
				//TODO: add atlas config ?

				//serialization data (none-config)
				writer << ConfigString::TERRAIN_PATH << CUIH_NONE << CAF_SERIALIZE << &TERRAIN_INFO::mTerrainPath;
				writer << ConfigString::RESOURCE_PATH << CUIH_NONE << CAF_SERIALIZE << &TERRAIN_INFO::mTerrainResPrefix;
			}
			writer.endVersion();
		}

		DataBinder globalBinder(GLOBAL_CONFIG_TYPENAME, &mTerrainConfig, true);
		mGlobalConfig = globalBinder.getConfig();
		DataBinder creationBinder(CREATION_CONFIG_TYPENAME, &mTerrainInfo, true);
		mCreationConfig = creationBinder.getConfig();

		mBlockSpacePartitions = NULL;
		mLastParitionBlock = 0;
		mLastVertexNormal = this->isUseVertexNormal();

		IEventManager::getSingleton().addEventHandler(RenderDeviceReadyEvent::NAME, EventDelegate(this, &TerrainConfigManager::onRenderDeviceReady) );
		IEventManager::getSingleton().addEventHandler(RenderDeviceCloseEvent::NAME, EventDelegate(this, &TerrainConfigManager::onRenderDeviceClose));
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainConfigManager::~TerrainConfigManager()
	{
		BLADE_RES_FREE(mBlockSpacePartitions);
		IEventManager::getSingleton().removeEventHandlers(RenderDeviceReadyEvent::NAME, this);
	}


	/************************************************************************/
	/* ITerrainConfigManager interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&		TerrainConfigManager::getTerrainCreationConfig() const
	{
		return mCreationConfig;
	}

	//////////////////////////////////////////////////////////////////////////
	const HCONFIG&		TerrainConfigManager::getTerrainGlobalConfig() const
	{
		return mGlobalConfig;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainConfigManager::initialize(IGraphicsScene* scene, size_t pageSize, size_t pageCount, size_t visiblePage,
		const TString& pathPrefix/* = TString::EMPTY*/, const TString& resourcePrefix/* = TString::EMPTY*/)
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);

		bool terrainSizeChanged = (mTerrainInfo.mTileSize != pageSize);

		//use stage config
		mTerrainInfo.mTileSize = (uint32)pageSize;
		mTerrainInfo.mTileCount = (uint32)pageCount;
		mTerrainInfo.mVisibleTileCount = (uint32)visiblePage;

		//update query data flag
		//if the the physics service is available, don't create query data
		mTerrainInfo.mEnableQueryData = IPhysicsService::getSingletonPtr() == NULL;

		if (mTerrainInfo.mVisibleTileCount % 2 == 0)
			BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("visible terrain count must be 2*n+1"));

		//update relative data from config data
		mTerrainInfo.mBlockVertexCount = (mTerrainConfig.mBlockSize + 1u)*(mTerrainConfig.mBlockSize + 1u);
		mTerrainInfo.mBlocksPerTileSide = mTerrainInfo.mTileSize / mTerrainConfig.mBlockSize;
		mTerrainInfo.mBlocksPerTile = mTerrainInfo.mBlocksPerTileSide*mTerrainInfo.mBlocksPerTileSide;
		mTerrainInfo.mTileVertexCount = (mTerrainInfo.mTileSize + mTerrainInfo.mBlocksPerTileSide)*(mTerrainInfo.mTileSize + mTerrainInfo.mBlocksPerTileSide);
		mTerrainConfig.mMaxLODLevels = static_cast<size_t>(std::log((scalar)mTerrainConfig.mBlockSize) / log(scalar(2.0f)) + scalar(0.5f));
		if (mTerrainConfig.mLODLevels > mTerrainConfig.mMaxLODLevels)
			mTerrainConfig.mLODLevels = (uint8)mTerrainConfig.mMaxLODLevels;

		mTerrainConfig.mFlatLODLevel = mTerrainConfig.mLODLevels;
		mTerrainConfig.mCliffLODLevel = std::min<uint8>(mTerrainConfig.mLODLevels, 1u);

		mTerrainInfo.mMaxBlockIndexCount = 0;

		if (pathPrefix != TString::EMPTY)
			mTerrainInfo.mTerrainPath = pathPrefix;
		if (resourcePrefix != TString::EMPTY)
			mTerrainInfo.mTerrainResPrefix = resourcePrefix;

		this->setIndexGenerator(mTerrainConfig.mTraingleIndexFormat);
		this->setBatchCombiner(mTerrainConfig.mBatchCominer);
		TerrainConfigManager::getRenderType().getMaterial()->addListener(this);

		const TString& spaceType = static_cast<IRenderScene*>(scene)->getSpaceCoordinatorImpl()->getDesc()->defaultSpace;
		bool rebuildPartition = terrainSizeChanged || spaceType != mLastPartitionSpace || mLastParitionBlock != mTerrainInfo.mBlocksPerTile;
		mLastPartitionSpace = spaceType;
		mLastParitionBlock = mTerrainInfo.mBlocksPerTile;

		this->resetBlockPartitions(rebuildPartition);
		if (terrainSizeChanged)
			Impl::TerrainGraphicsCache::getSingleton().clearCaches();
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainConfigManager::notifyGlobalConfigChanged()
	{
		ConfigGroup* group = static_cast<ConfigGroup*>( mGlobalConfig );

		size_t oldBlockSize = TStringHelper::toSizeT( group->getSubConfigByName( ConfigString::BLOCK_SIZE )->getBackupValue() );
		size_t oldLodLevel = TStringHelper::toSizeT( 
			group->getSubConfigByName( ConfigString::BLOCK_SIZE )->getSubConfigByName(ConfigString::LOD_LEVEL)->getBackupValue() 
			);

		bool newIndexGenerator = this->setIndexGenerator( mTerrainConfig.mTraingleIndexFormat );
		bool newBatchCombiner = this->setBatchCombiner( mTerrainConfig.mBatchCominer );

		bool newBlockSize = oldBlockSize != mTerrainConfig.mBlockSize;
		bool newLODLevel = mTerrainConfig.mLODLevels != oldLodLevel;

		if( newIndexGenerator || newBatchCombiner || newBlockSize || newLODLevel)
		{
			for( TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i )
			{
				ITerrainTile* tile = *i;
				if(tile->isUsed())
					tile->releaseIndexBuffer();
			}
			mBatchCominer->releaseIndexBuffer();
		}

		if( newBlockSize || newLODLevel )	//LOD level need update block's fixed LOD
		{
			mTerrainInfo.mBlockVertexCount = (mTerrainConfig.mBlockSize+1u)*(mTerrainConfig.mBlockSize+1u);
			mTerrainInfo.mBlocksPerTileSide = mTerrainInfo.mTileSize/mTerrainConfig.mBlockSize;
			mTerrainInfo.mBlocksPerTile = mTerrainInfo.mBlocksPerTileSide*mTerrainInfo.mBlocksPerTileSide;
			mTerrainInfo.mTileVertexCount = (mTerrainInfo.mTileSize+mTerrainInfo.mBlocksPerTileSide)*(mTerrainInfo.mTileSize+mTerrainInfo.mBlocksPerTileSide);
			//this is not frequently called, so just do the math , no tricks
			mTerrainConfig.mMaxLODLevels = static_cast<size_t>( log((scalar)mTerrainConfig.mBlockSize)/log( scalar(2.0f) ) + scalar(0.5f) );
			if( mTerrainConfig.mLODLevels > mTerrainConfig.mMaxLODLevels )
				mTerrainConfig.mLODLevels = (uint8)mTerrainConfig.mMaxLODLevels;

			mTerrainConfig.mFlatLODLevel = mTerrainConfig.mLODLevels;
			mTerrainConfig.mCliffLODLevel = std::min<uint8>(mTerrainConfig.mLODLevels, 1u);

			mTerrainInfo.mMaxBlockIndexCount = 0;

			//rebuild tiles
			for( TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i )
			{
				ITerrainTile* tile = *i;
				if(tile->isUsed())
					tile->rebuildBlocks();
			}
		}
		else if( newIndexGenerator || newBatchCombiner )
		{
			for( TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i )
			{
				ITerrainTile* tile = *i;
				if(tile->isUsed())
					tile->buildIndexBuffer();
			}
		}

		this->resetBlockPartitions(newBlockSize);

		//texture quality update
		for(TileSet::const_iterator i = mTiles.begin(); i != mTiles.end(); ++i)
		{
			ITerrainTile* tile = *i;
			if(tile->isUsed())
				tile->updateMaterialLOD();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t				TerrainConfigManager::calculateTotalIndexCount() const
	{
		//note: the current calculation using X-field-of view as 90 degree
		//if the real camera has a FOV-x larger than this,
		//then the calculated size is smaller than needed.

		//when looking vertically down to the terrain, the method above cannot generates enough index counts
		//(the LOD ignoring Y value, only basing on horizontal X,Z
		//so the maximum index count is when looking down vertically in the center of the tiles (the center of the center tile)


		size_t total = 0;
		//update mTerrainInfo.mMaxIndexCount from current IndexGenerator
		{
			const size_t maxVisibleDistance = mTerrainInfo.mTileSize*mTerrainInfo.mVisibleTileCount;
			const size_t totalBlockCount = mTerrainInfo.mBlocksPerTile * mTerrainInfo.mVisibleTileCount*mTerrainInfo.mVisibleTileCount;

			size_t lastBlocksCountLOD = 0;
			for(size_t i = 0; i <= mTerrainConfig.mLODLevels; ++i)
			{
				if( maxVisibleDistance < LOD_DISTANCES[i] )
					break;

				size_t BlocksPerLOD = (size_t)std::ceil(LOD_DISTANCES[i]/(scalar)mTerrainConfig.mBlockSize);
				size_t curBlockCountLOD = BlocksPerLOD*BlocksPerLOD;
				total += ( curBlockCountLOD - lastBlocksCountLOD ) * this->getIndexGenerator()->getLODMaxIndexCount(i);

				lastBlocksCountLOD = curBlockCountLOD;
			}

			if( maxVisibleDistance > LOD_DISTANCES[mTerrainConfig.mLODLevels] )
			{
				assert( totalBlockCount > lastBlocksCountLOD );
				size_t maxLOD = mTerrainConfig.mLODLevels == mTerrainConfig.mMaxLODLevels ? mTerrainConfig.mMaxLODLevels : mTerrainConfig.mLODLevels + 1;
				total += (totalBlockCount/4 - lastBlocksCountLOD) * this->getIndexGenerator()->getLODMaxIndexCount(maxLOD);
			}
			total *= 4;
		}

		return total;
	}

	//////////////////////////////////////////////////////////////////////////
	bool				TerrainConfigManager::isUseVertexNormal() const
	{
		const TString& profile = IGraphicsSystem::getSingleton().getCurrentProfile();
		return profile == BTString("2_0") || (mTerrainConfig.mTextureQuality == TerrainConsts::TEXTURE_QUALITY_LOW && profile < BTString("3_0"));
	}

	//////////////////////////////////////////////////////////////////////////
	void				TerrainConfigManager::onActivateTechnique(Technique* /*old*/, Technique* now)
	{
		//disable material LOD on low profile
		bool updateMat = (mLastVertexNormal != this->isUseVertexNormal());
		mLastVertexNormal = this->isUseVertexNormal();

		//note: if we support multiple scheme on runtime,
		//we need to check all scheme's are at low profile so that
		//high profile scheme still using material LOD

		if( !updateMat )
		{
			Technique* tech = now;
			if( tech->getProfile() == BTString("2_0") )
			{
				TPointerParam<IRenderScheme> list;
				IRenderSchemeManager::getSingleton().getActiveSchemes(list);
				TString maxProfile;
				for(size_t i = 0; i < list.size(); ++i)
				{
					if( maxProfile < list[i]->getProfile() )
						maxProfile = list[i]->getProfile();
				}
				updateMat = list.size() <= 1 || (maxProfile == BTString("2_0"));
			}
			else
				updateMat = true;
		}

		if(updateMat)
		{
			for(TileSet::const_iterator i = mTiles.begin(); i != mTiles.end(); ++i)
			{
				ITerrainTile* tile = *i;
				tile->updateMaterialLOD(updateMat);
			}
		}
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	const Material*		TerrainConfigManager::getTileMaterial() const
	{
		return TerrainConfigManager::getRenderType().getMaterial();
	}

	//////////////////////////////////////////////////////////////////////////
	ITerrainBatchCombiner*	TerrainConfigManager::getBatchCombiner()
	{
		if( mBatchCominer == NULL )
			mBatchCominer = BLADE_FACTORY_CREATE(ITerrainBatchCombiner,mBatchCombinationName);
		return mBatchCominer;
	}

	//////////////////////////////////////////////////////////////////////////
	uint8			TerrainConfigManager::getLODLevelBySquaredDistance(scalar fSquaredDistance)
	{
		for( index_t i = 0; i < mTerrainConfig.mLODLevels; ++i )
		{
			if( fSquaredDistance < LOD_DISTANCES_SQUARE[i] )
				return (uint8)i;
		}
		return mTerrainConfig.mLODLevels;
	}


	//////////////////////////////////////////////////////////////////////////
	uint8			TerrainConfigManager::getLODLevelByDistance(scalar fDistance)
	{
		for( index_t i = 0; i < mTerrainConfig.mLODLevels; ++i )
		{
			if( fDistance < LOD_DISTANCES[i] )
				return (uint8)i;
		}
		return mTerrainConfig.mLODLevels;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainConfigManager::onRenderDeviceReady(const Event& state)
	{
		const RenderDeviceReadyEvent& readyState = static_cast<const RenderDeviceReadyEvent&>(state);
		const TString& profile = readyState.mCurrentProfile;
		
		if( profile == BTString("2_0") )
		{
			//current profile is low
			//disable options
			mTerrainConfig.mTextureQuality = TerrainConsts::TEXTURE_QUALITY_LOW;
			mGlobalConfig->getSubConfigByName(ConfigString::TEXTURE_QUALITY)->setReadOnly(true);
		}
		else if( profile == BTString("3_0") )
		{
			//disable vertex normal for shader model 3.0
			if( mTerrainConfig.mTextureQuality == TerrainConsts::TEXTURE_QUALITY_LOW )
				mTerrainConfig.mTextureQuality = TerrainConsts::TEXTURE_QUALITY_MIDDLE;
			TempVector<TString> list;
			list.push_back(TerrainConsts::TEXTURE_QUALITY_MIDDLE);
			list.push_back(TerrainConsts::TEXTURE_QUALITY_HIGH);
			mGlobalConfig->getSubConfigByName(ConfigString::TEXTURE_QUALITY)->setOptions(&list[0], list.size(), 
				mTerrainConfig.mTextureQuality == TerrainConsts::TEXTURE_QUALITY_MIDDLE ? 0 : 1u);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainConfigManager::onRenderDeviceClose(const Event& /*evt*/)
	{
		Impl::TerrainGraphicsCache::getSingleton().clearCaches();
		this->releaseTiles();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t			TerrainConfigManager::getCurrentTileCount() const
	{
		size_t n = 0;
		for (TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i)
		{
			TerrainTile* tile = static_cast<TerrainTile*>(*i);
			if (tile->isUsed())
				++n;
		}
		return n;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainTile*	TerrainConfigManager::allocTile()
	{
		for (TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i)
		{
			TerrainTile* tile = static_cast<TerrainTile*>(*i);
			if (!tile->isUsed())
			{
				tile->cacheIn();
				return tile;
			}
		}
		TerrainTile* tile = BLADE_NEW TerrainTile();
		tile->cacheIn();
		mTiles.insert(tile);
		return tile;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainConfigManager::freeTile(TerrainTile* tile)
	{
		assert(tile != NULL);
		assert(tile->isUsed());
		assert(mTiles.find(tile) != mTiles.end());
		tile->cacheOut();
		assert(!tile->isUsed());

		bool anyUsed = false;
		for (TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i)
		{
			ITerrainTile* t = *i;
			if (t->isUsed())
			{
				anyUsed = true;
				break;
			}
		}

		if (!anyUsed)
			mBatchCominer.clear();

		return anyUsed;
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainTile*	TerrainConfigManager::getTile(index_t x, index_t z)
	{
		for (TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i)
		{
			TerrainTile* t = static_cast<TerrainTile*>(*i);
			if (!t->isUsed())
				continue;
			if (t->getTileIndexX() == x && t->getTileIndexZ() == z)
				return t;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainConfigManager::releaseTiles()
	{
		for (TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i)
		{
			TerrainTile* t = static_cast<TerrainTile*>(*i);
			BLADE_DELETE t;
		}
		mTiles.clear();
		TerrainBufferManager::getSingleton().releaseHorizontalPositionBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void TerrainConfigManager::resetBlockPartitions(bool doReset)
	{
		if (doReset || mBlockSpacePartitions == NULL)
		{
			BLADE_RES_FREE(mBlockSpacePartitions);
			mBlockSpacePartitions = NULL;

			if (mBlockSpacePartitions == NULL)
				mBlockSpacePartitions = BLADE_RES_ALLOCT(uint32, mTerrainInfo.mBlocksPerTile);

			for (size_t i = 0; i < mTerrainInfo.mBlocksPerTile; ++i)
				mBlockSpacePartitions[i] = ISpace::INVALID_PARTITION;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainConfigManager::setIndexGenerator(const TString& name)
	{
		if( mIndexGeneratorName != name)
		{
			mIndexGeneratorName = name;

			mLODComparator.clear();
			mLODComparator.bind( BLADE_FACTORY_CREATE(IIndexGenerator,mIndexGeneratorName ) );

			//on index generator change, update terrain vertex buffer (low LOD height)
			//because low LOD height data is created by index generator.
			for(TileSet::iterator i = mTiles.begin(); i != mTiles.end(); ++i)
				(*i)->updateVertexBuffer();

			return true;
		}
		else
			return false;
	}

	//////////////////////////////////////////////////////////////////////////
	bool			TerrainConfigManager::setBatchCombiner(const TString& name)
	{
		if( name != mBatchCombinationName )
		{
			HBC old = mBatchCominer;
			mBatchCominer.unbind();
			mBatchCombinationName = name;
			mBatchCominer.bind( BLADE_FACTORY_CREATE(ITerrainBatchCombiner, mBatchCombinationName) );

			//move the internal data,
			//so that the output buffer don't need to change for each tile, and this becomes transparent to tiles
			if( old != NULL )
				mBatchCominer->stealFrom(*old);
			return true;
		}
		else
			return false;
	}

}//namespace Blade


