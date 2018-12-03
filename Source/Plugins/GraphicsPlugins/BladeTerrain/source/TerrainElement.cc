/********************************************************************
	created:	2010/05/04
	filename: 	TerrainElement.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <utility/Profiling.h>
#include <interface/IResourceManager.h>
#include <interface/IStage.h>
#include <interface/IEntity.h>
#include <EntityResourceDesc.h>

#include <TextureResource.h>
#include <ConstDef.h>
#include "TerrainElement.h"

namespace Blade
{
	//////////////////////////////////////////////////////////////////////////
	TerrainElement::TerrainElement()
		:GraphicsElement( TerrainConsts::TERRAIN_ELEMENT )
		,mTile(NULL)
		,mModified(false)
	{

	}

	//////////////////////////////////////////////////////////////////////////
	TerrainElement::~TerrainElement()
	{
		TerrainConfigManager& tcm = TerrainConfigManager::getSingleton();

		if (mTile != NULL)
		{
			tcm.freeTile(mTile);
		}
	}

	/************************************************************************/
	/* GraphicsElement interface                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				TerrainElement::onInitialize()
	{
		const IPagingManager::DESC& desc = this->getStage()->getPagingManager()->getDesc();

		TerrainConfigManager& tcm = TerrainConfigManager::getSingleton();

		if (tcm.getTileCount() == 0 || tcm.getTerrainTileSize() != desc.mPageSize)
		{
			tcm.releaseTiles();

			tcm.initialize(this->getGraphicsScene(), desc.mPageSize, desc.mPageCount, desc.mVisiblePages);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainElement::onResourceUnload()
	{
		assert(mContent == mTile);
		GraphicsElement::onResourceUnload();
		if (mTile != NULL)
			TerrainConfigManager::getSingleton().freeTile(mTile);
		mContent = mTile = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	void			TerrainElement::postProcess(const ProgressNotifier& notifier)
	{
		assert(mTile == NULL);
		mTile = TerrainConfigManager::getSingleton().allocTile();
		mTile->setElement(this);
		mContent = mTile;
		assert(mTile->getSpace() == NULL && mTile->getSpacePartitionMask() == ISpace::INVALID_PARTITION);
		mTile->setTileIndex(mTileIndexX, mTileIndexZ);

		if (this->getBoundResource() != NULL)
			this->setupTile();
		else
		{
			//loading failed
			if (TerrainConfigManager::getSingleton().isEditingModeEnabled())
			{
				size_t size = TerrainConfigManager::getSingleton().getTerrainTileSize();
				Vector3 scale = Vector3(TerrainConfigManager::getSingleton().getTerrainScale());
				this->createTile(scale, size);
				this->setupTile();
				assert(this->getBoundResource() != NULL);
			}
			else
				assert(false);
		}
		GraphicsElement::postProcess(notifier);
	}

	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	void				TerrainElement::createTile(const Vector3& scale, size_t size)
	{
		if( this->getBoundResource() == NULL )
		{
			ParamList params;
			params[BTString("SIZE")] = (uint32)size;
			params[BTString("POS")] = mPosition;
			params[BTString("SCALE")] = scale;

			const HRESOURCE& hRes = IResourceManager::getSingleton().createResource(this->getResourcePath(), params);
			this->setBoundResource( hRes );
			mModified = true;
		}
		else
			assert(false);
	}

	//////////////////////////////////////////////////////////////////////////
	void						TerrainElement::setupTile()
	{
		BLADE_TS_VERIFY(TS_MAIN_SYNC);
		//BLADE_LW_PROFILING_FUNCTION();
		mTile->loadMaterial();

		{
			//BLADE_LW_PROFILING( REMOVE_CONTENT );
			if (mTile->getSpace() != NULL)
			{
				this->deactivateContent();
			}
		}

		TerrainResource* terrainRes = static_cast<TerrainResource*>( this->getBoundResource() );
		{
			BLADE_LW_PROFILING( TILE_SETUP );
			mTile->setup(
				terrainRes->getHeightBuffer(),
				(const uint8*)terrainRes->getBlendBuffer(),
				terrainRes->getNormalBuffer(),
				terrainRes->getLayerBuffer(),
				terrainRes->getSize(),
				mPosition,
				terrainRes->getAABBList(),
				terrainRes->getVertexPositionBuffer(),
				terrainRes->getVertexNormalBuffer(),
				terrainRes->getBlendMap(),
				terrainRes->getNormalMap(),
				terrainRes->getLayerMap(),
				terrainRes->getTerrainDiffuseAtlas(),
				terrainRes->getTerrainNormalAtlas(),
				terrainRes->isTextureInited(),
				false, 
				true);
		}

		{
			//BLADE_LW_PROFILING( ADD_CONTENT );
			this->activateContent();
		}

		{
			BLADE_LW_PROFILING( CLEAR_UP );
			terrainRes->clearup();
		}

		mTileTextures = mTile->getTileTextureList();
		for(size_t i = 0; i < mTileTextures.size(); ++i)
			mTileTextures[i] += BTString(".") + IGraphicsResourceManager::getSingleton().getGraphicsConfig().TextureExt;
	}

	//////////////////////////////////////////////////////////////////////////
	void	TerrainElement::onTextureChange(void* data)
	{
		assert( data == &mTileTextures );
		BLADE_UNREFERENCED(data);
		//set atlas texture
		const TStringList& textureList = mTile->getTileTextureList();
		assert( mTileTextures.size() == textureList.size() );
		for(size_t i = 0; i < textureList.size(); ++i)
		{
			TString base, text;
			TStringHelper::getFileInfo(mTileTextures[i], base, text);
			if( base != textureList[i] )
				mTile->setTileTexture(i, base);
		}
		mModified = true;
	}

}//namespace Blade