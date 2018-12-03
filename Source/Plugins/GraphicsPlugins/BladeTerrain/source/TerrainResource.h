/********************************************************************
	created:	2010/05/04
	filename: 	TerrainResource.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_TerrainResource_h__
#define __Blade_TerrainResource_h__
#include <memory/BladeMemory.h>
#include <interface/public/IResource.h>
#include <math/AxisAlignedBox.h>
#include <math/Vector3.h>
#include <interface/public/graphics/Color.h>
#include <interface/public/graphics/IVertexBuffer.h>
#include <GraphicsResource.h>
#include <utility/StringList.h>

#include "TerrainAtlas.h"
#include <interface/TerrainLayer.h>
#include "terrain_interface/ITerrainTile.h"

#define TERRAIN_RES_ALLOC(_bytes) ITerrainConfigManager::getSingleton().isEditingModeEnabled() ? BLADE_RES_ALLOC(_bytes) : BLADE_TMP_ALLOC(_bytes)
#define TERRAIN_RES_ALLOCT(_TYPE, _count) ITerrainConfigManager::getSingleton().isEditingModeEnabled() ? BLADE_RES_ALLOCT(_TYPE, _count) : BLADE_TMP_ALLOCT(_TYPE, _count)
#define TERRAIN_RES_FREE(_ptr)ITerrainConfigManager::getSingleton().isEditingModeEnabled() ? BLADE_RES_FREE(_ptr) : BLADE_TMP_FREE(_ptr)

namespace Blade
{
	class TerrainResource : public GraphicsResource, public Allocatable
	{
	public:
		static const TString TYPE;
	public:
		TerrainResource();
		~TerrainResource();

		/************************************************************************/
		/* custom methods                                                                     */
		/************************************************************************/
		/*
		@describe
		@param
		@return
		*/
		size_t					getSize() const;

		/*
		@describe
		@param
		@return
		*/
		scalar					getHeightBase() const;

		/*
		@describe
		@param
		@return
		*/
		const pint16			getHeightBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const puint8			getNormalBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const Color::RGBA*		getBlendBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		TERRAIN_LAYER*			getLayerBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HTERRAINATLAS&	getTerrainDiffuseAtlas() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HTERRAINATLAS&	getTerrainNormalAtlas() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HTEXTURE&			getBlendMap() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HTEXTURE&			getNormalMap() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HTEXTURE&			getLayerMap() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HVBUFFER&			getVertexPositionBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const HVBUFFER&			getVertexNormalBuffer() const;

		/*
		@describe 
		@param 
		@return 
		*/
		const TileAABBList&		getAABBList() const;

		/*
		@describe 
		@param 
		@return 
		*/
		bool					isTextureInited() const;

		/*
		@describe 
		@param 
		@return 
		*/
		void					setTextureInited();

		/*
		@describe 
		@param 
		@return 
		*/
		void					clearup(bool force = false);

	protected:
		friend class	TerrainSerializer;

		int16*			mHeightBuffer;
		uint8*			mNormalBuffer;
		Color::RGBA*	mBlendBuffer;
		TERRAIN_LAYER*	mLayerBuffer;
		HTERRAINATLAS	mAtlas;
		HTERRAINATLAS	mNormalAtlas;
		HTEXTURE		mBlendMap;
		HTEXTURE		mGlobalNormalMap;		//valid if !using vertex normal
		HTEXTURE		mLayerMap;

		HVBUFFER		mVertexPositionBuffer;
		HVBUFFER		mVertexNormalBuffer;	//valid if using vertex normal
		TileAABBList	mAABBList;

		scalar			mBase;
		size_t			mSize;
		bool			mHasTexture;
	};//class TerrainResource



	namespace Impl
	{

		class TerrainGraphicsCache : public Singleton<TerrainGraphicsCache>
		{
		public:
			struct CacheBlock
			{
				HTERRAINATLAS	mAtlas;
				HTERRAINATLAS	mNormalAtlas;
				HTEXTURE		mBlendMap;
				HTEXTURE		mGlobalNormalMap;
				HTEXTURE		mLayerMap;
				HVBUFFER		mVertexPositionBuffer;
				HVBUFFER		mVertexNormalBuffer;

				inline bool operator==(const CacheBlock& rhs)
				{
					return mAtlas == rhs.mAtlas && mNormalAtlas == rhs.mNormalAtlas && mBlendMap == rhs.mBlendMap && mGlobalNormalMap == rhs.mGlobalNormalMap
						&& mLayerMap == rhs.mLayerMap && mVertexPositionBuffer == rhs.mVertexPositionBuffer && mVertexNormalBuffer == rhs.mVertexNormalBuffer;
				}

				/** @brief  */
				inline void release()
				{
					mAtlas.clear();
					mNormalAtlas.clear();
					mBlendMap.clear();
					mGlobalNormalMap.clear();
					mLayerMap.clear();
					mVertexPositionBuffer.clear();
					mVertexNormalBuffer.clear();
				}

				mutable bool mUsed;
			};
			typedef List<CacheBlock> CacheList;

		public:
			TerrainGraphicsCache() {}
			~TerrainGraphicsCache() {}

			/** @brief  */
			const CacheBlock*	findCache()
			{
				for (CacheList::iterator i = mCache.begin(); i != mCache.end(); ++i)
				{
					const CacheBlock& cb = *i;
					if (!cb.mUsed)
					{
						cb.mUsed = true;
						return &cb;
					}

				}
				return NULL;
			}

			/** @brief  */
			void	addCache(const CacheBlock& cb)
			{
				size_t count = ITerrainConfigManager::getSingleton().getVisibleTileCount();
				count *= count;
				if (mCache.size() >= count)
				{
					mCache.resize(count);
					return;
				}

				CacheList::iterator i = std::find(mCache.begin(), mCache.end(), cb);
				if (i == mCache.end())
				{
					mCache.push_back(cb);
					mCache.back().mUsed = false;
				}
				else
					i->mUsed = false;
			}

			/** @brief  */
			void clearCaches()
			{
				mCache.clear();
			}

		protected:
			CacheList mCache;
		};

	}//namespace Impl

}//namespace Blade

#endif //__Blade_TerrainResource_h__
