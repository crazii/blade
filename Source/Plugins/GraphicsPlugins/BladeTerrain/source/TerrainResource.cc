/********************************************************************
	created:	2010/05/04
	filename: 	TerrainResource.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include "TerrainResource.h"
#include "terrain_interface/ITerrainTile.h"
#include <interface/ITerrainConfigManager.h>
#include <interface/IEventManager.h>
#include <interface/public/graphics/GraphicsSynchronizedEvent.h>

namespace Blade
{
	const TString TerrainResource::TYPE = BTString("TerrainResource");
	//////////////////////////////////////////////////////////////////////////
	TerrainResource::TerrainResource()
		:GraphicsResource(TerrainResource::TYPE)
		,mHeightBuffer(NULL)
		,mNormalBuffer(NULL)
		,mBlendBuffer(NULL)
		,mLayerBuffer(NULL)
		,mBase(scalar(0))
		,mSize(0)
		,mHasTexture(false)
	{

		const Impl::TerrainGraphicsCache::CacheBlock* cb = Impl::TerrainGraphicsCache::getSingleton().findCache();
		if (cb != NULL)
		{
			mAtlas = cb->mAtlas;
			mNormalAtlas = cb->mNormalAtlas;
			mBlendMap = cb->mBlendMap;
			mGlobalNormalMap = cb->mGlobalNormalMap;
			mLayerMap = cb->mLayerMap;
			mVertexPositionBuffer = cb->mVertexPositionBuffer;
			mVertexNormalBuffer = cb->mVertexNormalBuffer;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	TerrainResource::~TerrainResource()
	{
		Impl::TerrainGraphicsCache::CacheBlock cb;
		cb.mAtlas = mAtlas;
		cb.mNormalAtlas = mNormalAtlas;
		cb.mBlendMap = mBlendMap;
		cb.mGlobalNormalMap = mGlobalNormalMap;
		cb.mLayerMap = mLayerMap;
		cb.mVertexPositionBuffer = mVertexPositionBuffer;
		cb.mVertexNormalBuffer = mVertexNormalBuffer;
		Impl::TerrainGraphicsCache::getSingleton().addCache(cb);

		this->clearup(true);
	}
	/************************************************************************/
	/* custom methods                                                                     */
	/************************************************************************/
	//////////////////////////////////////////////////////////////////////////
	size_t					TerrainResource::getSize() const
	{
		return mSize;
	}

	//////////////////////////////////////////////////////////////////////////
	scalar					TerrainResource::getHeightBase() const
	{
		return mBase;
	}

	//////////////////////////////////////////////////////////////////////////
	const pint16			TerrainResource::getHeightBuffer() const
	{
		return mHeightBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	const puint8			TerrainResource::getNormalBuffer() const
	{
		return mNormalBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	const Color::RGBA*		TerrainResource::getBlendBuffer() const
	{
		return mBlendBuffer;
	}
	
	//////////////////////////////////////////////////////////////////////////
	TERRAIN_LAYER*			TerrainResource::getLayerBuffer() const
	{
		return mLayerBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTERRAINATLAS&	TerrainResource::getTerrainDiffuseAtlas() const
	{
		return mAtlas;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTERRAINATLAS&	TerrainResource::getTerrainNormalAtlas() const
	{
		return mNormalAtlas;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&			TerrainResource::getBlendMap() const
	{
		return mBlendMap;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&			TerrainResource::getNormalMap() const
	{
		if( ITerrainConfigManager::getSingleton().isUseVertexNormal() )
			assert(mGlobalNormalMap == NULL);
		else
			assert( mGlobalNormalMap != NULL );

		return mGlobalNormalMap;
	}

	//////////////////////////////////////////////////////////////////////////
	const HTEXTURE&			TerrainResource::getLayerMap() const
	{
		return mLayerMap;
	}

	//////////////////////////////////////////////////////////////////////////
	const HVBUFFER&			TerrainResource::getVertexPositionBuffer() const
	{
		return mVertexPositionBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	const HVBUFFER&			TerrainResource::getVertexNormalBuffer() const
	{
		return mVertexNormalBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	const TileAABBList&		TerrainResource::getAABBList() const
	{
		return mAABBList;
	}

	//////////////////////////////////////////////////////////////////////////
	bool					TerrainResource::isTextureInited() const
	{
		return mHasTexture;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainResource::setTextureInited()
	{
		mHasTexture = true;
	}

	//////////////////////////////////////////////////////////////////////////
	void					TerrainResource::clearup(bool force/* = false*/)
	{
		if( !ITerrainConfigManager::getSingleton().isEditingModeEnabled() || force)
		{
			if( mHeightBuffer != NULL )
			{
				TERRAIN_RES_FREE(mHeightBuffer);
				mHeightBuffer = NULL;
			}

			if( mNormalBuffer != NULL )
			{
				TERRAIN_RES_FREE(mNormalBuffer);
				mNormalBuffer = NULL;
			}

			if( mBlendBuffer != NULL )
			{
				TERRAIN_RES_FREE(mBlendBuffer);
				mBlendBuffer = NULL;
			}

			if( mLayerBuffer != NULL )
			{
				TERRAIN_RES_FREE(mLayerBuffer);
				mLayerBuffer = NULL;
			}
			mAtlas.clear();
			mNormalAtlas.clear();
		}

		mAABBList.clear();
	}
	
}//namespace Blade